/*
 * PROJECT:         Aspia
 * FILE:            aspia/winmain.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"
#include "driver.h"


HINSTANCE hInstance = NULL;
HINSTANCE hIconsInst = NULL;
HINSTANCE hLangInst = NULL;
HWND hMainWnd = NULL;
HIMAGELIST hListViewImageList = NULL;
UINT CurrentCategory = IDS_CAT_SUMMARY;
BOOL bAscendingSort = TRUE;
HANDLE hFillThread = NULL;
HANDLE hProcessHeap = NULL;
CRITICAL_SECTION CriticalSection;
BOOL IsLoadingDone = TRUE;
BOOL IsCanceled = FALSE;
PARAMS_STRUCT ParamsInfo = {0};


static VOID
SetMainWindowTitle(UINT uiCategory)
{
    WCHAR szText[MAX_STR_LEN], szTitle[MAX_STR_LEN];

    LoadMUIString(IDS_APPTITLE, szTitle, MAX_STR_LEN);

    if (!LoadMUIString(uiCategory, szText, MAX_STR_LEN))
    {
        SetWindowText(hMainWnd, szTitle);
    }
    else
    {
        StringCbPrintf(szTitle, sizeof(szTitle),
                       L"%s - %s", szTitle, szText);
        SetWindowText(hMainWnd, szTitle);
    }
}

INT
ListViewAddCategoryItem(INT IconIndex, UINT Category)
{
    WCHAR szText[MAX_STR_LEN];
    LV_ITEM Item = {0};

    LoadMUIString(Category, szText, MAX_STR_LEN);

    Item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
    Item.pszText = szText;
    Item.iItem = ListView_GetItemCount(hListView);
    Item.iImage = IconIndex;
    Item.lParam = (LPARAM)Category;

    return ListView_InsertItem(hListView, &Item);
}

static VOID
ShowCategoriesTree(UINT Category)
{
    HTREEITEM hRootItem, hChildItem;
    TVITEM tvItem = {0};

    COLUMN_LIST CategoryColumnList[] =
    {
        { IDS_CATEGORY_COLUMN, 400 },
        { 0 }
    };

    ListViewClear();

    /* Add column */
    IoAddColumnsList(CategoryColumnList, 0, 0);

    /* Get current selected item */
    hRootItem = TreeView_GetSelection(hTreeView);
    if (!hRootItem) return;

    /* Get first child item */
    hChildItem = TreeView_GetChild(hTreeView, hRootItem);
    if (!hChildItem) return;

    /* Fill TVITEM structure */
    tvItem.mask = TVIF_IMAGE | TVIF_PARAM;
    tvItem.hItem = hChildItem;

    if (TreeView_GetItem(hTreeView, &tvItem))
    {
        if (Category == (UINT)tvItem.lParam)
        {
            TreeView_Expand(hTreeView, hChildItem, TVM_EXPAND);
            TreeView_Select(hTreeView, hChildItem, TVGN_CARET);
            ShowCategoriesTree(Category);
            return;
        }

        ListViewAddCategoryItem(tvItem.iImage, (UINT)tvItem.lParam);
    }

    /* Enumerate child items */
    for (;;)
    {
        hChildItem = TreeView_GetNextItem(hTreeView, hChildItem, TVGN_NEXT);
        if (!hChildItem) break;

        ZeroMemory(&tvItem, sizeof(TVITEM));

        tvItem.mask = TVIF_IMAGE | TVIF_PARAM;
        tvItem.hItem = hChildItem;

        if (TreeView_GetItem(hTreeView, &tvItem))
        {
            if (Category == (UINT)tvItem.lParam)
            {
                TreeView_Expand(hTreeView, hChildItem, TVM_EXPAND);
                TreeView_Select(hTreeView, hChildItem, TVGN_CARET);
                ShowCategoriesTree(Category);
                return;
            }

            ListViewAddCategoryItem(tvItem.iImage, (UINT)tvItem.lParam);
        }
    }

    ImageList_Destroy(hListViewImageList);
    hListViewImageList = ImageList_Duplicate(hImageTreeView);
    ListView_SetImageList(hListView,
                          hListViewImageList,
                          LVSIL_SMALL);
}

/* Функция проверяет есть ли у элемента дочернии элементы.
   Если есть, то возвращает TRUE */
