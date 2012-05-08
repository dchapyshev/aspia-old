/*
 * PROJECT:         Aspia System Monitor
 * FILE:            aspia_exe/sysmon/battery.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <aspia.h>
#include "sysmon.h"


typedef struct _MYBATTERY
{
    INT ItemIndex;
    INT BatteryIndex;
    HANDLE hHandle;
    INT Max;
    INT Min;
    struct _MYBATTERY *Next;
} BATTERY, *PBATTERY;


static CRITICAL_SECTION BatteryCriticalSection;
static PBATTERY BatteryInfo = NULL;
static INT BatteryCount = 0;


BOOL
InitBatterySensors(VOID)
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDevDetail;
    SP_DEVICE_INTERFACE_DATA spDeviceData;
    BATTERY_STATUS BatteryStatus;
    BOOL Result = FALSE;
    PBATTERY Battery;
    DWORD cbRequired;
    HDEVINFO hDevInfo;
    INT iBattery = 0;

    BatteryCount = 0;

    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY,
                                   0, 0,
                                   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"SetupDiGetClassDevs() failed! Error code = 0x%x",
                   GetLastError());
        return FALSE;
    }

    BatteryInfo = (PBATTERY)Alloc(sizeof(BATTERY));
    if (!BatteryInfo) goto Cleanup;

    Battery = BatteryInfo;

    for (;;)
    {
        ZeroMemory(&spDeviceData, sizeof(SP_DEVICE_INTERFACE_DATA));
        spDeviceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        if (!SetupDiEnumDeviceInterfaces(hDevInfo,
                                         0,
                                         &GUID_DEVCLASS_BATTERY,
                                         iBattery,
                                         &spDeviceData))
        {
            if (GetLastError() == ERROR_NO_MORE_ITEMS)
                break;
        }

        cbRequired = 0;
        SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                        &spDeviceData,
                                        0, 0,
                                        &cbRequired, 0);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            continue;

        pDevDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)Alloc(cbRequired);
        if (!pDevDetail)
            continue;

        pDevDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if (!SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                             &spDeviceData,
                                             pDevDetail,
                                             cbRequired,
                                             &cbRequired,
                                             0))
        {
            Free(pDevDetail);
            continue;
        }

        Battery->hHandle = OpenBattery(pDevDetail->DevicePath);
        if (Battery->hHandle != INVALID_HANDLE_VALUE)
        {
            if (QueryBatteryStatus(Battery->hHandle,
                                   &BatteryStatus,
                                   sizeof(BatteryStatus)))
            {
                Battery->BatteryIndex = iBattery;
                Battery->ItemIndex    = -1;
                Battery->Max = Battery->Min = BatteryStatus.Voltage;

                ++BatteryCount;

                Battery->Next = (PBATTERY)Alloc(sizeof(BATTERY));
                Battery = Battery->Next;
            }
        }

        Free(pDevDetail);
        ++iBattery;
    }

    if (BatteryCount)
    {
        InitializeCriticalSection(&BatteryCriticalSection);
        Result = TRUE;
    }

Cleanup:
    SetupDiDestroyDeviceInfoList(hDevInfo);

    return Result;
}

VOID
EnumBatterySensors(PBATTERY_SENSORS_ENUMPROC lpEnumProc)
{
    BATTERY_STATUS BatteryStatus;
    INT Count = BatteryCount;
    PBATTERY Battery;

    if (!TryEnterCriticalSection(&BatteryCriticalSection))
        return;

    Battery = BatteryInfo;

    while (Count)
    {
        if (QueryBatteryStatus(Battery->hHandle,
                               &BatteryStatus,
                               sizeof(BatteryStatus)))
        {
            DWORD Voltage = BatteryStatus.Voltage;

            if (Voltage > Battery->Max)
                Battery->Max = Voltage;

            if (Voltage < Battery->Min)
                Battery->Min = Voltage;

            Battery->ItemIndex = lpEnumProc(Battery->ItemIndex,
                                            Battery->BatteryIndex,
                                            Voltage,
                                            Battery->Max,
                                            Battery->Min);
        }
        Battery = Battery->Next;
        --Count;
    }

    LeaveCriticalSection(&BatteryCriticalSection);
}

VOID
FreeBatterySensors(VOID)
{
    PBATTERY Battery, Temp;

    Battery = BatteryInfo;

    while (BatteryCount)
    {
        CloseBattery(Battery->hHandle);

        Temp = Battery->Next;
        Free(Battery);
        Battery = Temp;

        --BatteryCount;
    }

    DeleteCriticalSection(&BatteryCriticalSection);
}
