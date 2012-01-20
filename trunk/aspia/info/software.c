/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/software.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include "driver.h"
#include <tlhelp32.h>


static BOOL
EnumInstalledApplications(BOOL IsUpdates, BOOL IsUserKey, APPENUMPROC lpEnumProc)
{
    DWORD dwSize = MAX_PATH, dwType, dwValue;
    BOOL bIsSystemComponent, bIsUpdate;
    WCHAR szParentKeyName[MAX_PATH];
    WCHAR szDisplayName[MAX_PATH];
    HKEY hKey;
    INST_APP_INFO Info = {0};
    LONG ItemIndex = 0;

    Info.hRootKey = IsUserKey ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    if (RegOpenKey(Info.hRootKey,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall",
                   &hKey) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    while (RegEnumKeyEx(hKey,
                        ItemIndex,
                        Info.szKeyName,
                        &dwSize,
                        NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        if (RegOpenKey(hKey, Info.szKeyName, &Info.hAppKey) == ERROR_SUCCESS)
        {
            dwType = REG_DWORD;
            dwSize = sizeof(DWORD);

            if (RegQueryValueEx(Info.hAppKey,
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
            bIsUpdate = (RegQueryValueEx(Info.hAppKey,
                                         L"ParentKeyName",
                                         NULL,
                                         &dwType,
                                         (LPBYTE)szParentKeyName,
                                         &dwSize) == ERROR_SUCCESS);

            dwSize = MAX_PATH;
            if (RegQueryValueEx(Info.hAppKey,
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
                        lpEnumProc(szDisplayName, Info);
                    }
                }
            }
        }

        if (IsCanceled) break;

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

static VOID CALLBACK
EnumInstalledAppProc(LPWSTR lpName, INST_APP_INFO Info)
{
    WCHAR szText[MAX_PATH];
    SIZE_T TextSize = sizeof(szText);
    INST_APP_INFO *pInfo;
    INT Index;

    pInfo = Alloc(sizeof(INST_APP_INFO));
    if (!pInfo) return;

    *pInfo = Info;

    Index = IoAddItem(0, 0, lpName);

    if (IoGetTarget() == IO_TARGET_LISTVIEW)
    {
        ListViewSetItemParam(Index, (LPARAM)pInfo);
    }

    /* Get version info */
    GetApplicationString(pInfo->hAppKey,
                         L"DisplayVersion",
                         szText, TextSize);
    IoSetItemText(Index, 1, szText);
    /* Get publisher */
    GetApplicationString(pInfo->hAppKey,
                         L"Publisher",
                         szText, TextSize);
    IoSetItemText(Index, 2, szText);
    /* Get help link */
    GetApplicationString(pInfo->hAppKey,
                         L"HelpLink",
                         szText, TextSize);
    IoSetItemText(Index, 3, szText);
    /* Get help telephone */
    GetApplicationString(pInfo->hAppKey,
                         L"HelpTelephone",
                         szText, TextSize);
    IoSetItemText(Index, 4, szText);
    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"URLUpdateInfo",
                         szText, TextSize);
    IoSetItemText(Index, 5, szText);
    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"URLInfoAbout",
                         szText, TextSize);
    IoSetItemText(Index, 6, szText);
    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"InstallDate",
                         szText, TextSize);
    IoSetItemText(Index, 7, szText);
    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"InstallLocation",
                         szText, TextSize);
    IoSetItemText(Index, 8, szText);
    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"UninstallString",
                         szText, TextSize);
    IoSetItemText(Index, 9, szText);
    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"ModifyPath",
                         szText, TextSize);
    IoSetItemText(Index, 10, szText);

    if (IoGetTarget() != IO_TARGET_LISTVIEW)
    {
        RegCloseKey(pInfo->hAppKey);
        Free(pInfo);
    }
}

