/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/battery.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


typedef VOID (CALLBACK *BATTERYENUMPROC)(LPWSTR lpszBattery);

typedef struct
{
    LPWSTR lpszChemistry;
    LPWSTR lpszString;
} CHEMISTRY_INFO;


static BOOL
EnumBatteryDevices(BATTERYENUMPROC lpBatteryEnumProc)
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDevDetail;
    SP_DEVICE_INTERFACE_DATA spDeviceData;
    DWORD cbRequired;
    HDEVINFO hDevInfo;
    INT iBattery = 0;

    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY,
                                   0, 0,
                                   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"SetupDiGetClassDevs() failed! Error code = 0x%x",
                   GetLastError());
        return FALSE;
    }

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

        lpBatteryEnumProc(pDevDetail->DevicePath);

        Free(pDevDetail);
        ++iBattery;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    return TRUE;
}

const CHEMISTRY_INFO ChemistryList[] =
{
    { L"PbAc", L"Lead Acid" },
    { L"LION", L"Lithium Ion" },
    { L"Li-I", L"Lithium Ion" },
    { L"NiCd", L"Nickel Cadmium" },
    { L"NiMH", L"Nickel Metal Hydride" },
    { L"NiZn", L"Nickel Zinc" },
    { L"RAM", L"Rechargeable Alkaline-Manganese" },
    { 0 }
};

static VOID
GetBatteryTypeString(BATTERY_INFORMATION BatteryInfo,
                     LPWSTR lpszText,
                     SIZE_T Size)
{
    WCHAR szTechnology[MAX_STR_LEN];
    WCHAR szChemistry[5];
    UINT uTypeID = IDS_BAT_NONRECHARGEABLE;
    INT Index = 0;

    if (BatteryInfo.Technology)
        uTypeID = IDS_BAT_RECHARGEABLE;

    LoadMUIString(uTypeID, szTechnology, MAX_STR_LEN);

    StringCbPrintf(szChemistry, sizeof(szChemistry),
                   L"%S\0", BatteryInfo.Chemistry);

    do
    {
        if (wcscmp(szChemistry, ChemistryList[Index].lpszChemistry) == 0)
        {
            StringCbPrintf(lpszText, Size, L"%s (%s)",
                           szTechnology, ChemistryList[Index].lpszString);
            return;
        }
    }
    while (ChemistryList[++Index].lpszChemistry != NULL);
}

static VOID
BatteryPowerStateToText(ULONG PowerState, LPWSTR lpszText, SIZE_T Size)
{
    WCHAR szRes[MAX_STR_LEN];

    if (PowerState & BATTERY_POWER_ON_LINE)
    {
        LoadMUIString(IDS_BAT_POWERONLINE, lpszText, (INT)Size);

        if (PowerState & BATTERY_CHARGING)
        {
            LoadMUIString(IDS_BAT_CHARGING, szRes, MAX_STR_LEN);
            StringCbCat(lpszText, Size, L", ");
            StringCbCat(lpszText, Size, szRes);
        }
    }
    else
    {
        LoadMUIString(IDS_BAT_DISCHARGING, lpszText, (INT)Size);

        if (PowerState & BATTERY_CRITICAL)
        {
            LoadMUIString(IDS_BAT_CRITICAL, szRes, MAX_STR_LEN);
            StringCbCat(lpszText, Size, L", ");
            StringCbCat(lpszText, Size, szRes);
        }
    }
}

