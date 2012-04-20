/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/summary.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"
#include "edid.h"


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

    LoadMUIString(uiID, lpszText, Size);
}

static VOID
HardDrivesInfo(VOID)
{
    WCHAR szDrives[MAX_PATH], szDrive[MAX_PATH],
          szResult[MAX_PATH], szVolumeName[MAX_PATH],
          szFS[MAX_PATH];
    ULARGE_INTEGER TotalNumberOfBytes, TotalNumberOfFreeBytes;
    UINT uiDriveType;
    INT Count;

    if (!GetLogicalDriveStrings(sizeof(szDrives)/sizeof(WCHAR), szDrives))
    {
        DebugTrace(L"GetLogicalDriveStrings() failed. Error code = 0x%x",
                   GetLastError());
        return;
    }

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
            DebugTrace(L"GetVolumeInformation(szDrive = %s) failed! Error code = 0x%x",
                       szDrive, GetLastError());
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

        if (GetDiskFreeSpaceEx(szDrive,
                               NULL,
                               &TotalNumberOfBytes,
                               &TotalNumberOfFreeBytes))
        {
            IoAddItem(1, 2, szResult);

            IoSetItemText(L"%.2f GB (%I64d MB) / %.2f GB (%I64d MB)",
                          ((DOUBLE)TotalNumberOfFreeBytes.QuadPart / (DOUBLE)(1024 * 1024 * 1024)),
                          (TotalNumberOfFreeBytes.QuadPart / (1024 * 1024)),
                          ((DOUBLE)TotalNumberOfBytes.QuadPart / (DOUBLE)(1024 * 1024 * 1024)),
                          (TotalNumberOfBytes.QuadPart / (1024 * 1024)));
        }
        else
        {
            DebugTrace(L"GetDiskFreeSpaceEx(szDrive = %s) failed! Error code = 0x%x",
                       szDrive, GetLastError());
        }
    }
}

