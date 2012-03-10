/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/summary.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "edid.h"
#include "aspia_dll.h"

#include <iphlpapi.h>


static VOID
DriveTypeToText(UINT DriveType, LPWSTR lpszText, SIZE_T Size)
{
    UINT uiID = 0;

    if (lpszText) lpszText[0] = 0;
    switch (DriveType)
    {
        case DRIVE_UNKNOWN:
            uiID = IDS_DRIVE_UNKNOWN;
            break;
        case DRIVE_NO_ROOT_DIR:
            uiID = IDS_DRIVE_NO_ROOT_DIR;
            break;
        case DRIVE_REMOVABLE:
            uiID = IDS_DRIVE_REMOVABLE;
            break;
        case DRIVE_FIXED:
            uiID = IDS_DRIVE_FIXED;
            break;
        case DRIVE_REMOTE:
            uiID = IDS_DRIVE_REMOTE;
            break;
        case DRIVE_CDROM:
            uiID = IDS_DRIVE_CDROM;
            break;
        case DRIVE_RAMDISK:
            uiID = IDS_DRIVE_RAMDISK;
            break;
    }

    LoadMUIString( uiID, lpszText, Size);
}

static VOID
HardDrivesInfo(VOID)
{
    WCHAR szDrives[MAX_PATH], szDrive[MAX_PATH],
          szResult[MAX_PATH], szVolumeName[MAX_PATH],
          szFS[MAX_PATH];
    ULARGE_INTEGER TotalNumberOfBytes, TotalNumberOfFreeBytes;
    UINT uiDriveType;
    INT Count, Index;

    if (!GetLogicalDriveStrings(sizeof(szDrives)/sizeof(WCHAR), szDrives))
        return;

    for (Count = 0; szDrives[Count] != 0; Count += 4)
    {
        if (*(szDrives) > L'Z')
        {
            if (szDrives[Count] <= L'Z') szDrives[Count] += 32;
        }
        else
        {
            if (szDrives[Count] > L'Z') szDrives[Count] -= 32;
        }

        StringCbPrintf(szDrive,
                       sizeof(szDrive),
                       L"%c:\\",
                       szDrives[Count]);

        uiDriveType = GetDriveType(szDrive);
        if (uiDriveType != DRIVE_FIXED && uiDriveType != DRIVE_REMOVABLE)
            continue;

        if (!GetVolumeInformation(szDrive, szVolumeName,
                                  sizeof(szVolumeName)/sizeof(WCHAR),
                                  NULL, NULL, NULL,
                                  szFS, sizeof(szFS)/sizeof(WCHAR)))
        {
            continue;
        }

        if (szVolumeName[0] != 0)
        {
            StringCbPrintf(szResult, sizeof(szResult),
                           L"%c: (%s)(%s)",
                           szDrives[Count],
                           szVolumeName, szFS);
        }
        else
        {
            StringCbPrintf(szResult, sizeof(szResult),
                           L"%c: (%s)",
                           szDrives[Count], szFS);
        }
        Index = IoAddItem(1, 2, szResult);

        if (GetDiskFreeSpaceEx(szDrive,
                               NULL,
                               &TotalNumberOfBytes,
                               &TotalNumberOfFreeBytes))
        {
            StringCbPrintf(szResult,
                           sizeof(szResult),
                           L"%.2f GB (%I64d MB) / %.2f GB (%I64d MB)",
                           ((DOUBLE)TotalNumberOfFreeBytes.QuadPart / (DOUBLE)(1024 * 1024 * 1024)),
                           (TotalNumberOfFreeBytes.QuadPart / (1024 * 1024)),
                           ((DOUBLE)TotalNumberOfBytes.QuadPart / (DOUBLE)(1024 * 1024 * 1024)),
                           (TotalNumberOfBytes.QuadPart / (1024 * 1024)));
            IoSetItemText(Index, 1, szResult);
        }
    }
}

static VOID
ShowUptimeInformation(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    LARGE_INTEGER liCount, liFreq;
    INT Index;

    if (QueryPerformanceCounter(&liCount) &&
        QueryPerformanceFrequency(&liFreq))
    {
        Index = IoAddValueName(1, IDS_SYS_UPTIME, 0);
        ConvertSecondsToString(DllParams.hLangInst,
                               liCount.QuadPart / liFreq.QuadPart,
                               szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);
    }
}

