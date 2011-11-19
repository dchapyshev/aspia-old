/*
 * PROJECT:         Aspia
 * FILE:            aspia/sysmon/sysmon.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"


HWND hSysMonWnd = NULL;
HWND hSysMonList = NULL;
HWND hSysMonToolBar = NULL;
HIMAGELIST hSysMonImgList = NULL;
HIMAGELIST hSysMonImgList2 = NULL;

WCHAR szSysMonStart[MAX_STR_LEN] = {0};
WCHAR szSysMonStop[MAX_STR_LEN] = {0};
WCHAR szSysMonTest[MAX_STR_LEN] = {0};

/* Toolbar buttons */
static const TBBUTTON SysMonButtons[] =
{   /* iBitmap, idCommand, fsState, fsStyle, bReserved[2], dwData, iString */
    { 0, ID_SYSMON_START,TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szSysMonStart},
    { 1, ID_SYSMON_STOP, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szSysMonStop},
    { 2, ID_SYSMON_TEST, TBSTATE_ENABLED, BTNS_CHECK | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szSysMonTest}
};

VOID
SysMonAddColumn(SIZE_T Index, INT Width, UINT resIndex)
{
    WCHAR szText[MAX_STR_LEN];
    LV_COLUMN Column = {0};

    LoadMUIString(resIndex, szText, MAX_STR_LEN);

    Column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    Column.iSubItem = Index;
    Column.pszText = szText;
    Column.cx  = Width;
    Column.fmt = LVCFMT_LEFT;

    ListView_InsertColumn(hSysMonList, Index, &Column);
}