BOOL
IsRootCategory(UINT Category, CATEGORY_LIST *List)
{
    SIZE_T Index = 0;

    do
    {
        if (List[Index].Child)
        {
            if (List[Index].StringID == Category)
                return TRUE;
            if (IsRootCategory(Category, List[Index].Child))
                return TRUE;
        }
    }
    while (List[++Index].StringID != 0);

    return FALSE;
}

VOID
GUIInfoThread(LPVOID lpParameter)
{
    UINT Category = (UINT)lpParameter;
    INT i, count = IoGetColumnsCount();

    if (!IsLoadingDone) return;

    EnterCriticalSection(&CriticalSection);

    SetMainWindowTitle(Category);

    if (CurrentCategory != Category)
    {
        WCHAR szName[15], szCol[3], szIniPath[MAX_PATH];

        GetIniFilePath(szIniPath, MAX_PATH);
        StringCbPrintf(szName, sizeof(szName), L"col-%d", CurrentCategory);

        for (i = 0; i < count; i++)
        {
            LVCOLUMN lvcol = {0};

            lvcol.mask = LVCF_WIDTH;

            ListView_GetColumn(hListView, i, &lvcol);
            StringCbPrintf(szCol, sizeof(szCol), L"%d", i);

            WritePrivateProfileInt(szName, szCol, lvcol.cx, szIniPath);
        }

        CurrentCategory = Category;
    }

    /* Enable ListView control */
    EnableWindow(hListView, TRUE);

    if (IsRootCategory(Category, RootCategoryList))
    {
        ShowCategoriesTree(Category);

        LeaveCriticalSection(&CriticalSection);
        _endthread();

        return;
    }

    /* Delete all items and columns */
    ListViewClear();

    IoRunInfoFunc(Category, RootCategoryList);

    if (ListView_GetItemCount(hListView) == 0)
        EnableWindow(hListView, FALSE);
    else
        EnableWindow(hListView, TRUE);

    LeaveCriticalSection(&CriticalSection);
    _endthread();
}

/* Функция для отображения всплывающего меню в текущем положении курсора */
VOID
ShowPopupMenu(UINT MenuID)
{
    HMENU hPopupMenu = GetSubMenu(LoadMenu(hLangInst, MAKEINTRESOURCE(MenuID)), 0);
    POINT pt;

    GetCursorPos(&pt);

    SetForegroundWindow(hMainWnd);
    TrackPopupMenu(hPopupMenu, 0, pt.x, pt.y, 0, hMainWnd, NULL);

    DestroyMenu(hPopupMenu);
}

static VOID
ListViewCopyText(INT StartColumn, INT EndColumn)
{
    INT ItemIndex = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
    WCHAR szText[MAX_STR_LEN * 3] = {0}, szFullText[MAX_STR_LEN * 3] = {0};
    INT Index;

    if (ItemIndex == -1) return;

    for (Index = StartColumn; Index < EndColumn; Index++)
    {
        ListView_GetItemText(hListView, ItemIndex, Index, szText, MAX_STR_LEN);
        StringCbCat(szFullText, sizeof(szFullText), szText);
        StringCbCat(szFullText, sizeof(szFullText), L" ");
    }

    CopyTextToClipboard(szFullText);
}

/* Функция проверяет сколько элементов в ListView выделено,
   если больше одного, то возвращает TRUE */
BOOL
IsMultiItemsSelected(VOID)
{
    INT Selections = 0, Count = ListView_GetItemCount(hListView) - 1;

    while (Count >= 0)
    {
        if (ListView_GetItemState(hListView, Count, LVIF_STATE) != 0)
            ++Selections;
        --Count;
    }

    if (ListView_GetSelectedCount(hListView) > 1 || Selections > 1)
        return TRUE;
    else
        return FALSE;
}

