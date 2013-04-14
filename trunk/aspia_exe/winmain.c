/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/winmain.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"

/***************************************************************************/
/* VARIABLES ***************************************************************/
/***************************************************************************/

GLOBAL_VARS GlobalVars;
SETTINGS_INFO SettingsInfo;
WCHAR szToolBarSaveReport[MAX_STR_LEN];
WCHAR szToolBarReload[MAX_STR_LEN];

/* Toolbar buttons */
static const TBBUTTON Buttons[] =
{   /* iBitmap, idCommand, fsState, fsStyle, bReserved[2], dwData, iString */
    { 0, ID_SAVE,     TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szToolBarSaveReport},
    { 1, ID_RELOAD,   TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szToolBarReload},
    {-1, 0,           TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
    { 2, ID_SETTINGS, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, 0},
    {-1, 0,           TBSTATE_ENABLED, BTNS_SEP, {0}, 0, 0},
    { 3, ID_HELP_MENU,TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, 0}
};

/***************************************************************************/
/* FUNCTIONS ***************************************************************/
/***************************************************************************/

INT
AddImageListIcon(HIMAGELIST hImageList,
                 INT IconSize, UINT IconIndex)
{
    HICON hIcon = NULL;
    INT Index;

    hIcon = (HICON)LoadImage(GlobalVars.hIconsInstance,
                             MAKEINTRESOURCE(IconIndex),
                             IMAGE_ICON,
                             IconSize,
                             IconSize,
                             LR_CREATEDIBSECTION);

    if (!hIcon) return -1;

    Index = ImageList_AddIcon(hImageList, hIcon);
    DestroyIcon(hIcon);
    return Index;
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
                ScreenToClient(GlobalVars.hMainWindow, &Point);

                if ((GetClientWindowWidth(GlobalVars.hMainWindow) - SPLIT_WIDTH) < Point.x)
                    break;

                if (SPLIT_WIDTH > Point.x)
                    break;

                hdwp = BeginDeferWindowPos(3);

                /* Size SplitterBar */
                DeferWindowPos(hdwp,
                               hwnd,
                               0,
                               Point.x,
                               GetWindowHeight(GlobalVars.hToolBar),
                               SPLIT_WIDTH,
                               GetClientWindowHeight(GlobalVars.hMainWindow) -
                                   GetWindowHeight(GlobalVars.hToolBar),
                               SWP_NOZORDER|SWP_NOACTIVATE);

                /* Size TreeView */
                DeferWindowPos(hdwp,
                               GlobalVars.hTreeView,
                               0,
                               0,
                               GetWindowHeight(GlobalVars.hToolBar),
                               Point.x,
                               GetClientWindowHeight(GlobalVars.hMainWindow) -
                                   GetWindowHeight(GlobalVars.hToolBar),
                               SWP_NOZORDER|SWP_NOACTIVATE);

                /* Size ListView */
                DeferWindowPos(hdwp,
                               GlobalVars.hListView,
                               0,
                               Point.x + SPLIT_WIDTH,
                               GetWindowHeight(GlobalVars.hToolBar),
                               GetClientWindowWidth(GlobalVars.hMainWindow) -
                                   (Point.x + SPLIT_WIDTH),
                               GetClientWindowHeight(GlobalVars.hMainWindow) -
                                   GetWindowHeight(GlobalVars.hToolBar),
                               SWP_NOZORDER|SWP_NOACTIVATE);

                EndDeferWindowPos(hdwp);
            }
        break;
    }

    return DefWindowProc(hwnd, Msg, wParam, lParam);
}

