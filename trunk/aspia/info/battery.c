/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/battery.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"

#include <batclass.h>

typedef BOOL (CALLBACK *BATTERYENUMPROC)(LPWSTR lpszBattery);

typedef struct
{
    LPWSTR lpszChemistry;
    LPWSTR lpszString;
} CHEMISTRY_INFO;


static HANDLE
BatteryOpen(LPWSTR lpszDevice)
{
    return CreateFile(lpszDevice,
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE, 
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_SYSTEM,
                      NULL);
}

static BOOL
BatteryClose(HANDLE hHandle)
{
    return CloseHandle(hHandle);
}

static ULONG
BatteryGetTag(HANDLE hHandle)
{
    ULONG uBatteryTag = 0;
    DWORD cbBytesReturned, dwTmp = 0;

    if (!DeviceIoControl(hHandle,
                         IOCTL_BATTERY_QUERY_TAG,
                         &dwTmp,
                         sizeof(DWORD),
                         &uBatteryTag,
                         sizeof(ULONG),
                         &cbBytesReturned,
                         NULL))
    {
        return 0;
    }

    return uBatteryTag;
}

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

static BOOL
BatteryQueryInformation(HANDLE hHandle,
                        BATTERY_QUERY_INFORMATION_LEVEL InfoLevel,
                        LPVOID lpBuffer,
                        DWORD dwBufferSize)
{
    BATTERY_QUERY_INFORMATION BatteryQueryInfo;
    DWORD cbBytesReturned;

    BatteryQueryInfo.BatteryTag = BatteryGetTag(hHandle);
    if (!BatteryQueryInfo.BatteryTag)
        return FALSE;

    BatteryQueryInfo.InformationLevel = InfoLevel;
    return DeviceIoControl(hHandle,
                           IOCTL_BATTERY_QUERY_INFORMATION,
                           &BatteryQueryInfo,
                           sizeof(BATTERY_QUERY_INFORMATION),
                           lpBuffer,
                           dwBufferSize,
                           &cbBytesReturned,
                           NULL);
}

static BOOL
BatteryQueryStatus(HANDLE hHandle,
                   BATTERY_STATUS *lpBatteryStatus,
                   DWORD dwBufferSize)
{
    BATTERY_WAIT_STATUS BatteryWaitStatus = {0};
    DWORD cbBytesReturned;

    BatteryWaitStatus.BatteryTag = BatteryGetTag(hHandle);
    if (!BatteryWaitStatus.BatteryTag)
        return FALSE;

    return DeviceIoControl(hHandle,
                           IOCTL_BATTERY_QUERY_STATUS,
                           &BatteryWaitStatus,
                           sizeof(BATTERY_WAIT_STATUS),
                           lpBatteryStatus,
                           dwBufferSize,
                           &cbBytesReturned,
                           NULL);
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
    INT Index;

    hHandle = BatteryOpen(lpszBattery);
    if (hHandle == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"Invalid handle value");
        return TRUE;
    }

    /* Device name */
    if (BatteryQueryInformation(hHandle,
                                BatteryDeviceName,
                                (LPVOID)szText,
                                sizeof(szText)))
    {
        Index = IoAddValueName(IDS_NAME, -1);
        IoSetItemText(Index, 1, szText);
    }

    /* Manufacture name */
    if (BatteryQueryInformation(hHandle,
                                BatteryManufactureName,
                                (LPVOID)szText,
                                sizeof(szText)))
    {
        Index = IoAddValueName(IDS_MANUFACTURER, -1);
        IoSetItemText(Index, 1, szText);
    }

    /* Manufacture date */
    if (BatteryQueryInformation(hHandle,
                                BatteryManufactureDate,
                                (LPVOID)&BatteryDate,
                                sizeof(BatteryDate)))
    {
        Index = IoAddValueName(IDS_BAT_MANUFACTUREDATE, -1);
        StringCbPrintf(szText, sizeof(szText),
                       L"%d//%d//%d",
                       BatteryDate.Year,
                       BatteryDate.Month,
                       BatteryDate.Day);
        IoSetItemText(Index, 1, szText);
    }

    /* ID */
    if (BatteryQueryInformation(hHandle,
                                BatteryUniqueID,
                                (LPVOID)szText,
                                sizeof(szText)))
    {
        Index = IoAddValueName(IDS_BAT_ID, -1);
        IoSetItemText(Index, 1, szText);
    }

    /* Serial number */
    if (BatteryQueryInformation(hHandle,
                                BatterySerialNumber,
                                (LPVOID)szText,
                                sizeof(szText)))
    {
        Index = IoAddValueName(IDS_SERIAL_NUMBER, -1);
        IoSetItemText(Index, 1, szText);
    }

    /* Temperature */
    if (BatteryQueryInformation(hHandle,
                                BatteryTemperature,
                                (LPVOID)szText,
                                sizeof(szText)))
    {
        Index = IoAddValueName(IDS_BAT_TEMPERATURE, -1);
        IoSetItemText(Index, 1, szText);
    }

    if (BatteryQueryInformation(hHandle,
                                BatteryInformation,
                                (LPVOID)&BatteryInfo,
                                sizeof(BatteryInfo)))
    {
        /* Capacity */
        Index = IoAddValueName(IDS_BAT_CAPACITY, -1);
        StringCbPrintf(szText, sizeof(szText),
                       L"%ld mWh", BatteryInfo.DesignedCapacity);
        IoSetItemText(Index, 1, szText);

        /* Type */
        Index = IoAddValueName(IDS_BAT_TYPE, -1);
        GetBatteryTypeString(BatteryInfo, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);

        /* Full charged capacity */
        Index = IoAddValueName(IDS_BAT_FULL_CAPACITY, -1);
        StringCbPrintf(szText, sizeof(szText),
                       L"%ld mWh", BatteryInfo.FullChargedCapacity);
        IoSetItemText(Index, 1, szText);

        /* Depreciation */
        Index = IoAddValueName(IDS_BAT_DEPRECIATION, -1);
        StringCbPrintf(szText, sizeof(szText),
                       L"%ld%%",
                       100 - (BatteryInfo.FullChargedCapacity * 100) /
                       BatteryInfo.DesignedCapacity);
        IoSetItemText(Index, 1, szText);
    }

    if (BatteryQueryStatus(hHandle,
                           &BatteryStatus,
                           sizeof(BatteryStatus)))
    {
        /* Current capacity */
        Index = IoAddValueName(IDS_BAT_CURRENT_CAPACITY, -1);
        StringCbPrintf(szText, sizeof(szText),
                       L"%ld mWh (%ld%%)",
                       BatteryStatus.Capacity,
                       (BatteryStatus.Capacity * 100) / BatteryInfo.FullChargedCapacity);
        IoSetItemText(Index, 1, szText);

        /* Voltage */
        Index = IoAddValueName(IDS_BAT_VOLTAGE, -1);
        StringCbPrintf(szText, sizeof(szText),
                       L"%ld mV", BatteryStatus.Voltage);
        IoSetItemText(Index, 1, szText);

        /* Status */
        Index = IoAddValueName(IDS_STATUS, -1);
        BatteryPowerStateToText(BatteryStatus.PowerState, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();

    BatteryClose(hHandle);
    return TRUE;
}

BOOL
GetBatteryInformation(VOID)
{
    return EnumBatteryDevices(BatteryEnumProc);
}
