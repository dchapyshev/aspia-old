/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/controls.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"


HWND hToolBar = NULL;
HWND hTreeView = NULL;
HWND hListView = NULL;
HWND hSplitter = NULL;

static WCHAR szSaveReportBtn[MAX_STR_LEN] = {0};
static WCHAR szReloadBtn[MAX_STR_LEN] = {0};
static WCHAR szSysMonBtn[MAX_STR_LEN] = {0};
static WCHAR szBenchBtn[MAX_STR_LEN] = {0};

HIMAGELIST hImageTreeView = NULL;

/* Toolbar buttons */
static const TBBUTTON Buttons[] =
{   /* iBitmap, idCommand, fsState, fsStyle, bReserved[2], dwData, iString */
    { 0, ID_SAVE,     TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szSaveReportBtn},
    { 1, ID_RELOAD,   TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szReloadBtn},
    {-1, 0,           TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
    //{ 3, ID_SYSMON,   TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szSysMonBtn},
    //{ 4, ID_BENCH,    TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szBenchBtn},
    //{-1, 0,           TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
    { 2, ID_SETTINGS, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, 0},
    {-1, 0,           TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
    { 3, ID_HELP_MENU,TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, 0}
};


VOID
ListViewClear(VOID)
{
    INT i;

    ListView_DeleteAllItems(hListView);

    ImageList_RemoveAll(hListViewImageList);
    ImageList_Destroy(hListViewImageList);

    hListViewImageList = ImageList_Create(ParamsInfo.SxSmIcon,
                                          ParamsInfo.SySmIcon,
                                          ILC_MASK | ParamsInfo.SysColorDepth,
                                          1, 1);

    ListView_SetImageList(hListView,
                          hListViewImageList,
                          LVSIL_SMALL);

    InitInfoDll();

    for (i = IoGetColumnsCount(); i >= 0; --i)
    {
        ListView_DeleteColumn(hListView, i);
    }

    IoSetColumnsCount(0);
}

/* Callback for vertical splitter bar */
static LRESULT CALLBACK
SplitterWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_LBUTTONDOWN:
            SetCapture(hwnd);
        break;

        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
            if (GetCapture() == hwnd)
            {
                ReleaseCapture();
            }
        break;

        case WM_MOUSEMOVE:
            if (GetCapture() == hwnd)
            {
                HDWP hdwp;
                POINT Point;

                GetCursorPos(&Point);
                ScreenToClient(hMainWnd, &Point);

                if ((GetClientWindowWidth(hMainWnd) - SPLIT_WIDTH) < Point.x)
                    break;

                if (SPLIT_WIDTH > Point.x)
                    break;

                hdwp = BeginDeferWindowPos(3);

                /* Size SplitterBar */
                DeferWindowPos(hdwp,
                               hwnd,
                               0,
                               Point.x,
                               GetWindowHeight(hToolBar),
                               SPLIT_WIDTH,
                               GetClientWindowHeight(hMainWnd) -
                                   GetWindowHeight(hToolBar),
                               SWP_NOZORDER|SWP_NOACTIVATE);

                /* Size TreeView */
                DeferWindowPos(hdwp,
                               hTreeView,
                               0,
                               0,
                               GetWindowHeight(hToolBar),
                               Point.x,
                               GetClientWindowHeight(hMainWnd) -
                                   GetWindowHeight(hToolBar),
                               SWP_NOZORDER|SWP_NOACTIVATE);

                /* Size ListView */
                DeferWindowPos(hdwp,
                               hListView,
                               0,
                               Point.x + SPLIT_WIDTH,
                               GetWindowHeight(hToolBar),
                               GetClientWindowWidth(hMainWnd) -
                                   (Point.x + SPLIT_WIDTH),
                               GetClientWindowHeight(hMainWnd) -
                                   GetWindowHeight(hToolBar),
                               SWP_NOZORDER|SWP_NOACTIVATE);

                EndDeferWindowPos(hdwp);
            }
        break;
    }

    return DefWindowProc(hwnd, Msg, wParam, lParam);
}