static VOID
ShowUptimeInformation(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    LARGE_INTEGER liCount, liFreq;

    if (QueryPerformanceCounter(&liCount) &&
        QueryPerformanceFrequency(&liFreq))
    {
        IoAddValueName(1, 0, IDS_SYS_UPTIME);
        ConvertSecondsToString(DllParams.hLangInst,
                               liCount.QuadPart / liFreq.QuadPart,
                               szText, sizeof(szText));
        IoSetItemText(szText);
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
    INT Index, Count = 0;

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
            if (szText[0] == 0)
            {
                StringCbPrintf(szText, sizeof(szText), L"%s (NoDB)", szDeviceName);
            }

            if (++Count == 1) IoAddHeader(0, 5, IDS_CAT_HW_MONITOR);
            IoAddItem(1, 5, szText);

            Block = Edid + DETAILED_TIMING_DESCRIPTIONS_START;
            for (Index = 0; Index < NO_DETAILED_TIMING_DESCRIPTIONS; Index++,
                 Block += DETAILED_TIMING_DESCRIPTION_SIZE)
            {
                if (EdidBlockType(Block) == DETAILED_TIMING_BLOCK)
                {
                    /* Max. Resolution */
                    IoSetItemText(L"%dx%d (%.1f\")",
                                  H_ACTIVE, V_ACTIVE,
                                  GetDiagonalSize(Edid[0x15], Edid[0x16]));
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
    WCHAR szDefPrinter[MAX_STR_LEN] = {0};

    if (!EnumPrinters(dwFlag, 0, 2, 0, 0, &cbNeeded, &cReturned))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            DebugTrace(L"EnumPrinters() failed! Error code = 0x%x",
                       GetLastError());
            return;
        }
    }

    pPrinterInfo = (PPRINTER_INFO_2)Alloc(cbNeeded);
    if (!pPrinterInfo)
    {
        DebugTrace(L"Alloc(%d) failed!", cbNeeded);
        return;
    }

    if (!EnumPrinters(dwFlag,
                      NULL, 2,
                      (LPBYTE)pPrinterInfo,
                      cbNeeded,
                      &cbNeeded,
                      &cReturned))
    {
        DebugTrace(L"EnumPrinters() failed! Error code = 0x%x",
                   GetLastError());
        Free(pPrinterInfo);
        return;
    }

    dwSize = MAX_STR_LEN;
    if (!GetDefaultPrinter(szDefPrinter, &dwSize))
    {
        DebugTrace(L"GetDefaultPrinter() failed! Error code = 0x%x",
                   GetLastError());
    }

    for (dwIndex = 0; dwIndex < cReturned; dwIndex++)
    {
        /* Paper size */
        if (pPrinterInfo[dwIndex].pDevMode)
        {
            /* Printer name */
            IoAddItem(1, 6, pPrinterInfo[dwIndex].pPrinterName);

            IoSetItemText(L"%ld x %ld mm (%ld x %ld dpi)",
                          pPrinterInfo[dwIndex].pDevMode->dmPaperWidth / 10,
                          pPrinterInfo[dwIndex].pDevMode->dmPaperLength / 10,
                          pPrinterInfo[dwIndex].pDevMode->dmPrintQuality,
                          pPrinterInfo[dwIndex].pDevMode->dmPrintQuality);
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
    INT Count = 0;
    MIB_IFROW *pIfRow;

    pAdapterInfo = (PIP_ADAPTER_INFO)Alloc(sizeof(IP_ADAPTER_INFO));
    if (!pAdapterInfo)
    {
        DebugTrace(L"Alloc(%d) failed!", sizeof(IP_ADAPTER_INFO));
        return;
    }

    ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        Free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)Alloc(ulOutBufLen);
        if (!pAdapterInfo)
        {
            DebugTrace(L"Alloc(%d) failed!", ulOutBufLen);
            return;
        }
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != NOERROR)
    {
        DebugTrace(L"GetAdaptersInfo() failed!");
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
        IoAddItem(1, 7, szText);

        pIfRow = (MIB_IFROW*)Alloc(sizeof(MIB_IFROW));
        if (!pIfRow)
        {
            DebugTrace(L"Alloc(%d) failed!", sizeof(MIB_IFROW));
            Free(pAdapterInfo);
            return;
        }

        pIfRow->dwIndex = pAdapter->Index;
        if (GetIfEntry(pIfRow) != NO_ERROR)
        {
            DebugTrace(L"GetIfEntry() failed!");
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
        IoSetItemText(szText);

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
    INT Count;

    DebugStartReceiving();

    IoAddIcon(IDI_WINDOWS); /* OS */
    IoAddIcon(IDI_CPU); /* CPU */
    IoAddIcon(IDI_HDD); /* HDD */
    IoAddIcon(IDI_HW); /* Memory */
    IoAddIcon(IDI_IE); /* Internet Explorer */
    IoAddIcon(IDI_MONITOR); /* Monitors */
    IoAddIcon(IDI_PRINTER); /* Printers */
    IoAddIcon(IDI_NETWORK); /* Network Adapters */

    IoAddHeader(0, 0, IDS_SUMMARY_OS);

    /* Product name */
    szText[0] = 0;
    if (GetStringFromRegistry(TRUE,
                              HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                              L"ProductName",
                              szText,
                              MAX_STR_LEN))
    {
        IoAddValueName(1, 0, IDS_OS_PRODUCT_NAME);
        IoSetItemText(szText);
    }

    /* OS Arch */
    IoAddValueName(1, 0, IDS_OS_ARCH);
    IoSetItemText(IsWin64System() ? L"AMD64" : L"x86");

    /* Computer name */
    IoAddValueName(1, 0, IDS_COMPUTER_NAME);
    dwSize = MAX_STR_LEN;
    if (GetComputerName(szText, &dwSize))
        IoSetItemText(szText);
    else
        IoSetItemText(L"-");

    /* Current user name */
    IoAddValueName(1, 0, IDS_CURRENT_USERNAME);
    dwSize = MAX_STR_LEN;
    if (GetUserName(szText, &dwSize))
        IoSetItemText(szText);
    else
        IoSetItemText(L"-");

    /* Uptime information */
    ShowUptimeInformation();

    /* OS Installation Date */
    ShowInstallDate();

    /* Local date */
    if (GetDateFormat(LOCALE_USER_DEFAULT,
                      0, NULL, NULL, szText,
                      MAX_STR_LEN))
    {
        IoAddValueName(1, 0, IDS_LOCAL_DATE);
        IoSetItemText(szText);
    }

    /* Local time */
    if (GetTimeFormat(LOCALE_USER_DEFAULT,
                      0, NULL, NULL, szText,
                      MAX_STR_LEN))
    {
        IoAddValueName(1, 0, IDS_LOCAL_TIME);
        IoSetItemText(szText);
    }

    IoAddFooter();

    /* Internet Explorer */
    IoAddHeader(0, 4, IDS_CAT_NETWORK_IE);
    ShowIEShortInfo(4);
    IoAddFooter();

    IoAddHeader(0, 1, IDS_SUMMARY_CPU);

    /* Get CPU Name */
    if (GetCPUName(szText, sizeof(szText)))
    {
        IoAddValueName(1, 1, IDS_CPUID_NAME);
        IoSetItemText(szText);
    }

    /* Get CPU Vendor */
    GetCPUVendor(szText, sizeof(szText));
    IoAddValueName(1, 1, IDS_MANUFACTURER);
    IoSetItemText(szText);

    /* CPU Architecture */
    IoAddValueName(1, 1, IDS_HW_ARCH);
    IoSetItemText(Is64BitCpu() ? L"AMD64/EM64T" : L"x86");

    /* Physical processors count */
    Count = GetPhysicalProcessorsCount();
    if (Count > 0)
    {
        IoAddValueName(1, 1, IDS_CPUID_PHYSICAL_COUNT);
        IoSetItemText(L"%d", Count);
    }

    /* Logical processors count */
    Count = GetLogicalProcessorsCount();
    if (Count > 0)
    {
        IoAddValueName(1, 1, IDS_CPUID_LOGICAL_COUNT);
        IoSetItemText(L"%d", Count);
    }

    IoAddFooter();

    IoAddHeader(0, 2, IDS_SUMMARY_HDD);
    HardDrivesInfo();
    IoAddFooter();

    /* Monitors */
    ShowMonitorsInfo();

    /* Printers */
    //IoAddHeader(0, 6, IDS_CAT_HW_PRINTERS);
    //ShowPrintersInfo();
    //IoAddFooter();

    /* Network Adapters */
    IoAddHeader(0, 7, IDS_CAT_NETWORK_CARDS);
    ShowNetAdaptersInfo();
    IoAddFooter();

    IoAddHeader(0, 3, IDS_SUMMARY_MEM);

    MemStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&MemStatus))
    {
        /* Total physical memory */
        IoAddValueName(1, 3, IDS_ALL_MEMORY);
        IoSetItemText(L"%I64d MB (%.2f GB)",
                      (MemStatus.ullTotalPhys / (1024 * 1024)),
                      ((DOUBLE)MemStatus.ullTotalPhys / (DOUBLE)(1024 * 1024 * 1024)));

        /* Total physical free memory */
        IoAddValueName(1, 3, IDS_FREE_MEMORY);
        IoSetItemText(L"%I64d MB (%.2f GB)",
                      (MemStatus.ullAvailPhys / (1024 * 1024)),
                      ((DOUBLE)MemStatus.ullAvailPhys / (DOUBLE)(1024 * 1024 * 1024)));

        /* Total physical used memory */
        IoAddValueName(1, 3, IDS_USED_MEMORY);
        IoSetItemText(L"%I64d MB (%.2f GB)",
                      ((MemStatus.ullTotalPhys - MemStatus.ullAvailPhys) / (1024 * 1024)),
                      ((DOUBLE)(MemStatus.ullTotalPhys - MemStatus.ullAvailPhys) / (DOUBLE)(1024 * 1024 * 1024)));

        /* Percent used physical memory */
        IoAddValueName(1, 3, IDS_PERCENT_USED);
        IoSetItemText(L"%d%%",
                       ((MemStatus.ullTotalPhys - MemStatus.ullAvailPhys) * 100)/MemStatus.ullTotalPhys);

        IoAddFooter();

        /* Total page file */
        IoAddValueName(1, 3, IDS_TOTAL_PAGINGFILE);
        IoSetItemText(L"%I64d MB (%.2f GB)",
                      (MemStatus.ullTotalPageFile / (1024 * 1024)),
                      ((DOUBLE)MemStatus.ullTotalPageFile / (DOUBLE)(1024 * 1024 * 1024)));

        /* Free page file */
        IoAddValueName(1, 3, IDS_FREE_PAGINGFILE);
        IoSetItemText(L"%I64d MB (%.2f GB)",
                      (MemStatus.ullAvailPageFile / (1024 * 1024)),
                      ((DOUBLE)MemStatus.ullAvailPageFile / (DOUBLE)(1024 * 1024 * 1024)));

        /* Used page file */
        IoAddValueName(1, 3, IDS_USED_PAGINFILE);
        IoSetItemText(L"%I64d MB (%.2f GB)",
                      ((MemStatus.ullTotalPageFile - MemStatus.ullAvailPageFile)/(1024 * 1024)),
                      ((DOUBLE)(MemStatus.ullTotalPageFile - MemStatus.ullAvailPageFile)/(DOUBLE)(1024 * 1024 * 1024)));

        /* Percent used page file */
        IoAddValueName(1, 3, IDS_PERCENT_PAGINFILE);
        IoSetItemText(L"%d%%",
                      ((MemStatus.ullTotalPageFile - MemStatus.ullAvailPageFile) * 100)/MemStatus.ullTotalPageFile);

        IoAddFooter();

        /* Total virtual memory */
        IoAddValueName(1, 3, IDS_TOTAL_VIRUALMEM);
        IoSetItemText(L"%I64d MB (%.2f GB)",
                      (MemStatus.ullTotalVirtual / (1024 * 1024)),
                      ((DOUBLE)MemStatus.ullTotalVirtual / (DOUBLE)(1024 * 1024 * 1024)));

        /* Free virtual memory */
        IoAddValueName(1, 3, IDS_FREE_VIRUALMEM);
        IoSetItemText(L"%I64d MB (%.2f GB)",
                      (MemStatus.ullAvailVirtual / (1024 * 1024)),
                      ((DOUBLE)MemStatus.ullAvailVirtual / (DOUBLE)(1024 * 1024 * 1024)));

        /* Used virtual memory */
        IoAddValueName(1, 3, IDS_USED_VIRTMEM);
        IoSetItemText(L"%I64d MB (%.2f GB)",
                      ((MemStatus.ullTotalVirtual - MemStatus.ullAvailVirtual)/(1024 * 1024)),
                      ((DOUBLE)(MemStatus.ullTotalVirtual - MemStatus.ullAvailVirtual)/(DOUBLE)(1024 * 1024 * 1024)));

        /* Percent used virtual memory */
        IoAddValueName(1, 3, IDS_PERCENT_VIRTMEM);
        IoSetItemText(L"%d%%",
                      ((MemStatus.ullTotalVirtual - MemStatus.ullAvailVirtual) * 100)/MemStatus.ullTotalVirtual);
    }

    IoAddFooter();

    DebugEndReceiving();
}
