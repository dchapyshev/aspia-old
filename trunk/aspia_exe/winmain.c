/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/winmain.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"


HINSTANCE hInstance = NULL;
HINSTANCE hIconsInst = NULL;
HINSTANCE hLangInst = NULL;

HWND hMainWnd = NULL;
HIMAGELIST hListViewImageList = NULL;

UINT CurrentCategory = IDS_CAT_SUMMARY;
UINT CurrentMenu = IDR_POPUP;
BOOL IsSortingAllowed = FALSE;
PFREEFUNC InfoFreeFunction = NULL;

HANDLE hFillThread = NULL;
HANDLE hProcessHeap = NULL;

CRITICAL_SECTION CriticalSection;

BOOL IsLoadingDone = TRUE;
BOOL bSortAscending = TRUE;

PARAMS_STRUCT ParamsInfo = {0};


static VOID
SetMainWindowTitle(UINT uiCategory)
{
    WCHAR szText[MAX_STR_LEN], szTitle[MAX_STR_LEN];

    if (!LoadMUIStringF(hLangInst, uiCategory, szText, MAX_STR_LEN))
    {
        SetWindowText(hMainWnd, L"Aspia");
    }
    else
    {
        StringCbPrintf(szTitle, sizeof(szTitle),
                       L"Aspia - %s", szText);
        SetWindowText(hMainWnd, szTitle);
    }
}

