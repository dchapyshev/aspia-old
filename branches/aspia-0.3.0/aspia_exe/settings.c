/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/settings.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"


SETTINGS_STRUCT SettingsInfo = {0};
THEMES_STRUCT ThemesInfo = {0};

#define DIALOGS_COUNT   3
#define GENERAL_DIALOG  0
#define FILTER_DIALOG   1
#define SYSTRAY_DIALOG  2

HIMAGELIST hSettingsImageList;
HWND hDialogs[DIALOGS_COUNT];

COLORREF CpuFontColor = 0;
COLORREF CpuBackground = 0;
COLORREF HddFontColor = 0;
COLORREF HddBackground = 0;
HICON hCpuIcon = NULL;
HICON hHddIcon = NULL;


BOOL
GetIniFilePath(OUT LPWSTR lpszPath, IN SIZE_T PathLen)
{
    WCHAR szPath[MAX_PATH];

    if (!ParamsInfo.IsPortable)
    {
        if (!SHGetSpecialFolderPath(hMainWnd, szPath, CSIDL_APPDATA, FALSE))
        {
            return FALSE;
        }
        StringCchPrintf(lpszPath, PathLen, L"%s\\aspia.ini", szPath);

        if (GetFileAttributes(lpszPath) == INVALID_FILE_ATTRIBUTES)
        {
            if (!SHGetSpecialFolderPath(hMainWnd, szPath, CSIDL_COMMON_APPDATA, FALSE))
            {
                return FALSE;
            }

            StringCchPrintf(lpszPath, PathLen, L"%s\\aspia.ini", szPath);
        }
    }
    else
    {
        if (!GetCurrentPath(szPath, MAX_PATH))
            return FALSE;

        StringCchPrintf(lpszPath, PathLen, L"%saspia.ini", szPath);
    }

    return TRUE;
}

BOOL
WritePrivateProfileInt(IN LPCTSTR lpAppName,
                       IN LPCTSTR lpKeyName,
                       IN INT Value,
                       IN LPCTSTR lpFileName)
{
    WCHAR szText[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText), L"%d", Value);
    return WritePrivateProfileString(lpAppName, lpKeyName, szText, lpFileName);
}

VOID
SaveCategoriesSelections(LPWSTR lpszIniPath, CATEGORY_LIST *List)
{
    WCHAR szText[MAX_STR_LEN];
    SIZE_T Index = 0;

    do
    {
        StringCbPrintf(szText, sizeof(szText), L"%d", List[Index].StringID);
        WritePrivateProfileInt(L"categories", szText, (INT)List[Index].Checked, lpszIniPath);

        if (List[Index].Child)
        {
            SaveCategoriesSelections(lpszIniPath, List[Index].Child);
        }
    }
    while (List[++Index].StringID != 0);
}

VOID
LoadCategoriesSelections(IN LPWSTR lpszIniPath,
                         IN CATEGORY_LIST *List)
{
    WCHAR szText[MAX_STR_LEN];
    SIZE_T Index = 0;

    do
    {
        StringCbPrintf(szText, sizeof(szText), L"%d", List[Index].StringID);
        List[Index].Checked =
            (GetPrivateProfileInt(L"categories",
                                  szText, 1, lpszIniPath) == 0) ? FALSE : TRUE;

        if (List[Index].Child)
        {
            LoadCategoriesSelections(lpszIniPath, List[Index].Child);
        }
    }
    while (List[++Index].StringID != 0);
}

BOOL
GetAutostartState(VOID)
{
    WCHAR szPath[MAX_PATH * 2];
    DWORD dwSize, dwType;
    HKEY hKey;

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                     0,
                     KEY_READ,
                     &hKey) != ERROR_SUCCESS)
    {
        DebugTrace(L"RegOpenKeyEx() failed!");
        return FALSE;
    }

    dwSize = MAX_PATH * 2;

    dwType = REG_SZ;
    if (RegQueryValueEx(hKey, L"Aspia", 0, &dwType,
                        (LPBYTE)szPath, &dwSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    RegCloseKey(hKey);

    return TRUE;
}

VOID
SaveAutostartState(BOOL IsAutostart)
{
    WCHAR szExePath[MAX_PATH], szPath[MAX_PATH];
    HKEY hKey;

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                     0,
                     KEY_READ | KEY_WRITE | KEY_WOW64_64KEY,
                     &hKey) != ERROR_SUCCESS)
    {
        DebugTrace(L"RegOpenKeyEx() failed!");
        return;
    }

    if (IsAutostart)
    {
        if (GetModuleFileName(hInstance, szExePath, MAX_PATH))
        {
            StringCbPrintf(szPath, sizeof(szPath), L"%s", szExePath);
            RegSetValueEx(hKey, L"Aspia", 0, REG_SZ,
                          (LPBYTE)szPath, MAX_PATH);
        }
    }
    else
    {
        RegDeleteValue(hKey, L"Aspia");
    }

    RegCloseKey(hKey);
}

