/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/battery.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


typedef BOOL (CALLBACK *BATTERYENUMPROC)(LPWSTR lpszBattery);

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

        if (!lpBatteryEnumProc(pDevDetail->DevicePath))
        {
            Free(pDevDetail);
            SetupDiDestroyDeviceInfoList(hDevInfo);
            return FALSE;
        }

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

static BOOL CALLBACK
BatteryEnumProc(LPWSTR lpszBattery)
{
    BATTERY_MANUFACTURE_DATE BatteryDate;
    BATTERY_INFORMATION BatteryInfo;
    BATTERY_STATUS BatteryStatus;
    WCHAR szText[MAX_STR_LEN];
    HANDLE hHandle;
    INT Index, Wear;

    hHandle = OpenBattery(lpszBattery);
    if (hHandle == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"Invalid handle value");
        return TRUE;
    }

    /* Device name */
    if (QueryBatteryInfo(hHandle,
                         BatteryDeviceName,
                         (LPVOID)szText,
                         sizeof(szText)))
    {
        Index = IoAddValueName(1, IDS_NAME, 1);
        IoSetItemText(Index, 1, szText);
    }

    /* Manufacture name */
    if (QueryBatteryInfo(hHandle,
                         BatteryManufactureName,
                         (LPVOID)szText,
                         sizeof(szText)))
    {
        Index = IoAddValueName(1, IDS_MANUFACTURER, 1);
        IoSetItemText(Index, 1, szText);
    }

    /* Manufacture date */
    if (QueryBatteryInfo(hHandle,
                         BatteryManufactureDate,
                         (LPVOID)&BatteryDate,
                         sizeof(BatteryDate)))
    {
        Index = IoAddValueName(1, IDS_BAT_MANUFACTUREDATE, 1);
        IoSetItemText(Index, 1, L"%d//%d//%d",
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
            Index = IoAddValueName(1, IDS_BAT_ID, 1);
            IoSetItemText(Index, 1, szText);
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
            Index = IoAddValueName(1, IDS_SERIAL_NUMBER, 1);
            IoSetItemText(Index, 1, szText);
        }
    }

    /* Temperature */
    if (QueryBatteryInfo(hHandle,
                         BatteryTemperature,
                         (LPVOID)szText,
                         sizeof(szText)))
    {
        Index = IoAddValueName(1, IDS_BAT_TEMPERATURE, 1);
        IoSetItemText(Index, 1, szText);
    }

    if (QueryBatteryInfo(hHandle,
                               BatteryInformation,
                               (LPVOID)&BatteryInfo,
                               sizeof(BatteryInfo)))
    {
        /* Capacity */
        Index = IoAddValueName(1, IDS_BAT_CAPACITY, 1);
        IoSetItemText(Index, 1, L"%ld mWh",
                      BatteryInfo.DesignedCapacity);

        /* Type */
        Index = IoAddValueName(1, IDS_BAT_TYPE, 1);
        GetBatteryTypeString(BatteryInfo, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);

        /* Full charged capacity */
        Index = IoAddValueName(1, IDS_BAT_FULL_CAPACITY, 1);
        IoSetItemText(Index, 1, L"%ld mWh",
                      BatteryInfo.FullChargedCapacity);

        /* Depreciation */
        Index = IoAddValueName(1, IDS_BAT_DEPRECIATION, 1);

        Wear = 100 - (BatteryInfo.FullChargedCapacity * 100) /
                   BatteryInfo.DesignedCapacity;

        IoSetItemText(Index, 1, L"%ld%%", (Wear >= 0) ? Wear : 0);
    }

    if (QueryBatteryStatus(hHandle,
                           &BatteryStatus,
                           sizeof(BatteryStatus)))
    {
        /* Current capacity */
        Index = IoAddValueName(1, IDS_BAT_CURRENT_CAPACITY, 1);
        IoSetItemText(Index, 1, L"%ld mWh (%ld%%)",
                      BatteryStatus.Capacity,
                      (BatteryStatus.Capacity * 100) / BatteryInfo.FullChargedCapacity);

        /* Voltage */
        Index = IoAddValueName(1, IDS_BAT_VOLTAGE, 1);
        IoSetItemText(Index, 1, L"%ld mV", BatteryStatus.Voltage);

        /* Status */
        Index = IoAddValueName(1, IDS_STATUS, 1);
        BatteryPowerStateToText(BatteryStatus.PowerState, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();

    CloseBattery(hHandle);
    return TRUE;
}

BOOL
GetBatteryInformation(VOID)
{
    return EnumBatteryDevices(BatteryEnumProc);
}