INT
ListViewAddCategoryItem(INT IconIndex, UINT Category)
{
    WCHAR szText[MAX_STR_LEN];
    LV_ITEM Item = {0};

    LoadMUIStringF(hLangInst, Category, szText, MAX_STR_LEN);

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
SaveColumnsSizes(VOID)
{
    WCHAR szName[15], szCol[3], szIniPath[MAX_PATH];
    INT i, count = IoGetColumnsCount();

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
}

VOID
GUIInfoThread(LPVOID lpParameter)
{
    UINT Category = (UINT)lpParameter;

    if (!IsLoadingDone)
    {
        _endthread();
        return;
    }

    EnterCriticalSection(&CriticalSection);

    SetMainWindowTitle(Category);

    if (CurrentCategory != Category)
    {
        SaveColumnsSizes();

        CurrentCategory = Category;
    }

    if (InfoFreeFunction)
    {
        InfoFreeFunction();
        InfoFreeFunction = NULL;
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

    bSortAscending = TRUE;

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
                    StringCbCat(pText, Size, L"\t");
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

BOOL
StartRemoveProcess(LPWSTR lpPath, BOOL Wait)
{
    PROCESS_INFORMATION pi;
    STARTUPINFOW si = {0};
    DWORD dwRet;
    MSG msg;

    si.cb = sizeof(si);
    si.wShowWindow = SW_SHOW;

    if (!CreateProcess(NULL, lpPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        return FALSE;
    }

    CloseHandle(pi.hThread);
    if (Wait) EnableWindow(hMainWnd, FALSE);

    while (Wait)
    {
        dwRet = MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLEVENTS);
        if (dwRet == WAIT_OBJECT_0 + 1)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_FAILED)
                break;
        }
    }

    CloseHandle(pi.hProcess);

    if (Wait)
    {
        EnableWindow(hMainWnd, TRUE);
        SetForegroundWindow(hMainWnd);
        SetFocus(hMainWnd);
    }

    return TRUE;
}

typedef struct
{
    WCHAR MyChar;
    WCHAR *Encode;
} CHARS_TO_CONVERT;

CHARS_TO_CONVERT CharsToConvert[] =
{
    { L'<',  L"%3C" },
    { L'>',  L"%3E" },
    { L'.',  L"%22" },
    { L'#',  L"%23" },
    { L'{',  L"%7B" },
    { L'}',  L"%7D" },
    { L'|',  L"%7C" },
    { L'\\', L"%5C" },
    { L'^',  L"%5E" },
    { L'~',  L"%7E" },
    { L'[',  L"%5B" },
    { L']',  L"%5D" },
    { L'`',  L"%60" },
    { L'+',  L"%20" },
    { L'/',  L"%2F" },
    { L'?',  L"%3F" },
    { L'&',  L"%26" },
    { L' ',  L"%20" },
    { 0 }
};

WCHAR*
UrlEncode(LPWSTR lpInStr)
{
    INT chars_count = wcslen(lpInStr);
    INT i, j;
    INT enc_count = 0;
    WCHAR *pOut;
    SIZE_T size;

    for (i = 0; i < chars_count; i++)
    {
        j = 0;
        do
        {
            if (CharsToConvert[j].MyChar == lpInStr[i])
                enc_count++;
        }
        while (CharsToConvert[++j].Encode != NULL);
    }

    size = ((chars_count - enc_count) + (enc_count * 3)) * sizeof(WCHAR);
    pOut = (WCHAR*)Alloc(size);
    if (pOut)
    {
        ZeroMemory(pOut, size);

        for (i = 0; i < chars_count; i++)
        {
            WCHAR temp[4] = {0};

            enc_count = 0;

            j = 0;
            do
            {
                if (CharsToConvert[j].MyChar == lpInStr[i])
                {
                    enc_count++;
                    break;
                }
            }
            while (CharsToConvert[++j].Encode != NULL);

            if (enc_count)
            {
                StringCchPrintf(temp, 4, L"%s", CharsToConvert[j].Encode);
            }
            else
            {
                StringCchPrintf(temp, 4, L"%c", lpInStr[i]);
            }
            StringCbCat(pOut, size, temp);
        }
    }

    return pOut;
}

VOID
RunLocalizedHelp(VOID)
{
    WCHAR szHelpPath[MAX_PATH];

    StringCbPrintf(szHelpPath, sizeof(szHelpPath),
                   L"%shelp\\%s.chm",
                   ParamsInfo.szCurrentPath, ThemesInfo.szLangFile);

    if (GetFileAttributes(szHelpPath) == INVALID_FILE_ATTRIBUTES)
    {
        DebugTrace(L"Localized help file not found = %s",
                   szHelpPath);

        DebugTrace(L"Try to load en-US.chm file...");

        StringCbPrintf(szHelpPath, sizeof(szHelpPath),
                       L"%shelp\\en-US.chm",
                       ParamsInfo.szCurrentPath);
        if (GetFileAttributes(szHelpPath) == INVALID_FILE_ATTRIBUTES)
        {
            DebugTrace(L"en-US.chm not found!");
            return;
        }
    }

    DebugTrace(L"Run localized help file: %s", szHelpPath);

    ShellExecute(NULL, NULL, szHelpPath, NULL, NULL, SW_SHOW);
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
                ReportSave(TRUE, TRUE, szPath, TRUE);
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

        case ID_HELP:
            RunLocalizedHelp();
            break;

        case ID_HELP_MENU:
            ShowPopupMenu(IDR_HELP_POPUP);
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

        /* Drivers Search */
        case ID_DRIVERS_SEARCH:
        {
            WCHAR *lpId, szURL[MAX_PATH], szSearchUrl[MAX_STR_LEN];

            lpId = (WCHAR*)ListViewGetlParam(hListView, -1);

            if (SafeStrLen(lpId) > 5)
            {
                WCHAR *ptr = UrlEncode(lpId);

                if (ptr)
                {
                    LoadMUIStringF(hLangInst, IDS_DRIVER_SEARCH_URL,
                                   szSearchUrl, MAX_STR_LEN);

                    StringCbPrintf(szURL, sizeof(szURL),
                                   szSearchUrl,
                                   ptr);

                    ShellExecute(NULL, L"open", szURL,
                                 NULL, NULL, SW_SHOWNORMAL);

                    Free(ptr);
                }
            }
        }
        break;

        /* Task Manager Cases */

        case ID_KILL_PROCESS_TREE:
        case ID_KILL_PROCESS:
        {
            INT ItemIndex = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);

            if (KillProcess((DWORD)ListViewGetlParam(hListView, ItemIndex),
                            (Command == ID_KILL_PROCESS) ? FALSE : TRUE))
            {
                ListView_DeleteItem(hListView, ItemIndex);
            }
        }
        break;

        case ID_HIGH_PRIORITY:
        case ID_ABOVE_NORMAL_PRIORITY:
        case ID_NORMAL_PRIORITY:
        case ID_BELOW_NORMAL_PRIORITY:
        case ID_LOW_PRIORITY:
        case ID_REALTIME_PRIORITY:
        {
            INT ItemIndex = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
            DWORD pid = (DWORD)ListViewGetlParam(hListView, ItemIndex);
            HANDLE hProcess;

            hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
            if (hProcess)
            {
                DWORD dwClass;

                switch (Command)
                {
                    case ID_HIGH_PRIORITY:
                        dwClass = HIGH_PRIORITY_CLASS;
                        break;
                    case ID_ABOVE_NORMAL_PRIORITY:
                        dwClass = ABOVE_NORMAL_PRIORITY_CLASS;
                        break;
                    case ID_NORMAL_PRIORITY:
                        dwClass = NORMAL_PRIORITY_CLASS;
                        break;
                    case ID_BELOW_NORMAL_PRIORITY:
                        dwClass = BELOW_NORMAL_PRIORITY_CLASS;
                        break;
                    case ID_LOW_PRIORITY:
                        dwClass = IDLE_PRIORITY_CLASS;
                        break;
                    case ID_REALTIME_PRIORITY:
                        dwClass = REALTIME_PRIORITY_CLASS;
                        break;
                    default:
                        dwClass = NORMAL_PRIORITY_CLASS;
                        break;
                }
                SetPriorityClass(hProcess, dwClass);
                CloseHandle(hProcess);
            }
        }
        break;

        case ID_PROCESS_OPEN_FOLDER:
        {
            INT ItemIndex = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
            WCHAR szPath[MAX_PATH];

            ListView_GetItemText(hListView, ItemIndex, 1, szPath, MAX_PATH);
            if (szPath[0] != 0 && szPath[0] != L'-')
            {
                WCHAR szCmd[MAX_PATH];

                StringCbPrintf(szCmd, sizeof(szCmd), L"/select, %s", szPath);
                ShellExecute(NULL, NULL, L"explorer.exe", szCmd, NULL, SW_SHOWNORMAL);
            }
        }
        break;

        case ID_PROCESS_PROPERTIES:
        {
            INT ItemIndex = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
            WCHAR szPath[MAX_PATH];

            ListView_GetItemText(hListView, ItemIndex, 1, szPath, MAX_PATH);
            if (szPath[0] != 0 && szPath[0] != L'-')
            {
                SHObjectProperties(hMainWnd, SHOP_FILEPATH, szPath, NULL);
            }
        }
        break;

        /* Installed Applications & Updates */

        case ID_APP_MODIFY:
        case ID_APP_REMOVE:
        {
            INST_APP_INFO *pInfo = ListViewGetlParam(hListView, -1);
            DWORD dwType, dwSize;
            WCHAR szPath[MAX_PATH];

            if (!pInfo || (int)pInfo == -1) break;

            dwType = REG_SZ;
            dwSize = MAX_PATH;

            if (RegQueryValueEx(pInfo->hAppKey,
                                (Command == ID_APP_MODIFY) ? L"ModifyPath" : L"UninstallString",
                                NULL,
                                &dwType,
                                (LPBYTE)szPath,
                                &dwSize) == ERROR_SUCCESS)
            {
                if (StartRemoveProcess(szPath, TRUE))
                {
                    OnCommand(ID_RELOAD);
                }
            }
        }
        break;

        case ID_APP_REMOVE_REG:
        {
            WCHAR szFullName[MAX_PATH] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
            INST_APP_INFO *pInfo = ListViewGetlParam(hListView, -1);

            if (!pInfo || (int)pInfo == -1) return;

            StringCbCat(szFullName, sizeof(szFullName), pInfo->szKeyName);

            if (RegDeleteKey(pInfo->hRootKey, szFullName) == ERROR_SUCCESS)
            {
                OnCommand(ID_RELOAD);
            }
        }
        break;
    }
}