BOOL
LoadSettings(VOID)
{
    WCHAR szIniPath[MAX_PATH];
    BOOL Result;

    if (!GetIniFilePath(szIniPath, MAX_PATH))
    {
        DebugTrace(L"GetIniFilePath() failed!");
        return FALSE;
    }

    Result = GetPrivateProfileStruct(L"general",
                                     L"settings",
                                     &SettingsInfo,
                                     sizeof(SettingsInfo),
                                     szIniPath);

    GetPrivateProfileString(L"general",
                            L"language",
                            0,
                            ThemesInfo.szLangFile,
                            MAX_PATH,
                            szIniPath);

    GetPrivateProfileString(L"general",
                            L"icons",
                            0,
                            ThemesInfo.szIconsFile,
                            MAX_PATH,
                            szIniPath);

    LoadCategoriesSelections(szIniPath, RootCategoryList);

    SettingsInfo.Autorun = GetAutostartState();

    return Result;
}

BOOL
SaveSettings(VOID)
{
    WCHAR szIniPath[MAX_PATH];
    WINDOWPLACEMENT wp;

    if (!GetIniFilePath(szIniPath, MAX_PATH))
    {
        DebugTrace(L"GetIniFilePath() failed!");
        return FALSE;
    }

    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hMainWnd, &wp);

    SettingsInfo.SplitterPos = GetWindowWidth(hTreeView);

    SettingsInfo.IsMaximized =
        (IsZoomed(hMainWnd) || (wp.flags & WPF_RESTORETOMAXIMIZED));

    if (!SettingsInfo.IsMaximized)
    {
        SettingsInfo.Left = wp.rcNormalPosition.left;
        SettingsInfo.Top  = wp.rcNormalPosition.top;
        SettingsInfo.Right  = wp.rcNormalPosition.right;
        SettingsInfo.Bottom = wp.rcNormalPosition.bottom;
    }

    WritePrivateProfileStruct(L"general",
                              L"settings",
                              &SettingsInfo,
                              sizeof(SettingsInfo),
                              szIniPath);

    WritePrivateProfileString(L"general",
                              L"language",
                              ThemesInfo.szLangFile,
                              szIniPath);

    WritePrivateProfileString(L"general",
                              L"icons",
                              ThemesInfo.szIconsFile,
                              szIniPath);

    SaveCategoriesSelections(szIniPath, RootCategoryList);

    SaveAutostartState(SettingsInfo.Autorun);

    return TRUE;
}

VOID
InitCategoriesCombo(IN HWND hCombo,
                    IN LPWSTR lpszRootName,
                    IN CATEGORY_LIST *List)
{
    WCHAR szText[MAX_STR_LEN], szName[MAX_STR_LEN];
    SIZE_T Index = 0;
    INT ItemIndex;

    do
    {
        LoadMUIStringF(hLangInst, List[Index].StringID, szName, MAX_STR_LEN);

        if (!List[Index].Child)
        {
            if (lpszRootName)
                StringCbPrintf(szText, sizeof(szText),
                               L"%s - %s",
                               lpszRootName, szName);
            else
                StringCbCopy(szText, sizeof(szText), szName);

            ItemIndex = SendMessage(hCombo, CB_ADDSTRING, 0,
                                    (LPARAM)szText);
            SendMessage(hCombo, CB_SETITEMDATA, ItemIndex,
                        (LPARAM)List[Index].StringID);

            if (List[Index].StringID == SettingsInfo.StartupCategory)
            {
                SendMessage(hCombo, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)szText);
            }
        }
        else
        {
            InitCategoriesCombo(hCombo, szName, List[Index].Child);
        }
    }
    while (List[++Index].StringID != 0);
}