VOID
MainWndOnSize(LPARAM lParam)
{
    HDWP hdwp = BeginDeferWindowPos(3);
    INT VSplitterPos;

    /* Size tool bar */
    SendMessage(hToolBar, TB_AUTOSIZE, 0, 0);

    /*
     * HIWORD(lParam) - Height of main window
     * LOWORD(lParam) - Width of main window
     */

    /* Size vertical splitter bar */
    DeferWindowPos(hdwp,
                   hSplitter,
                   0,
                   (VSplitterPos = GetWindowWidth(hTreeView)),
                   GetWindowHeight(hToolBar),
                   SPLIT_WIDTH,
                   HIWORD(lParam) - GetWindowHeight(hToolBar),
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size TreeView */
    DeferWindowPos(hdwp,
                   hTreeView,
                   0,
                   0,
                   GetWindowHeight(hToolBar),
                   VSplitterPos,
                   HIWORD(lParam) - GetWindowHeight(hToolBar),
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size ListView */
    DeferWindowPos(hdwp,
                   hListView,
                   0,
                   VSplitterPos + SPLIT_WIDTH,
                   GetWindowHeight(hToolBar),
                   LOWORD(lParam) - (VSplitterPos + SPLIT_WIDTH),
                   HIWORD(lParam) - GetWindowHeight(hToolBar),
                   SWP_NOZORDER|SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);
}

VOID
ToolBarOnGetDispInfo(LPTOOLTIPTEXT lpttt)
{
    UINT idButton = (UINT)lpttt->hdr.idFrom;
    UINT StringID = 0;

    switch (idButton)
    {
        case ID_SAVE:
            StringID = IDS_SAVE;
            break;
        case ID_RELOAD:
            StringID = IDS_RELOAD;
            break;
        case ID_HELP_MENU:
            StringID = IDS_HELP;
            break;
        case ID_SETTINGS:
            StringID = IDS_SETTINGS;
            break;
        case ID_SYSMON:
            StringID = IDS_SYSMON;
            break;
        case ID_BENCH:
            StringID = IDS_HWBENCH;
            break;
    }

    LoadMUIStringF(hLangInst, StringID, lpttt->szText, 80);
}

HTREEITEM
AddCategory(HWND hTree,
            HIMAGELIST hImageList,
            HTREEITEM hRootItem,
            UINT TextIndex,
            UINT IconIndex)
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

    Index = ImageList_AddIcon(hImageList, hIcon);
    DestroyIcon(hIcon);

    LoadMUIStringF(hLangInst, TextIndex, szText, MAX_STR_LEN);

    Insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    Insert.hInsertAfter = TVI_LAST;
    Insert.hParent = hRootItem;
    Insert.item.iSelectedImage = Index;
    Insert.item.iImage = Index;
    Insert.item.lParam = (LPARAM)TextIndex;
    Insert.item.pszText = szText;

    return TreeView_InsertItem(hTree, &Insert);
}

HTREEITEM
InitCategoriesList(CATEGORY_LIST *List, HTREEITEM hRoot)
{
    HTREEITEM hSelected = NULL;
    SIZE_T Index = 0;

    do
    {
        List[Index].hTreeItem = AddCategory(hTreeView,
                                            hImageTreeView,
                                            hRoot,
                                            List[Index].StringID,
                                            List[Index].IconID);
        if (List[Index].StringID == SettingsInfo.StartupCategory)
            hSelected = List[Index].hTreeItem;

        if (List[Index].Child)
        {
            HTREEITEM hResult;

            hResult = InitCategoriesList(List[Index].Child,
                                         List[Index].hTreeItem);
            if (hResult) hSelected = hResult;
        }
    }
    while (List[++Index].StringID != 0);

    return hSelected;
}

static VOID
ExpandCategoriesList(CATEGORY_LIST *List)
{
    SIZE_T Index = 0;

    do
    {
        if (List[Index].Expanded)
            TreeView_Expand(hTreeView, List[Index].hTreeItem, TVE_EXPAND);

        if (List[Index].Child)
            ExpandCategoriesList(List[Index].Child);
    }
    while (List[++Index].StringID != 0);
}

static BOOL
InitToolBar(HWND hwnd)
{
    SIZE_T NumButtons = sizeof(Buttons) / sizeof(Buttons[0]);
    HIMAGELIST hImageList;

    LoadMUIStringF(hLangInst, IDS_RELOAD, szReloadBtn, MAX_STR_LEN);
    LoadMUIStringF(hLangInst, IDS_SAVE, szSaveReportBtn, MAX_STR_LEN);
    LoadMUIStringF(hLangInst, IDS_SYSMON_BTN, szSysMonBtn, MAX_STR_LEN);
    LoadMUIStringF(hLangInst, IDS_BENCH_BTN, szBenchBtn, MAX_STR_LEN);

    /* Create toolbar */
    hToolBar = CreateWindowEx(0,
                              TOOLBARCLASSNAME,
                              NULL,
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBSTYLE_FLAT |
                              TBSTYLE_TOOLTIPS | TBSTYLE_LIST,
                              0, 40,
                              340, 30,
                              hwnd,
                              NULL,
                              hInstance,
                              NULL);
    if (!hToolBar)
    {
        DebugTrace(L"CreateWindowEx() failed!");
        return FALSE;
    }

    SendMessage(hToolBar,
                TB_BUTTONSTRUCTSIZE,
                sizeof(Buttons[0]),
                0);

    /* Create image list for ToolBar */
    hImageList = ImageList_Create(TOOLBAR_HEIGHT,
                                  TOOLBAR_HEIGHT,
                                  ILC_MASK | ParamsInfo.SysColorDepth,
                                  1, 1);
    if (!hImageList)
    {
        DebugTrace(L"ImageList_Create() failed!");
        return FALSE;
    }

    /* Add images to ImageList */
    AddIconToImageList(hIconsInst, hImageList, IDI_SAVE);
    AddIconToImageList(hIconsInst, hImageList, IDI_RELOAD);
    //AddIconToImageList(hIconsInst, hImageList, IDI_TASKMGR);
    //AddIconToImageList(hIconsInst, hImageList, IDI_HWBENCH);
    AddIconToImageList(hIconsInst, hImageList, IDI_SETTINGS);
    AddIconToImageList(hIconsInst, hImageList, IDI_INFO);

    ImageList_Destroy((HIMAGELIST)SendMessage(hToolBar,
                                              TB_SETIMAGELIST,
                                              0,
                                              (LPARAM)hImageList));

    SendMessage(hToolBar, TB_ADDBUTTONS, NumButtons, (LPARAM)Buttons);

    return TRUE;
}

VOID
ReInitControls(VOID)
{
    HTREEITEM hSelectedItem;

    IsLoadingDone = FALSE;

    EnterCriticalSection(&CriticalSection);

    ListViewClear();

    TreeView_DeleteAllItems(hTreeView);
    ImageList_Destroy(hImageTreeView);

    DestroyWindow(hToolBar);

    /* Create image list */
    hImageTreeView = ImageList_Create(ParamsInfo.SxSmIcon,
                                      ParamsInfo.SySmIcon,
                                      ParamsInfo.SysColorDepth | ILC_MASK,
                                      0, 1);
    hSelectedItem = InitCategoriesList(RootCategoryList, TVI_ROOT);
    ExpandCategoriesList(RootCategoryList);
    TreeView_SelectItem(hTreeView, hSelectedItem);

    TreeView_SetImageList(hTreeView, hImageTreeView, TVSIL_NORMAL);

    InitToolBar(hMainWnd);
    SendMessage(hToolBar, TB_AUTOSIZE, 0, 0);

    LeaveCriticalSection(&CriticalSection);

    IsLoadingDone = TRUE;

    _beginthread(GUIInfoThread, 0, (LPVOID)SettingsInfo.StartupCategory);
}

BOOL
InitInfoDll(VOID)
{
    ASPIA_DLL_PARAMS DllParams;

    DllParams.ELogShowError   = SettingsInfo.ELogShowError;
    DllParams.ELogShowInfo    = SettingsInfo.ELogShowInfo;
    DllParams.ELogShowWarning = SettingsInfo.ELogShowWarning;

    DllParams.IEShowFile = SettingsInfo.IEShowFile;
    DllParams.IEShowFtp  = SettingsInfo.IEShowFtp;
    DllParams.IEShowHttp = SettingsInfo.IEShowHttp;

    DllParams.hMainWnd  = hMainWnd;
    DllParams.hListView = hListView;

    DllParams.hLangInst  = hLangInst;
    DllParams.hIconsInst = hIconsInst;

    DllParams.hListImgList = &hListViewImageList;

    DllParams.IoAddFooter       = IoAddFooter;
    DllParams.IoAddHeader       = IoAddHeader;
    DllParams.IoAddHeaderString = IoAddHeaderString;
    DllParams.IoAddIcon         = IoAddIcon;
    DllParams.IoAddItem         = IoAddItem;
    DllParams.IoAddValueName    = IoAddValueName;
    DllParams.IoGetTarget       = IoGetTarget;
    DllParams.IoSetItemText     = IoSetItemText;
    DllParams.IoGetCurrentItemIndex = IoGetCurrentItemIndex;

    StringCbCopy(DllParams.szCurrentPath,
                 sizeof(DllParams.szCurrentPath),
                 ParamsInfo.szCurrentPath);

    DllParams.SxSmIcon      = ParamsInfo.SxSmIcon;
    DllParams.SySmIcon      = ParamsInfo.SySmIcon;
    DllParams.SysColorDepth = ParamsInfo.SysColorDepth;

    return AspiaDllInitialize(&DllParams);
}

BOOL
InitControls(HWND hwnd)
{
    WCHAR szSplitWndClass[] = L"SplitterWindowClass";
    WNDCLASSEX SplitWndClass = {0};
    HTREEITEM hSelectedItem;

    if (!InitToolBar(hwnd))
        return FALSE;

    hTreeView = CreateWindowEx(WS_EX_CLIENTEDGE,
                               WC_TREEVIEW,
                               L"",
                               WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASLINES |
                               TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_LINESATROOT,
                               0, 28, SettingsInfo.SplitterPos, 350,
                               hwnd,
                               NULL,
                               hInstance,
                               NULL);

    if (!hTreeView)
    {
        DebugTrace(L"CreateWindowEx() failed!");
        return FALSE;
    }

    SendMessage(hTreeView, TVM_SETEXTENDEDSTYLE,
                (WPARAM)hTreeView, TVS_EX_DOUBLEBUFFER);

    /* Create image list */
    hImageTreeView = ImageList_Create(ParamsInfo.SxSmIcon,
                                      ParamsInfo.SySmIcon,
                                      ParamsInfo.SysColorDepth | ILC_MASK,
                                      0, 1);
    if (!hImageTreeView)
    {
        DebugTrace(L"ImageList_Create() failed!");
        return FALSE;
    }

    hSelectedItem = InitCategoriesList(RootCategoryList, TVI_ROOT);
    ExpandCategoriesList(RootCategoryList);
    TreeView_SelectItem(hTreeView, hSelectedItem);

    TreeView_SetImageList(hTreeView, hImageTreeView, TVSIL_NORMAL);

    hListView = CreateWindowEx(WS_EX_CLIENTEDGE,
                               WC_LISTVIEW,
                               L"",
                               WS_CHILD | WS_VISIBLE | LVS_REPORT |
                                   WS_TABSTOP | LVS_SHOWSELALWAYS,
                               205, 28, 465, 350,
                               hwnd,
                               NULL,
                               hInstance,
                               NULL);

    if (!hListView)
    {
        DebugTrace(L"CreateWindowEx() failed!");
        return FALSE;
    }

    ListView_SetExtendedListViewStyle(hListView,
                                      LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    hListViewImageList = ImageList_Create(ParamsInfo.SxSmIcon,
                                          ParamsInfo.SySmIcon,
                                          ILC_MASK | ParamsInfo.SysColorDepth,
                                          1, 1);
    if (!hListViewImageList)
    {
        DebugTrace(L"ImageList_Create() failed!");
        return FALSE;
    }

    ListView_SetImageList(hListView,
                          hListViewImageList,
                          LVSIL_SMALL);

    /* Create vertical splitter bar */
    SplitWndClass.cbSize        = sizeof(WNDCLASSEX);
    SplitWndClass.lpszClassName = szSplitWndClass;
    SplitWndClass.lpfnWndProc   = SplitterWindowProc;
    SplitWndClass.hInstance     = hInstance;
    SplitWndClass.style         = CS_HREDRAW | CS_VREDRAW;
    SplitWndClass.hCursor       = LoadCursor(0, IDC_SIZEWE);
    SplitWndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (RegisterClassEx(&SplitWndClass) == (ATOM)0)
    {
        DebugTrace(L"RegisterClassEx() failed!");
        return FALSE;
    }

    hSplitter = CreateWindowEx(WS_EX_TRANSPARENT,
                               szSplitWndClass,
                               NULL,
                               WS_CHILD | WS_VISIBLE,
                               SettingsInfo.SplitterPos, 28, SPLIT_WIDTH, 350,
                               hwnd,
                               NULL,
                               hInstance,
                               NULL);

    if (!hSplitter)
    {
        DebugTrace(L"CreateWindowEx() failed!");
        return FALSE;
    }

    ShowWindow(hSplitter, SW_SHOW);
    UpdateWindow(hSplitter);

    IntSetWindowTheme(hTreeView);
    IntSetWindowTheme(hListView);

    SetFocus(hListView);

    return TRUE;
}

INT_PTR CALLBACK
AboutDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    WCHAR szText[MAX_STR_LEN];
    HICON hIcon = NULL;

    UNREFERENCED_PARAMETER(lParam);

    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            WCHAR szFormat[MAX_STR_LEN], szInfo[MAX_STR_LEN * 5];

            LoadMUIStringF(hLangInst, IDS_ABOUT_STRING,
                           szInfo, sizeof(szInfo)/sizeof(WCHAR));
            SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_EDIT), szInfo);

            if (GetWindowText(GetDlgItem(hDlg, IDC_VERSION_TEXT),
                              szFormat, MAX_STR_LEN))
            {
                StringCbPrintf(szText, sizeof(szText),
                               szFormat, VER_FILEVERSION_STR);
                SetWindowText(GetDlgItem(hDlg, IDC_VERSION_TEXT), szText);
            }

            hIcon = (HICON)LoadImage(hInstance,
                                     MAKEINTRESOURCE(IDI_MAINICON),
                                     IMAGE_ICON, 48, 48,
                                     LR_DEFAULTCOLOR);
            if (hIcon)
            {
                SendMessage(GetDlgItem(hDlg, IDC_ABOUT_ICON),
                            STM_SETICON, (WPARAM)hIcon, 0);
            }

            SetFocus(GetDlgItem(hDlg, IDC_DONATE_BTN));
        }
        break;

        case WM_CLOSE:
            DestroyIcon(hIcon);
            EndDialog(hDlg, LOWORD(wParam));
            break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_SITE_BTN:
                {
                    LoadMUIStringF(hLangInst, IDS_SITE_LINK,
                                   szText, MAX_STR_LEN);
                    ShellExecute(NULL, L"open", szText,
                                 NULL, NULL, SW_SHOWNORMAL);
                }
                break;

                case IDC_DONATE_BTN:
                {
                    LoadMUIStringF(hLangInst, IDS_SITE_DONATE_LINK,
                                   szText, MAX_STR_LEN);
                    ShellExecute(NULL, L"open", szText,
                                 NULL, NULL, SW_SHOWNORMAL);
                }
                break;

                case IDOK:
                    PostMessage(hDlg, WM_CLOSE, 0, 0);
                    break;
            }
        }
        break;
    }

    return FALSE;
}
