/*
 * PROJECT:         Aspia
 * FILE:            aspia/controls.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"


HWND hToolBar = NULL;
HWND hTreeView = NULL;
HWND hListView = NULL;
HWND hSplitter = NULL;

static WCHAR szSaveReportBtn[MAX_STR_LEN] = {0};
static WCHAR szReloadBtn[MAX_STR_LEN] = {0};

HIMAGELIST hImageTreeView = NULL;

/* Toolbar buttons */
static const TBBUTTON Buttons[] =
{   /* iBitmap, idCommand, fsState, fsStyle, bReserved[2], dwData, iString */
    { 0, ID_SAVE,     TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szSaveReportBtn},
    { 1, ID_RELOAD,   TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szReloadBtn},
    {-1, 0,           TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
    { 2, ID_SETTINGS, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, 0},
    {-1, 0,           TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
    { 3, ID_ABOUT,    TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, 0}
};


VOID
ListViewClear(VOID)
{
    INT i;

    ImageList_RemoveAll(hListViewImageList);
    ListView_DeleteAllItems(hListView);

    for (i = IoGetColumnsCount(); i >= 0; --i)
        ListView_DeleteColumn(hListView, i);

    IoSetColumnsCount(0);
}

PVOID
ListViewGetlParam(HWND hList, INT Index)
{
    INT ItemIndex;
    LVITEM Item = {0};

    if (Index == -1)
    {
        ItemIndex = ListView_GetNextItem(hList, -1, LVNI_FOCUSED);
        if (ItemIndex == -1)
            return NULL;
    }
    else
    {
        ItemIndex = Index;
    }

    Item.mask = LVIF_PARAM;
    Item.iItem = ItemIndex;
    if (!ListView_GetItem(hList, &Item))
        return NULL;

    return (PVOID)Item.lParam;
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

INT
AddImageToImageList(HIMAGELIST hImageList, UINT ImageIndex)
{
    HICON hIcon = NULL;
    INT Index;

    hIcon = (HICON)LoadImage(hIconsInst,
                             MAKEINTRESOURCE(ImageIndex),
                             IMAGE_ICON,
                             TOOLBAR_HEIGHT,
                             TOOLBAR_HEIGHT,
                             LR_CREATEDIBSECTION);

    if (!hIcon) return -1;

    Index = ImageList_AddIcon(hImageList, hIcon);
    DestroyIcon(hIcon);
    return Index;
}

VOID
ToolBarOnGetDispInfo(LPTOOLTIPTEXT lpttt)
{
    UINT idButton = (UINT)lpttt->hdr.idFrom;
    UINT StringID;

    switch (idButton)
    {
        case ID_SAVE:
            StringID = IDS_SAVE;
            break;
        case ID_RELOAD:
            StringID = IDS_RELOAD;
            break;
        case ID_ABOUT:
            StringID = IDS_ABOUT;
            break;
        case ID_SETTINGS:
            StringID = IDS_SETTINGS;
            break;
        default:
            return;
    }

    LoadMUIString(StringID, lpttt->szText, 80);
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
                             SettingsInfo.SxSmIcon,
                             SettingsInfo.SySmIcon,
                             LR_CREATEDIBSECTION);

    Index = ImageList_AddIcon(hImageList, hIcon);
    DestroyIcon(hIcon);

    LoadMUIString(TextIndex, szText, MAX_STR_LEN);

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

VOID
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

VOID
InitToolBar(HWND hwnd)
{
    SIZE_T NumButtons = sizeof(Buttons) / sizeof(Buttons[0]);
    HIMAGELIST hImageList;

    LoadMUIString(IDS_RELOAD, szReloadBtn, MAX_STR_LEN);
    LoadMUIString(IDS_SAVE, szSaveReportBtn, MAX_STR_LEN);

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
    if (!hToolBar) return;

    SendMessage(hToolBar,
                TB_BUTTONSTRUCTSIZE,
                sizeof(Buttons[0]),
                0);

    /* Create image list for ToolBar */
    hImageList = ImageList_Create(TOOLBAR_HEIGHT,
                                  TOOLBAR_HEIGHT,
                                  ILC_MASK | SettingsInfo.SysColorDepth,
                                  1, 1);
    if (!hImageList)
        return;

    /* Add images to ImageList */
    AddImageToImageList(hImageList, IDI_SAVE);
    AddImageToImageList(hImageList, IDI_RELOAD);
    AddImageToImageList(hImageList, IDI_SETTINGS);
    AddImageToImageList(hImageList, IDI_INFO);

    ImageList_Destroy((HIMAGELIST)SendMessage(hToolBar,
                                              TB_SETIMAGELIST,
                                              0,
                                              (LPARAM)hImageList));

    SendMessage(hToolBar, TB_ADDBUTTONS, NumButtons, (LPARAM)Buttons);
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
    hImageTreeView = ImageList_Create(SettingsInfo.SxSmIcon,
                                      SettingsInfo.SySmIcon,
                                      SettingsInfo.SysColorDepth | ILC_MASK,
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

VOID
InitControls(HWND hwnd)
{
    WCHAR szSplitWndClass[] = L"SplitterWindowClass";
    WNDCLASSEX SplitWndClass = {0};
    HTREEITEM hSelectedItem;

    if (SettingsInfo.SaveWindowPos)
    {
        MoveWindow(hwnd, SettingsInfo.Left, SettingsInfo.Top,
                   SettingsInfo.Right - SettingsInfo.Left,
                   SettingsInfo.Bottom - SettingsInfo.Top, TRUE);

        if (SettingsInfo.IsMaximized) ShowWindow(hwnd, SW_MAXIMIZE);
    }

    InitToolBar(hwnd);

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

    if (!hTreeView) return;

    SendMessage(hTreeView, TVM_SETEXTENDEDSTYLE, (WPARAM)hTreeView, TVS_EX_DOUBLEBUFFER);

    /* Create image list */
    hImageTreeView = ImageList_Create(SettingsInfo.SxSmIcon,
                                      SettingsInfo.SySmIcon,
                                      SettingsInfo.SysColorDepth | ILC_MASK,
                                      0, 1);
    hSelectedItem = InitCategoriesList(RootCategoryList, TVI_ROOT);
    ExpandCategoriesList(RootCategoryList);
    TreeView_SelectItem(hTreeView, hSelectedItem);

    TreeView_SetImageList(hTreeView, hImageTreeView, TVSIL_NORMAL);

    hListView = CreateWindowEx(WS_EX_CLIENTEDGE,
                               WC_LISTVIEW,
                               L"",
                               WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_TABSTOP | LVS_SHOWSELALWAYS,
                               205, 28, 465, 350,
                               hwnd,
                               NULL,
                               hInstance,
                               NULL);

    if (!hListView) return;

    ListView_SetExtendedListViewStyle(hListView,
                                      LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    hListViewImageList = ImageList_Create(SettingsInfo.SxSmIcon,
                                          SettingsInfo.SySmIcon,
                                          ILC_MASK | SettingsInfo.SysColorDepth,
                                          1, 1);

    ListView_SetImageList(hListView,
                          hListViewImageList,
                          LVSIL_SMALL);

    /* Create vertical splitter bar */
    SplitWndClass.cbSize = sizeof(WNDCLASSEX);
    SplitWndClass.lpszClassName = szSplitWndClass;
    SplitWndClass.lpfnWndProc = SplitterWindowProc;
    SplitWndClass.hInstance = hInstance;
    SplitWndClass.style = CS_HREDRAW | CS_VREDRAW;
    SplitWndClass.hCursor = LoadCursor(0, IDC_SIZEWE);
    SplitWndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (RegisterClassEx(&SplitWndClass) == (ATOM)0)
        return;

    hSplitter = CreateWindowEx(WS_EX_TRANSPARENT,
                               szSplitWndClass,
                               NULL,
                               WS_CHILD | WS_VISIBLE,
                               SettingsInfo.SplitterPos, 28, SPLIT_WIDTH, 350,
                               hwnd,
                               NULL,
                               hInstance,
                               NULL);

    if (!hSplitter) return;

    ShowWindow(hSplitter, SW_SHOW);
    UpdateWindow(hSplitter);

    SetFocus(hListView);
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

            DebugTrace(L"Dialog init");

            LoadMUIString(IDS_ABOUT_STRING, szInfo, sizeof(szInfo)/sizeof(WCHAR));
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
            SendMessage(GetDlgItem(hDlg, IDC_ABOUT_ICON), STM_SETICON, (WPARAM)hIcon, 0);

            SetFocus(GetDlgItem(hDlg, IDC_DONATE_BTN));
        }
        break;

        case WM_CLOSE:
            DebugTrace(L"Dialog close");
            DestroyIcon(hIcon);
            EndDialog(hDlg, LOWORD(wParam));
            break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_SITE_BTN:
                {
                    LoadMUIString(IDS_SITE_LINK, szText, MAX_STR_LEN);
                    ShellExecute(NULL, L"open", szText,
                                 NULL, NULL, SW_SHOWNORMAL);
                }
                break;

                case IDC_DONATE_BTN:
                {
                    LoadMUIString(IDS_SITE_DONATE_LINK, szText, MAX_STR_LEN);
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
