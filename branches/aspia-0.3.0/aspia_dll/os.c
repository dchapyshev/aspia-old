/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/os.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"
#include <softpub.h>


/* Definition for the GetFontResourceInfo function */
typedef BOOL (WINAPI *PGFRI)(LPCTSTR, DWORD *, LPVOID, DWORD);


VOID
ShowRegInfo(UINT StringID, LPWSTR lpszKeyName)
{
    WCHAR szText[MAX_STR_LEN] = {0};

    GetStringFromRegistry(TRUE,
                          HKEY_LOCAL_MACHINE,
                          L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                          lpszKeyName,
                          szText,
                          MAX_STR_LEN);

    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, StringID);
        IoSetItemText(szText);
    }
}

VOID
ShowInstallDate(VOID)
{
    TCHAR szText[MAX_STR_LEN];
    DWORD dwInstallDate;

    if (GetBinaryFromRegistry(HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion",
                              L"InstallDate",
                              (LPBYTE)&dwInstallDate,
                              sizeof(dwInstallDate)))
    {
        if (TimeToString((time_t)dwInstallDate, szText, sizeof(szText)))
        {
            IoAddValueName(1, 0, IDS_OS_INSTALL_DATE);
            IoSetItemText(szText);
        }
    }
}

VOID
OS_RegInformation(VOID)
{
    WCHAR szText[MAX_STR_LEN];

    DebugStartReceiving();

    IoAddIcon(IDI_COMPUTER);
    IoAddHeader(0, 0, IDS_CAT_OS_REGDATA);

    /* Product Key */
    IoAddValueName(1, 0, IDS_PRODUCT_KEY);
    GetMSProductKey(FALSE,
                    "SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion",
                    szText, MAX_STR_LEN);
    IoSetItemText(szText);

    /* Product ID */
    ShowRegInfo(IDS_PRODUCT_ID, L"ProductId");
    /* Kernel type */
    ShowRegInfo(IDS_KERNEL_TYPE, L"CurrentType");
    /* System Root */
    ShowRegInfo(IDS_SYSTEM_ROOT, L"SystemRoot");
    /* OS Version */
    ShowRegInfo(IDS_OS_VERSION, L"CurrentVersion");
    /* OS Build */
    ShowRegInfo(IDS_OS_BUILD, L"CurrentBuild");

    /* Install Date */
    ShowInstallDate();

    /* OS Name */
    ShowRegInfo(IDS_OS_PRODUCT_NAME, L"ProductName");
    /* Organization */
    ShowRegInfo(IDS_OS_REG_ORG, L"RegisteredOrganization");
    /* User */
    ShowRegInfo(IDS_OS_REG_USER, L"RegisteredOwner");

    DebugEndReceiving();
}

VOID
OS_UsersInfo(VOID)
{
    WCHAR szYes[MAX_STR_LEN], szNo[MAX_STR_LEN],
          szText[MAX_STR_LEN];
    NET_API_STATUS netStatus;
    PUSER_INFO_3 pBuffer;
    DWORD entriesread;
    DWORD totalentries;
    DWORD resume_handle = 0;
    SIZE_T i;

    DebugStartReceiving();

    IoAddIcon(IDI_USER);
    IoAddIcon(IDI_DISABLED_USER);

    LoadMUIString(IDS_YES, szYes, MAX_STR_LEN);
    LoadMUIString(IDS_NO, szNo, MAX_STR_LEN);

    for (;;)
    {
        netStatus = NetUserEnum(NULL, 3, FILTER_NORMAL_ACCOUNT,
                                (LPBYTE*)&pBuffer,
                                (DWORD)-1, &entriesread,
                                &totalentries, &resume_handle);
        if (netStatus != NERR_Success && netStatus != ERROR_MORE_DATA)
            break;

        for (i = 0; i < entriesread; ++i)
        {
            __try
            {
                IoAddItem(0, (pBuffer[i].usri3_flags & UF_ACCOUNTDISABLE) ? 1 : 0,
                          pBuffer[i].usri3_name);

                IoSetItemText((pBuffer[i].usri3_full_name[0] != 0) ? pBuffer[i].usri3_full_name : L"N/A");

                IoSetItemText((pBuffer[i].usri3_comment[0] != 0) ? pBuffer[i].usri3_comment : L"N/A");

                IoSetItemText((pBuffer[i].usri3_flags & UF_ACCOUNTDISABLE) ? szYes : szNo);
                IoSetItemText((pBuffer[i].usri3_flags & UF_PASSWD_CANT_CHANGE) ? szYes : szNo);
                IoSetItemText((pBuffer[i].usri3_flags & UF_PASSWORD_EXPIRED) ? szYes : szNo);
                IoSetItemText((pBuffer[i].usri3_flags & UF_DONT_EXPIRE_PASSWD) ? szYes : szNo);
                IoSetItemText((pBuffer[i].usri3_flags & UF_LOCKOUT) ? szYes : szNo);

                if (pBuffer[i].usri3_last_logon == 0)
                {
                    LoadMUIString(IDS_NEVER, szText, MAX_STR_LEN);
                }
                else
                {
                    TimeToString(pBuffer[i].usri3_last_logon, szText, sizeof(szText));
                }
                IoSetItemText(szText);
                IoSetItemText(L"%ld", pBuffer[i].usri3_num_logons);
                IoSetItemText(L"%ld", pBuffer[i].usri3_bad_pw_count);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                NetApiBufferFree(pBuffer);
                continue;
            }
        }

        NetApiBufferFree(pBuffer);

        /* No more data left */
        if (netStatus != ERROR_MORE_DATA)
            break;
    }

    DebugEndReceiving();
}

VOID
OS_UsersGroupsInfo(VOID)
{
    NET_API_STATUS netStatus;
    PLOCALGROUP_INFO_1 pBuffer;
    DWORD entriesread;
    DWORD totalentries;
    DWORD_PTR resume_handle = 0;
    DWORD i;

    DebugStartReceiving();

    IoAddIcon(IDI_USERS);

    for (;;)
    {
        netStatus = NetLocalGroupEnum(NULL, 1, (LPBYTE*)&pBuffer,
                                      (DWORD)-1, &entriesread,
                                      &totalentries, &resume_handle);
        if (netStatus != NERR_Success && netStatus != ERROR_MORE_DATA)
            break;

        for (i = 0; i < entriesread; ++i)
        {
            __try
            {
                IoAddItem(0, 0, pBuffer[i].lgrpi1_name);
                IoSetItemText(pBuffer[i].lgrpi1_comment);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                NetApiBufferFree(pBuffer);
                continue;
            }
        }

        NetApiBufferFree(pBuffer);

        /* No more data left */
        if (netStatus != ERROR_MORE_DATA)
            break;
    }

    DebugEndReceiving();
}

