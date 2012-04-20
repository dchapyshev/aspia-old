/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/report.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"


static HIMAGELIST hReportImageList = NULL;

HWND hStatusDlg = NULL;
static HWND hReportTree = NULL;
static HWND hSelectAll = NULL;
static HWND hUnselectAll = NULL;
static HWND hContent = NULL;
static HWND hFilePath = NULL;
static HWND hChoosePath = NULL;
static HWND hSaveBtn = NULL;
static HWND hCloseBtn = NULL;
static HWND hFileTypeTxt = NULL;
static HWND hComboBox = NULL;
static BOOL IsGUIReport = TRUE;


static VOID
AddTreeViewItems(HWND hTree, CATEGORY_LIST *List, HTREEITEM hRoot)
{
    SIZE_T Index = 0;

    do
    {
        List[Index].hTreeItem = AddCategory(hTree,
                                            hReportImageList,
                                            hRoot,
                                            List[Index].StringID,
                                            List[Index].IconID);

        if (List[Index].Child)
        {
            AddTreeViewItems(hTree, List[Index].Child, List[Index].hTreeItem);
        }
    }
    while (List[++Index].StringID != 0);
}

static VOID
SetCheckStateTreeView(HWND hTree, CATEGORY_LIST *List)
{
    SIZE_T Index = 0;

    do
    {
        TreeView_SetCheckState(hTree, List[Index].hTreeItem, List[Index].Checked);
        if (List[Index].Child)
        {
            SetCheckStateTreeView(hTree, List[Index].Child);
        }
    }
    while (List[++Index].StringID != 0);
}

static VOID
GetCheckStateTreeView(HWND hTree, CATEGORY_LIST *List)
{
    SIZE_T Index = 0;

    do
    {
        List[Index].Checked = TreeView_GetCheckState(hTree, List[Index].hTreeItem);
        if (List[Index].Child)
            GetCheckStateTreeView(hTree, List[Index].Child);
    }
    while (List[++Index].StringID != 0);
}

static VOID
ReportAddNavigationMenu(BOOL IsSaveAll, CATEGORY_LIST *List)
{
    WCHAR szTemp[MAX_STR_LEN];
    SIZE_T Index = 0;

    do
    {
        if (List[Index].Checked || IsSaveAll)
        {
            LoadMUIStringF(hLangInst, List[Index].StringID, szTemp, MAX_STR_LEN);
            IoWriteContentTableItem(List[Index].StringID,
                                    szTemp,
                                    List[Index].Child ? TRUE : FALSE);

            if (List[Index].Child)
            {
                ReportAddNavigationMenu(IsSaveAll, List[Index].Child);
                IoWriteContentTableEndRootItem();
            }
        }
    }
    while (List[++Index].StringID != 0);
}

static BOOL
ReportAction(LPWSTR lpszRootName, BOOL IsSaveAll, CATEGORY_LIST *List)
{
    WCHAR szText[MAX_STR_LEN], szStatus[MAX_STR_LEN];
    HICON hIcon = NULL;
    SIZE_T Index = 0;
    BOOL Result = TRUE;
    INT Count = 0;

    do
    {
        if (GetCanceledState()) return FALSE;

        if (List[Index].Checked || IsSaveAll)
        {
            LoadMUIStringF(hLangInst, List[Index].StringID, szText, MAX_STR_LEN);

            IoWriteTableTitle(szText,
                              List[Index].StringID,
                              SettingsInfo.IsAddContent);

            if (hStatusDlg)
            {
                hIcon = (HICON)LoadImage(hIconsInst,
                                         MAKEINTRESOURCE(List[Index].IconID),
                                         IMAGE_ICON, 16, 16,
                                         LR_DEFAULTCOLOR);
                PostMessage(GetDlgItem(hStatusDlg, IDC_STATUS_ICON), STM_SETICON, (WPARAM)hIcon, 0);

                if (lpszRootName)
                    StringCbPrintf(szStatus, sizeof(szStatus), L"%s - %s", lpszRootName, szText);
                else
                    StringCbCopy(szStatus, sizeof(szStatus), szText);

                SetWindowText(GetDlgItem(hStatusDlg, IDC_STATUS_MSG), szStatus);
            }

            if (!List[Index].Child)
            {
                IoWriteBeginTable();
                IoAddColumnsList(List[Index].ColumnList, 0, 0);
                List[Index].InfoFunc();
                IoWriteEndTable();
                DestroyIcon(hIcon);
            }
            else
            {
                DestroyIcon(hIcon);
                Result = ReportAction(szText, IsSaveAll, List[Index].Child);
            }

            ++Count;
        }
    }
    while (List[++Index].StringID != 0);

    if (!Count) return FALSE;

    return Result;
}

UINT
GetIoTargetById(UINT id)
{
    switch (id)
    {
        case IDS_TYPE_HTML:
            return IO_TARGET_HTML;
        case IDS_TYPE_TEXT:
            return IO_TARGET_TXT;
        case IDS_TYPE_CSV:
            return IO_TARGET_CSV;
        //case IDS_TYPE_JSON:
            //return IO_TARGET_JSON;
        case IDS_TYPE_INI:
            return IO_TARGET_INI;
        case IDS_TYPE_RTF:
            return IO_TARGET_RTF;
        default:
            return IO_TARGET_HTML;
    }
}

