/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/os.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"

#include <initguid.h>
#include <mstask.h>
#include <ntsecapi.h>
#include <wtsapi32.h>

/* Definition for the GetFontResourceInfo function */
typedef BOOL (WINAPI *PGFRI)(LPCTSTR, DWORD *, LPVOID, DWORD);


VOID
ShowRegInfo(UINT StringID, LPWSTR lpszKeyName)
{
    WCHAR szText[MAX_STR_LEN] = {0};
    INT Index;

    GetStringFromRegistry(HKEY_LOCAL_MACHINE,
                          L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                          lpszKeyName,
                          szText,
                          MAX_STR_LEN);

    if (SafeStrLen(szText) > 0)
    {
        Index = IoAddValueName(StringID, -1);
        IoSetItemText(Index, 1, szText);
    }
}

VOID
ShowInstallDate(VOID)
{
    DWORD dwInstallDate;
    INT Index;

    if (GetBinaryFromRegistry(HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion",
                              L"InstallDate",
                              (LPBYTE)&dwInstallDate,
                              sizeof(dwInstallDate)))
    {
        WCHAR *time = TimeToString((time_t)dwInstallDate);

        if (time)
        {
            Index = IoAddValueName(IDS_OS_INSTALL_DATE, -1);
            IoSetItemText(Index, 1, time);
        }
    }
}

VOID
OS_RegInformation(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_COMPUTER);
    IoAddHeader(IDS_CAT_OS_REGDATA, 0);

    /* Product Key */
    Index = IoAddValueName(IDS_PRODUCT_KEY, -1);
    GetMSProductKey(FALSE,
                    "SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion",
                    szText, MAX_STR_LEN);
    IoSetItemText(Index, 1, szText);

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
    INT Index;

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
                Index = IoAddItem((pBuffer[i].usri3_flags & UF_ACCOUNTDISABLE) ? 1 : 0,
                                  pBuffer[i].usri3_name);

                IoSetItemText(Index, 1, pBuffer[i].usri3_full_name);
                IoSetItemText(Index, 2, pBuffer[i].usri3_comment);

                IoSetItemText(Index, 3, (pBuffer[i].usri3_flags & UF_ACCOUNTDISABLE) ? szYes : szNo);
                IoSetItemText(Index, 4, (pBuffer[i].usri3_flags & UF_PASSWD_CANT_CHANGE) ? szYes : szNo);
                IoSetItemText(Index, 5, (pBuffer[i].usri3_flags & UF_PASSWORD_EXPIRED) ? szYes : szNo);
                IoSetItemText(Index, 6, (pBuffer[i].usri3_flags & UF_DONT_EXPIRE_PASSWD) ? szYes : szNo);
                IoSetItemText(Index, 7, (pBuffer[i].usri3_flags & UF_LOCKOUT) ? szYes : szNo);

                if (pBuffer[i].usri3_last_logon == 0)
                {
                    LoadMUIString(IDS_NEVER, szText, MAX_STR_LEN);
                }
                else
                {
                    StringCbCopy(szText, sizeof(szText),
                                 TimeToString(pBuffer[i].usri3_last_logon));
                }
                IoSetItemText(Index, 8, szText);

                StringCbPrintf(szText, sizeof(szText),
                               L"%ld", pBuffer[i].usri3_num_logons);
                IoSetItemText(Index, 9, szText);

                StringCbPrintf(szText, sizeof(szText),
                               L"%ld", pBuffer[i].usri3_bad_pw_count);
                IoSetItemText(Index, 10, szText);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                NetApiBufferFree(&pBuffer);
                continue;
            }
        }

        NetApiBufferFree(&pBuffer);

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
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_USERS);

    for (;;)
    {
        netStatus = NetLocalGroupEnum(NULL, 1, (LPBYTE*)&pBuffer,
                                      1024, &entriesread,
                                      &totalentries, &resume_handle);
        if (netStatus != NERR_Success && netStatus != ERROR_MORE_DATA)
            break;

        for (i = 0; i < entriesread; ++i)
        {
            __try
            {
                Index = IoAddItem(0, pBuffer[i].lgrpi1_name);
                IoSetItemText(Index, 1, pBuffer[i].lgrpi1_comment);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                NetApiBufferFree(&pBuffer);
                continue;
            }
        }

        NetApiBufferFree(&pBuffer);

        /* No more data left */
        if (netStatus != ERROR_MORE_DATA)
            break;
    }

    DebugEndReceiving();
}

