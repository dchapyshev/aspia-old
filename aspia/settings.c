/*
 * PROJECT:         Aspia
 * FILE:            aspia/settings.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"
#include "smart/smart.h"


SETTINGS_STRUCT SettingsInfo = {0};

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
    if (!GetModuleFileName(hInstance, lpszPath, PathLen))
        return FALSE;

    lpszPath[SafeStrLen(lpszPath) - 1] = L'i';
    lpszPath[SafeStrLen(lpszPath) - 2] = L'n';
    lpszPath[SafeStrLen(lpszPath) - 3] = L'i';

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
    WCHAR szText[MAX_STR_LEN], szIniPath[MAX_PATH];

    if (!GetIniFilePath(szIniPath, MAX_PATH))
        return FALSE;

    /* Window position settings */
    SettingsInfo.SaveWindowPos =
        (GetPrivateProfileInt(L"window_pos",
                              L"SaveWindowPos",
                              0, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.IsMaximized =
        (GetPrivateProfileInt(L"window_pos",
                              L"IsMaximized",
                              0, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.Left =
        GetPrivateProfileInt(L"window_pos",
                             L"Left", 20, szIniPath);
    SettingsInfo.Top = GetPrivateProfileInt(L"window_pos",
                                            L"Top",
                                            20, szIniPath);
    SettingsInfo.Right = GetPrivateProfileInt(L"window_pos",
                                              L"Right",
                                              870, szIniPath);
    SettingsInfo.Bottom = GetPrivateProfileInt(L"window_pos",
                                               L"Bottom",
                                               660, szIniPath);
    SettingsInfo.SplitterPos = GetPrivateProfileInt(L"window_pos",
                                                    L"SplitterPos",
                                                    200, szIniPath);

    /* Language */
    GetPrivateProfileString(L"settings", L"LangFile", L"\0",
                            SettingsInfo.szLangFile,
                            sizeof(SettingsInfo.szLangFile)/sizeof(WCHAR),
                            szIniPath);

    /* Icons */
    GetPrivateProfileString(L"settings", L"IconsFile", L"\0",
                            SettingsInfo.szIconsFile,
                            sizeof(SettingsInfo.szIconsFile)/sizeof(WCHAR),
                            szIniPath);

    SettingsInfo.StartupCategory =
        GetPrivateProfileInt(L"settings",
                             L"StartupCategory",
                             IDS_CAT_SUMMARY, szIniPath);

    SettingsInfo.HideToTray =
        (GetPrivateProfileInt(L"settings",
                              L"HideToTray",
                              0, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.ShowProgIcon =
        (GetPrivateProfileInt(L"settings",
                              L"ShowProgIcon",
                              0, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.ShowSensorIcons =
        (GetPrivateProfileInt(L"settings",
                              L"ShowSensorIcons",
                              0, szIniPath) == 0) ? FALSE : TRUE;

    /* Sensor Colors */
    GetPrivateProfileString(L"sensor_colors", L"cpu_font_color", L"0",
                            szText, MAX_STR_LEN, szIniPath);
    SettingsInfo.CpuFontColor = StrToHex(szText);
    GetPrivateProfileString(L"sensor_colors", L"cpu_background", L"80ffff",
                            szText, MAX_STR_LEN, szIniPath);
    SettingsInfo.CpuBackground = StrToHex(szText);
    GetPrivateProfileString(L"sensor_colors", L"hdd_font_color", L"ffffff",
                            szText, MAX_STR_LEN, szIniPath);
    SettingsInfo.HddFontColor = StrToHex(szText);
    GetPrivateProfileString(L"sensor_colors", L"hdd_background", L"0",
                            szText, MAX_STR_LEN, szIniPath);
    SettingsInfo.HddBackground = StrToHex(szText);

    /* Sensors Refresh Rate */
    SettingsInfo.SensorsRefreshRate =
        GetPrivateProfileInt(L"settings", L"SensorsRefreshRate", 30, szIniPath);

    /* Report */
    SettingsInfo.IsAddContent =
        (GetPrivateProfileInt(L"settings",
                              L"IsAddContent",
                              1, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.ELogShowError =
        (GetPrivateProfileInt(L"report_filter",
                              L"ELogShowError",
                              1, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.ELogShowWarning =
        (GetPrivateProfileInt(L"report_filter",
                              L"ELogShowWarning",
                              1, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.ELogShowInfo =
        (GetPrivateProfileInt(L"report_filter",
                              L"ELogShowInfo",
                              0, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.IEShowHttp =
        (GetPrivateProfileInt(L"report_filter",
                              L"IEShowHttp",
                              1, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.IEShowFtp =
        (GetPrivateProfileInt(L"report_filter",
                              L"IEShowFtp",
                              1, szIniPath) == 0) ? FALSE : TRUE;
    SettingsInfo.IEShowFile =
        (GetPrivateProfileInt(L"report_filter",
                              L"IEShowFile",
                              1, szIniPath) == 0) ? FALSE : TRUE;
    GetPrivateProfileString(L"settings", L"ReportPath", L"\0",
                            SettingsInfo.szReportPath,
                            MAX_PATH,
                            szIniPath);

    LoadCategoriesSelections(szIniPath, RootCategoryList);

    SettingsInfo.Autorun = GetAutostartState();

    return TRUE;
}

BOOL
SaveSettings(VOID)
{
    WCHAR szIniPath[MAX_PATH], szText[MAX_STR_LEN];
    WINDOWPLACEMENT wp;

    if (!GetIniFilePath(szIniPath, MAX_PATH))
        return FALSE;

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

    /* Window position settings */
    WritePrivateProfileInt(L"window_pos",
                           L"SaveWindowPos",
                           SettingsInfo.SaveWindowPos,
                           szIniPath);
    WritePrivateProfileInt(L"window_pos",
                           L"IsMaximized",
                           SettingsInfo.IsMaximized,
                           szIniPath);
    if (!SettingsInfo.IsMaximized)
    {
        WritePrivateProfileInt(L"window_pos", L"Left", SettingsInfo.Left, szIniPath);
        WritePrivateProfileInt(L"window_pos", L"Top", SettingsInfo.Top, szIniPath);
        WritePrivateProfileInt(L"window_pos", L"Right", SettingsInfo.Right, szIniPath);
        WritePrivateProfileInt(L"window_pos", L"Bottom", SettingsInfo.Bottom, szIniPath);
    }
    WritePrivateProfileInt(L"window_pos",
                           L"SplitterPos",
                           SettingsInfo.SplitterPos,
                           szIniPath);

    /* Language */
    WritePrivateProfileString(L"settings",
                              L"LangFile",
                              SettingsInfo.szLangFile,
                              szIniPath);
    WritePrivateProfileString(L"settings",
                              L"IconsFile",
                              SettingsInfo.szIconsFile,
                              szIniPath);
    WritePrivateProfileInt(L"settings",
                           L"StartupCategory",
                           SettingsInfo.StartupCategory,
                           szIniPath);

    WritePrivateProfileInt(L"settings",
                           L"HideToTray",
                           (INT)SettingsInfo.HideToTray,
                           szIniPath);
    WritePrivateProfileInt(L"settings",
                           L"ShowProgIcon",
                           (INT)SettingsInfo.ShowProgIcon,
                           szIniPath);
    WritePrivateProfileInt(L"settings",
                           L"ShowSensorIcons",
                           (INT)SettingsInfo.ShowSensorIcons,
                           szIniPath);

    /* Sensor Colors */
    StringCbPrintf(szText, sizeof(szText), L"%x", SettingsInfo.CpuFontColor);
    WritePrivateProfileString(L"sensor_colors", L"cpu_font_color", szText, szIniPath);

    StringCbPrintf(szText, sizeof(szText), L"%x", SettingsInfo.CpuBackground);
    WritePrivateProfileString(L"sensor_colors", L"cpu_background", szText, szIniPath);

    StringCbPrintf(szText, sizeof(szText), L"%x", SettingsInfo.HddFontColor);
    WritePrivateProfileString(L"sensor_colors", L"hdd_font_color", szText, szIniPath);

    StringCbPrintf(szText, sizeof(szText), L"%x", SettingsInfo.HddBackground);
    WritePrivateProfileString(L"sensor_colors", L"hdd_background", szText, szIniPath);

    /* Sensors Refresh Rate */
    if (SettingsInfo.SensorsRefreshRate > MAX_SENSORS_REFRESH_RATE)
        SettingsInfo.SensorsRefreshRate = MAX_SENSORS_REFRESH_RATE;
    WritePrivateProfileInt(L"settings",
                           L"SensorsRefreshRate",
                           (INT)SettingsInfo.SensorsRefreshRate,
                           szIniPath);

    /* Report */
    WritePrivateProfileInt(L"settings",
                           L"IsAddContent",
                           (INT)SettingsInfo.IsAddContent,
                           szIniPath);
    WritePrivateProfileInt(L"report_filter",
                           L"ELogShowError",
                           (INT)SettingsInfo.ELogShowError,
                           szIniPath);
    WritePrivateProfileInt(L"report_filter",
                           L"ELogShowWarning",
                           (INT)SettingsInfo.ELogShowWarning,
                           szIniPath);
    WritePrivateProfileInt(L"report_filter",
                           L"ELogShowInfo",
                           (INT)SettingsInfo.ELogShowInfo,
                           szIniPath);
    WritePrivateProfileInt(L"report_filter",
                           L"IEShowFile",
                           (INT)SettingsInfo.IEShowFile,
                           szIniPath);
    WritePrivateProfileInt(L"report_filter",
                           L"IEShowFtp",
                           (INT)SettingsInfo.IEShowFtp,
                           szIniPath);
    WritePrivateProfileInt(L"report_filter",
                           L"IEShowHttp",
                           (INT)SettingsInfo.IEShowHttp,
                           szIniPath);
    WritePrivateProfileString(L"settings",
                              L"ReportPath",
                              SettingsInfo.szReportPath,
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
        LoadMUIString(List[Index].StringID, szName, MAX_STR_LEN);

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

    ItemIndex = SendMessage(hCombo, CB_ADDSTRING, 0,
                            (LPARAM)L"Tango Icons");
    if (SafeStrLen(SettingsInfo.szIconsFile) == 0)
        SendMessage(hCombo, CB_SETCURSEL, ItemIndex, 0);

    StringCbPrintf(szIconDir, sizeof(szIconDir),
                   L"%sicons\\",
                   SettingsInfo.szCurrentPath);

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

        if (wcscmp(FindFileData.cFileName, SettingsInfo.szIconsFile) == 0)
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

    if (Count == CB_ERR) return;

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
    HINSTANCE hDLL;
    INT ItemIndex;

    ItemIndex = SendMessage(hCombo, CB_ADDSTRING, 0,
                            (LPARAM)L"English");
    if (SafeStrLen(SettingsInfo.szLangFile) == 0)
        SendMessage(hCombo, CB_SETCURSEL, ItemIndex, 0);

#ifdef _ASPIA_PORTABLE_

    if (!GetTempPath(MAX_PATH, szLangDir))
        return;

    StringCbCat(szLangDir, sizeof(szLangDir), L"aspia\\");

#else

    StringCbCopy(szLangDir, sizeof(szLangDir), SettingsInfo.szCurrentPath);

#endif /* _ASPIA_PORTABLE_ */

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

                SendMessage(hCombo, CB_SETITEMDATA, ItemIndex,
                            (LPARAM)FileName);
            }

            FreeLibrary(hDLL);
        }

        if (wcscmp(FindFileData.cFileName, SettingsInfo.szLangFile) == 0)
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

    if (Count == CB_ERR) return;

    while (Count > 0)
    {
        FileName = (WCHAR*)SendMessage(hCombo, CB_GETITEMDATA, Count, 0);
        if (FileName) Free(FileName);
        Count--;
    }
}

VOID
AddColumn(HWND hList, SIZE_T Index, INT Width, LPWSTR lpszText)
{
    LV_COLUMN Column = {0};

    Column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    Column.iSubItem = Index;
    Column.pszText = lpszText;
    Column.cx  = Width;
    Column.fmt = LVCFMT_LEFT;

    ListView_InsertColumn(hList, Index, &Column);
}

INT
AddItem(HWND hList, INT IconIndex, LPWSTR lpText)
{
    LV_ITEM Item = {0};

    Item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
    Item.pszText = lpText;
    Item.iItem = ListView_GetItemCount(hList);
    Item.iImage = IconIndex;

    return ListView_InsertItem(hList, &Item);
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
        return;

    AddColumn(hList, 0, 250, L"");
    ListView_SetExtendedListViewStyle(hList, LVS_EX_CHECKBOXES);

    /* Hard Drives */
    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        hHandle = SMART_Open(bIndex);
        if (!hHandle) continue;

        if (SMART_ReadDriveInformation(hHandle, bIndex, &DriveInfo))
        {
            ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                            sizeof(DriveInfo.sModelNumber));
            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sModelNumber);
            ChopSpaces(szText, sizeof(szText));

            if (SafeStrLen(szText) > 3)
            {
                ItemIndex = AddItem(hList, -1, szText);
                if (GetPrivateProfileInt(L"sensors", szText, 0, szIniPath) > 0)
                {
                    ListView_SetCheckState(hList, ItemIndex, TRUE);
                }
            }
        }

        SMART_Close(hHandle);
    }

    /* CPUs */
    if (GetCPUName(szText, sizeof(szText)))
    {
        ItemIndex = AddItem(hList, -1, szText);
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
        return;

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
                             SettingsInfo.SxSmIcon,
                             SettingsInfo.SySmIcon,
                             LR_CREATEDIBSECTION);

    Index = ImageList_AddIcon(hSettingsImageList, hIcon);
    DestroyIcon(hIcon);

    LoadMUIString(TextIndex, szText, MAX_STR_LEN);

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
        ImageList_Create(SettingsInfo.SxSmIcon,
                         SettingsInfo.SySmIcon,
                         SettingsInfo.SysColorDepth | ILC_MASK,
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
    BOOL ReInitCtrls = FALSE;

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
        if (SafeStrLen(SettingsInfo.szLangFile) > 0)
        {
            SettingsInfo.szLangFile[0] = 0;
            LoadLanguage();
            ReInitCtrls = TRUE;
        }
    }
    else
    {
        if (SafeStrCmp(LangFile, SettingsInfo.szLangFile) != 0)
        {
            SafeStrCpyN(SettingsInfo.szLangFile, LangFile, MAX_PATH);
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
        if (SafeStrLen(SettingsInfo.szIconsFile) > 0)
        {
             SettingsInfo.szIconsFile[0] = 0;
             LoadIcons();
             ReInitCtrls = TRUE;
        }
    }
    else
    {
        if (SafeStrCmp(IconFile, SettingsInfo.szIconsFile) != 0)
        {
            SafeStrCpyN(SettingsInfo.szIconsFile, IconFile, MAX_PATH);
            LoadIcons();
            ReInitCtrls = TRUE;
        }
    }

    if (ReInitCtrls) ReInitControls();

    SaveSensorsState(hSensorsList);

    /* Save settings to .ini file */
    SaveSettings();

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
            DebugTrace(L"Dialog init");

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
            DebugTrace(L"Dialog close");

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