static VOID CALLBACK
BatteryEnumProc(LPWSTR lpszBattery)
{
    BATTERY_MANUFACTURE_DATE BatteryDate;
    BATTERY_INFORMATION BatteryInfo;
    BATTERY_STATUS BatteryStatus;
    WCHAR szText[MAX_STR_LEN];
    HANDLE hHandle;
    INT Wear;

    hHandle = OpenBattery(lpszBattery);
    if (hHandle == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"Invalid handle value");
        return;
    }

    /* Device name */
    if (QueryBatteryInfo(hHandle,
                         BatteryDeviceName,
                         (LPVOID)szText,
                         sizeof(szText)))
    {
        IoAddValueName(1, 1, IDS_NAME);
        IoSetItemText(szText);
    }

    /* Manufacture name */
    if (QueryBatteryInfo(hHandle,
                         BatteryManufactureName,
                         (LPVOID)szText,
                         sizeof(szText)))
    {
        IoAddValueName(1, 1, IDS_MANUFACTURER);
        IoSetItemText(szText);
    }

    /* Manufacture date */
    if (QueryBatteryInfo(hHandle,
                         BatteryManufactureDate,
                         (LPVOID)&BatteryDate,
                         sizeof(BatteryDate)))
    {
        IoAddValueName(1, 1, IDS_BAT_MANUFACTUREDATE);
        IoSetItemText(L"%d//%d//%d",
                      BatteryDate.Year,
                      BatteryDate.Month,
                      BatteryDate.Day);
    }

    /* ID */
    if (QueryBatteryInfo(hHandle,
                         BatteryUniqueID,
                         (LPVOID)szText,
                         sizeof(szText)))
    {
        ChopSpaces(szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 1, IDS_BAT_ID);
            IoSetItemText(szText);
        }
    }

    /* Serial number */
    if (QueryBatteryInfo(hHandle,
                         BatterySerialNumber,
                         (LPVOID)szText,
                         sizeof(szText)))
    {
        ChopSpaces(szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 1, IDS_SERIAL_NUMBER);
            IoSetItemText(szText);
        }
    }

    /* Temperature */
    if (QueryBatteryInfo(hHandle,
                         BatteryTemperature,
                         (LPVOID)szText,
                         sizeof(szText)))
    {
        IoAddValueName(1, 1, IDS_BAT_TEMPERATURE);
        IoSetItemText(szText);
    }

    if (QueryBatteryInfo(hHandle,
                               BatteryInformation,
                               (LPVOID)&BatteryInfo,
                               sizeof(BatteryInfo)))
    {
        /* Capacity */
        IoAddValueName(1, 1, IDS_BAT_CAPACITY);
        IoSetItemText(L"%ld mWh",
                      BatteryInfo.DesignedCapacity);

        /* Type */
        IoAddValueName(1, 1, IDS_BAT_TYPE);
        GetBatteryTypeString(BatteryInfo, szText, sizeof(szText));
        IoSetItemText(szText);

        /* Full charged capacity */
        IoAddValueName(1, 1, IDS_BAT_FULL_CAPACITY);
        IoSetItemText(L"%ld mWh",
                      BatteryInfo.FullChargedCapacity);

        /* Depreciation */
        IoAddValueName(1, 1, IDS_BAT_DEPRECIATION);

        Wear = 100 - (BatteryInfo.FullChargedCapacity * 100) /
                   BatteryInfo.DesignedCapacity;

        IoSetItemText(L"%ld%%", (Wear >= 0) ? Wear : 0);
    }

    if (QueryBatteryStatus(hHandle,
                           &BatteryStatus,
                           sizeof(BatteryStatus)))
    {
        /* Current capacity */
        IoAddValueName(1, 1, IDS_BAT_CURRENT_CAPACITY);
        IoSetItemText(L"%ld mWh (%ld%%)",
                      BatteryStatus.Capacity,
                      (BatteryStatus.Capacity * 100) / BatteryInfo.FullChargedCapacity);

        /* Voltage */
        IoAddValueName(1, 1, IDS_BAT_VOLTAGE);
        IoSetItemText(L"%ld mV", BatteryStatus.Voltage);

        /* Status */
        IoAddValueName(1, 1, IDS_STATUS);
        BatteryPowerStateToText(BatteryStatus.PowerState, szText, sizeof(szText));
        IoSetItemText(szText);
    }

    IoAddFooter();

    CloseBattery(hHandle);
}

BOOL
GetBatteryInformation(VOID)
{
    return EnumBatteryDevices(BatteryEnumProc);
}
