/*
 * PROJECT:         Aspia
 * FILE:            aspia/print.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"
#include "driver.h"
#include <richedit.h>


static HWND hPrintWnd = NULL;
static HWND hRichEdit = NULL;
static HWND hPrintToolBar = NULL;

HIMAGELIST hPrintImgList = NULL;

WCHAR szPrintStart[MAX_STR_LEN] = {0};
WCHAR szPrintClose[MAX_STR_LEN] = {0};


/* Toolbar buttons */
static const TBBUTTON PrintButtons[] =
{   /* iBitmap, idCommand, fsState, fsStyle, bReserved[2], dwData, iString */
    { 0, ID_PRINT_START, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szPrintStart},
    { 1, ID_PRINT_CLOSE, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szPrintClose}
};


BOOL
PrintWindowInitControls(HWND hwnd)
{
    SIZE_T NumButtons = sizeof(PrintButtons) / sizeof(PrintButtons[0]);

    LoadMUIString(IDS_PRINT_START, szPrintStart, MAX_STR_LEN);
    LoadMUIString(IDS_PRINT_CLOSE, szPrintClose, MAX_STR_LEN);

    hPrintToolBar = CreateWindowEx(0,
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

    if (!hPrintToolBar)
    {
        DebugTrace(L"CreateWindowEx() failed!");
        return FALSE;
    }

    /* Create image list for ToolBar */
    hPrintImgList = ImageList_Create(TOOLBAR_HEIGHT,
                                     TOOLBAR_HEIGHT,
                                     ILC_MASK | ParamsInfo.SysColorDepth,
                                     1, 1);
    if (!hPrintImgList)
    {
        DebugTrace(L"ImageList_Create() failed!");
        return FALSE;
    }

    SendMessage(hPrintToolBar,
                TB_BUTTONSTRUCTSIZE,
                sizeof(PrintButtons[0]),
                0);

    AddIconToImageList(hIconsInst, hPrintImgList, IDI_PRINTER);
    AddIconToImageList(hIconsInst, hPrintImgList, IDI_CLOSE);

    SendMessage(hPrintToolBar,
                TB_SETIMAGELIST,
                0,
                (LPARAM)hPrintImgList);

    SendMessage(hPrintToolBar, TB_ADDBUTTONS, NumButtons, (LPARAM)PrintButtons);

    LoadLibrary(L"riched20.dll");

    hRichEdit = CreateWindowEx(0,
                               L"RichEdit20W",
                               NULL,
                               WS_CHILD | WS_BORDER | WS_VISIBLE | ES_MULTILINE |
                               ES_LEFT | ES_READONLY,
                               205, 28, 465, 100,
                               hwnd,
                               NULL,
                               hInstance,
                               NULL);

    if (!hRichEdit)
    {
        DebugTrace(L"CreateWindowEx() failed!");
        return FALSE;
    }

    SendMessage(hRichEdit, WM_SETFONT,
                (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
    SendMessage(hRichEdit, EM_SHOWSCROLLBAR, SB_VERT, TRUE);

    return TRUE;
}

VOID
PrintWindowOnSize(LPARAM lParam)
{
    HDWP hdwp = BeginDeferWindowPos(1);

    SendMessage(hPrintToolBar, TB_AUTOSIZE, 0, 0);

    DeferWindowPos(hdwp,
                   hRichEdit,
                   0,
                   0,
                   GetWindowHeight(hPrintToolBar),
                   LOWORD(lParam),
                   HIWORD(lParam) - GetWindowHeight(hPrintToolBar),
                   SWP_NOZORDER|SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);
}

VOID
PrintToolBarOnGetDispInfo(LPTOOLTIPTEXT lpttt)
{
    UINT idButton = (UINT)lpttt->hdr.idFrom;
    UINT StringID;

    switch (idButton)
    {
        case ID_PRINT_START:
            StringID = IDS_PRINT_START;
            break;
        case ID_PRINT_CLOSE:
            StringID = IDS_PRINT_CLOSE;
            break;
        default:
            return;
    }

    LoadMUIString(StringID, lpttt->szText, 80);
}

LRESULT CALLBACK
PrintWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_CREATE:
            PrintWindowInitControls(hwnd);
            break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case ID_PRINT_START:
                    break;

                case ID_PRINT_CLOSE:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
            }
        }
        break;

        case WM_NOTIFY:
        {
            LPNMHDR data = (LPNMHDR)lParam;

            switch (data->code)
            {
                case TTN_GETDISPINFO:
                    PrintToolBarOnGetDispInfo((LPTOOLTIPTEXT)lParam);
                    break;
            }
        }
        break;

        case WM_SIZE:
            PrintWindowOnSize(lParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, Msg, wParam, lParam);
}

VOID
CreatePrintWindow(LPWSTR lpFile)
{
    WNDCLASSEX WndClass = {0};
    WCHAR szWindowClass[] = L"ASPIAISPRINT";
    WCHAR szWindowName[MAX_STR_LEN];
    HWND hPrintWnd;
    MSG Msg;

    /* Create the window */
    WndClass.cbSize        = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = szWindowClass;
    WndClass.lpfnWndProc   = PrintWindowProc;
    WndClass.hInstance     = hInstance;
    WndClass.style         = CS_HREDRAW | CS_VREDRAW;
    WndClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PRINTER));
    WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.lpszMenuName  = NULL;

    if (RegisterClassEx(&WndClass) == (ATOM)0)
        return;

    LoadMUIString(IDS_PRINTWND_TITLE, szWindowName, MAX_STR_LEN);

    /* Создаем главное окно программы */
    hPrintWnd = CreateWindowEx(WS_EX_WINDOWEDGE,
                               szWindowClass,
                               szWindowName,
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                               22, 16, 600, 480,
                               NULL, NULL, hInstance, NULL);

    if (!hPrintWnd)
    {
        UnregisterClass(szWindowClass, hInstance);
        return;
    }

    /* Show it */
    ShowWindow(hPrintWnd, SW_SHOW);
    UpdateWindow(hPrintWnd);

    /* Message Loop */
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    UnregisterClass(szWindowClass, hInstance);
}