static INT CALLBACK
SortingCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    WCHAR Item1[MAX_STR_LEN], Item2[MAX_STR_LEN];
    LVFINDINFO IndexInfo = {0};
    INT Index;

    IndexInfo.flags = LVFI_PARAM;

    IndexInfo.lParam = lParam1;
    Index = ListView_FindItem(hListView, -1, &IndexInfo);
    ListView_GetItemText(hListView, Index, (INT)lParamSort, Item1, MAX_STR_LEN);

    IndexInfo.lParam = lParam2;
    Index = ListView_FindItem(hListView, -1, &IndexInfo);
    ListView_GetItemText(hListView, Index, (INT)lParamSort, Item2, MAX_STR_LEN);

    _wcsupr(Item1);
    _wcsupr(Item2);

    if (bSortAscending)
        return wcscmp(Item2, Item1);
    else
        return wcscmp(Item1, Item2);
}

LRESULT CALLBACK
MainWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_CREATE:
        {
            if (!InitControls(hwnd) ||
                !InitInfoDll())
            {
                DebugTrace(L"Initialization failed!");
                PostQuitMessage(0);
                break;
            }

            ParamsInfo.IsIoInitialized = TRUE;
            _beginthread(GUIInfoThread, 0, (LPVOID)SettingsInfo.StartupCategory);
        }
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

                    if (data->hwndFrom != hListView ||
                        !SettingsInfo.ShowAltRows) break;

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
                            {
                                if (CurrentMenu)
                                    ShowPopupMenu(CurrentMenu);
                            }
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
                        {
                            if (CurrentMenu)
                                ShowPopupMenu(CurrentMenu);
                        }
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

                        SetCanceledState(TRUE);

                        if (CurrentCategory == Category ||
                            !ParamsInfo.IsIoInitialized) break;
                        _beginthread(GUIInfoThread, 0, (LPVOID)Category);
                    }
                }
                break;

                case LVN_COLUMNCLICK:
                {
                    LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;

                    if (IsSortingAllowed)
                    {
                        ListView_SortItems(hListView,
                                           SortingCompareFunc,
                                           pnmv->iSubItem);
                        bSortAscending = !bSortAscending;
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
            INT SysColorDepth = GetSystemColorDepth();

            if (ParamsInfo.SxSmIcon != SxSmIcon ||
                ParamsInfo.SySmIcon != SySmIcon ||
                ParamsInfo.SysColorDepth != SysColorDepth)
            {
                ParamsInfo.SxSmIcon = SxSmIcon;
                ParamsInfo.SySmIcon = SySmIcon;
                ParamsInfo.SysColorDepth = SysColorDepth;

                InitInfoDll();
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
            if (InfoFreeFunction) InfoFreeFunction();

            SaveColumnsSizes();
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

                LoadMUIStringF(hLangInst, IDS_INFORMATION, szTitle, MAX_STR_LEN);
                LoadMUIStringF(hLangInst, IDS_CMD_HELP, szText, MAX_STR_LEN);
                MessageBox(0, szText, szTitle, MB_OK | MB_ICONINFORMATION);
                return TRUE;
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

    LoadDriver();

    if (!InitInfoDll())
        return FALSE;

    ParamsInfo.IsIoInitialized = TRUE;
    ReportSave(FALSE, FALSE, szPath, bNavMenu);

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

    StringCbPrintf(lpFileName, Size, L"%s-%s",
                   szIso639, szIso3166);

    return TRUE;
}

BOOL
LoadLanguage(VOID)
{
    WCHAR szPath[MAX_PATH];

    if (hLangInst)
        FreeLibrary(hLangInst);

    hLangInst = NULL;

    if (!IsIniFileExists() && ThemesInfo.szLangFile[0] == 0)
    {
        if (!GetLangFileNameFromSystem(ThemesInfo.szLangFile,
                                       sizeof(ThemesInfo.szLangFile)))
        {
            StringCbCopy(ThemesInfo.szLangFile,
                         sizeof(ThemesInfo.szLangFile),
                         L"en-US");
        }
    }
    else
    {
        if (ThemesInfo.szLangFile[0] == 0)
        {
            StringCbCopy(ThemesInfo.szLangFile,
                         sizeof(ThemesInfo.szLangFile),
                         L"en-US");
        }
    }

    StringCbCopy(szPath, sizeof(szPath), ParamsInfo.szCurrentPath);

    StringCbCat(szPath, sizeof(szPath), L"languages\\");
    StringCbCat(szPath, sizeof(szPath), ThemesInfo.szLangFile);
    StringCbCat(szPath, sizeof(szPath), L".dll");

    DebugTrace(L"Loading language file: %s", szPath);

    hLangInst = LoadLibraryEx(szPath, NULL, LOAD_LIBRARY_AS_DATAFILE);

    if (!hLangInst) return FALSE;

    return TRUE;
}

BOOL
LoadIcons(VOID)
{
    WCHAR szPath[MAX_PATH];

    if (hIconsInst)
        FreeLibrary(hIconsInst);

    hIconsInst = NULL;

    if (ThemesInfo.szIconsFile[0] == 0)
    {
        StringCbCopy(ThemesInfo.szIconsFile,
                     sizeof(ThemesInfo.szIconsFile),
                     L"tango.dll");
    }

    StringCbPrintf(szPath, sizeof(szPath),
                   L"%sicons\\%s",
                   ParamsInfo.szCurrentPath,
                   ThemesInfo.szIconsFile);

    DebugTrace(L"Loading icon file: %s", szPath);

    hIconsInst = LoadLibraryEx(szPath, NULL, LOAD_LIBRARY_AS_DATAFILE);

    if (!hIconsInst) return FALSE;

    return TRUE;
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

BOOL
IsDebugModeEnabled(VOID)
{
    WCHAR szPath[MAX_PATH];

    if (!GetCurrentPath(szPath, MAX_PATH))
        return FALSE;

    StringCbCat(szPath, sizeof(szPath), L"\\debug");

    if (GetFileAttributes(szPath) == INVALID_FILE_ATTRIBUTES)
        return FALSE;

    return TRUE;
}

BOOL
IsPortable(VOID)
{
    WCHAR szPath[MAX_PATH];

    if (!GetCurrentPath(szPath, MAX_PATH))
        return FALSE;

    StringCbCat(szPath, sizeof(szPath), L"\\portable");

    if (GetFileAttributes(szPath) == INVALID_FILE_ATTRIBUTES)
        return FALSE;

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

    /* Устанавливаем для текущего процесса отладочные привилегии */
    SetPrivilege(SE_DEBUG_NAME);

    ParamsInfo.IsPortable = IsPortable();

    /* если у нас дебаг билд, то всегда включаем DebugMode */
#ifdef _DEBUG
    ParamsInfo.DebugMode = TRUE;
#else
    ParamsInfo.DebugMode = IsDebugModeEnabled();
#endif

    if (ParamsInfo.DebugMode)
        ParamsInfo.DebugMode = InitDebugLog(L"aspia.log", VER_FILEVERSION_STR);

    /* Load settings from .ini file */
    if (!LoadSettings())
    {
        DebugTrace(L"LoadSettings() failed");

        /* If loading filed fill default values */

        SettingsInfo.StartupCategory = IDS_CAT_SUMMARY;

        /* View Settings */
        SettingsInfo.SaveWindowPos = TRUE;
        SettingsInfo.StayOnTop = FALSE;
        SettingsInfo.ShowAltRows = TRUE;
        SettingsInfo.ShowWindowStyles = TRUE;

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
    ParamsInfo.SysColorDepth = GetSystemColorDepth();
    ParamsInfo.IsIoInitialized = FALSE;

    GetCurrentPath(ParamsInfo.szCurrentPath, MAX_PATH);

    hInstance = hInst;

    hProcessHeap = GetProcessHeap();

    /* Инициализируем критическую секцию. Она используется
       для синхронизации потоков получнеия информации */
    InitializeCriticalSection(&CriticalSection);

    if (!LoadLanguage())
    {
        DebugTrace(L"LoadLanguage() failed!");
        goto Exit;
    }

    /* Пытаемся обработать аргументы командной строки */
    if (HandleCommandLine())
    {
        DebugTrace(L"HandleCommandLine() success!");
        goto Exit;
    }

    if (!LoadIcons())
    {
        DebugTrace(L"LoadIcons() failed!");
        goto Exit;
    }

    /* Загружаем драйвер режима ядра */
    LoadDriver();

    InitCommonControls();

    /* Загружаем ускорители для обработки горячих клавиш */
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

    if (ParamsInfo.IsPortable)
    {
        SettingsInfo.ShowSensorIcons = FALSE;
    }

    if (ParamsInfo.IsPortable ||
        !SettingsInfo.SaveWindowPos)
    {
        SettingsInfo.Left   = 20;
        SettingsInfo.Top    = 20;
        SettingsInfo.Right  = 850;
        SettingsInfo.Bottom = 640;

        SettingsInfo.SplitterPos = 242;
    }

    /* Создаем главное окно программы */
    hMainWnd = CreateWindowEx(WS_EX_WINDOWEDGE,
                              szWindowClass,
                              L"Aspia",
                              WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                              SettingsInfo.Left,
                              SettingsInfo.Top,
                              SettingsInfo.Right - SettingsInfo.Left,
                              SettingsInfo.Bottom - SettingsInfo.Top,
                              NULL, NULL, hInstance, NULL);

    if (!hMainWnd)
    {
        DebugTrace(L"CreateWindowEx() failed!");
        goto Exit;
    }

    if (SettingsInfo.StayOnTop)
    {
        SetWindowPos(hMainWnd,
                     HWND_TOPMOST,
                     20, 20, 850, 640,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }

    if (SettingsInfo.ShowSensorIcons &&
        !ParamsInfo.IsPortable)
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
        ShowWindow(hMainWnd, (SettingsInfo.IsMaximized) ? SW_SHOWMAXIMIZED : SW_SHOW);
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

Exit:
    UnloadDriver();

    DeleteCriticalSection(&CriticalSection);

    if (hMutex) CloseHandle(hMutex);

    CloseDebugLog();

    return 0;
}