VOID
ReportThread(IN LPVOID lpParameter)
{
    WCHAR szTitle[MAX_STR_LEN];
    BOOL IsSaveAll = (BOOL)lpParameter;
    INT OldColumnsCount;

    DebugTrace(L"ReportThread(%d) called", IsSaveAll);

    EnterCriticalSection(&CriticalSection);

    SetCanceledState(FALSE);

    IoSetTarget(GetIoTargetById(SettingsInfo.ReportFileType));
    OldColumnsCount = IoGetColumnsCount();

    DebugTrace(L"IO Target = %d", IoGetTarget());

    if (!IoCreateReport(SettingsInfo.szReportPath))
    {
        goto Cleanup;
    }

    DebugTrace(L"Report file is created!");

    if (SettingsInfo.IsAddContent)
    {
        LoadMUIStringF(hLangInst, IDS_REPORT_TITLE, szTitle, MAX_STR_LEN);
        IoWriteBeginContentTable(szTitle);
        ReportAddNavigationMenu(IsSaveAll, RootCategoryList);
        IoWriteEndContentTable();
    }

    if (!ReportAction(NULL, IsSaveAll, RootCategoryList))
    {
        IoCloseReport();
        DeleteFile(SettingsInfo.szReportPath);
    }
    else
    {
        IoCloseReport();
    }

Cleanup:
    IoSetColumnsCount(OldColumnsCount);
    IoSetTarget(IO_TARGET_LISTVIEW);

    if (IsGUIReport) EndDialog(hStatusDlg, 0);

    LeaveCriticalSection(&CriticalSection);
    if (IsGUIReport) _endthread();
}

INT_PTR CALLBACK
ReportStatusDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_INITDIALOG:
            hStatusDlg = hDlg;
            _beginthread(ReportThread, 0, (LPVOID)lParam);
            PostMessage(GetDlgItem(hDlg, IDC_REPORT_PROGRESS), PBM_SETMARQUEE, TRUE, 50);
            break;

        case WM_CLOSE:
            SetCanceledState(TRUE);
            break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    SetCanceledState(TRUE);
                    break;
            }
        }
        break;
    }

    return FALSE;
}

VOID
ReportStatusThread(IN LPVOID lpParameter)
{
    DialogBoxParam(hLangInst,
                   MAKEINTRESOURCE(IDD_REPORT_STATUS_DIALOG),
                   hMainWnd,
                   ReportStatusDlgProc,
                   (LPARAM)lpParameter);
    _endthread();
}

VOID
ReportCreateThread(IN BOOL IsSaveAll)
{
    DebugTrace(L"ReportCreateThread(%d) called", IsSaveAll);

    if (TryEnterCriticalSection(&CriticalSection))
    {
        LeaveCriticalSection(&CriticalSection);
    }
    else
    {
        return;
    }

    if (IsGUIReport)
    {
        _beginthread(ReportStatusThread, 0, (LPVOID)IsSaveAll);
    }
    else
    {
        ReportThread((LPVOID)IsSaveAll);
    }
}

UINT
GetIoTargetByFileExt(LPWSTR lpPath)
{
    WCHAR szExt[MAX_PATH] = {0};

    if (!GetFileExt(lpPath, szExt, sizeof(szExt)))
    {
        return IO_TARGET_UNKNOWN;
    }

    if (wcscmp(szExt, L"htm") == 0 || wcscmp(szExt, L"html") == 0)
        return IO_TARGET_HTML;
    //else if (wcscmp(szExt, L"jsn") == 0)
        //return IO_TARGET_JSON;
    else if (wcscmp(szExt, L"ini") == 0)
        return IO_TARGET_INI;
    else if (wcscmp(szExt, L"txt") == 0)
        return IO_TARGET_TXT;
    else if (wcscmp(szExt, L"csv") == 0)
        return IO_TARGET_CSV;
    else if (wcscmp(szExt, L"rtf") == 0)
        return IO_TARGET_RTF;

    return IO_TARGET_UNKNOWN;
}

UINT
GetIdByIoTarget(UINT id)
{
    switch (id)
    {
        case IO_TARGET_HTML:
            return IDS_TYPE_HTML;
        case IO_TARGET_TXT:
            return IDS_TYPE_TEXT;
        case IO_TARGET_CSV:
            return IDS_TYPE_CSV;
        //case IO_TARGET_JSON:
            //return IDS_TYPE_JSON;
        case IO_TARGET_INI:
            return IDS_TYPE_INI;
        case IO_TARGET_RTF:
            return IDS_TYPE_RTF;
        default:
            return 0;
    }
}

VOID
ReportSave(IN BOOL IsGUI, IN BOOL IsSaveAll,
           IN LPWSTR lpszPath, IN BOOL bWithMenu)
{
    UINT IoTarget;

    DebugTrace(L"ReportSave(%d, %d, %s, %d) called.",
               IsGUI, IsSaveAll, lpszPath, bWithMenu);

    StringCbCopy(SettingsInfo.szReportPath,
                 sizeof(SettingsInfo.szReportPath),
                 lpszPath);

    SettingsInfo.IsAddContent = bWithMenu;

    IoTarget = GetIoTargetByFileExt(lpszPath);
    if (IoTarget == IO_TARGET_UNKNOWN)
    {
        DebugTrace(L"Unkown IO target = %d", IoTarget);
        return;
    }

    SettingsInfo.ReportFileType = GetIdByIoTarget(IoTarget);

    IsGUIReport = IsGUI;

    ReportCreateThread(IsSaveAll);
}

