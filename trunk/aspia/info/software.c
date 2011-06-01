/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/software.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include <tlhelp32.h>


typedef BOOL (CALLBACK *APPENUMPROC)(LPWSTR lpName, HKEY hAppKey);


static BOOL
EnumInstalledApplications(BOOL IsUpdates, BOOL IsUserKey, APPENUMPROC lpEnumProc)
{
    DWORD dwSize = MAX_PATH, dwType, dwValue;
    BOOL bIsSystemComponent, bIsUpdate;
    WCHAR szParentKeyName[MAX_PATH];
    WCHAR szDisplayName[MAX_PATH];
    WCHAR szKeyName[MAX_PATH];
    HKEY hKey, hRootKey, hAppKey;
    LONG ItemIndex = 0;

    hRootKey = IsUserKey ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    if (RegOpenKey(hRootKey,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
                   &hKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    while (RegEnumKeyEx(hKey,
                        ItemIndex,
                        szKeyName,
                        &dwSize,
                        NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        if (RegOpenKey(hKey, szKeyName, &hAppKey) == ERROR_SUCCESS)
        {
            dwType = REG_DWORD;
            dwSize = sizeof(DWORD);

            if (RegQueryValueEx(hAppKey,
                                L"SystemComponent",
                                NULL,
                                &dwType,
                                (LPBYTE)&dwValue,
                                &dwSize) == ERROR_SUCCESS)
            {
                bIsSystemComponent = (dwValue == 0x1);
            }
            else
            {
                bIsSystemComponent = FALSE;
            }

            dwType = REG_SZ;
            dwSize = MAX_PATH;
            bIsUpdate = (RegQueryValueEx(hAppKey,
                                         L"ParentKeyName",
                                         NULL,
                                         &dwType,
                                         (LPBYTE)szParentKeyName,
                                         &dwSize) == ERROR_SUCCESS);

            dwSize = MAX_PATH;
            if (RegQueryValueEx(hAppKey,
                                L"DisplayName",
                                NULL,
                                &dwType,
                                (LPBYTE)szDisplayName,
                                &dwSize) == ERROR_SUCCESS)
            {
                if (!bIsSystemComponent)
                {
                    if ((IsUpdates == FALSE && !bIsUpdate) || /* Applications only */
                        (IsUpdates == TRUE && bIsUpdate)) /* Updates only */
                    {
                        if (!lpEnumProc(szDisplayName, hAppKey))
                            break;
                    }
                }
            }
            RegCloseKey(hAppKey);
        }

        dwSize = MAX_PATH;
        ++ItemIndex;
    }

    RegCloseKey(hKey);

    return TRUE;
}

static BOOL
GetApplicationString(HKEY hKey, LPWSTR lpKeyName, LPWSTR lpString, SIZE_T Size)
{
    DWORD dwSize = MAX_PATH;

    if (RegQueryValueEx(hKey,
                        lpKeyName,
                        NULL,
                        NULL,
                        (LPBYTE)lpString,
                        &dwSize) == ERROR_SUCCESS)
    {
        if (SafeStrLen(lpString) == 0)
            StringCbCopy(lpString, Size, L"-");
        return TRUE;
    }

    StringCbCopy(lpString, Size, L"-");

    return FALSE;
}

static BOOL CALLBACK
EnumInstalledAppProc(LPWSTR lpName, HKEY hAppKey)
{
    WCHAR szText[MAX_PATH];
    SIZE_T TextSize = sizeof(szText);
    INT Index;

    Index = IoAddItem(0, lpName);

    /* Get version info */
    GetApplicationString(hAppKey,
                         L"DisplayVersion",
                         szText, TextSize);
    IoSetItemText(Index, 1, szText);
    /* Get publisher */
    GetApplicationString(hAppKey,
                         L"Publisher",
                         szText, TextSize);
    IoSetItemText(Index, 2, szText);
    /* Get help link */
    GetApplicationString(hAppKey,
                         L"HelpLink",
                         szText, TextSize);
    IoSetItemText(Index, 3, szText);
    /* Get help telephone */
    GetApplicationString(hAppKey,
                         L"HelpTelephone",
                         szText, TextSize);
    IoSetItemText(Index, 4, szText);
    /* Get URL update info */
    GetApplicationString(hAppKey,
                         L"URLUpdateInfo",
                         szText, TextSize);
    IoSetItemText(Index, 5, szText);
    /* Get URL update info */
    GetApplicationString(hAppKey,
                         L"URLInfoAbout",
                         szText, TextSize);
    IoSetItemText(Index, 6, szText);
    /* Get URL update info */
    GetApplicationString(hAppKey,
                         L"InstallDate",
                         szText, TextSize);
    IoSetItemText(Index, 7, szText);
    /* Get URL update info */
    GetApplicationString(hAppKey,
                         L"InstallLocation",
                         szText, TextSize);
    IoSetItemText(Index, 8, szText);
    /* Get URL update info */
    GetApplicationString(hAppKey,
                         L"UninstallString",
                         szText, TextSize);
    IoSetItemText(Index, 9, szText);
    /* Get URL update info */
    GetApplicationString(hAppKey,
                         L"ModifyPath",
                         szText, TextSize);
    IoSetItemText(Index, 10, szText);

    return TRUE;
}

static BOOL CALLBACK
EnumInstalledUpdProc(LPWSTR lpName, HKEY hAppKey)
{
    WCHAR szText[MAX_PATH];
    SIZE_T TextSize = sizeof(szText);
    INT Index;

    Index = IoAddItem(0, lpName);

    /* Get URL update info */
    GetApplicationString(hAppKey,
                         L"URLInfoAbout",
                         szText, TextSize);
    IoSetItemText(Index, 1, szText);
    /* Get URL update info */
    GetApplicationString(hAppKey,
                         L"UninstallString",
                         szText, TextSize);
    IoSetItemText(Index, 2, szText);

    return TRUE;
}

VOID
SOFTWARE_InstalledUpdInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_UPDATES);
    EnumInstalledApplications(TRUE, TRUE, EnumInstalledUpdProc);
    EnumInstalledApplications(TRUE, FALSE, EnumInstalledUpdProc);

    DebugEndReceiving();
}

VOID
SOFTWARE_InstalledAppInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_SOFTWARE);
    EnumInstalledApplications(FALSE, TRUE, EnumInstalledAppProc);
    EnumInstalledApplications(FALSE, FALSE, EnumInstalledAppProc);

    DebugEndReceiving();
}