VOID
OS_ActiveUsersInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    SIZE_T Count, Index;
    PWTS_SESSION_INFO Sessions;
    INT ItemIndex;
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
        ItemIndex = IoAddItem(0, lpUserName);

        /* Domain */
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                       Sessions[Index].SessionId,
                                       WTSDomainName,
                                       &lpDomain,
                                       &dwSize) &&
            (SafeStrLen(lpDomain) > 0))
        {
            IoSetItemText(ItemIndex, 1, lpDomain);
        }
        else
        {
            IoSetItemText(ItemIndex, 1, 0);
        }

        /* User ID */
        StringCbPrintf(szText, sizeof(szText), L"%ld", Sessions[Index].SessionId);
        IoSetItemText(ItemIndex, 2, szText);

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
            IoSetItemText(ItemIndex, 3, lpState);
        }
        else
        {
            IoSetItemText(ItemIndex, 3, 0);
        }

        /* Client Name */
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                       Sessions[Index].SessionId,
                                       WTSClientName,
                                       &lpClientName,
                                       &dwSize) &&
            (SafeStrLen(lpClientName) > 0))
        {
            IoSetItemText(ItemIndex, 4, lpClientName);
        }
        else
        {
            IoSetItemText(ItemIndex, 4, 0);
        }

        /* WinStationName */
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
                                       Sessions[Index].SessionId,
                                       WTSWinStationName,
                                       &lpWinStationName,
                                       &dwSize) &&
            (SafeStrLen(lpWinStationName) > 0))
        {
            IoSetItemText(ItemIndex, 5, lpWinStationName);
        }
        else
        {
            IoSetItemText(ItemIndex, 5, 0);
        }
    }

    DebugEndReceiving();
}

VOID
OS_FontsInfo(VOID)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WCHAR szText[MAX_STR_LEN], szPath[MAX_PATH],
          szSize[MAX_STR_LEN];
    WIN32_FIND_DATA FindFileData;
    PGFRI GetFontResourceInfo;
    HINSTANCE hDLL;
    DWORD dwSize;
    INT Index;

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

    SHGetSpecialFolderPath(hMainWnd, szPath, CSIDL_FONTS, FALSE);
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

        Index = IoAddItem(0, szText);
        IoSetItemText(Index, 1, FindFileData.cFileName);

        StringCbPrintf(szSize, sizeof(szSize), L"%ld KB",
                       ((FindFileData.nFileSizeHigh * ((DWORDLONG)MAXDWORD + 1)) +
                       FindFileData.nFileSizeLow) / 1024);
        IoSetItemText(Index, 2, szSize);

        RemoveFontResource(FindFileData.cFileName);
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);
    FreeLibrary(hDLL);

    DebugEndReceiving();
}

VOID
AutorunShowRegPath(HKEY hRootKey, LPWSTR lpszPath)
{
    WCHAR szName[MAX_PATH], szPath[MAX_PATH];
    DWORD dwIndex, dwSize, dwPathSize;
    INT Index, Count = 0;
    HKEY hKey;

    if (RegOpenKeyEx(hRootKey,
                     lpszPath,
                     0,
                     KEY_QUERY_VALUE,
                     &hKey) != ERROR_SUCCESS)
    {
        goto None;
    }

    for (dwIndex = 0; ; ++dwIndex)
    {
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

        if (SafeStrLen(szName) >= 1)
        {
            Index = IoAddItem(-1, szName);
            IoSetItemText(Index, 1, szPath);
            ++Count;
        }
    }

None:
    if (!Count)
    {
        LoadMUIString(IDS_NONE, szPath, MAX_PATH);
        Index = IoAddItem(-1, szPath);
        IoSetItemText(Index, 1, L"\0");
    }

    RegCloseKey(hKey);
}