static VOID
ListViewCopySelectedStrings(VOID)
{
    INT Count;
    WCHAR szText[MAX_STR_LEN];
    SIZE_T Size, OldSize;
    WCHAR *pText = NULL;
    INT Index;

    for (Count = 0; Count < ListView_GetItemCount(hListView); Count++)
    {
        if (ListView_GetItemState(hListView, Count, LVIF_STATE) != 0 ||
            ListView_GetItemState(hListView, Count, LVIS_SELECTED) != 0)
        {
            for (Index = 0; Index < IoGetColumnsCount(); Index++)
            {
                ListView_GetItemText(hListView, Count, Index, szText, MAX_STR_LEN);

                if (!pText)
                {
                    Size = (wcslen(szText) + 1) * sizeof(WCHAR);
                    pText = (WCHAR*)Alloc(Size);
                    if (!pText)
                    {
                        DebugAllocFailed();
                        return;
                    }

                    StringCbCopy(pText, Size, szText);
                }
                else
                {
                    OldSize = HeapSize(hProcessHeap, 0, pText);
                    Size = OldSize + ((wcslen(szText) + 1) * sizeof(WCHAR));

                    pText = (WCHAR*)ReAlloc(pText, Size);
                    if (HeapSize(hProcessHeap, 0, pText) != OldSize)
                        StringCbCat(pText, Size, szText);
                }

                OldSize = HeapSize(hProcessHeap, 0, pText);
                Size = OldSize + (1 * sizeof(WCHAR));

                pText = (WCHAR*)ReAlloc(pText, Size);
                if (HeapSize(hProcessHeap, 0, pText) != OldSize)
                    StringCbCat(pText, Size, L" ");
            }

            OldSize = HeapSize(hProcessHeap, 0, pText);
            Size = OldSize + (2 * sizeof(WCHAR));

            pText = (WCHAR*)ReAlloc(pText, Size);
            if (HeapSize(hProcessHeap, 0, pText) != OldSize)
                StringCbCat(pText, Size, L"\r\n");
        }
    }

    CopyTextToClipboard(pText);

    Free(pText);
    pText = NULL;
}

static VOID
OnCommand(UINT Command)
{
    switch (Command)
    {
        case ID_SAVE:
            ShowPopupMenu(IDR_SAVE_POPUP);
            break;

        case ID_SAVE_ALL:
        {
            WCHAR szPath[MAX_PATH];

            if (ReportSaveFileDialog(hMainWnd, szPath, sizeof(szPath)))
                ReportSaveAll(TRUE, szPath, TRUE);
        }
        break;

        case ID_SAVE_CURRENT:
        {
            WCHAR szPath[MAX_PATH];

            if (ReportSaveFileDialog(hMainWnd, szPath, sizeof(szPath)))
                ReportSavePage(szPath, CurrentCategory);
        }
        break;

        case ID_SAVE_REPORT:
            CreateReportWindow();
            break;

        case ID_RELOAD:
            _beginthread(GUIInfoThread, 0, (LPVOID)CurrentCategory);
            break;

        case ID_SELECT_ALL:
            ListView_SetItemState(hListView, -1, 0xF, LVIF_STATE);
            break;

        case ID_COPY_ALL:
            ListViewCopySelectedStrings();
            break;

        case ID_COPY_VALUE:
            ListViewCopyText(1, IoGetColumnsCount());
            break;

        case ID_COPY_NAME:
            ListViewCopyText(0, 1);
            break;

        case ID_SHOW_WINDOW:
            RestoreMainWindowFromTray();
            break;

        case ID_HIDE_WINDOW:
        {
            if (SettingsInfo.ShowProgIcon)
                HideMainWindowToTray();
            else if (!SettingsInfo.ShowSensorIcons)
                HideMainWindowToTray();
            else
            {
                ShowWindow(hMainWnd, SW_HIDE);
                IsMainWindowHiden = TRUE;
            }
        }
        break;

        case ID_SYSMON:
            CreateSysMonWindow();
            break;

        case ID_BENCH:
        {
            WCHAR szPath[MAX_PATH];

            if (GetCurrentPath(szPath, MAX_PATH))
            {
                StringCbCat(szPath, sizeof(szPath), L"diskbench.exe");
                ShellExecute(NULL, NULL, szPath, NULL, NULL, SW_SHOW);
            }
        }
        break;

        case ID_EXIT:
            PostMessage(hMainWnd, WM_CLOSE, 0, 0);
            break;

        case ID_ABOUT:
            DialogBox(hLangInst,
                      MAKEINTRESOURCE(IDD_ABOUT_DIALOG),
                      hMainWnd,
                      AboutDlgProc);
            break;

        case ID_SETTINGS:
            DialogBox(hLangInst,
                      MAKEINTRESOURCE(IDD_SETTINGS_DIALOG),
                      hMainWnd,
                      SettingsDlgProc);
            break;
    }
}