VOID
SOFTWARE_TaskMgr(VOID)
{
    HANDLE hProcessSnap, hProcess;
    WCHAR szText[MAX_STR_LEN], szFilePath[MAX_PATH];
    PROCESS_MEMORY_COUNTERS MemCounters;
    PROCESSENTRY32 pe32;
    INT Index, IconIndex;
    HICON hIcon;

    DebugStartReceiving();

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return;

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        return;
    }

    do
    {
        if (pe32.szExeFile[0] == L'[')
            continue;

        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                               FALSE,
                               pe32.th32ProcessID);

        if (GetModuleFileNameEx(hProcess, NULL,
                                szFilePath, MAX_PATH))
        {
            hIcon = ExtractIcon(hIconsInst, szFilePath, 0);
            if (!hIcon)
            {
                IconIndex = IoAddIcon(IDI_APPS);
            }
            else
            {
                IconIndex = ImageList_AddIcon(hListViewImageList, hIcon);
                DestroyIcon(hIcon);
            }
        }
        else
        {
            szFilePath[0] = 0;
            IconIndex = IoAddIcon(IDI_APPS);
        }

        Index = IoAddItem(IconIndex, pe32.szExeFile);

        IoSetItemText(Index, 1,
                      (SafeStrLen(szFilePath) > 0) ? szFilePath : L"-");

        MemCounters.cb = sizeof(MemCounters);
        if (GetProcessMemoryInfo(hProcess, &MemCounters, sizeof(MemCounters)))
        {
            /* Memory usage */
            StringCbPrintf(szText, sizeof(szText), L"%ld KB",
                           MemCounters.WorkingSetSize / 1024);
            IoSetItemText(Index, 2, szText);

            /* Pagefile usage */
            StringCbPrintf(szText, sizeof(szText), L"%ld KB",
                           MemCounters.PagefileUsage / 1024);
            IoSetItemText(Index, 3, szText);
        }
        else
        {
            IoSetItemText(Index, 2, L"-");
            IoSetItemText(Index, 3, L"-");
        }

        /* Description */
        if (!GetFileDescription(szFilePath, szText, sizeof(szText)))
            IoSetItemText(Index, 4, L"-");
        else
            IoSetItemText(Index, 4, szText);

        CloseHandle(hProcess);
    }
    while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    DebugEndReceiving();
}

VOID
InsertKeySep(LPWSTR szIn, LPWSTR szOut)
{
    SIZE_T i = 0, j, c = 0;

    for (j = 0; i < (SIZE_T)SafeStrLen(szIn); j++)
    {
        if (c == 5)
        {
            szOut[j] = L'-';
            c = 0;
        }
        else
        {
            szOut[j] = szIn[i];
            ++i, ++c;
        }
    }
}