VOID
ReportCategoryInfo(IN UINT Category,
                   IN CATEGORY_LIST *List)
{
    WCHAR szText[MAX_STR_LEN];
    SIZE_T Index = 0;

    do
    {
        if (!List[Index].Child)
        {
            if (Category == List[Index].StringID)
            {
                LoadMUIStringF(hLangInst, List[Index].StringID, szText, MAX_STR_LEN);

                IoWriteTableTitle(szText,
                                  List[Index].StringID,
                                  FALSE);
                IoWriteBeginTable();
                IoAddColumnsList(List[Index].ColumnList, 0, 0);
                List[Index].InfoFunc();
                IoWriteEndTable();
                return;
            }
        }
        else
        {
            ReportCategoryInfo(Category, List[Index].Child);
        }
    }
    while (List[++Index].StringID != 0);
}

VOID
ReportSavePage(IN LPWSTR lpszPath,
               IN UINT PageIndex)
{
    INT OldColumnsCount;
    INT IoTarget;

    if (!TryEnterCriticalSection(&CriticalSection))
        return;

    OldColumnsCount = IoGetColumnsCount();

    IoTarget = GetIoTargetByFileExt(lpszPath);
    if (IoTarget == IO_TARGET_UNKNOWN)
    {
        DebugTrace(L"Unkown IO target = %d", IoTarget);
        goto Cleanup;
    }

    SettingsInfo.ReportFileType = GetIdByIoTarget(IoTarget);
    IoSetTarget(IoTarget);

    StringCbCopy(SettingsInfo.szReportPath,
                 sizeof(SettingsInfo.szReportPath),
                 lpszPath);

    if (!IoCreateReport(SettingsInfo.szReportPath))
    {
        goto Cleanup;
    }

    ReportCategoryInfo(PageIndex, RootCategoryList);
    IoCloseReport();

Cleanup:
    IoSetColumnsCount(OldColumnsCount);
    IoSetTarget(IO_TARGET_LISTVIEW);

    LeaveCriticalSection(&CriticalSection);
}

static VOID
SetAllItemsStateTreeView(IN HWND hTree,
                         IN BOOL State,
                         IN CATEGORY_LIST *List)
{
    SIZE_T Index = 0;

    do
    {
        TreeView_SetCheckState(hTree, List[Index].hTreeItem, State);
        List[Index].Checked = State;
        if (List[Index].Child)
            SetAllItemsStateTreeView(hTree, State, List[Index].Child);
    }
    while (List[++Index].StringID != 0);
}

BOOL
IsChildItemsSelected(HWND hTree, HTREEITEM hItem)
{
    HTREEITEM hChild;

    hChild = TreeView_GetChild(hTree, hItem);

    while (hChild)
    {
        if (TreeView_GetCheckState(hTree, hChild) == 1)
            return TRUE;

        hChild = TreeView_GetNextItem(hTree, hChild, TVGN_NEXT);
    }
    return FALSE;
}

VOID
SetCheckStateForChildItems(HWND hTree, HTREEITEM hItem, BOOL State)
{
    HTREEITEM hChild, hSubChild, hSubSubChild;

    hChild = TreeView_GetChild(hTree, hItem);

    while (hChild)
    {
        TreeView_SetCheckState(hTree, hChild, State);

        hSubChild = TreeView_GetChild(hTree, hChild);

        while (hSubChild)
        {
            TreeView_SetCheckState(hTree, hSubChild, State);
            hSubChild = TreeView_GetNextSibling(hTree, hSubChild);

            hSubSubChild = TreeView_GetChild(hTree, hSubChild);
            if (!hSubSubChild) continue;

            if (IsChildItemsSelected(hTree, hSubChild)) continue;

            while (hSubSubChild)
            {
                TreeView_SetCheckState(hTree, hSubSubChild, State);
                hSubSubChild = TreeView_GetNextSibling(hTree, hSubSubChild);
            }
        }

        hChild = TreeView_GetNextItem(hTree, hChild, TVGN_NEXT);
        if (IsChildItemsSelected(hTree, hChild)) continue;
    }
}

HTREEITEM
GetChangedItem(HWND hTree, CATEGORY_LIST *List)
{
    HTREEITEM hItem = NULL;
    SIZE_T Index = 0;
    BOOL Checked;

    do
    {
        Checked = (TreeView_GetCheckState(hTree, List[Index].hTreeItem) == 1) ? TRUE : FALSE;

        if (Checked != List[Index].Checked)
            return List[Index].hTreeItem;

        if (List[Index].Child)
        {
            hItem = GetChangedItem(hTree, List[Index].Child);
            if (hItem) return hItem;
        }
    }
    while (List[++Index].StringID != 0);

    return hItem;
}

