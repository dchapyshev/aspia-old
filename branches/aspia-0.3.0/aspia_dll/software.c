/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/software.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"

typedef VOID (CALLBACK *APPENUMPROC)(LPWSTR lpName, INST_APP_INFO Info);

static BOOL
EnumInstalledApplications(BOOL IsX64, BOOL IsUpdates, BOOL IsUserKey, APPENUMPROC lpEnumProc)
{
    DWORD dwSize = MAX_PATH, dwType, dwValue;
    BOOL bIsSystemComponent, bIsUpdate;
    WCHAR szParentKeyName[MAX_PATH];
    WCHAR szDisplayName[MAX_PATH];
    WCHAR *pKeyPath;
    HKEY hKey;
    INST_APP_INFO Info = {0};
    LONG ItemIndex = 0;

    Info.hRootKey = IsUserKey ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

    if (IsX64)
    {
        pKeyPath = L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    }
    else
    {
        pKeyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    }

    if (RegOpenKey(Info.hRootKey,
                   pKeyPath,
                   &hKey) != ERROR_SUCCESS)
    {
        DebugTrace(L"RegOpenKey(%s) failed!", pKeyPath);
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
    INST_APP_INFO *pInfo;

    pInfo = Alloc(sizeof(INST_APP_INFO));
    if (!pInfo)
    {
        DebugTrace(L"Alloc(%d) failed!", sizeof(INST_APP_INFO));
        return;
    }

    *pInfo = Info;

    IoAddItem(0, 0, lpName);

    if (IoGetTarget() == IO_TARGET_LISTVIEW)
    {
        ListViewSetItemParam(DllParams.hListView, IoGetCurrentItemIndex(), (LPARAM)pInfo);
    }

    /* Get version info */
    GetApplicationString(pInfo->hAppKey,
                         L"DisplayVersion",
                         szText, sizeof(szText));
    IoSetItemText(szText);
    /* Get publisher */
    GetApplicationString(pInfo->hAppKey,
                         L"Publisher",
                         szText, sizeof(szText));
    IoSetItemText(szText);

    /* Get help link */
    GetApplicationString(pInfo->hAppKey,
                         L"HelpLink",
                         szText, sizeof(szText));
    IoSetItemText(L"%s", szText);

    /* Get help telephone */
    GetApplicationString(pInfo->hAppKey,
                         L"HelpTelephone",
                         szText, sizeof(szText));
    IoSetItemText(szText);

    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"URLUpdateInfo",
                         szText, sizeof(szText));
    IoSetItemText(L"%s", szText);

    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"URLInfoAbout",
                         szText, sizeof(szText));
    IoSetItemText(L"%s", szText);

    /* Get install date */
    GetApplicationString(pInfo->hAppKey,
                         L"InstallDate",
                         szText, sizeof(szText));
    IoSetItemText(szText);

    /* Get install location */
    GetApplicationString(pInfo->hAppKey,
                         L"InstallLocation",
                         szText, sizeof(szText));
    IoSetItemText(szText);

    /* Get uninstall string */
    GetApplicationString(pInfo->hAppKey,
                         L"UninstallString",
                         szText, sizeof(szText));
    IoSetItemText(szText);

    /* Get modify path */
    GetApplicationString(pInfo->hAppKey,
                         L"ModifyPath",
                         szText, sizeof(szText));
    IoSetItemText(szText);

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
    INST_APP_INFO *pInfo;

    pInfo = Alloc(sizeof(INST_APP_INFO));
    if (!pInfo)
    {
        DebugTrace(L"Alloc(%d) failed!", sizeof(INST_APP_INFO));
        return;
    }

    *pInfo = Info;

    IoAddItem(0, 0, lpName);

    if (IoGetTarget() == IO_TARGET_LISTVIEW)
    {
        ListViewSetItemParam(DllParams.hListView, IoGetCurrentItemIndex(), (LPARAM)pInfo);
    }

    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"URLInfoAbout",
                         szText, sizeof(szText));
    IoSetItemText(L"%s", szText);

    /* Get URL update info */
    GetApplicationString(pInfo->hAppKey,
                         L"UninstallString",
                         szText, sizeof(szText));
    IoSetItemText(szText);

    if (IoGetTarget() != IO_TARGET_LISTVIEW)
    {
        RegCloseKey(pInfo->hAppKey);
        Free(pInfo);
    }
}