VOID
InitIconsCombo(IN HWND hCombo)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    WCHAR szPath[MAX_PATH], szDllPath[MAX_PATH], szIconDir[MAX_PATH], szText[MAX_STR_LEN];
    HINSTANCE hDLL;
    INT ItemIndex;

    StringCbPrintf(szIconDir, sizeof(szIconDir),
                   L"%sicons\\",
                   ParamsInfo.szCurrentPath);

    StringCbPrintf(szPath, sizeof(szPath), L"%s*.dll", szIconDir);

    hFind = FindFirstFile(szPath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"Icons DLLs not found");
        return;
    }

    do
    {
        StringCbPrintf(szDllPath, sizeof(szDllPath), L"%s%s",
                       szIconDir, FindFileData.cFileName);

        hDLL = LoadLibraryEx(szDllPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hDLL)
        {
            WCHAR *FileName;
            SIZE_T Size = (SafeStrLen(FindFileData.cFileName) + 1) * sizeof(WCHAR);

            FileName = (WCHAR*)Alloc(Size);
            if (FileName)
            {
                StringCbCopy(FileName, Size, FindFileData.cFileName);

                LoadString(hDLL, 100, szText, MAX_STR_LEN);

                ItemIndex = SendMessage(hCombo, CB_ADDSTRING, 0,
                                        (LPARAM)szText);

                SendMessage(hCombo, CB_SETITEMDATA, ItemIndex,
                            (LPARAM)FileName);
            }

            FreeLibrary(hDLL);
        }

        if (wcscmp(FindFileData.cFileName, ThemesInfo.szIconsFile) == 0)
        {
            SendMessage(hCombo, CB_SELECTSTRING, (WPARAM)-1,
                        (LPARAM)szText);
        }
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);
}

VOID
FreeIconsCombo(HWND hCombo)
{
    INT Count = SendMessage(hCombo, CB_GETCOUNT, 0, 0) - 1;
    WCHAR *FileName;

    if (Count == CB_ERR)
    {
        DebugTrace(L"SendMessage(CB_GETCOUNT) failed!");
        return;
    }

    while (Count >= 0)
    {
        FileName = (WCHAR*)SendMessage(hCombo, CB_GETITEMDATA, Count, 0);
        if (FileName) Free(FileName);
        Count--;
    }
}