VOID
RebuildTreeChecks(HWND hTree)
{
    HTREEITEM hItem = GetChangedItem(hTree, RootCategoryList);
    HTREEITEM hParent;
    BOOL Checked;

    if (!hItem) return;

    Checked = (TreeView_GetCheckState(hTree, hItem) == 1) ? TRUE : FALSE;

    if (Checked)
    {
        hParent = TreeView_GetParent(hTree, hItem);

        while (hParent)
        {
            TreeView_SetCheckState(hTree, hParent, TRUE);
            hParent = TreeView_GetParent(hTree, hParent);
        }

        if (!IsChildItemsSelected(hTree, hItem))
        {
            SetCheckStateForChildItems(hTree, hItem, TRUE);
        }
    }
    else
    {
        HTREEITEM hChild;
        BOOL State = FALSE;

        /* Получаем сначала родительский элемент, а потом первый дочерний */
        hChild = TreeView_GetChild(hTree, TreeView_GetParent(hTree, hItem));

        while (hChild)
        {
            /* Если хотя бы один дочерний элемент выделен, то выходим */
            if (TreeView_GetCheckState(hTree, hChild) == 1)
            {
                State = TRUE;
                break;
            }

            hChild = TreeView_GetNextItem(hTree, hChild, TVGN_NEXT);
        }

        /* Если ни один дочерний элемент не выделен ... */
        if (!State)
        {
            hParent = TreeView_GetParent(hTree, hItem);

            if (hParent)
            {
                State = FALSE;

                hChild = TreeView_GetChild(hTree, hParent);

                while (hChild)
                {
                    if (TreeView_GetCheckState(hTree, hItem) == 1)
                    {
                        State = TRUE;
                        break;
                    }
                    hChild = TreeView_GetNextItem(hTree, hChild, TVGN_NEXT);
                }

                if (!State)
                {
                    TreeView_SetCheckState(hTree, hParent, FALSE);

                    State = FALSE;

                    hChild = TreeView_GetChild(hTree, TreeView_GetParent(hTree, hParent));
                    while (hChild)
                    {
                        /* Если хотя бы один дочерний элемент выделен, то выходим */
                        if (TreeView_GetCheckState(hTree, hChild) == 1)
                        {
                            State = TRUE;
                            break;
                        }

                        hChild = TreeView_GetNextItem(hTree, hChild, TVGN_NEXT);
                    }

                    if (!State)
                    {
                        TreeView_SetCheckState(hTree, TreeView_GetParent(hTree, hParent), FALSE);
                    }
                }
            }
        }

        SetCheckStateForChildItems(hTree, hItem, FALSE);
    }
}

static VOID CALLBACK
UpdateProc(HWND hwnd, UINT msg, UINT id, DWORD systime)
{
    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(systime);

    RebuildTreeChecks(hwnd);
    GetCheckStateTreeView(hwnd, RootCategoryList);
}

#define IDT_UPDATE_TIMER 501

static HICON hCheckAllIcon = NULL;
static HICON hUnCheckAllIcon = NULL;


VOID
AddFileTypeToComboBox(HWND hCombo, UINT StringID)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    LoadMUIStringF(hLangInst, StringID, szText, MAX_STR_LEN);

    Index = SendMessage(hCombo,
                        CB_ADDSTRING, 0,
                        (LPARAM)szText);

    SendMessage(hCombo, CB_SETITEMDATA, Index, StringID);

    if (SettingsInfo.ReportFileType == StringID)
        SendMessage(hCombo, CB_SETCURSEL, Index, 0);
}

VOID
GetReportExtById(UINT id, LPWSTR lpExt, SIZE_T Size)
{
    WCHAR *szExt;

    switch (id)
    {
        case IDS_TYPE_HTML:
            szExt = L".htm";
            break;
        case IDS_TYPE_TEXT:
            szExt = L".txt";
            break;
        case IDS_TYPE_CSV:
            szExt = L".csv";
            break;
        //case IDS_TYPE_JSON:
            //szExt = L".jsn";
            //break;
        case IDS_TYPE_INI:
            szExt = L".ini";
            break;
        case IDS_TYPE_RTF:
            szExt = L".rtf";
            break;
        default:
            szExt = L".htm";
            break;
    }
    StringCbCopy(lpExt, Size, szExt);
}

BOOL
ReportSaveFileDialog(HWND hDlg, LPWSTR lpszPath, SIZE_T PathSize)
{
    OPENFILENAME saveas = {0};
    WCHAR szPath[MAX_PATH];
    DWORD dwSize;

    dwSize = MAX_PATH;
    GetComputerName(szPath, &dwSize);
    StringCbCat(szPath, sizeof(szPath), L".htm");

    saveas.lStructSize     = sizeof(OPENFILENAME);
    saveas.hwndOwner       = hDlg;
    saveas.hInstance       = hInstance;
    //saveas.lpstrFilter     = L"HTML File (*.htm)\0*.htm\0RTF File (*.rtf)\0*.rtf\0Text File (*.txt)\0*.txt\0JSON File (*.jsn)\0*.jsn\0INI File (*.ini)\0*.ini\0CSV File (*.csv)\0*.csv\0\0";
    saveas.lpstrFilter     = L"HTML File (*.htm)\0*.htm\0RTF File (*.rtf)\0*.rtf\0Text File (*.txt)\0*.txt\0INI File (*.ini)\0*.ini\0CSV File (*.csv)\0*.csv\0\0";
    saveas.lpstrFile       = szPath;
    saveas.nMaxFile        = MAX_PATH;
    saveas.lpstrInitialDir = NULL;
    saveas.Flags           = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT |
                             OFN_HIDEREADONLY | OFN_EXPLORER;
    saveas.lpstrDefExt     = L"htm";

    if (GetSaveFileName(&saveas))
    {
        StringCbCopy(lpszPath, PathSize, szPath);
        return TRUE;
    }

    return FALSE;
}