LRESULT CALLBACK
MainWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_CREATE:
            InitControls(hwnd);
            _beginthread(GUIInfoThread, 0, (LPVOID)SettingsInfo.StartupCategory);
            break;

        case WM_COMMAND:
            OnCommand(LOWORD(wParam));
            break;

        case WM_NOTIFY:
        {
            LPNMHDR data = (LPNMHDR)lParam;

            switch (data->code)
            {
                case NM_CUSTOMDRAW:
                {
                    NMLVCUSTOMDRAW *nmlvcd = (NMLVCUSTOMDRAW*)lParam;

                    if (data->hwndFrom != hListView) break;

                    if (nmlvcd->nmcd.dwDrawStage == CDDS_PREPAINT)
                    {
                        return CDRF_NOTIFYITEMDRAW;
                    }
                    else if (nmlvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
                    {
                        if (nmlvcd->nmcd.dwItemSpec & 1)
                        {
                            nmlvcd->clrTextBk = 0xF3F3F3;
                            return CDRF_NEWFONT;
                        }
                        else
                        {
                            nmlvcd->clrTextBk = 0xFFFFFF;
                            return CDRF_NEWFONT;
                        }
                    }
                }
                break;

                case NM_RCLICK:
                {
                    if (data->hwndFrom == hListView)
                    {
                        if (!IsRootCategory(CurrentCategory, RootCategoryList))
                        {
                            if (IsMultiItemsSelected())
                                ShowPopupMenu(IDR_COPY_ALL_POPUP);
                            else
                                ShowPopupMenu(IDR_POPUP);
                        }
                    }
                }
                break;

                case NM_CLICK:
                {
                    /* Unselect items */
                    if (data->hwndFrom == hListView)
                        ListView_SetItemState(hListView, -1, 0, LVIF_STATE);
                }
                break;

                case NM_DBLCLK:
                {
                    if (data->hwndFrom == hListView)
                    {
                        if (IsRootCategory(CurrentCategory, RootCategoryList))
                            _beginthread(GUIInfoThread, 0,
                                         (LPVOID)ListViewGetlParam(hListView, -1));
                        else
                            ShowPopupMenu(IDR_POPUP);
                    }
                }
                break;

                case TTN_GETDISPINFO:
                    ToolBarOnGetDispInfo((LPTOOLTIPTEXT)lParam);
                    break;

                case TVN_SELCHANGED:
                {
                    if (data->hwndFrom == hTreeView)
                    {
                        UINT Category = (UINT)((LPNMTREEVIEW)lParam)->itemNew.lParam;

                        IsCanceled = TRUE;

                        if (CurrentCategory == Category) break;
                        _beginthread(GUIInfoThread, 0, (LPVOID)Category);
                    }
                }
                break;
            }
        }
        break;

        case WM_NOTIFYICONMSG_WINDOW:
        case WM_NOTIFYICONMSG_SENSORS:
        {
            switch (lParam)
            {
                case WM_RBUTTONDOWN:
                    ShowPopupMenu(IsMainWindowHiden ? IDR_SHOW_TRAY_POPUP : IDR_HIDE_TRAY_POPUP);
                    break;
                case WM_LBUTTONDOWN:
                    if (Msg == WM_NOTIFYICONMSG_WINDOW)
                    {
                        if (IsMainWindowHiden)
                        {
                            if (SettingsInfo.ShowProgIcon)
                            {
                                ShowWindow(hMainWnd, SW_SHOW);
                                IsMainWindowHiden = FALSE;
                            }
                            else
                            {
                                RestoreMainWindowFromTray();
                            }
                        }
                        else
                        {
                            HideMainWindowToTray();
                        }
                    }
                    break;
            }
        }
        break;

        case WM_SIZE:
            MainWndOnSize(lParam);
            break;

        case WM_SIZING:
        {
            LPRECT pRect = (LPRECT)lParam;

            if (pRect->right - pRect->left < 400)
                pRect->right = pRect->left + 400;

            if (pRect->bottom - pRect->top < 200)
                pRect->bottom = pRect->top + 200;
        }
        break;

        case WM_SETTINGCHANGE:
        {
            INT SxSmIcon = GetSystemMetrics(SM_CXSMICON);
            INT SySmIcon = GetSystemMetrics(SM_CYSMICON);
            INT SysColorDepth = drv_get_system_color_depth();

            if (ParamsInfo.SxSmIcon != SxSmIcon ||
                ParamsInfo.SySmIcon != SySmIcon ||
                ParamsInfo.SysColorDepth != SysColorDepth)
            {
                ParamsInfo.SxSmIcon = SxSmIcon;
                ParamsInfo.SySmIcon = SySmIcon;
                ParamsInfo.SysColorDepth = SysColorDepth;

                ReInitControls();
            }
        }
        break;

        case WM_SYSCOLORCHANGE:
        {
            /* Forward WM_SYSCOLORCHANGE to common controls */
            SendMessage(hListView, WM_SYSCOLORCHANGE, 0, 0);
            SendMessage(hTreeView, WM_SYSCOLORCHANGE, 0, 0);
            SendMessage(hToolBar, WM_SYSCOLORCHANGE, 0, 0);
        }
        break;

        case WM_DESTROY:
        {
            SaveSettings();

            DeleteTraySensors();
            DeleteMainWindowFromTray();

            if (hImageTreeView) ImageList_Destroy(hImageTreeView);
            if (hListViewImageList) ImageList_Destroy(hListViewImageList);

            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, Msg, wParam, lParam);
}