VOID
InitLangCombo(IN HWND hCombo)
{
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData;
    WCHAR szPath[MAX_PATH], szDllPath[MAX_PATH], szLangDir[MAX_PATH], szText[MAX_STR_LEN];
    WCHAR szLangDll[MAX_PATH];
    HINSTANCE hDLL;
    INT ItemIndex;

    StringCbCopy(szLangDir, sizeof(szLangDir), ParamsInfo.szCurrentPath);

    StringCbCat(szLangDir, sizeof(szLangDir), L"languages\\");
    StringCbPrintf(szPath, sizeof(szPath), L"%s*.dll", szLangDir);

    hFind = FindFirstFile(szPath, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        StringCbPrintf(szDllPath, sizeof(szDllPath), L"%s%s",
                       szLangDir, FindFileData.cFileName);

        hDLL = LoadLibraryEx(szDllPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (hDLL)
        {
            WCHAR *FileName;
            SIZE_T Size = (SafeStrLen(FindFileData.cFileName) + 1) * sizeof(WCHAR);

            FileName = (WCHAR*)Alloc(Size);
            if (FileName)
            {
                StringCbCopy(FileName, Size, FindFileData.cFileName);

                LoadString(hDLL, 10000, szText, MAX_STR_LEN);

                ItemIndex = SendMessage(hCombo, CB_ADDSTRING, 0,
                                        (LPARAM)szText);

                /* Отсекаем расширение .dll */
                FileName[wcslen(FileName) - 4] = 0;

                SendMessage(hCombo, CB_SETITEMDATA, ItemIndex,
                            (LPARAM)FileName);
            }

            FreeLibrary(hDLL);
        }

        StringCbPrintf(szLangDll, sizeof(szLangDll),
                       L"%s.dll", ThemesInfo.szLangFile);

        if (wcscmp(FindFileData.cFileName, szLangDll) == 0)
        {
            SendMessage(hCombo, CB_SELECTSTRING, (WPARAM)-1,
                        (LPARAM)szText);
        }
    }
    while (FindNextFile(hFind, &FindFileData) != 0);

    FindClose(hFind);
}

VOID
FreeLangCombo(HWND hCombo)
{
    INT Count = SendMessage(hCombo, CB_GETCOUNT, 0, 0) - 1;
    WCHAR *FileName;

    if (Count == CB_ERR)
    {
        DebugTrace(L"SendMessage(CB_GETCOUNT) failed!");
        return;
    }

    while (Count > 0)
    {
        FileName = (WCHAR*)SendMessage(hCombo, CB_GETITEMDATA, Count, 0);
        if (FileName) Free(FileName);
        --Count;
    }
}

VOID
ShowSensorsList(HWND hList)
{
    WCHAR szText[MAX_STR_LEN], szIniPath[MAX_PATH];
    IDSECTOR DriveInfo;
    HANDLE hHandle;
    BYTE bIndex;
    INT ItemIndex;

    if (!GetIniFilePath(szIniPath, MAX_PATH))
    {
        DebugTrace(L"GetIniFilePath() failed!");
        return;
    }

    AddColumn(hList, 0, 250, L"");
    ListView_SetExtendedListViewStyle(hList, LVS_EX_CHECKBOXES);

    /* Hard Drives */
    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        hHandle = OpenSmart(bIndex);
        if (!hHandle) continue;

        if (ReadSmartInfo(hHandle, bIndex, &DriveInfo))
        {
            ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                            sizeof(DriveInfo.sModelNumber));
            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sModelNumber);
            ChopSpaces(szText, sizeof(szText));

            if (szText[0] != 0)
            {
                ItemIndex = AddItem(hList, -1, szText, (LPARAM)0);
                if (GetPrivateProfileInt(L"sensors", szText, 0, szIniPath) > 0)
                {
                    ListView_SetCheckState(hList, ItemIndex, TRUE);
                }
            }
        }

        CloseSmart(hHandle);
    }

    /* CPUs */
    if (GetCPUName(szText, sizeof(szText)))
    {
        ItemIndex = AddItem(hList, -1, szText, (LPARAM)0);
        if (GetPrivateProfileInt(L"sensors", szText, 0, szIniPath) > 0)
        {
            ListView_SetCheckState(hList, ItemIndex, TRUE);
        }
    }
}

VOID
SaveSensorsState(HWND hList)
{
    INT Count = ListView_GetItemCount(hList) - 1;
    WCHAR szText[MAX_STR_LEN], szIniPath[MAX_PATH];

    if (!GetIniFilePath(szIniPath, MAX_PATH))
    {
        DebugTrace(L"GetIniFilePath() failed!");
        return;
    }

    while (Count >= 0)
    {
        ListView_GetItemText(hList, Count, 0, szText, MAX_STR_LEN);
        WritePrivateProfileInt(L"sensors",
                               szText,
                               (INT)ListView_GetCheckState(hList, Count),
                               szIniPath);
        --Count;
    }
}

BOOL
ChooseColorDialog(HWND hDlg,
                  COLORREF CurrentColor,
                  COLORREF *Color)
{
    CHOOSECOLOR CColor = {0};
    static COLORREF acrCustClr[16];
    BOOL Result;

    CColor.lStructSize = sizeof(CHOOSECOLOR);
    CColor.hwndOwner = hDlg;
    CColor.Flags = CC_FULLOPEN | CC_RGBINIT;
    CColor.lpCustColors = acrCustClr;
    CColor.rgbResult = CurrentColor;

    Result = ChooseColor(&CColor);

    *Color = CColor.rgbResult;

    return Result;
}

INT_PTR CALLBACK
GeneralPageWndProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    HWND hCatList = GetDlgItem(hDlg, IDC_STARTUP_CAT_COMBO);
    HWND hIconsList = GetDlgItem(hDlg, IDC_ICONS_COMBO);
    HWND hLangList = GetDlgItem(hDlg, IDC_LANGUAGE_COMBO);

    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);

    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            CheckDlgButton(hDlg, IDC_SAVE_WINDOW_POS,
                           SettingsInfo.SaveWindowPos ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_START_WITH_WINDOWS,
                           SettingsInfo.Autorun ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_STAY_ON_TOP,
                           SettingsInfo.StayOnTop ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_STYLES_WNDS,
                           SettingsInfo.ShowWindowStyles ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_ALT_ROWS,
                           SettingsInfo.ShowAltRows ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_ALLOW_KM_DRIVER,
                           SettingsInfo.AllowKmDriver ? BST_CHECKED : BST_UNCHECKED);

            /* Init languages combobox */
            InitLangCombo(hLangList);

            InitCategoriesCombo(hCatList, NULL, RootCategoryList);
            InitIconsCombo(hIconsList);
        }
        break;

        case WM_CLOSE:
        {
            FreeIconsCombo(hIconsList);
            FreeLangCombo(hLangList);
        }
        break;
    }

    return FALSE;
}