VOID
OS_ActiveUsersInfo(VOID)
{
    SIZE_T Count, Index;
    PWTS_SESSION_INFO Sessions;
    DWORD dwSize;
    LPWSTR lpUserName, lpDomain, lpWinStationName, lpState, lpClientName;
    INT *ConnectState;

    DebugStartReceiving();

    IoAddIcon(IDI_USER);

    if (!WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &Sessions, &Count))
    {
        return;
    }

    for (Index = 0; Index < Count; Index++)
    {
        if (!WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                        Sessions[Index].SessionId,
                                        WTSUserName,
                                        &lpUserName,
                                        &dwSize) ||
            (SafeStrLen(lpUserName) == 0))
        {
            continue;
        }

        /* User Name */
        IoAddItem(0, 0, lpUserName);

        /* Domain */
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                       Sessions[Index].SessionId,
                                       WTSDomainName,
                                       &lpDomain,
                                       &dwSize) &&
            (SafeStrLen(lpDomain) > 0))
        {
            IoSetItemText(lpDomain);
        }
        else
        {
            IoSetItemText(L"-");
        }

        /* User ID */
        IoSetItemText(L"%ld", Sessions[Index].SessionId);

        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                       Sessions[Index].SessionId,
                                       WTSConnectState,
                                       (LPWSTR*)&ConnectState,
                                       &dwSize))
        {
            switch (*ConnectState)
            {
                case WTSActive:
                    lpState = L"Active";
                    break;
                case WTSConnected:
                    lpState = L"Connected";
                    break;
                case WTSConnectQuery:
                    lpState = L"Connect Query";
                    break;
                case WTSShadow:
                    lpState = L"Shadow";
                    break;
                case WTSDisconnected:
                    lpState = L"Disconnected";
                    break;
                case WTSIdle:
                    lpState = L"Idle";
                    break;
                case WTSListen:
                    lpState = L"Listen";
                    break;
                case WTSReset:
                    lpState = L"Reset";
                    break;
                case WTSDown:
                    lpState = L"Down";
                    break;
                case WTSInit:
                    lpState = L"Init";
                    break;
                default:
                    lpState = L"Unknown";
                    break;
            }
            IoSetItemText(lpState);
        }
        else
        {
            IoSetItemText(L"-");
        }

        /* Client Name */
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                       Sessions[Index].SessionId,
                                       WTSClientName,
                                       &lpClientName,
                                       &dwSize) &&
            (SafeStrLen(lpClientName) > 0))
        {
            IoSetItemText(lpClientName);
        }
        else
        {
            IoSetItemText(L"N/A");
        }

        /* WinStationName */
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                       Sessions[Index].SessionId,
                                       WTSWinStationName,
                                       &lpWinStationName,
                                       &dwSize) &&
            (SafeStrLen(lpWinStationName) > 0))
        {
            IoSetItemText(lpWinStationName);
        }
        else
        {
            IoSetItemText(L"-");
        }
    }

    DebugEndReceiving();
}

VOID
OS_FontsInfo(VOID)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WCHAR szText[MAX_STR_LEN], szPath[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    PGFRI GetFontResourceInfo;
    HINSTANCE hDLL;
    DWORD dwSize;

    DebugStartReceiving();

    IoAddIcon(IDI_FONTS);

    hDLL = LoadLibrary(L"GDI32.DLL");
    if (!hDLL) return;

    GetFontResourceInfo = (PGFRI)GetProcAddress(hDLL, "GetFontResourceInfoW");

    if (!GetFontResourceInfo)
    {
        FreeLibrary(hDLL);
        return;
    }

    SHGetSpecialFolderPath(DllParams.hMainWnd, szPath, CSIDL_FONTS, FALSE);
    StringCbCat(szPath, sizeof(szPath), L"\\*.ttf");

    hFind = FindFirstFile(szPath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        FreeLibrary(hDLL);
        return;
    }

    do
    {
        AddFontResource(FindFileData.cFileName);

        dwSize = sizeof(szText);
        GetFontResourceInfo(FindFileData.cFileName, &dwSize, szText, 1);

        IoAddItem(0, 0, szText);
        IoSetItemText(FindFileData.cFileName);

        IoSetItemText(L"%ld KB",
                      ((FindFileData.nFileSizeHigh * ((DWORDLONG)MAXDWORD + 1)) +
                      FindFileData.nFileSizeLow) / 1024);

        RemoveFontResource(FindFileData.cFileName);

        if (IsCanceled) break;
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);
    FreeLibrary(hDLL);

    DebugEndReceiving();
}

VOID
OS_CPLAppletsInfo(VOID)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    WCHAR szPath[MAX_PATH];
    NEWCPLINFO NewInfo;
    HMODULE hModule;
    APPLET_PROC Proc;
    INT IconIndex, Count, i;
    CPLINFO Info;

    DebugStartReceiving();

    GetSystemDirectory(szPath, MAX_PATH);

    StringCbCat(szPath, sizeof(szPath), L"\\*.cpl");

    DebugTrace(L"szPath = %s", szPath);

    hFind = FindFirstFile(szPath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
        DebugTrace(L"Loading library '%s'", FindFileData.cFileName);

        hModule = LoadLibrary(FindFileData.cFileName);
        if (hModule == NULL)
        {
            DebugTrace(L"Loading failed!");
            continue;
        }

        Proc = (APPLET_PROC)GetProcAddress(hModule, "CPlApplet");
        if (Proc == NULL)
        {
            DebugTrace(L"Entry point 'CPlApplet' not found!");
            continue;
        }

        if (Proc(DllParams.hMainWnd, CPL_INIT, 0, 0) == 0)
        {
            DebugTrace(L"CPl initialization failed!");
            continue;
        }

        Count = Proc(DllParams.hMainWnd, CPL_GETCOUNT, 0, 0);
        DebugTrace(L"CPls count = ", Count);
        if (Count == 0)
        {
            continue;
        }

        for (i = 0; i < Count; i++)
        {
            ZeroMemory(&NewInfo, sizeof(NewInfo));
            NewInfo.dwSize = sizeof(NEWCPLINFO);

            Proc(DllParams.hMainWnd, CPL_NEWINQUIRE, i, (LPARAM)&NewInfo);

            if (NewInfo.hIcon == 0)
            {
                Proc(DllParams.hMainWnd, CPL_INQUIRE, i, (LPARAM)&Info);
                if (Info.idIcon != 0 && Info.idName != 0)
                {
                    WCHAR szName[MAX_STR_LEN] = {0}, szDesc[MAX_STR_LEN] = {0};

                    LoadString(hModule, Info.idName, szName, MAX_STR_LEN);
                    LoadString(hModule, Info.idInfo, szDesc, MAX_STR_LEN);

                    IconIndex = 0;

                    if (IoGetTarget() == IO_TARGET_LISTVIEW)
                    {
                        HICON hIcon;

                        hIcon = (HICON)LoadImage(hModule,
                                                 MAKEINTRESOURCE(Info.idIcon),
                                                 IMAGE_ICON,
                                                 DllParams.SxSmIcon,
                                                 DllParams.SySmIcon,
                                                 LR_CREATEDIBSECTION);

                        if (hIcon == NULL)
                        {
                            IconIndex = IoAddIcon(IDI_APPS);
                        }
                        else
                        {
                            IconIndex = ImageList_AddIcon(*DllParams.hListImgList,
                                                          hIcon);
                        }

                        DestroyIcon(hIcon);
                    }

                    if (szName[0] != 0 && szDesc[0] != 0)
                    {
                        IoAddItem(0, IconIndex, szName);
                        IoSetItemText(szDesc);
                    }
                }
            }
            else
            {
                IconIndex = 0;

                if (IoGetTarget() == IO_TARGET_LISTVIEW)
                {
                    IconIndex = ImageList_AddIcon(*DllParams.hListImgList,
                                                  NewInfo.hIcon);
                }

                IoAddItem(0, IconIndex, NewInfo.szName);
                IoSetItemText(NewInfo.szInfo);
            }
        }
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);
    FreeLibrary(hModule);

    DebugEndReceiving();
}

