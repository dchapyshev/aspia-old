/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/summary.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"


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

        if (SafeStrLen(szVolumeName) > 0)
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
                           L"%I64d MB / %I64d MB",
                           (TotalNumberOfFreeBytes.QuadPart / (1024 * 1024)),
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
        ConvertSecondsToString(liCount.QuadPart / liFreq.QuadPart,
                               szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);
    }
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

    IoAddHeader(0, IDS_SUMMARY_OS, 0);

    /* Product name */
    Index = IoAddValueName(1, IDS_OS_PRODUCT_NAME, 0);

    szText[0] = 0;
    if (GetStringFromRegistry(HKEY_LOCAL_MACHINE,
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

    IoAddHeader(0, IDS_SUMMARY_MEM, 3);

    MemStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&MemStatus))
    {
        /* Total physical memory */
        Index = IoAddValueName(1, IDS_ALL_MEMORY, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB",
                       (MemStatus.ullTotalPhys / (1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Total physical free memory */
        Index = IoAddValueName(1, IDS_FREE_MEMORY, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB",
                       (MemStatus.ullAvailPhys / (1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Total physical used memory */
        Index = IoAddValueName(1, IDS_USED_MEMORY, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB",
                       ((MemStatus.ullTotalPhys - MemStatus.ullAvailPhys) / (1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Percent used physical memory */
        Index = IoAddValueName(1, IDS_PERCENT_USED, 3);
        StringCbPrintf(szText, sizeof(szText), L"%d%%",
                       ((MemStatus.ullTotalPhys - MemStatus.ullAvailPhys) * 100)/MemStatus.ullTotalPhys);
        IoSetItemText(Index, 1, szText);

        IoAddFooter();

        /* Total page file */
        Index = IoAddValueName(1, IDS_TOTAL_PAGINGFILE, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB",
                       (MemStatus.ullTotalPageFile / (1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Free page file */
        Index = IoAddValueName(1, IDS_FREE_PAGINGFILE, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB",
                       (MemStatus.ullAvailPageFile / (1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Used page file */
        Index = IoAddValueName(1, IDS_USED_PAGINFILE, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB",
                       ((MemStatus.ullTotalPageFile - MemStatus.ullAvailPageFile)/(1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Percent used page file */
        Index = IoAddValueName(1, IDS_PERCENT_PAGINFILE, 3);
        StringCbPrintf(szText, sizeof(szText), L"%d%%",
                       ((MemStatus.ullTotalPageFile - MemStatus.ullAvailPageFile) * 100)/MemStatus.ullTotalPageFile);
        IoSetItemText(Index, 1, szText);

        IoAddFooter();

        /* Total virtual memory */
        Index = IoAddValueName(1, IDS_TOTAL_VIRUALMEM, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB",
                       (MemStatus.ullTotalVirtual / (1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Free virtual memory */
        Index = IoAddValueName(1, IDS_FREE_VIRUALMEM, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB",
                       (MemStatus.ullAvailVirtual / (1024 * 1024)));
        IoSetItemText(Index, 1, szText);

        /* Used virtual memory */
        Index = IoAddValueName(1, IDS_USED_VIRTMEM, 3);
        StringCbPrintf(szText, sizeof(szText), L"%I64d MB",
                       ((MemStatus.ullTotalVirtual - MemStatus.ullAvailVirtual)/(1024 * 1024)));
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