VOID
ReportWindowOnSize(LPARAM lParam)
{
    HDWP hdwp = BeginDeferWindowPos(10);

#define SELECT_BUTTON_WIDTH      30
#define SELECT_BUTTON_HEIGHT     30
#define CHECKBOX_HEIGHT          20
#define FILEPATH_EDIT_HEIGHT     20
#define CHOOSE_PATH_BUTTON_WIDTH 30
#define BUTTON_WIDTH             100
#define BUTTON_HEIGHT            25
#define FILE_TYPE_TEXT_HEIGHT    20
#define FILE_TYPE_TEXT_WIDTH     100
#define FILE_TYPE_COMBO_HEIGHT   18

#define ITEMS_DIVIDER 6

    /*
     * HIWORD(lParam) - Height of main window
     * LOWORD(lParam) - Width of main window
     */

    /* Size TreeView */
    DeferWindowPos(hdwp,
                   hReportTree,
                   0,
                   ITEMS_DIVIDER, ITEMS_DIVIDER,
                   LOWORD(lParam) - SELECT_BUTTON_WIDTH - (ITEMS_DIVIDER * 3),
                   HIWORD(lParam) - (ITEMS_DIVIDER * 6) - CHECKBOX_HEIGHT - FILEPATH_EDIT_HEIGHT - FILE_TYPE_COMBO_HEIGHT - BUTTON_HEIGHT - 10,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "Select All" button */
    DeferWindowPos(hdwp,
                   hSelectAll,
                   0,
                   LOWORD(lParam) - SELECT_BUTTON_WIDTH - ITEMS_DIVIDER,
                   ITEMS_DIVIDER,
                   SELECT_BUTTON_WIDTH,
                   SELECT_BUTTON_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "Unselect All" button */
    DeferWindowPos(hdwp,
                   hUnselectAll,
                   0,
                   LOWORD(lParam) - SELECT_BUTTON_WIDTH - ITEMS_DIVIDER,
                   SELECT_BUTTON_HEIGHT + (ITEMS_DIVIDER * 2),
                   SELECT_BUTTON_WIDTH,
                   SELECT_BUTTON_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "Content Table" checkbox */
    DeferWindowPos(hdwp,
                   hContent,
                   0,
                   ITEMS_DIVIDER * 2,
                   HIWORD(lParam) - CHECKBOX_HEIGHT - FILEPATH_EDIT_HEIGHT - BUTTON_HEIGHT - FILE_TYPE_COMBO_HEIGHT - (ITEMS_DIVIDER * 4) - 10,
                   LOWORD(lParam) - (ITEMS_DIVIDER * 3),
                   CHECKBOX_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "File Path" edit */
    DeferWindowPos(hdwp,
                   hFilePath,
                   0,
                   ITEMS_DIVIDER,
                   HIWORD(lParam) - FILEPATH_EDIT_HEIGHT- BUTTON_HEIGHT - FILE_TYPE_COMBO_HEIGHT - (ITEMS_DIVIDER * 3) - 10,
                   LOWORD(lParam) - CHOOSE_PATH_BUTTON_WIDTH - (ITEMS_DIVIDER * 3),
                   FILEPATH_EDIT_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "Choose File Path" button */
    DeferWindowPos(hdwp,
                   hChoosePath,
                   0,
                   LOWORD(lParam) - CHOOSE_PATH_BUTTON_WIDTH - ITEMS_DIVIDER,
                   HIWORD(lParam) - FILEPATH_EDIT_HEIGHT - BUTTON_HEIGHT - FILE_TYPE_COMBO_HEIGHT - (ITEMS_DIVIDER * 3) - 10,
                   CHOOSE_PATH_BUTTON_WIDTH,
                   FILEPATH_EDIT_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "File Type" text */
    DeferWindowPos(hdwp,
                   hFileTypeTxt,
                   0,
                   ITEMS_DIVIDER,
                   HIWORD(lParam) - BUTTON_HEIGHT- FILE_TYPE_TEXT_HEIGHT - (ITEMS_DIVIDER * 2) - 5,
                   FILE_TYPE_TEXT_WIDTH,
                   FILE_TYPE_TEXT_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "File Type" combobox */
    DeferWindowPos(hdwp,
                   hComboBox,
                   0,
                   FILE_TYPE_TEXT_WIDTH + (ITEMS_DIVIDER * 2),
                   HIWORD(lParam) - BUTTON_HEIGHT - FILE_TYPE_TEXT_HEIGHT - (ITEMS_DIVIDER * 2) - 10,
                   LOWORD(lParam) - FILE_TYPE_TEXT_WIDTH - (ITEMS_DIVIDER * 3),
                   FILE_TYPE_COMBO_HEIGHT + 150,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "Save" button */
    DeferWindowPos(hdwp,
                   hSaveBtn,
                   0,
                   LOWORD(lParam) - (BUTTON_WIDTH * 2) - (ITEMS_DIVIDER * 2),
                   HIWORD(lParam) - BUTTON_HEIGHT - ITEMS_DIVIDER,
                   BUTTON_WIDTH,
                   BUTTON_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "Close" button */
    DeferWindowPos(hdwp,
                   hCloseBtn,
                   0,
                   LOWORD(lParam) - BUTTON_WIDTH - ITEMS_DIVIDER,
                   HIWORD(lParam) - BUTTON_HEIGHT - ITEMS_DIVIDER,
                   BUTTON_WIDTH,
                   BUTTON_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);
}

VOID
ReportWindowInitControls(HWND hwnd)
{
    WCHAR szText[MAX_STR_LEN];
    DWORD dwSize;

    /* Initialize TreeView */
    hReportTree = CreateWindowEx(WS_EX_CLIENTEDGE,
                                 WC_TREEVIEW,
                                 L"",
                                 WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASLINES |
                                 TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_LINESATROOT |
                                 TVS_CHECKBOXES,
                                 0, 0, 0, 0,
                                 hwnd,
                                 NULL,
                                 hInstance,
                                 NULL);

    if (!hReportTree)
    {
        DebugTrace(L"Unable to create TreeView control!");
        return;
    }

    hReportImageList = ImageList_Create(ParamsInfo.SxSmIcon,
                                        ParamsInfo.SySmIcon,
                                        ILC_MASK | ParamsInfo.SysColorDepth,
                                        1, 1);
    if (!hReportImageList)
    {
        DebugTrace(L"Unable to create ImageList!");
        return;
    }

    AddTreeViewItems(hReportTree, RootCategoryList, TVI_ROOT);
    TreeView_SetImageList(hReportTree, hReportImageList, LVSIL_NORMAL);

    /* TreeView checkboxes state */
    SetCheckStateTreeView(hReportTree, RootCategoryList);

    /* Try to set theme for TreeView */
    IntSetWindowTheme(hReportTree);

    /* Create "Select All" button */
    hSelectAll = CreateWindow(L"Button", L"",
                              WS_CHILD | WS_VISIBLE | BS_ICON,
                              0, 0, 0, 0,
                              hwnd, 0, hInstance, NULL);

    /* Set icon for button */
    hCheckAllIcon = (HICON)LoadImage(hIconsInst,
                                     MAKEINTRESOURCE(IDI_CHECK_ALL),
                                     IMAGE_ICON,
                                     ParamsInfo.SxSmIcon,
                                     ParamsInfo.SySmIcon,
                                     LR_DEFAULTCOLOR);
    SendMessage(hSelectAll, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hCheckAllIcon);

    /* Create "Unselect All" button */
    hUnselectAll = CreateWindow(L"Button", L"",
                                WS_CHILD | WS_VISIBLE | BS_ICON,
                                0, 0, 0, 0,
                                hwnd, 0, hInstance, NULL);

    /* Set icon for button */
    hUnCheckAllIcon = (HICON)LoadImage(hIconsInst,
                                       MAKEINTRESOURCE(IDI_UNCHECK_ALL),
                                       IMAGE_ICON,
                                       ParamsInfo.SxSmIcon,
                                       ParamsInfo.SySmIcon,
                                       LR_DEFAULTCOLOR);
    SendMessage(hUnselectAll, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hUnCheckAllIcon);

    hContent = CreateWindow(L"Button",
                            L"",
                            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                            0, 0, 0, 0,
                            hwnd, 0, hInstance, NULL);

    if (!hContent)
    {
        DebugTrace(L"Unable to create CheckBox control!");
        return;
    }

    /* Set checkbox text and font */
    SendMessage(hContent, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
    LoadMUIStringF(hLangInst, IDS_ADD_CONTENT_TABLE, szText, MAX_STR_LEN);
    SetWindowText(hContent, szText);

    /* Set checkbox state */
    SendMessage(hContent, BM_SETCHECK,
                (SettingsInfo.IsAddContent ? BST_CHECKED : BST_UNCHECKED), 0);

    /* File path edit */
    hFilePath = CreateWindowEx(WS_EX_CLIENTEDGE,
                               L"Edit", NULL,
                               WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | ES_READONLY | ES_AUTOHSCROLL,
                               0, 0, 0, 0,
                               hwnd, 0, hInstance, NULL);
    if (!hFilePath)
    {
        DebugTrace(L"Unable to create Edit control!");
        return;
    }
    /* Set font for file path edit */
    SendMessage(hFilePath, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

    /* Set default file path */
    if (SettingsInfo.szReportPath[0] == 0)
    {
        WCHAR szExt[MAX_STR_LEN];

        SHGetSpecialFolderPath(hwnd,
                               SettingsInfo.szReportPath,
                               CSIDL_MYDOCUMENTS, FALSE);

        dwSize = MAX_PATH;
        GetComputerName(szText, &dwSize);

        StringCbCat(SettingsInfo.szReportPath,
                    sizeof(SettingsInfo.szReportPath), L"\\");
        StringCbCat(SettingsInfo.szReportPath,
                    sizeof(SettingsInfo.szReportPath), szText);

        GetReportExtById(SettingsInfo.ReportFileType, szExt, sizeof(szExt));

        StringCbCat(SettingsInfo.szReportPath,
                    sizeof(SettingsInfo.szReportPath), szExt);
    }

    SetWindowText(hFilePath, SettingsInfo.szReportPath);

    /* Choose file path button */
    hChoosePath = CreateWindow(L"Button", L"",
                               WS_CHILD | WS_VISIBLE,
                               0, 0, 0, 0,
                               hwnd, 0, hInstance, NULL);
    if (!hChoosePath)
    {
        DebugTrace(L"Unable to create button!");
        return;
    }
    SetWindowText(hChoosePath, L"...");
    SendMessage(hChoosePath, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

    /* "File Type" text */
    hFileTypeTxt = CreateWindow(L"STATIC", L"",
                                WS_CHILD | WS_VISIBLE,
                                0, 0, 0, 0,
                                hwnd, 0, hInstance, NULL);
    if (!hFileTypeTxt)
    {
        DebugTrace(L"Unable to create window!");
        return;
    }
    LoadMUIStringF(hLangInst, IDS_FILE_TYPE_TEXT, szText, MAX_STR_LEN);
    SetWindowText(hFileTypeTxt, szText);
    SendMessage(hFileTypeTxt, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

    /* File type combobox */
    hComboBox = CreateWindow(L"COMBOBOX", L"",
                             CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL,
                             0, 0, 0, 150,
                             hwnd, 0, hInstance, NULL);
    if (!hComboBox)
    {
        DebugTrace(L"Unable to create ComboBox!");
        return;
    }
    SendMessage(hComboBox, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

    AddFileTypeToComboBox(hComboBox, IDS_TYPE_HTML);
    AddFileTypeToComboBox(hComboBox, IDS_TYPE_TEXT);
    AddFileTypeToComboBox(hComboBox, IDS_TYPE_CSV);
    //AddFileTypeToComboBox(hComboBox, IDS_TYPE_JSON);
    AddFileTypeToComboBox(hComboBox, IDS_TYPE_INI);
    AddFileTypeToComboBox(hComboBox, IDS_TYPE_RTF);

    /* "Save" button */
    hSaveBtn = CreateWindow(L"Button", L"",
                            WS_CHILD | WS_VISIBLE,
                            0, 0, 0, 0,
                            hwnd, 0, hInstance, NULL);
    if (!hSaveBtn)
    {
        DebugTrace(L"Unable to create button!");
        return;
    }
    LoadMUIStringF(hLangInst, IDS_SAVE_BTN, szText, MAX_STR_LEN);
    SetWindowText(hSaveBtn, szText);
    SendMessage(hSaveBtn, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

    /* "Close" button */
    hCloseBtn = CreateWindow(L"Button", L"",
                             WS_CHILD | WS_VISIBLE,
                             0, 0, 0, 0,
                             hwnd, 0, hInstance, NULL);
    if (!hCloseBtn)
    {
        DebugTrace(L"Unable to create button!");
        return;
    }
    LoadMUIStringF(hLangInst, IDS_CLOSE_BTN, szText, MAX_STR_LEN);
    SetWindowText(hCloseBtn, szText);
    SendMessage(hCloseBtn, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

    SetTimer(hReportTree, IDT_UPDATE_TIMER, 10, UpdateProc);
}

VOID
SelectStringByItemData(UINT StringID)
{
    INT Count = SendMessage(hComboBox, CB_GETCOUNT, 0, 0) - 1;

    while (Count >= 0)
    {
        if (SendMessage(hComboBox, CB_GETITEMDATA, Count, 0) == StringID)
        {
            SendMessage(hComboBox, CB_SETCURSEL, Count, 0);
        }
        Count--;
    }
}

VOID
ReportWindowOnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    if (HIWORD(wParam) == BN_CLICKED)
    {
        if (lParam == (LPARAM)hSaveBtn)
        {
            SettingsInfo.IsAddContent =
                (SendMessage(hContent, BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE;

            GetCheckStateTreeView(hReportTree, RootCategoryList);

            KillTimer(hMainWnd, IDT_UPDATE_TIMER);

            IsGUIReport = TRUE;
            ReportCreateThread(FALSE);

            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        else if (lParam == (LPARAM)hCloseBtn)
        {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        else if (lParam == (LPARAM)hChoosePath)
        {
            if (ReportSaveFileDialog(hwnd,
                                     SettingsInfo.szReportPath,
                                     sizeof(SettingsInfo.szReportPath)))
            {
                UINT StringId;

                SetWindowText(hFilePath, SettingsInfo.szReportPath);
                StringId = GetIdByIoTarget(GetIoTargetByFileExt(SettingsInfo.szReportPath));

                SettingsInfo.ReportFileType = StringId;
                SelectStringByItemData(StringId);
            }
        }
        else if (lParam == (LPARAM)hSelectAll)
        {
            SetAllItemsStateTreeView(hReportTree, TRUE, RootCategoryList);
        }
        else if (lParam == (LPARAM)hUnselectAll)
        {
            SetAllItemsStateTreeView(hReportTree, FALSE, RootCategoryList);
        }
    }
    else if (HIWORD(wParam) == CBN_SELCHANGE && lParam == (LPARAM)hComboBox)
    {
        INT Index = SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
        UINT Data = (UINT)SendMessage(hComboBox, CB_GETITEMDATA, Index, 0);
        WCHAR szPath[MAX_PATH];
        WCHAR szExt[MAX_STR_LEN];

        if (Data == CB_ERR) return;

        SettingsInfo.ReportFileType = Data;

        GetReportExtById(Data, szExt, sizeof(szExt));

        GetWindowText(hFilePath, szPath, MAX_PATH);
        for (Index = wcslen(szPath); Index > 0; Index--)
        {
            if (szPath[Index] == L'.')
            {
                szPath[Index] = L'\0';
                break;
            }
        }
        StringCbCat(szPath, sizeof(szPath), szExt);
        SetWindowText(hFilePath, szPath);
        StringCbCopy(SettingsInfo.szReportPath,
                     sizeof(SettingsInfo.szReportPath),
                     szPath);
    }
}

LRESULT CALLBACK
ReportWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_CREATE:
            ReportWindowInitControls(hwnd);
            break;

        case WM_COMMAND:
            ReportWindowOnCommand(hwnd, wParam, lParam);
            break;

        case WM_SIZE:
            ReportWindowOnSize(lParam);
            break;

        case WM_SIZING:
        {
            LPRECT pRect = (LPRECT)lParam;

            if (pRect->right - pRect->left < 300)
                pRect->right = pRect->left + 300;

            if (pRect->bottom - pRect->top < 350)
                pRect->bottom = pRect->top + 350;
        }
        break;

        case WM_DESTROY:
        {
            WINDOWPLACEMENT wp;

            SettingsInfo.IsAddContent =
                (SendMessage(hContent, BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE;

            RebuildTreeChecks(hReportTree);
            GetCheckStateTreeView(hReportTree, RootCategoryList);

            KillTimer(hMainWnd, IDT_UPDATE_TIMER);

            DestroyIcon(hCheckAllIcon);
            DestroyIcon(hUnCheckAllIcon);

            wp.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement(hwnd, &wp);

            SettingsInfo.ReportIsMaximized =
                (IsZoomed(hwnd) || (wp.flags & WPF_RESTORETOMAXIMIZED));

            if (!SettingsInfo.ReportIsMaximized)
            {
                SettingsInfo.ReportLeft   = wp.rcNormalPosition.left;
                SettingsInfo.ReportTop    = wp.rcNormalPosition.top;
                SettingsInfo.ReportRight  = wp.rcNormalPosition.right;
                SettingsInfo.ReportBottom = wp.rcNormalPosition.bottom;
            }

            PostQuitMessage(0);

            ShowWindow(hMainWnd, SW_SHOW);
            BringWindowToTop(hMainWnd);
        }
        return 0;
    }

    return DefWindowProc(hwnd, Msg, wParam, lParam);
}

VOID
CreateReportWindow(VOID)
{
    WNDCLASSEX WndClass = {0};
    WCHAR szWindowClass[] = L"ASPIAISREPORT";
    WCHAR szWindowName[MAX_STR_LEN];
    HWND hReportWnd;
    MSG Msg;

    /* Create the window */
    WndClass.cbSize        = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = szWindowClass;
    WndClass.lpfnWndProc   = ReportWindowProc;
    WndClass.hInstance     = hInstance;
    WndClass.style         = CS_HREDRAW | CS_VREDRAW;
    WndClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.lpszMenuName  = NULL;

    if (RegisterClassEx(&WndClass) == (ATOM)0)
    {
        DebugTrace(L"RegisterClassEx() failed!");
        return;
    }

    LoadMUIStringF(hLangInst, IDS_REPORTWND_TITLE,
                   szWindowName, MAX_STR_LEN);

    if (!SettingsInfo.SaveWindowPos)
    {
        SettingsInfo.Left   = 20;
        SettingsInfo.Top    = 20;
        SettingsInfo.Right  = 400;
        SettingsInfo.Bottom = 500;
    }

    /* Создаем главное окно программы */
    hReportWnd = CreateWindowEx(WS_EX_WINDOWEDGE,
                                szWindowClass,
                                szWindowName,
                                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                SettingsInfo.ReportLeft,
                                SettingsInfo.ReportTop,
                                SettingsInfo.ReportRight - SettingsInfo.ReportLeft,
                                SettingsInfo.ReportBottom - SettingsInfo.ReportTop,
                                NULL, NULL, hInstance, NULL);

    if (!hReportWnd)
    {
        DebugTrace(L"CreateWindowEx() failed!");
        UnregisterClass(szWindowClass, hInstance);
        return;
    }

    /* Show it */
    ShowWindow(hReportWnd, (SettingsInfo.ReportIsMaximized) ? SW_SHOWMAXIMIZED : SW_SHOW);
    UpdateWindow(hReportWnd);

    /* Hide main window */
    ShowWindow(hMainWnd, SW_HIDE);

    /* Message Loop */
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    UnregisterClass(szWindowClass, hInstance);
}