BOOL
GetShortcutCommandLine(LPWSTR lpszLnkPath, LPWSTR lpszPath, SIZE_T Size)
{
    WCHAR szPath[MAX_PATH], szCmd[MAX_PATH];
    IShellLink *pShellLink;
    IPersistFile *pPersistFile;
    HRESULT hr;

    CoInitialize(NULL);

    hr = CoCreateInstance(&CLSID_ShellLink,
                          NULL,
                          CLSCTX_ALL,
                          &IID_IShellLink,
                          (LPVOID*)&pShellLink);
    if (FAILED(hr))
    {
        CoUninitialize();
        return FALSE;
    }

    hr = pShellLink->lpVtbl->QueryInterface(pShellLink,
                                            &IID_IPersistFile,
                                            (LPVOID*)&pPersistFile);
    if (FAILED(hr))
    {
        pShellLink->lpVtbl->Release(pShellLink);
        CoUninitialize();
        return FALSE;
    }

    hr = pPersistFile->lpVtbl->Load(pPersistFile,
                                    (LPCOLESTR)lpszLnkPath,
                                    STGM_READ);
    if (FAILED(hr))
    {
        pShellLink->lpVtbl->Release(pShellLink);
        CoUninitialize();
        return FALSE;
    }

    hr = pShellLink->lpVtbl->GetPath(pShellLink,
                                     szPath,
                                     MAX_PATH,
                                     NULL, 0);
    if (FAILED(hr))
    {
        pShellLink->lpVtbl->Release(pShellLink);
        CoUninitialize();
        return FALSE;
    }

    hr = pShellLink->lpVtbl->GetArguments(pShellLink,
                                          szCmd,
                                          MAX_PATH);
    if (!FAILED(hr))
    {
        StringCbCat(szPath, sizeof(szPath), L" ");
        StringCbCat(szPath, sizeof(szPath), szCmd);
    }

    StringCbCopy(lpszPath, Size, szPath);
    pShellLink->lpVtbl->Release(pShellLink);

    CoUninitialize();

    return TRUE;
}

VOID
AutorunShowFolderContent(LPWSTR lpszPath)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WCHAR szPath[MAX_PATH], szCmd[MAX_PATH], szFilePath[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    INT Index, Count = 0;

    StringCbCopy(szPath, sizeof(szPath), lpszPath);
    StringCbCat(szPath, sizeof(szPath), L"\\*.*");

    hFind = FindFirstFile(szPath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            /* HACK */
            if (wcscmp(FindFileData.cFileName, L"desktop.ini") == 0)
                continue;

            Index = IoAddItem(-1, FindFileData.cFileName);

            StringCbCopy(szFilePath, sizeof(szFilePath), lpszPath);
            StringCbCat(szFilePath, sizeof(szFilePath), L"\\");
            StringCbCat(szFilePath, sizeof(szFilePath), FindFileData.cFileName);

            GetShortcutCommandLine(szFilePath, szCmd, sizeof(szCmd));
            IoSetItemText(Index, 1, szCmd);
            ++Count;
        }
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    if (!Count)
    {
        LoadMUIString(IDS_NONE, szPath, MAX_PATH);
        Index = IoAddItem(-1, szPath);
        IoSetItemText(Index, 1, L"\0");
    }

    FindClose(hFind);
}