VOID
SysMonInitControls(HWND hwnd)
{
    SIZE_T NumButtons = sizeof(SysMonButtons) / sizeof(SysMonButtons[0]);

    LoadMUIString(IDS_SYSMON_START, szSysMonStart, MAX_STR_LEN);
    LoadMUIString(IDS_SYSMON_STOP, szSysMonStop, MAX_STR_LEN);
    LoadMUIString(IDS_SYSMON_TEST, szSysMonTest, MAX_STR_LEN);

    /* Create toolbar */
    hSysMonToolBar = CreateWindowEx(0,
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
    if (!hSysMonToolBar) return;

    SendMessage(hSysMonToolBar,
                TB_BUTTONSTRUCTSIZE,
                sizeof(SysMonButtons[0]),
                0);

    /* Create image list for ToolBar */
    hSysMonImgList2 = ImageList_Create(TOOLBAR_HEIGHT,
                                       TOOLBAR_HEIGHT,
                                       ILC_MASK | ParamsInfo.SysColorDepth,
                                       1, 1);
    if (!hSysMonImgList2)
        return;

    AddImageToImageList(hSysMonImgList2, IDI_START);
    AddImageToImageList(hSysMonImgList2, IDI_STOP);
    AddImageToImageList(hSysMonImgList2, IDI_STRESS_TEST);

    SendMessage(hSysMonToolBar,
                TB_SETIMAGELIST,
                0,
                (LPARAM)hSysMonImgList2);

    SendMessage(hSysMonToolBar, TB_ADDBUTTONS, NumButtons, (LPARAM)SysMonButtons);

    /* Create bar for stress-test controls */

    hSysMonList = CreateWindowEx(WS_EX_CLIENTEDGE,
                                 WC_LISTVIEW,
                                 L"",
                                 WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_TABSTOP | LVS_SHOWSELALWAYS,
                                 205, 28, 465, 350,
                                 hwnd,
                                 NULL,
                                 hInstance,
                                 NULL);

    if (!hSysMonList) return;

    ListView_SetExtendedListViewStyle(hSysMonList,
                                      LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    hSysMonImgList = ImageList_Create(ParamsInfo.SxSmIcon,
                                      ParamsInfo.SySmIcon,
                                      ILC_MASK | ParamsInfo.SysColorDepth,
                                      1, 1);

    ListView_SetImageList(hSysMonList,
                          hSysMonImgList,
                          LVSIL_SMALL);

    SysMonAddColumn(0, 250, IDS_SYSMON_SENSOR);
    SysMonAddColumn(1, 90, IDS_SYSMON_CURRENT);
    SysMonAddColumn(2, 90, IDS_SYSMON_MIN);
    SysMonAddColumn(3, 90, IDS_SYSMON_MAX);

    /* Hide main window */
    ShowWindow(hMainWnd, SW_HIDE);
}

VOID
SysMonOnSize(LPARAM lParam)
{
    HDWP hdwp = BeginDeferWindowPos(1);

    /* Size tool bar */
    SendMessage(hSysMonToolBar, TB_AUTOSIZE, 0, 0);

    /* Size hSysMonList */
    DeferWindowPos(hdwp,
                   hSysMonList,
                   0,
                   2,
                   GetWindowHeight(hSysMonToolBar),
                   LOWORD(lParam) - 4,
                   HIWORD(lParam) - GetWindowHeight(hSysMonToolBar) - 2,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);
}

VOID
SysMonToolBarOnGetDispInfo(LPTOOLTIPTEXT lpttt)
{
    UINT idButton = (UINT)lpttt->hdr.idFrom;
    UINT StringID;

    switch (idButton)
    {
        case ID_SYSMON_START:
            StringID = IDS_SYSMON_START;
            break;
        case ID_SYSMON_STOP:
            StringID = IDS_SYSMON_STOP;
            break;
        case ID_SYSMON_TEST:
            StringID = IDS_SYSMON_TEST;
            break;
        default:
            return;
    }

    LoadMUIString(StringID, lpttt->szText, 80);
}

LRESULT CALLBACK
SysMonWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_CREATE:
            SysMonInitControls(hwnd);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_SYSMON_START:
                {

                }
                break;

                case ID_SYSMON_TEST:
                {
                    BOOL Checked =
                        SendMessage(hSysMonToolBar,
                                    TB_ISBUTTONCHECKED,
                                    ID_SYSMON_TEST,
                                    0);

                    if (Checked)
                    {
                        MessageBox(0, L"CHECKED", 0, 0);
                    }
                    else
                    {
                        MessageBox(0, L"UNCHECKED", 0, 0);
                    }
                }
                break;
            }
            break;

        case WM_NOTIFY:
        {
            LPNMHDR data = (LPNMHDR)lParam;

            switch (data->code)
            {
                case TTN_GETDISPINFO:
                    SysMonToolBarOnGetDispInfo((LPTOOLTIPTEXT)lParam);
                    break;
            }
        }
        break;

        case WM_SIZE:
            SysMonOnSize(lParam);
            break;

        case WM_SIZING:
            break;

        case WM_DESTROY:
            ShowWindow(hMainWnd, SW_SHOW);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, Msg, wParam, lParam);
}

VOID
CreateSysMonWindow(VOID)
{
    WNDCLASSEX WndClass = {0};
    WCHAR szWindowClass[] = L"ASPIAISSYSMON";
    WCHAR szWindowName[MAX_STR_LEN];
    MSG Msg;

    /* Create the window */
    WndClass.cbSize        = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = szWindowClass;
    WndClass.lpfnWndProc   = SysMonWindowProc;
    WndClass.hInstance     = hInstance;
    WndClass.style         = CS_HREDRAW | CS_VREDRAW;
    WndClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.lpszMenuName  = NULL;

    if (RegisterClassEx(&WndClass) == (ATOM)0)
        return;

    LoadMUIString(IDS_ASPIA_SYSMON, szWindowName, MAX_STR_LEN);

    /* Создаем главное окно программы */
    hSysMonWnd = CreateWindowEx(WS_EX_WINDOWEDGE,
                                szWindowClass,
                                szWindowName,
                                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                20, 20, 570, 500,
                                NULL, NULL, hInstance, NULL);

    if (!hSysMonWnd)
    {
        UnregisterClass(szWindowClass, hInstance);
        return;
    }

    CenterWindow(hSysMonWnd, NULL);

    /* Show it */
    ShowWindow(hSysMonWnd, SW_SHOW);
    UpdateWindow(hSysMonWnd);

    /* Message Loop */
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    UnregisterClass(szWindowClass, hInstance);
}