INT_PTR CALLBACK
FilterPageWndProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);

    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            CheckDlgButton(hDlg, IDC_FILTER_ELOG_ERROR,
                           SettingsInfo.ELogShowError ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_FILTER_ELOG_WARNING,
                           SettingsInfo.ELogShowWarning ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_FILTER_ELOG_INFO,
                           SettingsInfo.ELogShowInfo ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_FILTER_IE_FILE,
                           SettingsInfo.IEShowFile ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_FILTER_IE_HTTP,
                           SettingsInfo.IEShowHttp ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_FILTER_IE_FTP,
                           SettingsInfo.IEShowFtp ? BST_CHECKED : BST_UNCHECKED);
        }
        break;
    }

    return FALSE;
}

INT_PTR CALLBACK
SysTrayPageWndProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    HWND hSensorsList = GetDlgItem(hDlg, IDC_SENSOR_ICON_LIST);

    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);

    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            WCHAR szText[MAX_STR_LEN];

            CheckDlgButton(hDlg, IDC_START_MINIMIZED,
                           SettingsInfo.HideToTray ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_SHOW_PROG_ICON,
                           SettingsInfo.ShowProgIcon ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_SHOW_SENSOR_ICON,
                           SettingsInfo.ShowSensorIcons ? BST_CHECKED : BST_UNCHECKED);

            SendMessage(GetDlgItem(hDlg, IDC_REF_RATE_UPDOWN),
                        UDM_SETRANGE, 0,
                        MAKELONG(MAX_SENSORS_REFRESH_RATE, 0));
            SendMessage(GetDlgItem(hDlg, IDC_REF_RATE_UPDOWN),
                        UDM_SETPOS, 0,
                        MAKELONG(SettingsInfo.SensorsRefreshRate, 0));
            SendMessage(GetDlgItem(hDlg, IDC_REFRESH_RATE_EDIT),
                        EM_LIMITTEXT, 5, 0);
            StringCbPrintf(szText, sizeof(szText), L"%d", SettingsInfo.SensorsRefreshRate);
            SetWindowText(GetDlgItem(hDlg, IDC_REFRESH_RATE_EDIT), szText);

            CpuBackground = SettingsInfo.CpuBackground;
            CpuFontColor = SettingsInfo.CpuFontColor;
            HddBackground = SettingsInfo.HddBackground;
            HddFontColor = SettingsInfo.HddFontColor;

            hCpuIcon = CreateTrayIcon(L"36", CpuBackground, CpuFontColor);
            SendMessage(GetDlgItem(hDlg, IDC_CPU_ICON), STM_SETICON, (WPARAM)hCpuIcon, 0);

            hHddIcon = CreateTrayIcon(L"53", HddBackground, HddFontColor);
            SendMessage(GetDlgItem(hDlg, IDC_HDD_ICON), STM_SETICON, (WPARAM)hHddIcon, 0);

            ShowSensorsList(hSensorsList);

            EnableWindow(GetDlgItem(hDlg, IDC_SENSOR_ICON_LIST), SettingsInfo.ShowSensorIcons);

            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_CPU_FONT_COLOR:
                {
                    if (ChooseColorDialog(GetParent(hDlg), CpuFontColor, &CpuFontColor))
                    {
                        if (hCpuIcon) DestroyIcon(hCpuIcon);
                        hCpuIcon = CreateTrayIcon(L"36", CpuBackground, CpuFontColor);
                        SendMessage(GetDlgItem(hDlg, IDC_CPU_ICON), STM_SETICON, (WPARAM)hCpuIcon, 0);
                    }
                }
                break;

                case IDC_CPU_BACKGROUND:
                {
                    if (ChooseColorDialog(hDlg, CpuBackground, &CpuBackground))
                    {
                        if (hCpuIcon) DestroyIcon(hCpuIcon);
                        hCpuIcon = CreateTrayIcon(L"36", CpuBackground, CpuFontColor);
                        SendMessage(GetDlgItem(hDlg, IDC_CPU_ICON), STM_SETICON, (WPARAM)hCpuIcon, 0);
                    }
                }
                break;

                case IDC_HDD_FONT_COLOR:
                {
                    if (ChooseColorDialog(hDlg, HddFontColor, &HddFontColor))
                    {
                        if (hHddIcon) DestroyIcon(hHddIcon);
                        hHddIcon = CreateTrayIcon(L"53", HddBackground, HddFontColor);
                        SendMessage(GetDlgItem(hDlg, IDC_HDD_ICON), STM_SETICON, (WPARAM)hHddIcon, 0);
                    }
                }
                break;

                case IDC_HDD_BACKGROUND:
                {
                    if (ChooseColorDialog(hDlg, HddBackground, &HddBackground))
                    {
                        if (hHddIcon) DestroyIcon(hHddIcon);
                        hHddIcon = CreateTrayIcon(L"53", HddBackground, HddFontColor);
                        SendMessage(GetDlgItem(hDlg, IDC_HDD_ICON), STM_SETICON, (WPARAM)hHddIcon, 0);
                    }
                }
                break;

                case IDC_SHOW_SENSOR_ICON:
                {
                    if (IsDlgButtonChecked(hDlg, IDC_SHOW_SENSOR_ICON) == BST_CHECKED)
                        EnableWindow(GetDlgItem(hDlg, IDC_SENSOR_ICON_LIST), TRUE);
                    else
                        EnableWindow(GetDlgItem(hDlg, IDC_SENSOR_ICON_LIST), FALSE);
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}