VOID
MainWindowOnCreate(HWND hWindow)
{
    SIZE_T NumButtons = sizeof(Buttons) / sizeof(Buttons[0]);
    WCHAR szSplitWndClass[] = L"SplitterWindowClass";
    WNDCLASSEX SplitWndClass = {0};
    HIMAGELIST hImageList;

/************************************************************
                      ToolBar init
*************************************************************/
    StringLoad(IDS_SAVE, szToolBarSaveReport,
        sizeof(szToolBarSaveReport) / sizeof(WCHAR));

    StringLoad(IDS_RELOAD, szToolBarReload,
        sizeof(szToolBarReload) / sizeof(WCHAR));

    /* Create toolbar */
    GlobalVars.hToolBar = CreateWindowEx(0,
                                         TOOLBARCLASSNAME,
                                         NULL,
                                         WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBSTYLE_FLAT |
                                         TBSTYLE_TOOLTIPS | TBSTYLE_LIST,
                                         0, 40,
                                         340, 30,
                                         hWindow,
                                         NULL,
                                         GlobalVars.hInstance,
                                         NULL);
    if (!GlobalVars.hToolBar)
    {
        DebugTrace(L"CreateWindowEx() failed!");
    }

    SendMessage(GlobalVars.hToolBar,
                TB_BUTTONSTRUCTSIZE,
                sizeof(Buttons[0]),
                0);

    /* Create image list for ToolBar */
    hImageList = ImageList_Create(SettingsInfo.ToolBarIconsSize,
                                  SettingsInfo.ToolBarIconsSize,
                                  ILC_MASK | GlobalVars.SystemColorDepth,
                                  1, 1);
    if (!hImageList)
    {
        DebugTrace(L"ImageList_Create() failed!");
    }

    /* Add images to ImageList */
    AddImageListIcon(hImageList, SettingsInfo.ToolBarIconsSize, IDI_SAVE);
    AddImageListIcon(hImageList, SettingsInfo.ToolBarIconsSize, IDI_RELOAD);
    AddImageListIcon(hImageList, SettingsInfo.ToolBarIconsSize, IDI_SETTINGS);
    AddImageListIcon(hImageList, SettingsInfo.ToolBarIconsSize, IDI_INFO);

    ImageList_Destroy((HIMAGELIST)SendMessage(GlobalVars.hToolBar,
                                              TB_SETIMAGELIST,
                                              0,
                                              (LPARAM)hImageList));

    SendMessage(GlobalVars.hToolBar, TB_ADDBUTTONS, NumButtons, (LPARAM)Buttons);

/************************************************************
                      TreeView init
*************************************************************/
    GlobalVars.hTreeView =
        CreateWindowEx(WS_EX_CLIENTEDGE,
                       WC_TREEVIEW,
                       L"",
                       WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASLINES |
                       TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_LINESATROOT,
                       0, 28, SettingsInfo.MainWindowSplitter, 350,
                       hWindow,
                       NULL,
                       GlobalVars.hInstance,
                       NULL);

    if (!GlobalVars.hTreeView)
    {
        DebugTrace(L"CreateWindowEx() failed!");
    }

    SendMessage(GlobalVars.hTreeView, TVM_SETEXTENDEDSTYLE,
                (WPARAM)GlobalVars.hTreeView, TVS_EX_DOUBLEBUFFER);

    /* Create image list */
    GlobalVars.hTreeViewImageList =
        ImageList_Create(GlobalVars.SmallIconX,
                         GlobalVars.SmallIconY,
                         GlobalVars.SystemColorDepth | ILC_MASK,
                         0, 1);
    if (!GlobalVars.hTreeViewImageList)
    {
        DebugTrace(L"ImageList_Create() failed!");
    }

    TreeView_SetImageList(GlobalVars.hTreeView,
        GlobalVars.hTreeViewImageList, TVSIL_NORMAL);

/************************************************************
                      ListView init
*************************************************************/
    GlobalVars.hListView =
        CreateWindowEx(WS_EX_CLIENTEDGE,
                       WC_LISTVIEW,
                       L"",
                       WS_CHILD | WS_VISIBLE | LVS_REPORT |
                       WS_TABSTOP | LVS_SHOWSELALWAYS,
                       205, 28, 465, 350,
                       hWindow,
                       NULL,
                       GlobalVars.hInstance,
                       NULL);

    if (!GlobalVars.hListView)
    {
        DebugTrace(L"CreateWindowEx() failed!");
    }

    ListView_SetExtendedListViewStyle(GlobalVars.hListView,
                                      LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    GlobalVars.hListViewImageList =
        ImageList_Create(GlobalVars.SmallIconX,
                         GlobalVars.SmallIconY,
                         ILC_MASK | GlobalVars.SystemColorDepth,
                         1, 1);
    if (!GlobalVars.hListViewImageList)
    {
        DebugTrace(L"ImageList_Create() failed!");
    }

    ListView_SetImageList(GlobalVars.hListView,
                          GlobalVars.hListViewImageList,
                          LVSIL_SMALL);

/************************************************************
                      Splitter init
*************************************************************/

    SplitWndClass.cbSize        = sizeof(WNDCLASSEX);
    SplitWndClass.lpszClassName = szSplitWndClass;
    SplitWndClass.lpfnWndProc   = SplitterWindowProc;
    SplitWndClass.hInstance     = GlobalVars.hInstance;
    SplitWndClass.style         = CS_HREDRAW | CS_VREDRAW;
    SplitWndClass.hCursor       = LoadCursor(0, IDC_SIZEWE);
    SplitWndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (RegisterClassEx(&SplitWndClass) == (ATOM)0)
    {
        DebugTrace(L"RegisterClassEx() failed!");
    }

    GlobalVars.hSplitter =
        CreateWindowEx(WS_EX_TRANSPARENT,
                       szSplitWndClass,
                       NULL,
                       WS_CHILD | WS_VISIBLE,
                       SettingsInfo.MainWindowSplitter, 28, SPLIT_WIDTH, 350,
                       hWindow,
                       NULL,
                       GlobalVars.hInstance,
                       NULL);

    if (!GlobalVars.hSplitter)
    {
        DebugTrace(L"CreateWindowEx() failed!");
    }

    ShowWindow(GlobalVars.hSplitter, SW_SHOW);
    UpdateWindow(GlobalVars.hSplitter);

    SetFocus(GlobalVars.hListView);
}

/* Функция для отображения всплывающего меню в текущем положении курсора */
VOID
ShowPopupMenu(UINT MenuID)
{
    HMENU hPopupMenu =
        GetSubMenu(MenuLoad(MAKEINTRESOURCE(MenuID)), 0);
    POINT pt;

    GetCursorPos(&pt);

    SetForegroundWindow(GlobalVars.hMainWindow);
    TrackPopupMenu(hPopupMenu, 0, pt.x, pt.y, 0, GlobalVars.hMainWindow, NULL);

    DestroyMenu(hPopupMenu);
}

INT_PTR CALLBACK
AboutDialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    WCHAR szText[MAX_STR_LEN];
    HICON hIcon = NULL;

    UNREFERENCED_PARAMETER(lParam);

    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            WCHAR szFormat[MAX_STR_LEN], szInfo[MAX_STR_LEN * 5];

            StringLoad(IDS_ABOUT_STRING,
                       szInfo, sizeof(szInfo)/sizeof(WCHAR));
            SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_EDIT), szInfo);

            if (GetWindowText(GetDlgItem(hDlg, IDC_VERSION_TEXT),
                              szFormat, sizeof(szFormat)/sizeof(WCHAR)))
            {
                StringCbPrintf(szText, sizeof(szText),
                               szFormat, VER_FILEVERSION_STR);
                SetWindowText(GetDlgItem(hDlg, IDC_VERSION_TEXT), szText);
            }

            hIcon = (HICON)LoadImage(GlobalVars.hInstance,
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
                    StringLoad(IDS_SITE_LINK,
                               szText, MAX_STR_LEN);
                    ShellExecute(NULL, L"open", szText,
                                 NULL, NULL, SW_SHOWNORMAL);
                }
                break;

                case IDC_DONATE_BTN:
                {
                    StringLoad(IDS_SITE_DONATE_LINK,
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

VOID
MainWindowOnCommand(UINT Command)
{
    switch (Command)
    {
        case ID_SAVE:
            ShowPopupMenu(IDR_SAVE_POPUP);
            break;

        case ID_SAVE_ALL:
            break;

        case ID_SAVE_CURRENT:
            break;

        case ID_SAVE_REPORT:
            break;

        case ID_HELP_MENU:
            ShowPopupMenu(IDR_HELP_POPUP);
            break;

        case ID_HELP:
            //RunLocalizedHelp(); TODO
            break;

        case ID_ABOUT:
            DialogLoad(MAKEINTRESOURCE(IDD_ABOUT_DIALOG),
                       GlobalVars.hMainWindow,
                       AboutDialogProc);
            break;
    }
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
    }

    StringLoad(StringID, lpttt->szText,
               sizeof(lpttt->szText) / sizeof(WCHAR));
}

VOID
MainWindowOnNotify(LPARAM lParam)
{
    LPNMHDR Data = (LPNMHDR)lParam;

    switch (Data->code)
    {
        case NM_CUSTOMDRAW:
            // TODO
            break;

        case NM_RCLICK:
            // TODO
            break;

        case NM_CLICK:
            // TODO
            break;

        case NM_DBLCLK:
            // TODO
            break;

        case TTN_GETDISPINFO:
            ToolBarOnGetDispInfo((LPTOOLTIPTEXT)lParam);
            break;

        case TVN_SELCHANGED:
            // TODO
            break;

        case LVN_COLUMNCLICK:
            // TODO
            break;
    }
}

VOID
MainWindowOnSize(INT Width, INT Height)
{
    HDWP hdwp = BeginDeferWindowPos(3);
    INT VSplitterPos;

    /* Size tool bar */
    SendMessage(GlobalVars.hToolBar, TB_AUTOSIZE, 0, 0);

    /* Size vertical splitter bar */
    DeferWindowPos(hdwp,
                   GlobalVars.hSplitter,
                   0,
                   (VSplitterPos = GetWindowWidth(GlobalVars.hTreeView)),
                   GetWindowHeight(GlobalVars.hToolBar),
                   SPLIT_WIDTH,
                   Height - GetWindowHeight(GlobalVars.hToolBar),
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size TreeView */
    DeferWindowPos(hdwp,
                   GlobalVars.hTreeView,
                   0,
                   0,
                   GetWindowHeight(GlobalVars.hToolBar),
                   VSplitterPos,
                   Height - GetWindowHeight(GlobalVars.hToolBar),
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size ListView */
    DeferWindowPos(hdwp,
                   GlobalVars.hListView,
                   0,
                   VSplitterPos + SPLIT_WIDTH,
                   GetWindowHeight(GlobalVars.hToolBar),
                   Width - (VSplitterPos + SPLIT_WIDTH),
                   Height - GetWindowHeight(GlobalVars.hToolBar),
                   SWP_NOZORDER|SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);
}

VOID
MainWindowOnSizing(LPARAM lParam)
{
    LPRECT pRect = (LPRECT)lParam;

    if (pRect->right - pRect->left < 400)
        pRect->right = pRect->left + 400;

    if (pRect->bottom - pRect->top < 200)
        pRect->bottom = pRect->top + 200;
}

VOID
MainWindowOnSettingsChange(VOID)
{
    INT SmallIconX = GetSystemMetrics(SM_CXSMICON);
    INT SmallIconY = GetSystemMetrics(SM_CYSMICON);
    INT SystemColorDepth = GetSystemColorDepth();

    if (GlobalVars.SmallIconX != SmallIconX ||
        GlobalVars.SmallIconY != SmallIconY ||
        GlobalVars.SystemColorDepth != SystemColorDepth)
    {
        GlobalVars.SmallIconX = SmallIconX;
        GlobalVars.SmallIconY = SmallIconY;
        GlobalVars.SystemColorDepth = SystemColorDepth;

        // TODO
        //ReInitControls();
    }
}

VOID
MainWindowOnSysColorChange(VOID)
{
    /* Forward WM_SYSCOLORCHANGE to common controls */
    SendMessage(GlobalVars.hListView, WM_SYSCOLORCHANGE, 0, 0);
    SendMessage(GlobalVars.hTreeView, WM_SYSCOLORCHANGE, 0, 0);
    SendMessage(GlobalVars.hToolBar, WM_SYSCOLORCHANGE, 0, 0);
}

LRESULT
MainWindowOnDestroy(VOID)
{
    PostQuitMessage(0);
    return 0;
}

LRESULT CALLBACK
MainWindowProc(HWND hWindow, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_CREATE:
            MainWindowOnCreate(hWindow);
            break;

        case WM_COMMAND:
            MainWindowOnCommand(LOWORD(wParam));
            break;

        case WM_NOTIFY:
            MainWindowOnNotify(lParam);
            break;

        case WM_SIZE:
            MainWindowOnSize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_SIZING:
            MainWindowOnSizing(lParam);
            break;

        case WM_SETTINGCHANGE:
            MainWindowOnSettingsChange();
            break;

        case WM_SYSCOLORCHANGE:
            MainWindowOnSysColorChange();
            break;

        case WM_DESTROY:
            MainWindowOnDestroy();
            break;
    }

    return DefWindowProc(hWindow, Message, wParam, lParam);
}

BOOL
GetIniFilePath(LPWSTR lpszPath, SIZE_T PathLen)
{
    WCHAR szPath[MAX_PATH];

    if (!GlobalVars.IsPortable)
    {
        if (!SHGetSpecialFolderPath(GlobalVars.hMainWindow, szPath, CSIDL_APPDATA, FALSE))
        {
            return FALSE;
        }
        StringCchPrintf(lpszPath, PathLen, L"%s\\aspia.ini", szPath);

        if (GetFileAttributes(lpszPath) == INVALID_FILE_ATTRIBUTES)
        {
            if (!SHGetSpecialFolderPath(GlobalVars.hMainWindow, szPath,
                                        CSIDL_COMMON_APPDATA, FALSE))
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
LoadSettings(VOID)
{
    WCHAR szIniPath[MAX_PATH];
    BOOL Result;

    if (!GetIniFilePath(szIniPath, MAX_PATH))
    {
        DebugTrace(L"GetIniFilePath() failed!");
        return FALSE;
    }

    Result = GetPrivateProfileStruct(L"General",
                                     L"Settings",
                                     &SettingsInfo,
                                     sizeof(SettingsInfo),
                                     szIniPath);

    if (!Result)
    {
        SettingsInfo.IsSaveWindowPosition  = TRUE;
        SettingsInfo.IsMainWindowStayOnTop = FALSE;
        SettingsInfo.IsMainWindowMaximized = FALSE;
        SettingsInfo.MainWindowLeft     = 20;
        SettingsInfo.MainWindowTop      = 20;
        SettingsInfo.MainWindowRight    = 870;
        SettingsInfo.MainWindowBottom   = 660;
        SettingsInfo.MainWindowSplitter = 250;
        SettingsInfo.ToolBarIconsSize   = 24;
    }

    return Result;
}

VOID
SaveSettings(VOID)
{
    WCHAR szIniPath[MAX_PATH];
    WINDOWPLACEMENT wp;

    if (!GetIniFilePath(szIniPath, MAX_PATH))
    {
        DebugTrace(L"GetIniFilePath() failed!");
        return;
    }

    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(GlobalVars.hMainWindow, &wp);

    SettingsInfo.MainWindowSplitter =
        GetWindowWidth(GlobalVars.hTreeView);
    SettingsInfo.IsMainWindowMaximized =
        (IsZoomed(GlobalVars.hMainWindow) || (wp.flags & WPF_RESTORETOMAXIMIZED));

    if (!SettingsInfo.IsMainWindowMaximized)
    {
        SettingsInfo.MainWindowLeft   = wp.rcNormalPosition.left;
        SettingsInfo.MainWindowTop    = wp.rcNormalPosition.top;
        SettingsInfo.MainWindowRight  = wp.rcNormalPosition.right;
        SettingsInfo.MainWindowBottom = wp.rcNormalPosition.bottom;
    }

    WritePrivateProfileStruct(L"General",
                              L"Settings",
                              &SettingsInfo,
                              sizeof(SettingsInfo),
                              szIniPath);
}

BOOL
LoadIcons(VOID)
{
    WCHAR szPath[MAX_PATH], szCurrentPath[MAX_PATH];

    if (GlobalVars.hIconsInstance)
        FreeLibrary(GlobalVars.hIconsInstance);

    GlobalVars.hIconsInstance = NULL;

    if (GlobalVars.szIconsFile[0] == 0)
    {
        StringCbCopy(GlobalVars.szIconsFile,
                     sizeof(GlobalVars.szIconsFile),
                     L"tango.dll");
    }

    GetCurrentPath(szCurrentPath, sizeof(szCurrentPath) / sizeof(WCHAR));

    StringCbPrintf(szPath, sizeof(szPath),
                   L"%sicons\\%s",
                   szCurrentPath,
                   GlobalVars.szIconsFile);

    DebugTrace(L"Loading icon file: %s", szPath);

    GlobalVars.hIconsInstance =
        LoadLibraryEx(szPath, NULL, LOAD_LIBRARY_AS_DATAFILE);

    if (!GlobalVars.hIconsInstance) return FALSE;

    return TRUE;
}

INT WINAPI
wWinMain(HINSTANCE hInst,
         HINSTANCE hPrevInstance,
         LPWSTR lpCmdLine,
         INT nShowCmd)
{
    WNDCLASSEX WndClass = {0};
    WCHAR szWindowClass[] = L"ASPIAIS";
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

    GlobalVars.hInstance = hInst;
    GlobalVars.hProcessHeap = GetProcessHeap();

    /* если у нас дебаг билд, то всегда включаем DebugMode */
#ifdef _DEBUG
    GlobalVars.DebugMode = TRUE;
#else
    GlobalVars.DebugMode = IsDebugModeEnabled();
#endif

    if (GlobalVars.DebugMode)
        GlobalVars.DebugMode = InitDebugLog(L"aspia.log", VER_FILEVERSION_STR);

    GlobalVars.IsPortable = IsPortable();

    if (!LoadSettings())
    {
        DebugTrace(L"LoadSettings() failed");
    }

    if (!LoadIcons())
    {
        DebugTrace(L"LoadIcons() failed!");
        goto Exit;
    }

    /* Устанавливаем для текущего процесса отладочные привилегии */
    SetProcessPrivilege(SE_DEBUG_NAME);

    GlobalVars.SmallIconX = GetSystemMetrics(SM_CXSMICON);
    GlobalVars.SmallIconY = GetSystemMetrics(SM_CYSMICON);
    GlobalVars.SystemColorDepth = GetSystemColorDepth();

    /* Инициализация локализации */
    GlobalVars.wLanguage = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

    /* Загружаем драйвер режима ядра */
    GlobalVars.IsDriverInitialized = LoadDriver();
    if (!GlobalVars.IsDriverInitialized)
    {
        DebugTrace(L"LoadDriver() failed!");
    }

    InitCommonControls();

    /* Загружаем ускорители для обработки горячих клавиш */
    hAccel = LoadAccelerators(GlobalVars.hInstance,
                              MAKEINTRESOURCE(IDC_ACCELERATORS));
    if (hAccel == NULL)
    {
        DebugTrace(L"LoadAccelerators() failed!");
    }

    /* Create the window */
    WndClass.cbSize        = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = szWindowClass;
    WndClass.lpfnWndProc   = MainWindowProc;
    WndClass.hInstance     = GlobalVars.hInstance;
    WndClass.style         = CS_HREDRAW | CS_VREDRAW;
    WndClass.hIcon         = LoadIcon(GlobalVars.hInstance,
                                      MAKEINTRESOURCE(IDI_MAINICON));
    WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.lpszMenuName  = NULL;

    if (RegisterClassEx(&WndClass) == (ATOM)0)
    {
        DebugTrace(L"RegisterClassEx() failed!");
        goto Exit;
    }

    if (GlobalVars.IsPortable ||
        !SettingsInfo.IsSaveWindowPosition)
    {
        SettingsInfo.MainWindowLeft     = 20;
        SettingsInfo.MainWindowTop      = 20;
        SettingsInfo.MainWindowRight    = 850;
        SettingsInfo.MainWindowBottom   = 640;
        SettingsInfo.MainWindowSplitter = 242;
    }

    /* Создаем главное окно программы */
    GlobalVars.hMainWindow =
        CreateWindowEx(WS_EX_WINDOWEDGE,
                       szWindowClass,
                       L"Aspia",
                       WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                       SettingsInfo.MainWindowLeft,
                       SettingsInfo.MainWindowTop,
                       SettingsInfo.MainWindowRight - SettingsInfo.MainWindowLeft,
                       SettingsInfo.MainWindowBottom - SettingsInfo.MainWindowTop,
                       NULL, NULL, GlobalVars.hInstance, NULL);

    if (!GlobalVars.hMainWindow)
    {
        DebugTrace(L"CreateWindowEx() failed!");
        goto Exit;
    }

    if (SettingsInfo.IsMainWindowStayOnTop)
    {
        SetWindowPos(GlobalVars.hMainWindow,
                     HWND_TOPMOST,
                     20, 20, 850, 640,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }

    ShowWindow(GlobalVars.hMainWindow,
        (SettingsInfo.IsMainWindowMaximized) ? SW_SHOWMAXIMIZED : SW_SHOW);
    UpdateWindow(GlobalVars.hMainWindow);

    /* Message Loop */
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(GlobalVars.hMainWindow, hAccel, &Msg))
        {
            if (!IsDialogMessage(GlobalVars.hMainWindow, &Msg))
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }
        }
    }

Exit:
    UnloadDriver();
    if (hMutex) CloseHandle(hMutex);
    CloseDebugLog();

    return 0;
}