VOID
AutorunShowRegPath(HKEY hRootKey, LPWSTR lpszPath, LPWSTR lpszName)
{
    DWORD dwIndex, dwSize, dwPathSize;
    INT Count = 0;
    INT IconIndex;
    HKEY hKey;

    DebugTrace(L"AutorunShowRegPath(%x, %s, %s) called",
               hRootKey, lpszPath, lpszName);

    if (RegOpenKeyEx(hRootKey,
                     lpszPath,
                     0,
                     KEY_QUERY_VALUE,
                     &hKey) != ERROR_SUCCESS)
    {
        DebugTrace(L"RegOpenKeyEx() failed");
        goto None;
    }

    for (dwIndex = 0; ; ++dwIndex)
    {
        WCHAR szFullPath[MAX_PATH] = {0};
        WCHAR szPath[MAX_PATH] = {0};
        WCHAR szName[MAX_PATH] = {0};

        dwPathSize = sizeof(szPath);
        dwSize = sizeof(szName);

        if (RegEnumValue(hKey,
                         dwIndex,
                         szName,
                         &dwSize,
                         NULL, NULL,
                         (LPBYTE)&szPath,
                         &dwPathSize) != ERROR_SUCCESS)
        {
            break;
        }

        if (szName[0] != 0)
        {
            HICON hIcon = NULL;
            WCHAR szNewPath[MAX_PATH];
            UINT i, len;

            if (szPath[0] == L'"')
            {
                for (i = 1, len = wcslen(szPath); i < len; i++)
                {
                    if (szPath[i] == L'"')
                    {
                        szNewPath[i - 1] = L'\0';
                        ExtractIconEx(szNewPath, 0, NULL, &hIcon, 1);
                        break;
                    }
                    szNewPath[i - 1] = szPath[i];
                }
            }
            else
            {
                ExtractIconEx(szPath, 0, NULL, &hIcon, 1);
            }

            if (!hIcon)
            {
                IconIndex = IoAddIcon(IDI_APPS);
            }
            else
            {
                IconIndex = ImageList_AddIcon(*DllParams.hListImgList,
                                              hIcon);

                DestroyIcon(hIcon);
            }

            if (Count == 0)
            {
                IoAddHeaderString(0, 0, lpszName);
            }

            IoAddItem(1, IconIndex, szName);

            DebugTrace(L"IoAddItem: IconIndex = %d, szName = %s",
                       IconIndex, szName);

            ExpandEnvironmentStrings(szPath, szFullPath, MAX_PATH);
            IoSetItemText(szFullPath);

            DebugTrace(L"IoSetItemText: szPath = %s", szPath);

            ++Count;
        }
    }

    if (Count > 0)
    {
        IoAddFooter();
    }

None:
    RegCloseKey(hKey);
}

BOOL
GetShortcutCommandLine(LPWSTR lpszLnkPath, LPWSTR lpszPath, SIZE_T Size)
{
    WCHAR szPath[MAX_PATH], szCmd[MAX_PATH];
    IShellLink *pShellLink = NULL;
    IPersistFile *pPersistFile = NULL;
    BOOL Result = FALSE;
    HRESULT hr;

    CoInitialize(NULL);

    hr = CoCreateInstance(&CLSID_ShellLink,
                          NULL,
                          CLSCTX_ALL,
                          &IID_IShellLink,
                          (LPVOID*)&pShellLink);
    if (FAILED(hr)) goto Cleanup;

    hr = pShellLink->lpVtbl->QueryInterface(pShellLink,
                                            &IID_IPersistFile,
                                            (LPVOID*)&pPersistFile);
    if (FAILED(hr)) goto Cleanup;

    hr = pPersistFile->lpVtbl->Load(pPersistFile,
                                    (LPCOLESTR)lpszLnkPath,
                                    STGM_READ);
    if (FAILED(hr)) goto Cleanup;

    hr = pShellLink->lpVtbl->GetPath(pShellLink,
                                     szPath,
                                     MAX_PATH,
                                     NULL, 0);
    if (FAILED(hr)) goto Cleanup;

    hr = pShellLink->lpVtbl->GetArguments(pShellLink,
                                          szCmd,
                                          MAX_PATH);
    if (!FAILED(hr))
    {
        StringCbCat(szPath, sizeof(szPath), L" ");
        StringCbCat(szPath, sizeof(szPath), szCmd);
    }

    StringCbCopy(lpszPath, Size, szPath);
    Result = TRUE;

Cleanup:
    if (pPersistFile) pPersistFile->lpVtbl->Release(pPersistFile);
    if (pShellLink) pShellLink->lpVtbl->Release(pShellLink);

    CoUninitialize();

    return Result;
}

VOID
AutorunShowFolderContent(LPWSTR lpszPath)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WCHAR szPath[MAX_PATH], szCmd[MAX_PATH], szFilePath[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    INT Count = 0;
    INT IconIndex;

    StringCbCopy(szPath, sizeof(szPath), lpszPath);
    StringCbCat(szPath, sizeof(szPath), L"\\*.*");

    hFind = FindFirstFile(szPath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            WCHAR szExt[MAX_PATH];
            HICON hIcon = NULL;

            /* HACK */
            if (wcscmp(FindFileData.cFileName, L"desktop.ini") == 0)
                continue;

            StringCbCopy(szFilePath, sizeof(szFilePath), lpszPath);
            StringCbCat(szFilePath, sizeof(szFilePath), L"\\");
            StringCbCat(szFilePath, sizeof(szFilePath), FindFileData.cFileName);

            if (GetFileExt(szFilePath, szExt, sizeof(szExt)) &&
                (wcscmp(szExt, L"lnk") == 0))
            {
                GetShortcutCommandLine(szFilePath, szCmd, sizeof(szCmd));
            }
            else
            {
                StringCbCopy(szCmd, sizeof(szCmd), szFilePath);
            }

            ExtractIconEx(szCmd, 0, NULL, &hIcon, 1);
            if (hIcon)
            {
                IconIndex = ImageList_AddIcon(*DllParams.hListImgList,
                                              hIcon);

                DestroyIcon(hIcon);
            }
            else
            {
                IconIndex = IoAddIcon(IDI_APPS);
            }

            if (Count == 0)
            {
                IoAddHeaderString(0, 1, lpszPath);
            }

            IoAddItem(1, IconIndex, FindFileData.cFileName);
            IoSetItemText(szCmd);

            ++Count;
        }
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    if (Count > 0) IoAddFooter();

    FindClose(hFind);
}

typedef struct
{
    HKEY hKey;
    LPWSTR lpKeyPath;
    LPWSTR lpName;
} AUTORUN_INFO;

AUTORUN_INFO AutorunInfo[] =
{
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
    },
#ifdef _AMD64_
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
    },
#endif
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
    },
#ifdef _AMD64_
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
    },
#endif
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx"
    },
#ifdef _AMD64_
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx"
    },
#endif
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices"
    },
#ifdef _AMD64_
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunServices",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices"
    },
#endif
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce"
    },
#ifdef _AMD64_
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce"
    },
#endif
    {
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        L"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
    },
#ifdef _AMD64_
    {
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",
        L"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
    },
#endif
    {
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        L"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
    },
#ifdef _AMD64_
    {
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        L"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
    },
#endif
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run"
    },
#ifdef _AMD64_
    {
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run",
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run"
    },
#endif
    {
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run",
        L"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run"
    },
#ifdef _AMD64_
    {
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run",
        L"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run"
    },
#endif
    {0}
};