/* Функция для обработки аргументов командной строки
 * Если успешно получилось обработать, то возвращает
 * FALSE
 */
BOOL
HandleCommandLine(VOID)
{
    LPWSTR *lpCmd, ptr;
    WCHAR szPath[MAX_PATH];
    BOOL bNavMenu = TRUE;
    INT NumArgs;
    SIZE_T Index = 1;

    lpCmd = CommandLineToArgvW(GetCommandLine(), &NumArgs);
    if (!lpCmd || NumArgs < 2) return FALSE;

    ptr = lpCmd[Index];

    for (;;)
    {
        if (*ptr == L'-' || *ptr == L'/')
        {
            ++ptr;

            if ((wcscmp(ptr, L"help") == 0) ||
                (wcscmp(ptr, L"h") == 0) ||
                (wcscmp(ptr, L"?") == 0))
            {
                WCHAR szText[MAX_STR_LEN * 2], szTitle[MAX_STR_LEN];

                LoadMUIString(IDS_INFORMATION, szTitle, MAX_STR_LEN);
                LoadMUIString(IDS_CMD_HELP, szText, MAX_STR_LEN);
                MessageBox(0, szText, szTitle, MB_OK | MB_ICONINFORMATION);
                return TRUE;
            }
            else if (wcscmp(ptr, L"debug") == 0)
            {
                ParamsInfo.DebugMode = TRUE;
            }
            else if (wcscmp(ptr, L"nonav") == 0)
            {
                bNavMenu = FALSE;
            }
        }
        else
        {
            StringCbCopy(szPath, sizeof(szPath), ptr);
        }

        if (++Index >= (SIZE_T)NumArgs) break;
        ptr = lpCmd[Index];
    }

    if (ParamsInfo.DebugMode && NumArgs == 2)
        return FALSE;

    drv_load();

    ReportSaveAll(FALSE, szPath, bNavMenu);

    return TRUE;
}

BOOL
IsIniFileExists(VOID)
{
    WCHAR szIniPath[MAX_PATH];

    if (!GetIniFilePath(szIniPath, MAX_PATH))
        return FALSE;

    if (GetFileAttributes(szIniPath) == INVALID_FILE_ATTRIBUTES)
        return FALSE;

    return TRUE;
}

BOOL
GetLangFileNameFromSystem(LPWSTR lpFileName, SIZE_T Size)
{
    WCHAR szIso639[MAX_STR_LEN];
    WCHAR szIso3166[MAX_STR_LEN];

    if (!GetLocaleInfo(LOCALE_USER_DEFAULT,
                       LOCALE_SISO639LANGNAME,
                       szIso639, MAX_STR_LEN))
       return FALSE;

    if (!GetLocaleInfo(LOCALE_USER_DEFAULT,
                       LOCALE_SISO3166CTRYNAME,
                       szIso3166, MAX_STR_LEN))
       return FALSE;

    StringCbPrintf(lpFileName, Size, L"%s-%s.dll",
                   szIso639, szIso3166);

    return TRUE;
}