VOID
OnSelCategoryChange(INT CurSel)
{
    INT Index;

    /* retrieve new page */
    if (CurSel < 0 || CurSel > DIALOGS_COUNT)
        return;

    /* hide all windows */
    for (Index = 0; Index < DIALOGS_COUNT; Index++)
        ShowWindow(hDialogs[Index], SW_HIDE);

    ShowWindow(hDialogs[CurSel], SW_SHOW);
    BringWindowToTop(hDialogs[CurSel]);
}

HTREEITEM
AddSettingsCategory(HWND hTree,
                    UINT TextIndex,
                    UINT IconIndex,
                    UINT DialogIndex)
{
    TV_INSERTSTRUCT Insert = {0};
    WCHAR szText[MAX_STR_LEN];
    INT Index;
    HICON hIcon;

    hIcon = (HICON)LoadImage(hIconsInst,
                             MAKEINTRESOURCE(IconIndex),
                             IMAGE_ICON,
                             ParamsInfo.SxSmIcon,
                             ParamsInfo.SySmIcon,
                             LR_CREATEDIBSECTION);

    Index = ImageList_AddIcon(hSettingsImageList, hIcon);
    DestroyIcon(hIcon);

    LoadMUIStringF(hLangInst, TextIndex, szText, MAX_STR_LEN);

    Insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    Insert.hInsertAfter = TVI_LAST;
    Insert.hParent = TVI_ROOT;
    Insert.item.iSelectedImage = Index;
    Insert.item.iImage = Index;
    Insert.item.lParam = (LPARAM)DialogIndex;
    Insert.item.pszText = szText;

    return TreeView_InsertItem(hTree, &Insert);
}

VOID
InitSettingsControls(HWND hDlg, HWND hTree)
{
    hSettingsImageList =
        ImageList_Create(ParamsInfo.SxSmIcon,
                         ParamsInfo.SySmIcon,
                         ParamsInfo.SysColorDepth | ILC_MASK,
                         0, 1);

    TreeView_SelectItem(hTree,
                        AddSettingsCategory(hTree, IDS_SETTINGS_GENERAL, IDI_COMPUTER, 0));
    AddSettingsCategory(hTree, IDS_SETTINGS_FILTER, IDI_APPS, 1);
    AddSettingsCategory(hTree, IDS_SETTINGS_SYSTRAY, IDI_SERVICES, 2);

    TreeView_SetImageList(hTree, hSettingsImageList, TVSIL_NORMAL);

    hDialogs[GENERAL_DIALOG] =
        CreateDialog(hLangInst,
                     MAKEINTRESOURCE(IDD_SETTINGS_GENERAL),
                     hDlg,
                     GeneralPageWndProc);
    hDialogs[FILTER_DIALOG] =
        CreateDialog(hLangInst,
                     MAKEINTRESOURCE(IDD_SETTINGS_FILTER),
                     hDlg,
                     FilterPageWndProc);
    hDialogs[SYSTRAY_DIALOG] =
        CreateDialog(hLangInst,
                     MAKEINTRESOURCE(IDD_SETTINGS_SYSTRAY),
                     hDlg,
                     SysTrayPageWndProc);

    IntSetWindowTheme(hTree);

    OnSelCategoryChange(0);
}