static VOID
ShowMonitorsInfo(VOID)
{
    WCHAR szDevPath[MAX_PATH], szKeyPath[MAX_PATH],
          szDeviceName[MAX_STR_LEN], szIniPath[MAX_PATH],
          szMonitorId[MAX_STR_LEN], szText[MAX_STR_LEN];
    WCHAR *pVendorSign;
    SP_DEVINFO_DATA DeviceInfoData = {0};
    HDEVINFO hDevInfo;
    INT DeviceIndex = 0;
    BYTE Edid[0x80];
    BYTE* Block;
    INT Index, ItemIndex, Count = 0;

    StringCbPrintf(szIniPath, sizeof(szIniPath),
                   L"%s%s",
                   DllParams.szCurrentPath,
                   L"mon_dev.ini");

    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_MONITOR,
                                   0, 0,
                                   DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return;

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(hDevInfo,
                                 DeviceIndex,
                                 &DeviceInfoData))
    {
        ++DeviceIndex;

        if (!SetupDiGetDeviceInstanceId(hDevInfo,
                                        &DeviceInfoData,
                                        szDevPath,
                                        MAX_PATH,
                                        NULL))
        {
            continue;
        }

        if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                              &DeviceInfoData,
                                              SPDRP_FRIENDLYNAME,
                                              0,
                                              (BYTE*)szDeviceName,
                                              MAX_STR_LEN,
                                              NULL))
        {
            if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_DEVICEDESC,
                                                  0,
                                                  (BYTE*)szDeviceName,
                                                  MAX_STR_LEN,
                                                  NULL))
            {
                LoadMUIString(IDS_DEVICE_UNKNOWN_DEVICE,
                              szDeviceName, MAX_STR_LEN);
            }
        }

        StringCbPrintf(szKeyPath, sizeof(szKeyPath),
                       L"SYSTEM\\CurrentControlSet\\Enum\\%s\\Device Parameters",
                       szDevPath);

        if (GetBinaryFromRegistry(HKEY_LOCAL_MACHINE,
                                  szKeyPath, L"EDID",
                                  (LPBYTE)&Edid, sizeof(Edid)))
        {
            pVendorSign = GetEdidVendorSign(Edid + ID_MANUFACTURER_NAME);

            StringCbPrintf(szMonitorId, sizeof(szMonitorId), L"%s%02x%02x",
                           pVendorSign, Edid[ID_MODEL + 1], Edid[ID_MODEL]);
            _wcsupr(szMonitorId);

            GetPrivateProfileString(L"devices", szMonitorId, L"",
                                    szText, MAX_STR_LEN, szIniPath);
            if (szText[0] == L'\0')
            {
                StringCbPrintf(szText, sizeof(szText), L"%s (NoDB)", szDeviceName);
            }

            if (++Count == 1) IoAddHeader(0, IDS_CAT_HW_MONITOR, 5);
            ItemIndex = IoAddItem(1, 5, szText);

            Block = Edid + DETAILED_TIMING_DESCRIPTIONS_START;
            for (Index = 0; Index < NO_DETAILED_TIMING_DESCRIPTIONS; Index++,
                 Block += DETAILED_TIMING_DESCRIPTION_SIZE)
            {
                if (EdidBlockType(Block) == DETAILED_TIMING_BLOCK)
                {
                    /* Max. Resolution */
                    StringCbPrintf(szText, sizeof(szText),
                                   L"%dx%d (%.1f\")",
                                   H_ACTIVE, V_ACTIVE,
                                   GetDiagonalSize(Edid[0x15], Edid[0x16]));
                    IoSetItemText(ItemIndex, 1, szText);
                }
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    if (DeviceIndex) IoAddFooter();
}

static VOID
ShowPrintersInfo(VOID)
{
    PPRINTER_INFO_2 pPrinterInfo;
    DWORD cbNeeded, cReturned, dwSize, dwIndex;
    DWORD dwFlag = PRINTER_ENUM_FAVORITE | PRINTER_ENUM_LOCAL |
                   PRINTER_ENUM_NETWORK;
    WCHAR szText[MAX_STR_LEN], szDefPrinter[MAX_STR_LEN] = {0};
    INT Index;

    if (!EnumPrinters(dwFlag, 0, 2, 0, 0, &cbNeeded, &cReturned))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return;
    }

    pPrinterInfo = (PPRINTER_INFO_2)Alloc(cbNeeded);
    if (!pPrinterInfo)
        return;

    if (!EnumPrinters(dwFlag,
                      NULL, 2,
                      (LPBYTE)pPrinterInfo,
                      cbNeeded,
                      &cbNeeded,
                      &cReturned))
    {
        Free(pPrinterInfo);
        return;
    }

    dwSize = MAX_STR_LEN;
    GetDefaultPrinter(szDefPrinter, &dwSize);

    for (dwIndex = 0; dwIndex < cReturned; dwIndex++)
    {
        /* Printer name */
        Index = IoAddItem(1, 6, pPrinterInfo[dwIndex].pPrinterName);

        /* Paper size */
        if (pPrinterInfo[dwIndex].pDevMode)
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"%ld x %ld mm (%ld x %ld dpi)",
                           pPrinterInfo[dwIndex].pDevMode->dmPaperWidth / 10,
                           pPrinterInfo[dwIndex].pDevMode->dmPaperLength / 10,
                           pPrinterInfo[dwIndex].pDevMode->dmPrintQuality,
                           pPrinterInfo[dwIndex].pDevMode->dmPrintQuality);
            IoSetItemText(Index, 1, szText);
        }
    }

    Free(pPrinterInfo);
}