VOID
LoadLanguage(VOID)
{
    if (hLangInst && hLangInst != hInstance)
        FreeLibrary(hLangInst);

    if (ThemesInfo.szLangFile[0] == 0 && IsIniFileExists())
    {
        hLangInst = hInstance;
    }
    else
    {
        WCHAR szPath[MAX_PATH];

        if (!IsIniFileExists())
        {
            if (!GetLangFileNameFromSystem(ThemesInfo.szLangFile,
                                           sizeof(ThemesInfo.szLangFile)))
            {
                hLangInst = hInstance;
                return;
            }
        }
        else
        {
            hLangInst = hInstance;
        }

        StringCbCopy(szPath, sizeof(szPath), ParamsInfo.szCurrentPath);

        StringCbCat(szPath, sizeof(szPath), L"languages\\");
        StringCbCat(szPath, sizeof(szPath), ThemesInfo.szLangFile);

        DebugTrace(L"Loading language file: %s", szPath);
        
        hLangInst = LoadLibraryEx(szPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (!hLangInst) hLangInst = hInstance;
    }
}

VOID
LoadIcons(VOID)
{
    if (hIconsInst && hIconsInst != hInstance)
        FreeLibrary(hIconsInst);

    if (ThemesInfo.szIconsFile[0] == 0)
    {
        hIconsInst = hInstance;
    }
    else
    {
        WCHAR szPath[MAX_PATH];

        StringCbPrintf(szPath, sizeof(szPath),
                       L"%sicons\\%s",
                       ParamsInfo.szCurrentPath,
                       ThemesInfo.szIconsFile);

        DebugTrace(L"Loading icon file: %s", szPath);

        hIconsInst = LoadLibraryEx(szPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (!hIconsInst) hIconsInst = hInstance;
    }
}

/* Функция для установки привилегий для текущего процесса */
VOID
SetPrivilege(LPCTSTR lpName)
{
    HANDLE hToken;

    if (OpenProcessToken(GetCurrentProcess(),
                         TOKEN_ALL_ACCESS,
                         &hToken))
    {
        TOKEN_PRIVILEGES tkp;
        LUID Value;

        if (LookupPrivilegeValue(NULL, lpName, &Value))
        {
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Luid = Value;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL);
        }

        CloseHandle(hToken);
    }
}

INT WINAPI
wWinMain(HINSTANCE hInst,
         HINSTANCE hPrevInstance,
         LPWSTR lpCmdLine,
         INT nShowCmd)
{
    WNDCLASSEX WndClass = {0};
    WCHAR szWindowClass[] = L"ASPIAIS";
    WCHAR szWindowName[MAX_STR_LEN];
    HANDLE hMutex;
    HACCEL hAccel;
    MSG Msg;

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    /* Пытаемся создать Mutex, если не получилось,
       то Aspia уже запущена */
    hMutex = CreateMutex(NULL, FALSE, szWindowClass);
    if (!hMutex || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        /* If already started, it is found its window */
        HWND hWindow = FindWindow(szWindowClass, NULL);

        if (hWindow)
        {
            /* Activate window */
            ShowWindow(hWindow, SW_SHOWNORMAL);
            SetForegroundWindow(hWindow);
        }
        return 1;
    }

    /* Устанавливаем для текущего процесса отладочные привилегии */
    SetPrivilege(SE_DEBUG_NAME);

    /* Load settings from .ini file */
    if (!LoadSettings())
    {
        DebugTrace(L"LoadSettings() failed");

        /* If loading filed fill default values */

        SettingsInfo.StartupCategory = IDS_CAT_SUMMARY;

        /* Windows Position */
        SettingsInfo.SaveWindowPos = TRUE;

        SettingsInfo.Bottom = 660;
        SettingsInfo.Left = 20;
        SettingsInfo.Right = 870;
        SettingsInfo.Top = 20;
        SettingsInfo.IsMaximized = FALSE;

        SettingsInfo.SplitterPos = 250;

        SettingsInfo.ReportBottom = 409;
        SettingsInfo.ReportLeft = 20;
        SettingsInfo.ReportRight = 342;
        SettingsInfo.ReportTop = 20;
        SettingsInfo.ReportIsMaximized = FALSE;

        /* Autorun */
        SettingsInfo.Autorun = FALSE;
        SettingsInfo.HideToTray = FALSE;
        SettingsInfo.ShowProgIcon = FALSE;
        SettingsInfo.ShowSensorIcons = FALSE;

        SettingsInfo.AllowKmDriver = TRUE;
        SettingsInfo.StayOnTop = FALSE;

        /* Report: Content Filtering */
        SettingsInfo.ELogShowError = TRUE;
        SettingsInfo.ELogShowWarning = TRUE;
        SettingsInfo.ELogShowInfo = FALSE;
        SettingsInfo.IEShowFile = TRUE;
        SettingsInfo.IEShowFtp = TRUE;
        SettingsInfo.IEShowHttp = TRUE;

        /* Sensors Icons */
        SettingsInfo.CpuBackground = 0x80ffff;
        SettingsInfo.CpuFontColor = 0;
        SettingsInfo.HddBackground = 0;
        SettingsInfo.HddFontColor = 0xffffff;

        SettingsInfo.SensorsRefreshRate = 30;

        SettingsInfo.IsAddContent = TRUE;
        SettingsInfo.ReportFileType = IDS_TYPE_HTML;

        ParamsInfo.DebugMode = FALSE;
    }
    ParamsInfo.SxSmIcon = GetSystemMetrics(SM_CXSMICON);
    ParamsInfo.SySmIcon = GetSystemMetrics(SM_CYSMICON);
    ParamsInfo.SysColorDepth = drv_get_system_color_depth();

    GetCurrentPath(ParamsInfo.szCurrentPath, MAX_PATH);

    hInstance = hInst;

    LoadLanguage();
    LoadIcons();

    hProcessHeap = GetProcessHeap();

    /* Инициализируем критическую секцию. Она используется
       для синхронизации потоков получнеия информации */
    InitializeCriticalSection(&CriticalSection);

    /* Пытаемся обработать аргументы командной строки */
    if (HandleCommandLine())
        goto Exit;

    /* если у нас дебаг билд, то всегда включаем DebugMode */
#ifdef _DEBUG
    ParamsInfo.DebugMode = TRUE;
#endif
    if (ParamsInfo.DebugMode)
        ParamsInfo.DebugMode = drv_init_debug_log(L"aspia.log", VER_FILEVERSION_STR);

    DebugTrace(L"Start with debug mode");

    /* Загружаем драйвер режима ядра */
    drv_load();

    InitCommonControls();

    DetectUnknownDevices();

    /* Загружаем ускорители для обработки горячих клавишь */
    hAccel = LoadAccelerators(hInstance,
                              MAKEINTRESOURCE(IDC_ACCELERATORS));

    /* Create the window */
    WndClass.cbSize        = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = szWindowClass;
    WndClass.lpfnWndProc   = MainWindowProc;
    WndClass.hInstance     = hInstance;
    WndClass.style         = CS_HREDRAW | CS_VREDRAW;
    WndClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.lpszMenuName  = NULL;

    if (RegisterClassEx(&WndClass) == (ATOM)0) goto Exit;

    LoadMUIString(IDS_APPTITLE, szWindowName, MAX_STR_LEN);

    /* Создаем главное окно программы */
    hMainWnd = CreateWindowEx(WS_EX_WINDOWEDGE,
                              szWindowClass,
                              szWindowName,
                              WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                              20, 20, 850, 640,
                              NULL, NULL, hInstance, NULL);

    if (!hMainWnd) goto Exit;

    if (SettingsInfo.StayOnTop)
    {
        SetWindowPos(hMainWnd,
                     HWND_TOPMOST,
                     20, 20, 850, 640,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }

    if (SettingsInfo.ShowSensorIcons)
    {
        AddTraySensors();
    }

    if (SettingsInfo.HideToTray)
    {
        if (SettingsInfo.ShowProgIcon)
            HideMainWindowToTray();
        else if (!SettingsInfo.ShowSensorIcons)
            HideMainWindowToTray();
        else
        {
            ShowWindow(hMainWnd, SW_HIDE);
            IsMainWindowHiden = TRUE;
        }
    }
    else
    {
        if (SettingsInfo.ShowProgIcon)
            AddMainWindowToTray();

        /* Show it */
        ShowWindow(hMainWnd, SW_SHOW);
        UpdateWindow(hMainWnd);
    }

    /* Message Loop */
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(hMainWnd, hAccel, &Msg))
        {
            if (!IsDialogMessage(hMainWnd, &Msg))
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
    }

    drv_unload();

    DeleteCriticalSection(&CriticalSection);

Exit:
    if (hMutex) CloseHandle(hMutex);

    drv_close_debug_log();

    return 0;
}