VOID
OS_AutorunInfo(VOID)
{
    WCHAR szPath[MAX_PATH];
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_REG);
    IoAddIcon(IDI_FOLDER);

    /* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run */
    IoAddHeaderString(L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0);
    AutorunShowRegPath(HKEY_LOCAL_MACHINE,
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");

    IoAddFooter();

    /* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce */
    IoAddHeaderString(L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", 0);
    AutorunShowRegPath(HKEY_LOCAL_MACHINE,
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");

    IoAddFooter();

    /* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnceEx */
    IoAddHeaderString(L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx", 0);
    AutorunShowRegPath(HKEY_LOCAL_MACHINE,
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");

    IoAddFooter();

    /* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunServices */
    IoAddHeaderString(L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices", 0);
    AutorunShowRegPath(HKEY_LOCAL_MACHINE,
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServices");

    IoAddFooter();

    /* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunServicesOnce */
    IoAddHeaderString(L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce", 0);
    AutorunShowRegPath(HKEY_LOCAL_MACHINE,
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce");

    IoAddFooter();

    /* HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run */
    IoAddHeaderString(L"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0);
    AutorunShowRegPath(HKEY_CURRENT_USER,
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");

    IoAddFooter();

    /* HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce */
    IoAddHeaderString(L"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", 0);
    AutorunShowRegPath(HKEY_CURRENT_USER,
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");

    IoAddFooter();

    /* Autorun folder for all users */
    SHGetSpecialFolderPath(hMainWnd, szPath, CSIDL_COMMON_STARTUP, FALSE);
    IoAddHeaderString(szPath, 1);
    AutorunShowFolderContent(szPath);

    IoAddFooter();

    /* Autorun folder for current user */
    SHGetSpecialFolderPath(hMainWnd, szPath, CSIDL_STARTUP, FALSE);
    IoAddHeaderString(szPath, 1);
    AutorunShowFolderContent(szPath);

    IoAddFooter();

    /* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Polices\Explorer\Run */
    IoAddHeaderString(L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run", 0);
    AutorunShowRegPath(HKEY_LOCAL_MACHINE,
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run");

    IoAddFooter();

    /* HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Polices\Explorer\Run */
    IoAddHeaderString(L"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run", 0);
    AutorunShowRegPath(HKEY_CURRENT_USER,
                       L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Polices\\Explorer\\Run");

    IoAddFooter();

    /* HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon */
    IoAddHeaderString(L"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 0);

    /* winlogon shell for all users */
    Index = IoAddItem(-1, L"Shell");
    GetStringFromRegistry(HKEY_LOCAL_MACHINE,
                          L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                          L"Shell",
                          szPath,
                          MAX_PATH);
    IoSetItemText(Index, 1, szPath);

    /* winlogon userinit for all users */
    Index = IoAddItem(-1, L"Userinit");
    GetStringFromRegistry(HKEY_LOCAL_MACHINE,
                          L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                          L"Userinit",
                          szPath,
                          MAX_PATH);
    IoSetItemText(Index, 1, szPath);

    IoAddFooter();

    /* HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon */
    IoAddHeaderString(L"HKCU\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 0);

    /* winlogon shell for current user */
    Index = IoAddItem(-1, L"Shell");
    if (GetStringFromRegistry(HKEY_CURRENT_USER,
                              L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                              L"Shell",
                              szPath,
                              MAX_PATH))
    {
        IoSetItemText(Index, 1, szPath);
    }
    else
    {
        LoadMUIString(IDS_NONE, szPath, MAX_PATH);
        IoSetItemText(Index, 1, szPath);
    }

    DebugEndReceiving();
}

typedef struct
{
    UINT NameIndex;
    INT FolderIndex;
} SHELL_FOLDERS;

SHELL_FOLDERS ShellFolders[] =
{
    { IDS_DIR_ADMIN_TOOLS,        CSIDL_ADMINTOOLS },
    { IDS_DIR_APPDATA,            CSIDL_APPDATA },
    { IDS_DIR_CACHE,              CSIDL_INTERNET_CACHE },
    { IDS_DIR_CD_BURNING,         CSIDL_CDBURN_AREA },
    { IDS_DIR_COMMON_ADMIN_TOOLS, CSIDL_COMMON_ADMINTOOLS },
    { IDS_DIR_COMMON_APPDATA,     CSIDL_COMMON_APPDATA },
    { IDS_DIR_COMMON_DESKTOP,     CSIDL_COMMON_DESKTOPDIRECTORY },
    { IDS_DIR_COMMON_DOCUMENTS,   CSIDL_COMMON_DOCUMENTS },
    { IDS_DIR_COMMON_FAVORITES,   CSIDL_COMMON_FAVORITES },
    { IDS_DIR_COMMON_FILES,       CSIDL_PROGRAM_FILES_COMMON },
    { IDS_DIR_COMMON_MUSIC,       CSIDL_COMMON_MUSIC },
    { IDS_DIR_COMMON_PICTURES,    CSIDL_COMMON_PICTURES },
    { IDS_DIR_COMMON_PROGRAMS,    CSIDL_COMMON_PROGRAMS },
    { IDS_DIR_COMMON_STARTMENU,   CSIDL_COMMON_STARTMENU },
    { IDS_DIR_COMMON_STARTUP,     CSIDL_COMMON_STARTUP },
    { IDS_DIR_COMMON_TEMPLATES,   CSIDL_COMMON_TEMPLATES },
    { IDS_DIR_COMMON_VIDEO,       CSIDL_COMMON_VIDEO },
    { IDS_DIR_COOKIES,            CSIDL_COOKIES },
    { IDS_DIR_DESKTOP,            CSIDL_DESKTOP },
    { IDS_DIR_FAVORITES,          CSIDL_FAVORITES },
    { IDS_DIR_FONTS,              CSIDL_FONTS },
    { IDS_DIR_HISTORY,            CSIDL_HISTORY },
    { IDS_DIR_LOCAL_APPDATA,      CSIDL_LOCAL_APPDATA },
    { IDS_DIR_MY_DOCS,            CSIDL_MYDOCUMENTS },
    { IDS_DIR_MY_MUSIC,           CSIDL_MYMUSIC },
    { IDS_DIR_MY_PICTURES,        CSIDL_MYPICTURES },
    { IDS_DIR_MY_VIDEO,           CSIDL_MYVIDEO },
    { IDS_DIR_NETHOOD,            CSIDL_NETHOOD },
    { IDS_DIR_PRINTHOOD,          CSIDL_PRINTHOOD },
    { IDS_DIR_PROFILE,            CSIDL_PROFILE },
    { IDS_DIR_PROGRAMFILES,       CSIDL_PROGRAM_FILES },
    { IDS_DIR_PROGRAMS,           CSIDL_PROGRAMS },
    { IDS_DIR_RECENT,             CSIDL_RECENT },
    { IDS_DIR_RESOURCES,          CSIDL_RESOURCES },
    { IDS_DIR_SENDTO,             CSIDL_SENDTO },
    { IDS_DIR_STARTMENU,          CSIDL_STARTMENU },
    { IDS_DIR_STARTUP,            CSIDL_ALTSTARTUP },
    { IDS_DIR_SYSTEM,             CSIDL_SYSTEM },
    { IDS_DIR_TEMPLATES,          CSIDL_TEMPLATES },
    { IDS_DIR_WINDOWS,            CSIDL_WINDOWS },
    { 0 }
};

static VOID
AddFolderInfoToListView(UINT uiNameID, INT nFolder)
{
    WCHAR szPath[MAX_PATH];
    INT Index, IconIndex = -1;
    HICON hIcon = NULL;

    if (SHGetSpecialFolderPath(hMainWnd, szPath, nFolder, FALSE))
    {
        if (IoGetTarget() == IO_TARGET_LISTVIEW)
        {
            hIcon = GetFolderAssocIcon(szPath);
            IconIndex = ImageList_AddIcon(hListViewImageList,
                                          hIcon);
        }

        Index = IoAddValueName(uiNameID, IconIndex);
        IoSetItemText(Index, 1, szPath);

        if (IoGetTarget() == IO_TARGET_LISTVIEW)
        {
            DestroyIcon(hIcon);
        }
    }
}

VOID
OS_SysFoldersInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    INT ItemIndex;
    SIZE_T Index = 0;

    DebugStartReceiving();

    do
    {
        AddFolderInfoToListView(ShellFolders[Index].NameIndex,
                                ShellFolders[Index].FolderIndex);
    }
    while (ShellFolders[++Index].NameIndex != 0);

    ItemIndex = IoAddValueName(IDS_DIR_TEMP, 0);
    ExpandEnvironmentStrings(L"%TEMP%", szText, MAX_STR_LEN);
    IoSetItemText(ItemIndex, 1, szText);

    DebugEndReceiving();
}

static VOID
EnumEnvironmentVariables(HKEY hRootKey,
                         LPWSTR lpSubKeyName)
{
    HKEY hKey;
    DWORD dwValues;
    DWORD dwMaxValueNameLength;
    DWORD dwMaxValueDataLength;
    DWORD dwIndex;
    LPWSTR lpName;
    LPWSTR lpData;
    LPWSTR lpExpandData;
    DWORD dwNameLength;
    DWORD dwDataLength;
    DWORD dwType;
    INT ItemIndex;

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
        RegCloseKey(hKey);
        return;
    }

    lpName = Alloc((dwMaxValueNameLength + 1) * sizeof(WCHAR));
    if (!lpName)
    {
        DebugAllocFailed();
        RegCloseKey(hKey);
        return;
    }

    lpData = Alloc((dwMaxValueDataLength + 1) * sizeof(WCHAR));
    if (!lpData)
    {
        DebugAllocFailed();
        Free(lpData);
        RegCloseKey(hKey);
        return;
    }

    lpExpandData = Alloc(2048 * sizeof(WCHAR));
    if (!lpExpandData)
    {
        DebugAllocFailed();
        Free(lpName);
        Free(lpData);
        RegCloseKey(hKey);
        return;
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
            Free(lpName);
            Free(lpData);
            RegCloseKey(hKey);
            return;
        }

        if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
            continue;

        ExpandEnvironmentStrings(lpData, lpExpandData, 2048);

        ItemIndex = IoAddItem(0, lpName);
        IoSetItemText(ItemIndex, 1, lpExpandData);
    }

    Free(lpExpandData);
    Free(lpName);
    Free(lpData);
    RegCloseKey(hKey);
}

VOID
OS_EnvironInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_APPS);

    EnumEnvironmentVariables(HKEY_CURRENT_USER,
                             L"Environment");

    EnumEnvironmentVariables(HKEY_LOCAL_MACHINE,
                             L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");

    Index = IoAddItem(0, L"ALLUSERSPROFILE");
    ExpandEnvironmentStrings(L"%ALLUSERSPROFILE%", szText, MAX_STR_LEN);
    IoSetItemText(Index, 1, szText);

    Index = IoAddItem(0, L"APPDATA");
    ExpandEnvironmentStrings(L"%APPDATA%", szText, MAX_STR_LEN);
    IoSetItemText(Index, 1, szText);

    Index = IoAddItem(0, L"COMPUTERNAME");
    ExpandEnvironmentStrings(L"%COMPUTERNAME%", szText, MAX_STR_LEN);
    IoSetItemText(Index, 1, szText);

    Index = IoAddItem(0, L"HOMEPATH");
    ExpandEnvironmentStrings(L"%HOMEPATH%", szText, MAX_STR_LEN);
    IoSetItemText(Index, 1, szText);

    Index = IoAddItem(0, L"LOCALAPPDATA");
    ExpandEnvironmentStrings(L"%LOCALAPPDATA%", szText, MAX_STR_LEN);
    IoSetItemText(Index, 1, szText);

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
    INT Index;

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
    IoAddHeader(IDS_CAT_OS_DESKTOP, 0);

    /* Resolution */
    Index = IoAddValueName(IDS_DESK_RESOLUTION, -1);
    StringCbPrintf(szText, sizeof(szText),
                   L"%ld x %ld",
                   DevMode.dmPelsWidth,
                   DevMode.dmPelsHeight);
    IoSetItemText(Index, 1, szText);

    /* Color depth */
    Index = IoAddValueName(IDS_DESK_COLOR_DEPTH, -1);
    StringCbPrintf(szText, sizeof(szText),
                   L"%ld bit",
                   DevMode.dmBitsPerPel);
    IoSetItemText(Index, 1, szText);

    /* DPI */
    // TODO

    /* Refresh rate */
    Index = IoAddValueName(IDS_DESK_REFRESH_RATE, -1);
    StringCbPrintf(szText, sizeof(szText),
                   L"%ld Hz",
                   DevMode.dmDisplayFrequency);
    IoSetItemText(Index, 1, szText);

    /* Wallpaper */
    if (GetStringFromRegistry(HKEY_CURRENT_USER,
                              L"Control Panel\\Desktop",
                              L"Wallpaper",
                              szText,
                              MAX_STR_LEN))
    {
        Index = IoAddValueName(IDS_DESK_WALLPAPER, -1);
        IoSetItemText(Index, 1, szText);
    }

    /* Font smoothing */
    if (SystemParametersInfo(SPI_GETFONTSMOOTHING, 0, &BoolParam, 0))
    {
        UINT Type;

        Index = IoAddValueName(IDS_DESK_FONT_SMOOTHING, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);

        /* Smoothing type */
        if (SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &Type, 0))
        {
            UINT StringID = IDS_SMOOTHING_STANDARD;

            Index = IoAddValueName(IDS_DESK_FONT_SMOOTHING_TYPE, -1);
            if (Type == FE_FONTSMOOTHINGCLEARTYPE)
                StringID = IDS_SMOOTHING_CLEARTYPE;

            LoadMUIString(StringID, szText, MAX_STR_LEN);
            IoSetItemText(Index, 1, szText);
        }
    }

    IoAddFooter();

    /* MOUSE */
    IoAddHeader(IDS_DESK_MOUSE, 1);

    /* Speed */
    if (SystemParametersInfo(SPI_GETMOUSESPEED, 0, &IntParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_MOUSE_SPEED, -1);
        StringCbPrintf(szText, sizeof(szText), L"%d", IntParam);
        IoSetItemText(Index, 1, szText);
    }

    /* Trails */
    if (SystemParametersInfo(SPI_GETMOUSETRAILS, 0, &IntParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_MOUSE_TRAILS, -1);
        IoSetItemText(Index, 1, IntParam ? szYes : szNo);
    }

    /* Scroll lines */
    if (SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &UintParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_WHELL_SCROLL_LINES, -1);
        StringCbPrintf(szText, sizeof(szText), L"%ld", UintParam);
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();

    /* KEYBOARD */
    IoAddHeader(IDS_DESK_KEYBOARD, 2);

    /* Speed */
    if (SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &DwordParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_KEYBOARD_SPEED, -1);
        StringCbPrintf(szText, sizeof(szText), L"%ld", DwordParam);
        IoSetItemText(Index, 1, szText);
    }

    /* Delay */
    if (SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &IntParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_KEYBOARD_DELAY, -1);
        StringCbPrintf(szText, sizeof(szText), L"%ld", IntParam);
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();

    /* UI EFFECTS */
    IoAddHeader(IDS_DESK_UI_EFFECTS, 3);

    /* Combobox animation */
    if (SystemParametersInfo(SPI_GETCOMBOBOXANIMATION, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_COMBOBOX_ANIMATION, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    /* Cursor shadow */
    if (SystemParametersInfo(SPI_GETCURSORSHADOW, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_CURSOR_SHADOW, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    /* Gradient captions */
    if (SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_GRADIENT_CAPTIONS, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    /* Listbox smooth scrolling */
    if (SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_LISTBOXSMOOTHSCROLLING, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    /* Menu animation */
    if (SystemParametersInfo(SPI_GETMENUANIMATION, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_MENUANIMATION, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    /* Tooltip animation */
    if (SystemParametersInfo(SPI_GETTOOLTIPANIMATION, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_TOOLTIPANIMATION, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    IoAddFooter();

    /* WINDOW */
    IoAddHeader(IDS_DESK_WINDOW, 4);

    /* Window animation */
    AnimationInfo.cbSize = sizeof(ANIMATIONINFO);
    if (SystemParametersInfo(SPI_GETANIMATION, sizeof(ANIMATIONINFO), &AnimationInfo, 0))
    {
        Index = IoAddValueName(IDS_DESK_WND_ANIMATION, -1);
        IoSetItemText(Index, 1, AnimationInfo.iMinAnimate ? szYes : szNo);
    }

    /* Border */
    if (SystemParametersInfo(SPI_GETBORDER, 0, &IntParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_WND_BORDER, -1);
        StringCbPrintf(szText, sizeof(szText), L"%ld", IntParam);
        IoSetItemText(Index, 1, szText);
    }

    /* Drag full */
    if (SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_WND_DRAG_FULL, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    /* Drops shadow */
    if (SystemParametersInfo(SPI_GETDROPSHADOW, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_DROPS_SHADOW, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    IoAddFooter();

    /* MENU */
    IoAddHeader(IDS_DESK_MENU, 5);

    /* Menu animation */
    if (SystemParametersInfo(SPI_GETMENUFADE, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_MENU_FADE_ANIMATION, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    /* Menu show delay */
    if (SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &DwordParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_MENU_SHOW_DELAY, -1);
        StringCbPrintf(szText, sizeof(szText), L"%ld", DwordParam);
        IoSetItemText(Index, 1, szText);
    }

    /* Flat menu */
    if (SystemParametersInfo(SPI_GETFLATMENU, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_FLAT_MENU, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);
    }

    IoAddFooter();

    /* SCREEN SAVER */
    IoAddHeader(IDS_DESK_SS, 6);

    /* Active */
    if (SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &BoolParam, 0))
    {
        Index = IoAddValueName(IDS_DESK_SS_ACTIVE, -1);
        IoSetItemText(Index, 1, BoolParam ? szYes : szNo);

        /* Timeout */
        if (BoolParam && SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &IntParam, 0))
        {
            Index = IoAddValueName(IDS_DESK_SS_TIMEOUT, -1);
            StringCbPrintf(szText, sizeof(szText), L"%ld", IntParam);
            IoSetItemText(Index, 1, szText);
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
    INT Index;

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
            Index = IoAddHeaderString(lpszNames[--dwFetchedTasks], 0);

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

                Index = IoAddValueName(IDS_STATUS, -1);
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
                IoSetItemText(Index, 1, szText);
            }

            /* Application name */
            hr = pITask->lpVtbl->GetApplicationName(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                Index = IoAddValueName(IDS_TASK_APP_NAME, -1);
                IoSetItemText(Index, 1, lpszText);
                CoTaskMemFree(lpszText);
            }

            /* Application parameters */
            hr = pITask->lpVtbl->GetParameters(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                Index = IoAddValueName(IDS_TASK_APP_PARAMS, -1);
                IoSetItemText(Index, 1, lpszText);
                CoTaskMemFree(lpszText);
            }

            /* Working directory */
            hr = pITask->lpVtbl->GetWorkingDirectory(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                Index = IoAddValueName(IDS_TASK_WORK_DIR, -1);
                IoSetItemText(Index, 1, lpszText);
                CoTaskMemFree(lpszText);
            }

            /* Comment */
            hr = pITask->lpVtbl->GetComment(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                Index = IoAddValueName(IDS_TASK_COMMENT, -1);
                IoSetItemText(Index, 1, lpszText);
                CoTaskMemFree(lpszText);
            }

            /* Account information */
            hr = pITask->lpVtbl->GetAccountInformation(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                Index = IoAddValueName(IDS_TASK_ACCAUNT, -1);
                IoSetItemText(Index, 1, lpszText);
                CoTaskMemFree(lpszText);
            }

            /* Creator */
            hr = pITask->lpVtbl->GetCreator(pITask, &lpszText);
            if (SUCCEEDED(hr))
            {
                Index = IoAddValueName(IDS_TASK_CREATOR, -1);
                IoSetItemText(Index, 1, lpszText);
                CoTaskMemFree(lpszText);
            }

            /* Last run */
            hr = pITask->lpVtbl->GetMostRecentRunTime(pITask, &stTime);
            if (SUCCEEDED(hr))
            {
                Index = IoAddValueName(IDS_TASK_LAST_RUN_TIME, -1);
                StringCbPrintf(szText,
                               sizeof(szText),
                               L"%02u.%02u.%u %u:%02u",
                               stTime.wDay,
                               stTime.wMonth,
                               stTime.wYear,
                               stTime.wHour,
                               stTime.wMinute);
                IoSetItemText(Index, 1, szText);
            }

            /* Next run */
            hr = pITask->lpVtbl->GetNextRunTime(pITask, &stTime);
            if (SUCCEEDED(hr))
            {
                Index = IoAddValueName(IDS_TASK_NEXT_RUN_TIME, -1);
                StringCbPrintf(szText,
                               sizeof(szText),
                               L"%02u.%02u.%u %u:%02u",
                               stTime.wDay,
                               stTime.wMonth,
                               stTime.wYear,
                               stTime.wHour,
                               stTime.wMinute);
                IoSetItemText(Index, 1, szText);
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
                    StringCbPrintf(szText, sizeof(szText),
                                   szFormat, wTriggerIndex + 1);
                    Index = IoAddItem(-1, szText);
                    IoSetItemText(Index, 1, lpszText);
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
    INT Index;

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

    Index = IoAddValueName(ValueName, IsAllowed ? 0 : 1);
    IoSetItemText(Index, 1, szText);
}

VOID
OS_PreventsInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);
    IoAddIcon(IDI_COMPUTER);
    IoAddIcon(IDI_FOLDER);
    IoAddIcon(IDI_REG);
    IoAddIcon(IDI_PREVENT);

    IoAddHeader(IDS_PREV_TOOLS, 2);

    AddPreventItem(IDS_PREV_TASKMGR, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                   L"DisableTaskMgr");

    AddPreventItem(IDS_PREV_REGEDIT, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                   L"DisableRegistryTools");

    AddPreventItem(IDS_PREV_DISABLE_CMD, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                   L"DisableCMD");

    IoAddFooter();

    IoAddHeader(IDS_PREV_EXPLORER, 3);

    AddPreventItem(IDS_PREV_DESKTOP_ITEMS, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoDesktop");
    AddPreventItem(IDS_PREV_EXPLORER_CONTEXT_MENU, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoViewContextMenu");
    AddPreventItem(IDS_PREV_TASKBAR_CONTEXT_MENU, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoTrayContextMenu");
    AddPreventItem(IDS_PREV_HIDE_TRAY, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoTrayItemsDisplay");
    AddPreventItem(IDS_PREV_TOOLBARS_ON_TASKBAR, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoToolBarsOnTaskbar");
    AddPreventItem(IDS_PREV_HIDE_CLOCK, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"HideClock");
    AddPreventItem(IDS_PREV_RECENT_DOCS, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoRecentDocsMenu");
    AddPreventItem(IDS_PREV_FAVORITES, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoFavoritiesMenu");
    AddPreventItem(IDS_PREV_FIND_COMMAND, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoFind");
    AddPreventItem(IDS_PREV_RUN_COMMAND, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoRun");
    AddPreventItem(IDS_PREV_LOG_OFF, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoLogOff");
    AddPreventItem(IDS_PREV_SHUTDOWN, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"NoClose");

    IoAddFooter();

    IoAddHeader(IDS_PREV_AUTORUN, 4);

    AddPreventItem(IDS_PREV_HKLM_RUN, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"DisableLocalMachineRun");
    AddPreventItem(IDS_PREV_HKLM_RUNONCE, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"DisableLocalMachineRunOnce");
    AddPreventItem(IDS_PREV_HKCU_RUN, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"DisableCurrentUserRun");
    AddPreventItem(IDS_PREV_HKCU_RUNONCE, HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
                   L"DisableCurrentUserRunOnce");

    IoAddFooter();

    IoAddHeader(IDS_PREV_OTHER, 5);

    AddPreventItem(IDS_PREV_DISABLE_GPO, HKEY_LOCAL_MACHINE,
                   L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",
                   L"DisableGPO");

    DebugEndReceiving();
}