static VOID
ShowNetAdaptersInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szKey[MAX_PATH];
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    ULONG ulOutBufLen;
    INT Index, Count = 0;
    MIB_IFROW *pIfRow;

    pAdapterInfo = (PIP_ADAPTER_INFO)Alloc(sizeof(IP_ADAPTER_INFO));
    if (!pAdapterInfo) return;

    ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        Free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)Alloc(ulOutBufLen);
        if (!pAdapterInfo)
            return;
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != NOERROR)
    {
        Free(pAdapterInfo);
        return;
    }

    pAdapter = pAdapterInfo;

    while (pAdapter)
    {
        StringCbPrintf(szKey, sizeof(szKey),
                       L"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%S\\Connection",
                       pAdapter->AdapterName);

        StringCbPrintf(szText, sizeof(szText), L"%S", pAdapter->Description);
        if (szText[0] == 0)
        {
            if (!GetAdapterFriendlyName(szKey, szText, MAX_STR_LEN))
                StringCbCopy(szText, sizeof(szText), L"Unknown Adapter");
        }
        Index = IoAddItem(1, 7, szText);

        pIfRow = (MIB_IFROW*)Alloc(sizeof(MIB_IFROW));
        if (!pIfRow)
        {
            Free(pAdapterInfo);
            return;
        }

        pIfRow->dwIndex = pAdapter->Index;
        if (GetIfEntry(pIfRow) != NO_ERROR)
        {
            Free(pAdapterInfo);
            Free(pIfRow);
            return;
        }

        if (pAdapter->IpAddressList.IpAddress.String[0] == '0' ||
            pIfRow->dwSpeed == 0)
        {
            LoadMUIString(IDS_NOT_CONNECTED, szText, MAX_STR_LEN);
        }
        else
        {
            StringCbPrintf(szText, sizeof(szText), L"%S (%ld Mbps)",
                           pAdapter->IpAddressList.IpAddress.String,
                           pIfRow->dwSpeed / (1000 * 1000));
        }
        IoSetItemText(Index, 1, szText);

        pAdapter = pAdapter->Next;
        Free(pIfRow);
        ++Count;
    }

    Free(pAdapterInfo);
}