VOID
SOFTWARE_InstalledAppsFree(VOID)
{
    INT Count = ListView_GetItemCount(DllParams.hListView) - 1;
    INST_APP_INFO *pInfo;

    while (Count >= 0)
    {
        pInfo = ListViewGetlParam(DllParams.hListView, Count);
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
    EnumInstalledApplications(FALSE, TRUE, TRUE, EnumInstalledUpdProc);
    EnumInstalledApplications(FALSE, TRUE, FALSE, EnumInstalledUpdProc);

#ifndef _M_IX86
    EnumInstalledApplications(TRUE, TRUE, TRUE, EnumInstalledUpdProc);
    EnumInstalledApplications(TRUE, TRUE, FALSE, EnumInstalledUpdProc);
#endif

    DebugEndReceiving();
}

VOID
SOFTWARE_InstalledAppInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_SOFTWARE);
    EnumInstalledApplications(FALSE, FALSE, TRUE, EnumInstalledAppProc);
    EnumInstalledApplications(FALSE, FALSE, FALSE, EnumInstalledAppProc);

#ifndef _M_IX86
    EnumInstalledApplications(TRUE, FALSE, TRUE, EnumInstalledAppProc);
    EnumInstalledApplications(TRUE, FALSE, FALSE, EnumInstalledAppProc);
#endif

    DebugEndReceiving();
}

VOID
SOFTWARE_TaskMgr(VOID)
{
    HANDLE hProcessSnap, hProcess;
    WCHAR szText[MAX_STR_LEN], szFilePath[MAX_PATH];
    PROCESS_MEMORY_COUNTERS MemCounters;
    PROCESSENTRY32 pe32;
    INT IconIndex;
    HICON hIcon;

    DebugStartReceiving();

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"CreateToolhelp32Snapshot() failed! Error code = %x",
                   GetLastError());
        goto Cleanup;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        DebugTrace(L"Process32First() failed! Error code = %x",
                   GetLastError());
        goto Cleanup;
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
            hIcon = ExtractIcon(DllParams.hIconsInst, szFilePath, 0);
            if (!hIcon)
            {
                IconIndex = IoAddIcon(IDI_APPS);
            }
            else
            {
                IconIndex = ImageList_AddIcon(*DllParams.hListImgList, hIcon);
                DestroyIcon(hIcon);
            }
        }
        else
        {
            szFilePath[0] = 0;
            IconIndex = IoAddIcon(IDI_APPS);
        }

        IoAddItem(0, IconIndex, pe32.szExeFile);

        if (IoGetTarget() == IO_TARGET_LISTVIEW)
        {
            ListViewSetItemParam(DllParams.hListView, IoGetCurrentItemIndex(), pe32.th32ProcessID);
        }

        IoSetItemText((szFilePath[0] != 0) ? szFilePath : L"-");

        MemCounters.cb = sizeof(MemCounters);
        if (GetProcessMemoryInfo(hProcess, &MemCounters, sizeof(MemCounters)))
        {
            /* Memory usage */
            IoSetItemText(L"%ld KB",
                          MemCounters.WorkingSetSize / 1024);

            /* Pagefile usage */
            IoSetItemText(L"%ld KB",
                          MemCounters.PagefileUsage / 1024);
        }
        else
        {
            IoSetItemText(L"-");
            IoSetItemText(L"-");
        }

        /* Description */
        if (!GetFileDescription(szFilePath, szText, sizeof(szText)))
            IoSetItemText(L"-");
        else
        {
            IoSetItemText((szText[0] != 0) ? szText : L"-");
        }

        CloseHandle(hProcess);

        if (IsCanceled) break;
    }
    while (Process32Next(hProcessSnap, &pe32));

Cleanup:
    if (hProcessSnap != INVALID_HANDLE_VALUE)
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
                szDesc[0] = 0;
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

                IoAddItem(0, 0, szKeyName);
                if (szDesc[0] != 0)
                    IoSetItemText(szDesc);
                else
                    IoSetItemText(L"-");
                IoSetItemText(szContentType);
            }
        }

        if (IsCanceled) break;

        dwSize = MAX_PATH;
        ++lIndex;
    }

    DebugEndReceiving();
}