static VOID CALLBACK
EnumInstalledUpdProc(LPWSTR lpName, INST_APP_INFO Info)
{
    WCHAR szText[MAX_PATH];
    SIZE_T TextSize = sizeof(szText);
    INST_APP_INFO *pInfo;
    INT Index;

    pInfo = Alloc(sizeof(INST_APP_INFO));
    if (!pInfo) return;

    *pInfo = Info;

    Index = IoAddItem(0, 0, lpName);

    if (IoGetTarget() == IO_TARGET_LISTVIEW)
    {
        ListViewSetItemParam(Index, (LPARAM)pInfo);
    }

    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"URLInfoAbout",
                         szText, TextSize);
    IoSetItemText(Index, 1, szText);
    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"UninstallString",
                         szText, TextSize);
    IoSetItemText(Index, 2, szText);

    if (IoGetTarget() != IO_TARGET_LISTVIEW)
    {
        RegCloseKey(pInfo->hAppKey);
        Free(pInfo);
    }
}

VOID
SOFTWARE_InstalledAppsFree(VOID)
{
    INT Count = ListView_GetItemCount(hListView) - 1;
    INST_APP_INFO *pInfo;

    while (Count >= 0)
    {
        pInfo = ListViewGetlParam(hListView, Count);
        if (pInfo > 0)
        {
            RegCloseKey(pInfo->hAppKey);
            Free(pInfo);
        }
        --Count;
    }
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

        Index = IoAddItem(0, IconIndex, pe32.szExeFile);

        ListViewSetItemParam(Index, pe32.th32ProcessID);

        IoSetItemText(Index, 1,
                      (szFilePath[0] != 0) ? szFilePath : L"-");

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

        if (IsCanceled) break;
    }
    while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    DebugEndReceiving();
}

VOID
SOFTWARE_FileTypesInfo(VOID)
{
    WCHAR szKeyName[MAX_PATH], szRoot[MAX_PATH],
          szDesc[MAX_PATH], szContentType[MAX_PATH];
    DWORD dwSize;
    HKEY hKey;
    LONG lIndex = 0, res;
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_APPS);

    dwSize = MAX_PATH;
    while (RegEnumKeyEx(HKEY_CLASSES_ROOT,
                        lIndex,
                        szKeyName,
                        &dwSize,
                        NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        if (szKeyName[0] != L'.')
        {
            dwSize = MAX_PATH;
            ++lIndex;
            continue;
        }

        if (RegOpenKey(HKEY_CLASSES_ROOT, szKeyName, &hKey) == ERROR_SUCCESS)
        {
            dwSize = MAX_PATH;

            res = RegQueryValueEx(hKey, NULL, NULL, NULL,
                                  (LPBYTE)szRoot,
                                  &dwSize);

            if (res != ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
                dwSize = MAX_PATH;
                ++lIndex;
                continue;
            }

            dwSize = MAX_PATH;
            res = RegQueryValueEx(hKey, L"Content Type",
                                  NULL, NULL,
                                  (LPBYTE)szContentType,
                                  &dwSize);
            if (res != ERROR_SUCCESS)
            {
                StringCbCopy(szContentType, sizeof(szContentType), L"-");
            }

            RegCloseKey(hKey);

            if (RegOpenKey(HKEY_CLASSES_ROOT, szRoot, &hKey) == ERROR_SUCCESS)
            {
                dwSize = MAX_PATH;
                res = RegQueryValueEx(hKey, NULL, NULL, NULL,
                                      (LPBYTE)szDesc,
                                      &dwSize);
                RegCloseKey(hKey);

                if (res != ERROR_SUCCESS)
                {
                    dwSize = MAX_PATH;
                    ++lIndex;
                    continue;
                }

                Index = IoAddItem(0, 0, szKeyName);
                IoSetItemText(Index, 1, szDesc);
                IoSetItemText(Index, 2, szContentType);
            }
        }

        if (IsCanceled) break;

        dwSize = MAX_PATH;
        ++lIndex;
    }

    DebugEndReceiving();
}