VOID
SOFTWARE_LicensesInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szTemp[MAX_STR_LEN];
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_CONTACT);

    /* Window Key */
    if (GetMSProductKey(FALSE,
                        "SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion",
                        szText, sizeof(szText)/sizeof(WCHAR)))
    {
        Index = IoAddItem(0, L"Microsoft Windows");
        IoSetItemText(Index, 1, szText);
    }

    /* Office 2010 Key */
    if (GetMSProductKey(FALSE,
                        "SOFTWARE\\Microsoft\\Office\\14.0\\Registration\\{90140000-0057-0000-0000-0000000FF1CE}",
                        szText, sizeof(szText)/sizeof(WCHAR)))
    {
        Index = IoAddItem(0, L"Microsoft Office 2010");
        IoSetItemText(Index, 1, szText);
    }

    /* Office 2007 Key */
    if (GetMSProductKey(FALSE,
                        "SOFTWARE\\Microsoft\\Office\\12.0\\Registration\\{90120000-0030-0000-0000-0000000FF1CE}",
                        szText, sizeof(szText)/sizeof(WCHAR)))
    {
        Index = IoAddItem(0, L"Microsoft Office 2007");
        IoSetItemText(Index, 1, szText);
    }

    /* Office 2003 Key */
    if (GetMSProductKey(FALSE,
                        "SOFTWARE\\Microsoft\\Office\\11.0\\Registration\\{90170409-6000-11D3-8CFE-0150048383C9}",
                        szText, sizeof(szText)/sizeof(WCHAR)))
    {
        Index = IoAddItem(0, L"Microsoft Office 2003");
        IoSetItemText(Index, 1, szText);
    }

    /* Office XP Key */
    if (GetMSProductKey(FALSE,
                        "SOFTWARE\\Microsoft\\Office\\10.0\\Registration\\{90280409-6000-11D3-8CFE-0050048383C9}",
                        szText, sizeof(szText)/sizeof(WCHAR)))
    {
        Index = IoAddItem(0, L"Microsoft Office XP");
        IoSetItemText(Index, 1, szText);
    }

    /* Office Web Developer 2007 Key */
    if (GetMSProductKey(FALSE,
                        "SOFTWARE\\Microsoft\\Office\\12.0\\Registration\\{90120000-0021-0000-0000-0000000FF1CE}",
                        szText, sizeof(szText)/sizeof(WCHAR)))
    {
        Index = IoAddItem(0, L"Microsoft Office Visual Web Developer 2007");
        IoSetItemText(Index, 1, szText);
    }

    /* Windows Mobile Device Center 6.1 */
    if (GetMSProductKey(FALSE,
                        "SOFTWARE\\Microsoft\\Windows Mobile Device Center\\6.1\\Registration",
                        szText, sizeof(szText)/sizeof(WCHAR)))
    {
        Index = IoAddItem(0, L"Windows Mobile Device Center 6.1");
        IoSetItemText(Index, 1, szText);
    }

    /* Internet Explorer Key */
    if (GetMSProductKey(FALSE,
                        "SOFTWARE\\Microsoft\\Internet Explorer\\Registration",
                        szText, sizeof(szText)/sizeof(WCHAR)))
    {
        Index = IoAddItem(0, L"Internet Explorer");
        IoSetItemText(Index, 1, szText);
    }

    /* Visual Studio 2010 Key */
    if (GetStringFromRegistry(HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\VisualStudio\\10.0\\Registration",
                              L"PIDKEY",
                              szTemp,
                              sizeof(szTemp)/sizeof(WCHAR)))
    {
        InsertKeySep(szTemp, szText);
        Index = IoAddItem(0, L"Microsoft Visual Studio 2010");
        IoSetItemText(Index, 1, szText);
    }

    /* Visual Studio 2008 Key */
    if (GetStringFromRegistry(HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\VisualStudio\\9.0\\Registration",
                              L"PIDKEY",
                              szTemp,
                              sizeof(szTemp)/sizeof(WCHAR)))
    {
        InsertKeySep(szTemp, szText);
        Index = IoAddItem(0, L"Microsoft Visual Studio 2008");
        IoSetItemText(Index, 1, szText);
    }

    /* Visual Studio 2005 Key */
    if (GetStringFromRegistry(HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Registration",
                              L"PIDKEY",
                              szTemp,
                              sizeof(szTemp)/sizeof(WCHAR)))
    {
        InsertKeySep(szTemp, szText);
        Index = IoAddItem(0, L"Microsoft Visual Studio 2005");
        IoSetItemText(Index, 1, szText);
    }

    DebugEndReceiving();
}