VOID
SaveSettingsFromDialog(HWND hDlg)
{
    HWND hCatList = GetDlgItem(hDialogs[GENERAL_DIALOG], IDC_STARTUP_CAT_COMBO);
    HWND hLangList = GetDlgItem(hDialogs[GENERAL_DIALOG], IDC_LANGUAGE_COMBO);
    HWND hSensorsList = GetDlgItem(hDialogs[SYSTRAY_DIALOG], IDC_SENSOR_ICON_LIST);
    HWND hIconsList = GetDlgItem(hDialogs[GENERAL_DIALOG], IDC_ICONS_COMBO);
    WCHAR *IconFile, *LangFile, szText[MAX_STR_LEN];
    INT Selected;
    BOOL KmDriverNew, ReInitCtrls = FALSE;
    BOOL State;

    SettingsInfo.CpuBackground = CpuBackground;
    SettingsInfo.CpuFontColor = CpuFontColor;
    SettingsInfo.HddBackground = HddBackground;
    SettingsInfo.HddFontColor = HddFontColor;

    GetWindowText(GetDlgItem(hDialogs[SYSTRAY_DIALOG], IDC_REFRESH_RATE_EDIT),
                  szText, MAX_STR_LEN);
    SettingsInfo.SensorsRefreshRate = (UINT)_wtoi(szText);

    /* Save checkbox'es states */
    SettingsInfo.SaveWindowPos =
        (IsDlgButtonChecked(hDialogs[GENERAL_DIALOG], IDC_SAVE_WINDOW_POS) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.Autorun =
        (IsDlgButtonChecked(hDialogs[GENERAL_DIALOG], IDC_START_WITH_WINDOWS) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.StayOnTop =
        (IsDlgButtonChecked(hDialogs[GENERAL_DIALOG], IDC_STAY_ON_TOP) == BST_CHECKED) ? TRUE : FALSE;

    State = (IsDlgButtonChecked(hDialogs[GENERAL_DIALOG], IDC_STYLES_WNDS) == BST_CHECKED) ? TRUE : FALSE;
    if (SettingsInfo.ShowWindowStyles != State)
    {
        SettingsInfo.ShowWindowStyles = State;

        IntSetWindowTheme(hTreeView);
        IntSetWindowTheme(hListView);
    }

    State = (IsDlgButtonChecked(hDialogs[GENERAL_DIALOG], IDC_ALT_ROWS) == BST_CHECKED) ? TRUE : FALSE;
    if (SettingsInfo.ShowAltRows != State)
    {
        SettingsInfo.ShowAltRows = State;

        /* Update ListView */
        ShowWindow(hListView, SW_HIDE);
        ShowWindow(hListView, SW_SHOW);
        UpdateWindow(hListView);
    }

    SetWindowPos(hMainWnd,
                 SettingsInfo.StayOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                 20, 20, 850, 640,
                 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

    KmDriverNew =
        (IsDlgButtonChecked(hDialogs[GENERAL_DIALOG], IDC_ALLOW_KM_DRIVER) == BST_CHECKED) ? TRUE : FALSE;
    if (SettingsInfo.AllowKmDriver != KmDriverNew)
    {
        if (KmDriverNew)
        {
            SettingsInfo.AllowKmDriver = KmDriverNew;
            LoadDriver();
        }
        else
        {
            UnloadDriver();
            SettingsInfo.AllowKmDriver = KmDriverNew;
        }
    }

    SettingsInfo.HideToTray =
        (IsDlgButtonChecked(hDialogs[SYSTRAY_DIALOG], IDC_START_MINIMIZED) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.ShowProgIcon =
        (IsDlgButtonChecked(hDialogs[SYSTRAY_DIALOG], IDC_SHOW_PROG_ICON) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.ShowSensorIcons =
        (IsDlgButtonChecked(hDialogs[SYSTRAY_DIALOG], IDC_SHOW_SENSOR_ICON) == BST_CHECKED) ? TRUE : FALSE;

    SettingsInfo.ELogShowError =
        (IsDlgButtonChecked(hDialogs[FILTER_DIALOG], IDC_FILTER_ELOG_ERROR) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.ELogShowWarning =
        (IsDlgButtonChecked(hDialogs[FILTER_DIALOG], IDC_FILTER_ELOG_WARNING) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.ELogShowInfo =
        (IsDlgButtonChecked(hDialogs[FILTER_DIALOG], IDC_FILTER_ELOG_INFO) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.IEShowFile =
        (IsDlgButtonChecked(hDialogs[FILTER_DIALOG], IDC_FILTER_IE_FILE) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.IEShowHttp =
        (IsDlgButtonChecked(hDialogs[FILTER_DIALOG], IDC_FILTER_IE_HTTP) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.IEShowFtp =
        (IsDlgButtonChecked(hDialogs[FILTER_DIALOG], IDC_FILTER_IE_FTP) == BST_CHECKED) ? TRUE : FALSE;

    /* Находим текущий выделенный элемент. Если язык был изменен,
       то реинициализируем элементы управления */
    Selected = SendMessage(hLangList, CB_GETCURSEL, 0, 0);
    LangFile = (WCHAR*)SendMessage(hLangList, CB_GETITEMDATA, Selected, 0);
    if (!LangFile)
    {
        if (ThemesInfo.szLangFile[0] != 0)
        {
            ThemesInfo.szLangFile[0] = 0;
            LoadLanguage();
            ReInitCtrls = TRUE;
        }
    }
    else
    {
        if (SafeStrCmp(LangFile, ThemesInfo.szLangFile) != 0)
        {
            SafeStrCpyN(ThemesInfo.szLangFile, LangFile, MAX_PATH);
            LoadLanguage();
            ReInitCtrls = TRUE;
        }
    }

    Selected = SendMessage(hCatList, CB_GETCURSEL, 0, 0);
    SettingsInfo.StartupCategory =
        (UINT)SendMessage(hCatList, CB_GETITEMDATA, Selected, 0);

    Selected = SendMessage(hIconsList, CB_GETCURSEL, 0, 0);
    IconFile = (WCHAR*)SendMessage(hIconsList, CB_GETITEMDATA, Selected, 0);

    if (!IconFile)
    {
        if (ThemesInfo.szIconsFile[0] != 0)
        {
             ThemesInfo.szIconsFile[0] = 0;
             LoadIcons();
             ReInitCtrls = TRUE;
        }
    }
    else
    {
        if (SafeStrCmp(IconFile, ThemesInfo.szIconsFile) != 0)
        {
            SafeStrCpyN(ThemesInfo.szIconsFile, IconFile, MAX_PATH);
            LoadIcons();
            ReInitCtrls = TRUE;
        }
    }

    if (ReInitCtrls) ReInitControls();

    SaveSensorsState(hSensorsList);

    /* Save settings to .ini file */
    SaveSettings();

    InitInfoDll();

    UpdateTrayIcons();

    /* Close settings dialog */
    PostMessage(hDlg, WM_CLOSE, 0, 0);
}

INT_PTR CALLBACK
SettingsDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            SettingsInfo.Autorun = GetAutostartState();
            InitSettingsControls(hDlg, GetDlgItem(hDlg, IDC_SETTINGS_TREE));
        }
        break;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            if (pnmh->idFrom == IDC_SETTINGS_TREE && pnmh->code == TVN_SELCHANGED)
            {
                OnSelCategoryChange((UINT)((LPNMTREEVIEW)lParam)->itemNew.lParam);
            }
        }
        break;

        case WM_CLOSE:
        {
            UINT Index = 0;

            while (Index < DIALOGS_COUNT)
                DestroyWindow(hDialogs[Index++]);

            DestroyIcon(hCpuIcon);
            DestroyIcon(hHddIcon);
            EndDialog(hDlg, LOWORD(wParam));
        }
        break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    PostMessage(hDlg, WM_CLOSE, 0, 0);
                    break;

                case IDOK:
                    SaveSettingsFromDialog(hDlg);
                    break;
            }
        }
        break;
    }

    return FALSE;
}