VOID
ShowSummaryInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    MEMORYSTATUSEX MemStatus;
    DWORD dwSize;
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_WINDOWS); /* OS */
    IoAddIcon(IDI_CPU); /* CPU */
    IoAddIcon(IDI_HDD); /* HDD */
    IoAddIcon(IDI_HW); /* Memory */
    IoAddIcon(IDI_IE); /* Internet Explorer */
    IoAddIcon(IDI_MONITOR); /* Monitors */
    IoAddIcon(IDI_PRINTER); /* Printers */
    IoAddIcon(IDI_NETWORK); /* Network Adapters */

    IoAddHeader(0, IDS_SUMMARY_OS, 0);

    /* Product name */
    Index = IoAddValueName(1, IDS_OS_PRODUCT_NAME, 0);

    szText[0] = 0;
    if (GetStringFromRegistry(TRUE,
                              HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                              L"ProductName",
                              szText,
                              MAX_STR_LEN))
    {
        IoSetItemText(Index, 1, szText);
    }

    /* OS Arch */
    Index = IoAddValueName(1, IDS_OS_ARCH, 0);
    IoSetItemText(Index, 1, IsWin64System() ? L"AMD64" : L"x86");

    /* Computer name */
    Index = IoAddValueName(1, IDS_COMPUTER_NAME, 0);
    dwSize = MAX_STR_LEN;
    if (GetComputerName(szText, &dwSize))
        IoSetItemText(Index, 1, szText);

    /* Current user name */
    Index = IoAddValueName(1, IDS_CURRENT_USERNAME, 0);
    dwSize = MAX_STR_LEN;
    if (GetUserName(szText, &dwSize))
        IoSetItemText(Index, 1, szText);

    /* Uptime information */
    ShowUptimeInformation();

    /* OS Installation Date */
    ShowInstallDate();

    /* Local date */
    Index = IoAddValueName(1, IDS_LOCAL_DATE, 0);
    if (GetDateFormat(LOCALE_USER_DEFAULT,
                      0, NULL, NULL, szText,
                      MAX_STR_LEN))
    {
        IoSetItemText(Index, 1, szText);
    }

    /* Local time */
    Index = IoAddValueName(1, IDS_LOCAL_TIME, 0);
    if (GetTimeFormat(LOCALE_USER_DEFAULT,
                      0, NULL, NULL, szText,
                      MAX_STR_LEN))
    {
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();

    /* Internet Explorer */
    IoAddHeader(0, IDS_CAT_NETWORK_IE, 4);
    ShowIEShortInfo(4);
    IoAddFooter();

    IoAddHeader(0, IDS_SUMMARY_CPU, 1);

    /* Get CPU Name */
    if (GetCPUName(szText, sizeof(szText)))
    {
        Index = IoAddValueName(1, IDS_CPUID_NAME, 1);
        IoSetItemText(Index, 1, szText);
    }

    /* Get CPU Vendor */
    GetCPUVendor(szText, sizeof(szText));
    Index = IoAddValueName(1, IDS_MANUFACTURER, 1);
    IoSetItemText(Index, 1, szText);

    /* CPU Architecture */
    Index = IoAddValueName(1, IDS_HW_ARCH, 1);
    IoSetItemText(Index, 1, Is64BitCpu() ? L"AMD64/EM64T" : L"x86");

    /* Logical processors count */
    if (GetLogicalProcessorsCount() > 0)
    {
        Index = IoAddValueName(1, IDS_CPUID_LOGICAL_COUNT, 1);
        StringCbPrintf(szText, sizeof(szText),
                       L"%d", GetLogicalProcessorsCount());
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();

    IoAddHeader(0, IDS_SUMMARY_HDD, 2);
    HardDrivesInfo();
    IoAddFooter();

    /* Monitors */
    ShowMonitorsInfo();

    /* Printers */
    IoAddHeader(0, IDS_CAT_HW_PRINTERS, 6);
    ShowPrintersInfo();
    IoAddFooter();

    /* Network Adapters */
    IoAddHeader(0, IDS_CAT_NETWORK_CARDS, 7);
    ShowNetAdaptersInfo();
    IoAddFooter();

    IoAddHeader(0, IDS_SUMMARY_MEM, 3);

    MemStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&MemStatus))
    {
        /* Total physical memory */
        Index = IoAddValueName(1, IDS_ALL_MEMORY, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB (%.2f GB)",
                       (MemStatus.ullTotalPhys / (1024 * 1024)),
                       ((DOUBLE)MemStatus.ullTotalPhys / (DOUBLE)(1024 * 1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Total physical free memory */
        Index = IoAddValueName(1, IDS_FREE_MEMORY, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB (%.2f GB)",
                       (MemStatus.ullAvailPhys / (1024 * 1024)),
                       ((DOUBLE)MemStatus.ullAvailPhys / (DOUBLE)(1024 * 1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Total physical used memory */
        Index = IoAddValueName(1, IDS_USED_MEMORY, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB (%.2f GB)",
                       ((MemStatus.ullTotalPhys - MemStatus.ullAvailPhys) / (1024 * 1024)),
                       ((DOUBLE)(MemStatus.ullTotalPhys - MemStatus.ullAvailPhys) / (DOUBLE)(1024 * 1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Percent used physical memory */
        Index = IoAddValueName(1, IDS_PERCENT_USED, 3);
        StringCbPrintf(szText, sizeof(szText), L"%d%%",
                       ((MemStatus.ullTotalPhys - MemStatus.ullAvailPhys) * 100)/MemStatus.ullTotalPhys);
        IoSetItemText(Index, 1, szText);

        IoAddFooter();

        /* Total page file */
        Index = IoAddValueName(1, IDS_TOTAL_PAGINGFILE, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB (%.2f GB)",
                       (MemStatus.ullTotalPageFile / (1024 * 1024)),
                       ((DOUBLE)MemStatus.ullTotalPageFile / (DOUBLE)(1024 * 1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Free page file */
        Index = IoAddValueName(1, IDS_FREE_PAGINGFILE, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB (%.2f GB)",
                       (MemStatus.ullAvailPageFile / (1024 * 1024)),
                       ((DOUBLE)MemStatus.ullAvailPageFile / (DOUBLE)(1024 * 1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Used page file */
        Index = IoAddValueName(1, IDS_USED_PAGINFILE, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB (%.2f GB)",
                       ((MemStatus.ullTotalPageFile - MemStatus.ullAvailPageFile)/(1024 * 1024)),
                       ((DOUBLE)(MemStatus.ullTotalPageFile - MemStatus.ullAvailPageFile)/(DOUBLE)(1024 * 1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Percent used page file */
        Index = IoAddValueName(1, IDS_PERCENT_PAGINFILE, 3);
        StringCbPrintf(szText, sizeof(szText), L"%d%%",
                       ((MemStatus.ullTotalPageFile - MemStatus.ullAvailPageFile) * 100)/MemStatus.ullTotalPageFile);
        IoSetItemText(Index, 1, szText);

        IoAddFooter();

        /* Total virtual memory */
        Index = IoAddValueName(1, IDS_TOTAL_VIRUALMEM, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB (%.2f GB)",
                       (MemStatus.ullTotalVirtual / (1024 * 1024)),
                       ((DOUBLE)MemStatus.ullTotalVirtual / (DOUBLE)(1024 * 1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Free virtual memory */
        Index = IoAddValueName(1, IDS_FREE_VIRUALMEM, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB (%.2f GB)",
                       (MemStatus.ullAvailVirtual / (1024 * 1024)),
                       ((DOUBLE)MemStatus.ullAvailVirtual / (DOUBLE)(1024 * 1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Used virtual memory */
        Index = IoAddValueName(1, IDS_USED_VIRTMEM, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB (%.2f GB)",
                       ((MemStatus.ullTotalVirtual - MemStatus.ullAvailVirtual)/(1024 * 1024)),
                       ((DOUBLE)(MemStatus.ullTotalVirtual - MemStatus.ullAvailVirtual)/(DOUBLE)(1024 * 1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Percent used virtual memory */
        Index = IoAddValueName(1, IDS_PERCENT_VIRTMEM, 3);
        StringCbPrintf(szText, sizeof(szText), L"%d%%",
                       ((MemStatus.ullTotalVirtual - MemStatus.ullAvailVirtual) * 100)/MemStatus.ullTotalVirtual);
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();

    DebugEndReceiving();
}