VOID
OS_AutorunInfo(VOID)
{
    WCHAR szPath[MAX_PATH];
    INT IconIndex, i = 0;
    HICON hIcon;

    DebugStartReceiving();

    IoAddIcon(IDI_REG);
    IoAddIcon(IDI_FOLDER);

    do
    {
        AutorunShowRegPath(AutorunInfo[i].hKey,
                           AutorunInfo[i].lpKeyPath,
                           AutorunInfo[i].lpName);
    }
    while (AutorunInfo[++i].hKey != 0);

    /* Autorun folder for all users */
    SHGetSpecialFolderPath(DllParams.hMainWnd, szPath, CSIDL_COMMON_STARTUP, FALSE);
    AutorunShowFolderContent(szPath);

    /* Autorun folder for current user */
    SHGetSpecialFolderPath(DllParams.hMainWnd, szPath, CSIDL_STARTUP, FALSE);
    AutorunShowFolderContent(szPath);

    /* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon */
    IoAddHeaderString(0, 0, L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");

    /* winlogon shell for all users */
    if (GetStringFromRegistry(TRUE,
                              HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                              L"Shell",
                              szPath,
                              MAX_PATH))
    {
        ExtractIconEx(szPath, 0, NULL, &hIcon, 1);

        if (hIcon)
        {
            IconIndex = ImageList_AddIcon(*DllParams.hListImgList, hIcon);
            DestroyIcon(hIcon);
        }
        else
            IconIndex = IoAddIcon(IDI_APPS);

        IoAddItem(1, IconIndex, L"Shell");
        IoSetItemText(szPath);
    }

    /* winlogon userinit for all users */
    IconIndex = IoAddIcon(IDI_APPS);
    IoAddItem(1, IconIndex, L"Userinit");
    GetStringFromRegistry(TRUE,
                          HKEY_LOCAL_MACHINE,
                          L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                          L"Userinit",
                          szPath,
                          MAX_PATH);
    IoSetItemText(szPath);
    IoAddFooter();

    /* HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon */
    /* winlogon shell for current user */
    if (GetStringFromRegistry(TRUE,
                              HKEY_CURRENT_USER,
                              L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                              L"Shell",
                              szPath,
                              MAX_PATH))
    {
        ExtractIconEx(szPath, 0, NULL, &hIcon, 1);

        if (hIcon)
        {
            IconIndex = ImageList_AddIcon(*DllParams.hListImgList, hIcon);
            DestroyIcon(hIcon);
        }
        else
            IconIndex = IoAddIcon(IDI_APPS);

        IoAddHeaderString(0, 0, L"HKCU\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
        IoAddItem(1, IconIndex, L"Shell");
        IoSetItemText(szPath);

        IoAddFooter();
    }

    /* AppInit_DLLs */
    GetStringFromRegistry(TRUE,
                          HKEY_LOCAL_MACHINE,
                          L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
                          L"AppInit_DLLs",
                          szPath,
                          MAX_PATH);
    if (szPath[0] != 0)
    {
        IconIndex = IoAddIcon(IDI_APPS);
        IoAddHeaderString(0, 0, L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows");
        IoAddItem(1, IconIndex, L"AppInit_DLLs");
        IoSetItemText(szPath);
    }

    if (IsWin64System())
    {
        GetStringFromRegistry(FALSE,
                              HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
                              L"AppInit_DLLs",
                              szPath,
                              MAX_PATH);
        if (szPath[0] != 0)
        {
            IconIndex = IoAddIcon(IDI_APPS);
            IoAddHeaderString(0, 0, L"HKLM\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Windows");
            IoAddItem(1, IconIndex, L"AppInit_DLLs");
            IoSetItemText(szPath);
        }
    }

    DebugEndReceiving();
}

typedef struct
{
    LPWSTR lpName;
    INT FolderIndex;
} SHELL_FOLDERS;

SHELL_FOLDERS ShellFolders[] =
{
    { L"Administrative Tools",        CSIDL_ADMINTOOLS },
    { L"AppData",                     CSIDL_APPDATA },
    { L"Cache",                       CSIDL_INTERNET_CACHE },
    { L"CD Burning",                  CSIDL_CDBURN_AREA },
    { L"Common Administrative Tools", CSIDL_COMMON_ADMINTOOLS },
    { L"Common AppData",              CSIDL_COMMON_APPDATA },
    { L"Common Desktop",              CSIDL_COMMON_DESKTOPDIRECTORY },
    { L"Common Documents",            CSIDL_COMMON_DOCUMENTS },
    { L"Common Favorites",            CSIDL_COMMON_FAVORITES },
    { L"Common Files",                CSIDL_PROGRAM_FILES_COMMON },
    { L"Common Music",                CSIDL_COMMON_MUSIC },
    { L"Common Pictures",             CSIDL_COMMON_PICTURES },
    { L"Common Programs",             CSIDL_COMMON_PROGRAMS },
    { L"Common Start Menu",           CSIDL_COMMON_STARTMENU },
    { L"Common Startup",              CSIDL_COMMON_STARTUP },
    { L"Common Templates",            CSIDL_COMMON_TEMPLATES },
    { L"Common Video",                CSIDL_COMMON_VIDEO },
    { L"Cookies",                     CSIDL_COOKIES },
    { L"Desktop",                     CSIDL_DESKTOP },
    { L"Favorites",                   CSIDL_FAVORITES },
    { L"Fonts",                       CSIDL_FONTS },
    { L"History",                     CSIDL_HISTORY },
    { L"Local AppData",               CSIDL_LOCAL_APPDATA },
    { L"My Documents",                CSIDL_MYDOCUMENTS },
    { L"My Music",                    CSIDL_MYMUSIC },
    { L"My Pictures",                 CSIDL_MYPICTURES },
    { L"My Video",                    CSIDL_MYVIDEO },
    { L"NetHood",                     CSIDL_NETHOOD },
    { L"PrintHood",                   CSIDL_PRINTHOOD },
    { L"Profile",                     CSIDL_PROFILE },
    { L"Program Files",               CSIDL_PROGRAM_FILES },
    { L"Programs",                    CSIDL_PROGRAMS },
    { L"Recent",                      CSIDL_RECENT },
    { L"Resources",                   CSIDL_RESOURCES },
    { L"SendTo",                      CSIDL_SENDTO },
    { L"Start Menu",                  CSIDL_STARTMENU },
    { L"Startup",                     CSIDL_ALTSTARTUP },
    { L"System",                      CSIDL_SYSTEM },
    { L"Templates",                   CSIDL_TEMPLATES },
    { L"Windows",                     CSIDL_WINDOWS },
    { 0 }
};

static VOID
AddFolderInfoToListView(LPWSTR lpName, INT nFolder)
{
    WCHAR szPath[MAX_PATH];
    INT IconIndex = -1;
    HICON hIcon = NULL;

    if (SHGetSpecialFolderPath(DllParams.hMainWnd, szPath, nFolder, FALSE))
    {
        if (IoGetTarget() == IO_TARGET_LISTVIEW)
        {
            hIcon = GetFolderAssocIcon(szPath);
            if (hIcon != NULL)
            {
                IconIndex = ImageList_AddIcon(*DllParams.hListImgList,
                                              hIcon);
            }
            else
            {
                IconIndex = IoAddIcon(IDI_FOLDER);
            }
        }

        IoAddItem(0, IconIndex, lpName);
        IoSetItemText(szPath);

        if (IoGetTarget() == IO_TARGET_LISTVIEW)
        {
            if (hIcon != NULL) DestroyIcon(hIcon);
        }
    }
}

VOID
OS_SysFoldersInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    SIZE_T Index = 0;

    DebugStartReceiving();

    do
    {
        AddFolderInfoToListView(ShellFolders[Index].lpName,
                                ShellFolders[Index].FolderIndex);

        if (IsCanceled) break;
    }
    while (ShellFolders[++Index].lpName != NULL);

    IoAddItem(0, 0, L"Temp");
    ExpandEnvironmentStrings(L"%TEMP%", szText, MAX_STR_LEN);
    IoSetItemText(szText);

    DebugEndReceiving();
}

static VOID
EnumEnvironmentVariables(HKEY hRootKey,
                         LPWSTR lpSubKeyName)
{
    HKEY hKey = NULL;
    DWORD dwValues;
    DWORD dwMaxValueNameLength;
    DWORD dwMaxValueDataLength;
    DWORD dwIndex;
    LPWSTR lpName = NULL;
    LPWSTR lpData = NULL;
    LPWSTR lpExpandData = NULL;
    DWORD dwNameLength;
    DWORD dwDataLength;
    DWORD dwType;

    if (RegOpenKeyEx(hRootKey,
                     lpSubKeyName,
                     0,
                     KEY_READ,
                     &hKey) != ERROR_SUCCESS)
        return;

    if (RegQueryInfoKey(hKey,
                        NULL, NULL,
                        NULL, NULL,
                        NULL, NULL,
                        &dwValues,
                        &dwMaxValueNameLength,
                        &dwMaxValueDataLength,
                        NULL, NULL) != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    lpName = Alloc((dwMaxValueNameLength + 1) * sizeof(WCHAR));
    if (!lpName)
    {
        DebugAllocFailed();
        goto Cleanup;
    }

    lpData = Alloc((dwMaxValueDataLength + 1) * sizeof(WCHAR));
    if (!lpData)
    {
        DebugAllocFailed();
        goto Cleanup;
    }

    lpExpandData = Alloc(2048 * sizeof(WCHAR));
    if (!lpExpandData)
    {
        DebugAllocFailed();
        goto Cleanup;
    }

    for (dwIndex = 0; dwIndex < dwValues; ++dwIndex)
    {
        dwNameLength = dwMaxValueNameLength + 1;
        dwDataLength = dwMaxValueDataLength + 1;

        if (RegEnumValue(hKey,
                         dwIndex,
                         lpName,
                         &dwNameLength,
                         NULL,
                         &dwType,
                         (LPBYTE)lpData,
                         &dwDataLength) != ERROR_SUCCESS)
        {
            goto Cleanup;
        }

        if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
            continue;

        ExpandEnvironmentStrings(lpData, lpExpandData, 2048);

        IoAddItem(0, 0, lpName);
        IoSetItemText(lpExpandData);
    }

Cleanup:
    if (lpExpandData) Free(lpExpandData);
    if (lpName) Free(lpName);
    if (lpData) Free(lpData);
    if (hKey) RegCloseKey(hKey);
}

VOID
OS_EnvironInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];

    DebugStartReceiving();

    IoAddIcon(IDI_APPS);

    EnumEnvironmentVariables(HKEY_CURRENT_USER,
                             L"Environment");

    EnumEnvironmentVariables(HKEY_LOCAL_MACHINE,
                             L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");

    IoAddItem(0, 0, L"ALLUSERSPROFILE");
    ExpandEnvironmentStrings(L"%ALLUSERSPROFILE%", szText, MAX_STR_LEN);
    IoSetItemText(szText);

    IoAddItem(0, 0, L"APPDATA");
    ExpandEnvironmentStrings(L"%APPDATA%", szText, MAX_STR_LEN);
    IoSetItemText(szText);

    IoAddItem(0, 0, L"COMPUTERNAME");
    ExpandEnvironmentStrings(L"%COMPUTERNAME%", szText, MAX_STR_LEN);
    IoSetItemText(szText);

    IoAddItem(0, 0, L"HOMEPATH");
    ExpandEnvironmentStrings(L"%HOMEPATH%", szText, MAX_STR_LEN);
    IoSetItemText(szText);

    IoAddItem(0, 0, L"LOCALAPPDATA");
    ExpandEnvironmentStrings(L"%LOCALAPPDATA%", szText, MAX_STR_LEN);
    IoSetItemText(szText);

    DebugEndReceiving();
}

VOID
OS_DesktopInfo(VOID)
{
    WCHAR szYes[MAX_STR_LEN], szNo[MAX_STR_LEN],
          szText[MAX_STR_LEN] = {0};
    ANIMATIONINFO AnimationInfo;
    DWORD DwordParam;
    BOOL BoolParam;
    UINT UintParam;
    INT IntParam;
    DEVMODE DevMode;

    DebugStartReceiving();

    IoAddIcon(IDI_DESKTOP);
    IoAddIcon(IDI_MOUSE);
    IoAddIcon(IDI_KEYBOARD);
    IoAddIcon(IDI_EFFECTS);
    IoAddIcon(IDI_WINDOW);
    IoAddIcon(IDI_MENU);
    IoAddIcon(IDI_SCREENSAVER);

    LoadMUIString(IDS_YES, szYes, MAX_STR_LEN);
    LoadMUIString(IDS_NO, szNo, MAX_STR_LEN);

    DevMode.dmSize = sizeof(DEVMODE);
    DevMode.dmDriverExtra = 0;

    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DevMode))
        return;

    /* DESKTOP */
    IoAddHeader(0, 0, IDS_CAT_OS_DESKTOP);

    /* Resolution */
    IoAddValueName(1, 0, IDS_DESK_RESOLUTION);
    IoSetItemText(L"%ld x %ld",
                  DevMode.dmPelsWidth,
                  DevMode.dmPelsHeight);

    /* Color depth */
    IoAddValueName(1, 0, IDS_DESK_COLOR_DEPTH);
    IoSetItemText(L"%ld bit",
                  DevMode.dmBitsPerPel);

    /* DPI */
    // TODO

    /* Refresh rate */
    IoAddValueName(1, 0, IDS_DESK_REFRESH_RATE);
    IoSetItemText(L"%ld Hz",
                  DevMode.dmDisplayFrequency);

    /* Wallpaper */
    szText[0] = 0;
    if (GetStringFromRegistry(TRUE,
                              HKEY_CURRENT_USER,
                              L"Control Panel\\Desktop",
                              L"Wallpaper",
                              szText,
                              MAX_STR_LEN))
    {
        IoAddValueName(1, 0, IDS_DESK_WALLPAPER);
        IoSetItemText((szText[0] != 0) ? szText : szNo);
    }

    /* Font smoothing */
    if (SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &BoolParam, 0))
    {
        UINT Type;

        IoAddValueName(1, 0, IDS_DESK_FONT_SMOOTHING);
        IoSetItemText(BoolParam ? szYes : szNo);

        /* Smoothing type */
        if (SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &Type, 0))
        {
            UINT StringID = IDS_SMOOTHING_STANDARD;

            IoAddValueName(1, 0, IDS_DESK_FONT_SMOOTHING_TYPE);
            if (Type == FE_FONTSMOOTHINGCLEARTYPE)
                StringID = IDS_SMOOTHING_CLEARTYPE;

            LoadMUIString(StringID, szText, MAX_STR_LEN);
            IoSetItemText(szText);
        }
    }

    IoAddFooter();

    /* MOUSE */
    IoAddHeader(0, 1, IDS_DESK_MOUSE);

    /* Speed */
    if (SystemParametersInfo(SPI_GETMOUSESPEED, 0, &IntParam, 0))
    {
        IoAddValueName(1, 1, IDS_DESK_MOUSE_SPEED);
        IoSetItemText(L"%d", IntParam);
    }

    /* Trails */
    if (SystemParametersInfo(SPI_GETMOUSETRAILS, 0, &IntParam, 0))
    {
        IoAddValueName(1, 1, IDS_DESK_MOUSE_TRAILS);
        IoSetItemText(IntParam ? szYes : szNo);
    }

    /* Scroll lines */
    if (SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &UintParam, 0))
    {
        IoAddValueName(1, 1, IDS_DESK_WHELL_SCROLL_LINES);
        IoSetItemText(L"%ld", UintParam);
    }

    IoAddFooter();

    /* KEYBOARD */
    IoAddHeader(0, 2, IDS_DESK_KEYBOARD);

    /* Speed */
    if (SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &DwordParam, 0))
    {
        IoAddValueName(1, 2, IDS_DESK_KEYBOARD_SPEED);
        IoSetItemText(L"%ld", DwordParam);
    }

    /* Delay */
    if (SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &IntParam, 0))
    {
        IoAddValueName(1, 2, IDS_DESK_KEYBOARD_DELAY);
        IoSetItemText(L"%ld", IntParam);
    }

    IoAddFooter();

    /* UI EFFECTS */
    IoAddHeader(0, 3, IDS_DESK_UI_EFFECTS);

    /* Combobox animation */
    if (SystemParametersInfo(SPI_GETCOMBOBOXANIMATION, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 3, IDS_DESK_COMBOBOX_ANIMATION);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    /* Cursor shadow */
    if (SystemParametersInfo(SPI_GETCURSORSHADOW, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 3, IDS_DESK_CURSOR_SHADOW);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    /* Gradient captions */
    if (SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 3, IDS_DESK_GRADIENT_CAPTIONS);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    /* Listbox smooth scrolling */
    if (SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 3, IDS_DESK_LISTBOXSMOOTHSCROLLING);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    /* Menu animation */
    if (SystemParametersInfo(SPI_GETMENUANIMATION, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 3, IDS_DESK_MENUANIMATION);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    /* Tooltip animation */
    if (SystemParametersInfo(SPI_GETTOOLTIPANIMATION, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 3, IDS_DESK_TOOLTIPANIMATION);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    IoAddFooter();

    /* WINDOW */
    IoAddHeader(0, 4, IDS_DESK_WINDOW);

    /* Window animation */
    AnimationInfo.cbSize = sizeof(ANIMATIONINFO);
    if (SystemParametersInfo(SPI_GETANIMATION, sizeof(ANIMATIONINFO), &AnimationInfo, 0))
    {
        IoAddValueName(1, 4, IDS_DESK_WND_ANIMATION);
        IoSetItemText(AnimationInfo.iMinAnimate ? szYes : szNo);
    }

    /* Border */
    if (SystemParametersInfo(SPI_GETBORDER, 0, &IntParam, 0))
    {
        IoAddValueName(1, 4, IDS_DESK_WND_BORDER);
        IoSetItemText(L"%ld", IntParam);
    }

    /* Drag full */
    if (SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 4, IDS_DESK_WND_DRAG_FULL);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    /* Drops shadow */
    if (SystemParametersInfo(SPI_GETDROPSHADOW, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 4, IDS_DESK_DROPS_SHADOW);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    IoAddFooter();

    /* MENU */
    IoAddHeader(0, 5, IDS_DESK_MENU);

    /* Menu animation */
    if (SystemParametersInfo(SPI_GETMENUFADE, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 5, IDS_DESK_MENU_FADE_ANIMATION);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    /* Menu show delay */
    if (SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &DwordParam, 0))
    {
        IoAddValueName(1, 5, IDS_DESK_MENU_SHOW_DELAY);
        IoSetItemText(L"%ld", DwordParam);
    }

    /* Flat menu */
    if (SystemParametersInfo(SPI_GETFLATMENU, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 5, IDS_DESK_FLAT_MENU);
        IoSetItemText(BoolParam ? szYes : szNo);
    }

    IoAddFooter();

    /* SCREEN SAVER */
    IoAddHeader(0, 6, IDS_DESK_SS);

    /* Active */
    if (SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &BoolParam, 0))
    {
        IoAddValueName(1, 6, IDS_DESK_SS_ACTIVE);
        IoSetItemText(BoolParam ? szYes : szNo);

        /* Timeout */
        if (BoolParam && SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &IntParam, 0))
        {
            IoAddValueName(1, 6, IDS_DESK_SS_TIMEOUT);
            IoSetItemText(L"%ld", IntParam);
        }
    }

    DebugEndReceiving();
}

VOID
OS_TaskSchedulerInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szFormat[MAX_STR_LEN];
    WORD wTriggerCount, wTriggerIndex;
    SIZE_T dwFetchedTasks = 0;
    IEnumWorkItems *pIEnum;
    ITaskScheduler *pITS;
    HRESULT Status, hr;
    SYSTEMTIME stTime;
    LPWSTR *lpszNames;
    LPWSTR lpszText;
    ITask *pITask;

    DebugStartReceiving();

    IoAddIcon(IDI_TIME);

    CoInitialize(NULL);

    hr = CoCreateInstance(&CLSID_CTaskScheduler,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_ITaskScheduler,
                          (LPVOID*)&pITS);
    if (FAILED(hr))
    {
        CoUninitialize();
        return;
    }

    hr = pITS->lpVtbl->Enum(pITS, &pIEnum);
    if (FAILED(hr))
    {
        pITS->lpVtbl->Release(pITS);
        CoUninitialize();
        return;
    }

    while (SUCCEEDED(pIEnum->lpVtbl->Next(pIEnum,
                                          5,
                                          &lpszNames,
                                          &dwFetchedTasks))
                                          && (dwFetchedTasks != 0))
    {
        while (dwFetchedTasks)
        {
            /* Task name */
            IoAddHeaderString(0, 0, lpszNames[--dwFetchedTasks]);

            hr = pITS->lpVtbl->Activate(pITS,
                                        lpszNames[dwFetchedTasks],
                                        &IID_ITask,
                                        (IUnknown**)&pITask);
            if (FAILED(hr))
            {
                CoTaskMemFree(lpszNames[dwFetchedTasks]);
                continue;
            }

            /* Status */
            hr = pITask->lpVtbl->GetStatus(pITask, &Status);
            if (SUCCEEDED(hr))
            {
                UINT StatusID;

                IoAddValueName(1, 0, IDS_STATUS);
                switch (Status)
                {
                    case SCHED_S_TASK_READY:
                        StatusID = IDS_TASK_READY;
                        break;
                    case SCHED_S_TASK_RUNNING:
                        StatusID = IDS_TASK_RUNNING;
                        break;
                    case SCHED_S_TASK_DISABLED:
                        StatusID = IDS_TASK_DISABLED;
                        break;
                    case SCHED_S_TASK_HAS_NOT_RUN:
                        StatusID = IDS_TASK_HAS_NOT_RUN;
                        break;
                    case SCHED_S_TASK_NOT_SCHEDULED:
                        StatusID = IDS_TASK_NOT_SCHEDULED;
                        break;
                    case SCHED_S_TASK_NO_MORE_RUNS:
                        StatusID = IDS_TASK_NO_MORE_RUNS;
                        break;
                    case SCHED_S_TASK_NO_VALID_TRIGGERS:
                        StatusID = IDS_TASK_NO_VALID_TRIGGERS;
                        break;
                    default:
                        StatusID = IDS_NONE;
                        break;
                }
                LoadMUIString(StatusID, szText, MAX_STR_LEN);
                IoSetItemText(szText);
            }

            /* Application name */
            hr = pITask->lpVtbl->GetApplicationName(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                IoAddValueName(1, 0, IDS_TASK_APP_NAME);
                IoSetItemText((SafeStrLen(lpszText) > 0) ? lpszText : L"-");
                CoTaskMemFree(lpszText);
            }

            /* Application parameters */
            hr = pITask->lpVtbl->GetParameters(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                IoAddValueName(1, 0, IDS_TASK_APP_PARAMS);
                IoSetItemText((SafeStrLen(lpszText) > 0) ? lpszText : L"-");
                CoTaskMemFree(lpszText);
            }

            /* Working directory */
            hr = pITask->lpVtbl->GetWorkingDirectory(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                IoAddValueName(1, 0, IDS_TASK_WORK_DIR);
                IoSetItemText((SafeStrLen(lpszText) > 0) ? lpszText : L"-");
                CoTaskMemFree(lpszText);
            }

            /* Comment */
            hr = pITask->lpVtbl->GetComment(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                IoAddValueName(1, 0, IDS_TASK_COMMENT);
                IoSetItemText((SafeStrLen(lpszText) > 0) ? lpszText : L"-");
                CoTaskMemFree(lpszText);
            }

            /* Account information */
            hr = pITask->lpVtbl->GetAccountInformation(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                IoAddValueName(1, 0, IDS_TASK_ACCAUNT);
                IoSetItemText((SafeStrLen(lpszText) > 0) ? lpszText : L"-");
                CoTaskMemFree(lpszText);
            }

            /* Creator */
            hr = pITask->lpVtbl->GetCreator(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                IoAddValueName(1, 0, IDS_TASK_CREATOR);
                IoSetItemText((SafeStrLen(lpszText) > 0) ? lpszText : L"-");
                CoTaskMemFree(lpszText);
            }

            /* Last run */
            hr = pITask->lpVtbl->GetMostRecentRunTime(pITask, &stTime);
            if (SUCCEEDED(hr))
            {
                IoAddValueName(1, 0, IDS_TASK_LAST_RUN_TIME);
                IoSetItemText(L"%02u.%02u.%u %u:%02u",
                              stTime.wDay,
                              stTime.wMonth,
                              stTime.wYear,
                              stTime.wHour,
                              stTime.wMinute);
            }

            /* Next run */
            hr = pITask->lpVtbl->GetNextRunTime(pITask, &stTime);
            if (SUCCEEDED(hr))
            {
                IoAddValueName(1, 0, IDS_TASK_NEXT_RUN_TIME);
                IoSetItemText(L"%02u.%02u.%u %u:%02u",
                              stTime.wDay,
                              stTime.wMonth,
                              stTime.wYear,
                              stTime.wHour,
                              stTime.wMinute);
            }

            /* Triggrs */
            hr = pITask->lpVtbl->GetTriggerCount(pITask, &wTriggerCount);
            if (SUCCEEDED(hr) && wTriggerCount > 0)
            {
                for (wTriggerIndex = 0; wTriggerIndex < wTriggerCount; ++wTriggerIndex)
                {
                    hr = pITask->lpVtbl->GetTriggerString(pITask,
                                                          wTriggerIndex,
                                                          &lpszText); 
                    if (FAILED(hr)) continue;

                    LoadMUIString(IDS_TASK_TRIGGER_FORMAT,
                                  szFormat, MAX_STR_LEN);
                    IoAddItem(1, 0, szFormat, wTriggerIndex + 1);
                    IoSetItemText((SafeStrLen(lpszText) > 0) ? lpszText : L"-");
                    CoTaskMemFree(lpszText);
                }
            }

            /* Free task name var */
            CoTaskMemFree(lpszNames[dwFetchedTasks]);
            pITask->lpVtbl->Release(pITask);

            IoAddFooter();
        }
        CoTaskMemFree(lpszNames);
    }

    pITS->lpVtbl->Release(pITS);
    pIEnum->lpVtbl->Release(pIEnum);

    CoUninitialize();

    DebugEndReceiving();
}

VOID
AddPreventItem(UINT ValueName, HKEY hRootKey, LPWSTR lpPath, LPWSTR lpKeyName)
{
    WCHAR szText[MAX_STR_LEN];
    BOOL IsAllowed;
    DWORD dwValue;

    if (GetBinaryFromRegistry(hRootKey, lpPath, lpKeyName,
                              (LPBYTE)&dwValue, sizeof(DWORD)))
    {
        IsAllowed = (dwValue == 0) ? TRUE : FALSE;
    }
    else
    {
        IsAllowed = TRUE;
    }

    LoadMUIString(IsAllowed ? IDS_PREV_ALLOWED : IDS_PREV_NOT_ALLOWED,
                  szText, MAX_STR_LEN);

    IoAddValueName(1, IsAllowed ? 0 : 1, ValueName);
    IoSetItemText(szText);
}

VOID
OS_PreventsInfo(VOID)
{
    LPWSTR lpExplorer = L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer";
    LPWSTR lpSystem = L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System";

    DebugStartReceiving();

    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);
    IoAddIcon(IDI_COMPUTER);
    IoAddIcon(IDI_FOLDER);
    IoAddIcon(IDI_REG);
    IoAddIcon(IDI_PREVENT);

    IoAddHeader(0, 2, IDS_PREV_TOOLS);

    AddPreventItem(IDS_PREV_TASKMGR, HKEY_CURRENT_USER,
                   lpSystem,
                   L"DisableTaskMgr");

    AddPreventItem(IDS_PREV_REGEDIT, HKEY_CURRENT_USER,
                   lpSystem,
                   L"DisableRegistryTools");

    AddPreventItem(IDS_PREV_DISABLE_CMD, HKEY_CURRENT_USER,
                   lpSystem,
                   L"DisableCMD");

    IoAddFooter();

    IoAddHeader(0, 3, IDS_PREV_EXPLORER);

    AddPreventItem(IDS_PREV_DESKTOP_ITEMS, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoDesktop");
    AddPreventItem(IDS_PREV_EXPLORER_CONTEXT_MENU, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoViewContextMenu");
    AddPreventItem(IDS_PREV_TASKBAR_CONTEXT_MENU, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoTrayContextMenu");
    AddPreventItem(IDS_PREV_HIDE_TRAY, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoTrayItemsDisplay");
    AddPreventItem(IDS_PREV_TOOLBARS_ON_TASKBAR, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoToolBarsOnTaskbar");
    AddPreventItem(IDS_PREV_HIDE_CLOCK, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"HideClock");
    AddPreventItem(IDS_PREV_RECENT_DOCS, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoRecentDocsMenu");
    AddPreventItem(IDS_PREV_FAVORITES, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoFavoritiesMenu");
    AddPreventItem(IDS_PREV_FIND_COMMAND, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoFind");
    AddPreventItem(IDS_PREV_RUN_COMMAND, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoRun");
    AddPreventItem(IDS_PREV_LOG_OFF, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoLogOff");
    AddPreventItem(IDS_PREV_SHUTDOWN, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"NoClose");

    IoAddFooter();

    IoAddHeader(0, 4, IDS_PREV_AUTORUN);

    AddPreventItem(IDS_PREV_HKLM_RUN, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"DisableLocalMachineRun");
    AddPreventItem(IDS_PREV_HKLM_RUNONCE, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"DisableLocalMachineRunOnce");
    AddPreventItem(IDS_PREV_HKCU_RUN, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"DisableCurrentUserRun");
    AddPreventItem(IDS_PREV_HKCU_RUNONCE, HKEY_CURRENT_USER,
                   lpExplorer,
                   L"DisableCurrentUserRunOnce");

    IoAddFooter();

    IoAddHeader(0, 5, IDS_PREV_OTHER);

    AddPreventItem(IDS_PREV_DISABLE_GPO, HKEY_LOCAL_MACHINE,
                   lpSystem,
                   L"DisableGPO");

    DebugEndReceiving();
}

BOOL
IsSignedFile(LPWSTR lpFilePath)
{
    WINTRUST_FILE_INFO TrustFileInfo = { 0 };
    WINTRUST_DATA TrustData          = { 0 };
    GUID guid = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    TrustFileInfo.cbStruct       = sizeof(WINTRUST_FILE_INFO);
    TrustFileInfo.pcwszFilePath  = lpFilePath;

    TrustData.cbStruct            = sizeof(WINTRUST_DATA);
    TrustData.dwUIChoice          = WTD_UI_NONE;
    TrustData.dwUnionChoice       = WTD_CHOICE_FILE;
    TrustData.pFile               = &TrustFileInfo;

    return (WinVerifyTrust(NULL, &guid, &TrustData) == ERROR_SUCCESS) ? TRUE : FALSE;
}

VOID
FindSysFiles(LPWSTR lpDir, LPWSTR lpExt)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WCHAR szPath[MAX_PATH], szText[MAX_STR_LEN],
          szFilePath[MAX_PATH], szYes[MAX_STR_LEN],
          szNo[MAX_STR_LEN];
    WIN32_FIND_DATA FindFileData;
    DWORD dwSize, dwHandle;
    LPVOID pData, pResult;

    struct LANGANDCODEPAGE
    {
        WORD wLanguage;
        WORD wCodePage;
    } *lpTranslate;

    DebugStartReceiving();

    LoadMUIString(IDS_YES, szYes, MAX_STR_LEN);
    LoadMUIString(IDS_NO, szNo, MAX_STR_LEN);

    StringCbPrintf(szPath, sizeof(szPath), L"%s\\%s", lpDir, lpExt);

    hFind = FindFirstFile(szPath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        /* File name */
        IoAddItem(0, 0, FindFileData.cFileName);

        /* File size */
        IoSetItemText(L"%ld KB",
                      ((FindFileData.nFileSizeHigh * ((DWORDLONG)MAXDWORD + 1)) +
                      FindFileData.nFileSizeLow) / 1024);

        StringCbPrintf(szFilePath, sizeof(szFilePath),
                       L"%s\\%s", lpDir, FindFileData.cFileName);

        IoSetItemText((IsSignedFile(szFilePath) == TRUE) ? szYes : szNo);

        dwSize = GetFileVersionInfoSize(szFilePath, &dwHandle);

        pData = Alloc(dwSize);
        if (!pData)
        {
            IoSetItemText(L"-");
            IoSetItemText(L"-");
            IoSetItemText(L"-");
            continue;
        }

        if (GetFileVersionInfo(szFilePath, dwHandle, dwSize, pData))
        {
            if (VerQueryValue(pData,
                              L"\\VarFileInfo\\Translation",
                              (LPVOID*)&lpTranslate,
                              (PUINT)&dwSize))
            {
                if (lpTranslate)
                {
                    StringCbPrintf(szText, sizeof(szText),
                                   L"\\StringFileInfo\\%04x%04x\\FileVersion",
                                   lpTranslate->wLanguage,
                                   lpTranslate->wCodePage);

                    if (VerQueryValue(pData, szText, (LPVOID*)&pResult, (PUINT)&dwSize))
                    {
                        /* File version */
                        IoSetItemText((SafeStrLen(pResult) > 0) ? pResult : L"-");
                    }
                    else
                    {
                        IoSetItemText(L"-");
                    }

                    StringCbPrintf(szText, sizeof(szText),
                                   L"\\StringFileInfo\\%04x%04x\\CompanyName",
                                   lpTranslate->wLanguage,
                                   lpTranslate->wCodePage);

                    if (VerQueryValue(pData, szText, (LPVOID*)&pResult, (PUINT)&dwSize))
                    {
                        /* Company Name */
                        IoSetItemText((SafeStrLen(pResult) > 0) ? pResult : L"-");
                    }
                    else
                    {
                        IoSetItemText(L"-");
                    }

                    StringCbPrintf(szText, sizeof(szText),
                                   L"\\StringFileInfo\\%04x%04x\\FileDescription",
                                   lpTranslate->wLanguage,
                                   lpTranslate->wCodePage);

                    if (VerQueryValue(pData, szText, (LPVOID*)&pResult, (PUINT)&dwSize))
                    {
                        /* File description */
                        IoSetItemText((SafeStrLen(pResult) > 0) ? pResult : L"-");
                    }
                    else
                    {
                        IoSetItemText(L"-");
                    }
                }
                else
                {
                    IoSetItemText(L"-");
                    IoSetItemText(L"-");
                    IoSetItemText(L"-");
                }
            }
            else
            {
                IoSetItemText(L"-");
                IoSetItemText(L"-");
                IoSetItemText(L"-");
            }
        }
        else
        {
            IoSetItemText(L"-");
            IoSetItemText(L"-");
            IoSetItemText(L"-");
        }

        Free(pData);

        if (IsCanceled) break;
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);
}

VOID
OS_SysFilesDLLInfo(VOID)
{
    WCHAR szPath[MAX_PATH];

    DebugStartReceiving();

    IoAddIcon(IDI_APPS);

    GetSystemDirectory(szPath, MAX_PATH);
    FindSysFiles(szPath, L"*.dll");

    DebugEndReceiving();
}

VOID
OS_SysFilesAXInfo(VOID)
{
    WCHAR szPath[MAX_PATH];

    DebugStartReceiving();

    IoAddIcon(IDI_APPS);

    GetSystemDirectory(szPath, MAX_PATH);
    FindSysFiles(szPath, L"*.ax");

    DebugEndReceiving();
}

VOID
OS_SysFilesEXEInfo(VOID)
{
    WCHAR szPath[MAX_PATH];

    DebugStartReceiving();

    IoAddIcon(IDI_APPS);

    GetSystemDirectory(szPath, MAX_PATH);
    FindSysFiles(szPath, L"*.exe");

    DebugEndReceiving();
}

VOID
OS_SysFilesSYSInfo(VOID)
{
    WCHAR szPath[MAX_PATH];

    DebugStartReceiving();

    IoAddIcon(IDI_APPS);

    GetSystemDirectory(szPath, MAX_PATH);
    StringCbCat(szPath, sizeof(szPath), L"\\drivers");
    FindSysFiles(szPath, L"*.sys");

    DebugEndReceiving();
}

VOID
OS_SysFilesKnownInfo(VOID)
{
    WCHAR szValueName[MAX_PATH], szValue[MAX_PATH];
    DWORD dwSize, dwValueSize, dwIndex = 0;
    HKEY hKey;

    DebugStartReceiving();

    IoAddIcon(IDI_APPS);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDlls",
                     0,
                     KEY_READ,
                     &hKey) != ERROR_SUCCESS)
    {
        return;
    }


    dwValueSize = MAX_PATH;
    dwSize = MAX_PATH;

    while (RegEnumValue(hKey,
                        dwIndex,
                        szValueName,
                        &dwSize,
                        0, NULL,
                        (LPBYTE)szValue,
                        &dwValueSize) == ERROR_SUCCESS)
    {
        IoAddItem(0, 0, szValueName);
        IoSetItemText(L"%s", szValue);

        dwValueSize = MAX_PATH;
        dwSize = MAX_PATH;
        ++dwIndex;
    }

    RegCloseKey(hKey);

    DebugEndReceiving();
}
