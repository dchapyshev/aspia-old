/*
 * PROJECT:         Aspia
 * FILE:            aspia/report.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"


static HIMAGELIST hReportImageList = NULL;
HWND hStatusDlg = NULL;
static BOOL IsCanceled = FALSE;


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
            LoadMUIString(List[Index].StringID, szTemp, MAX_STR_LEN);
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
        if (IsCanceled) return FALSE;

        if (List[Index].Checked || IsSaveAll)
        {
            LoadMUIString(List[Index].StringID, szText, MAX_STR_LEN);

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
                IoAddColumnsList(List[Index].ColumnList);
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
        case IDS_TYPE_JSON:
            return IO_TARGET_JSON;
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

    EnterCriticalSection(&CriticalSection);

    IsCanceled = FALSE;

    IoSetTarget(GetIoTargetById(SettingsInfo.ReportFileType));
    OldColumnsCount = IoGetColumnsCount();

    IoCreateReport(SettingsInfo.szReportPath);

    if (SettingsInfo.IsAddContent)
    {
        LoadMUIString(IDS_REPORT_TITLE, szTitle, MAX_STR_LEN);
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

    IoSetColumnsCount(OldColumnsCount);
    IoSetTarget(IO_TARGET_LISTVIEW);

    EndDialog(hStatusDlg, 0);

    LeaveCriticalSection(&CriticalSection);
    _endthread();
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
            IsCanceled = TRUE;
            break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    IsCanceled = TRUE;
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
ReportCreateThread(IN BOOL IsGUI, IN BOOL IsSaveAll)
{
    if (TryEnterCriticalSection(&CriticalSection))
        LeaveCriticalSection(&CriticalSection);
    else
        return;

    if (IsGUI)
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
    WCHAR szExt[MAX_PATH];
    INT i, j = 0, len = wcslen(lpPath);

    if (len < 3) return IO_TARGET_HTML;

    for (i = len - 3; i < len; i++, j++)
    {
        szExt[j] = lpPath[i];
    }
    szExt[3] = 0;

    if (wcscmp(szExt, L"htm") == 0)
        return IO_TARGET_HTML;
    else if (wcscmp(szExt, L"jsn") == 0)
        return IO_TARGET_JSON;
    else if (wcscmp(szExt, L"ini") == 0)
        return IO_TARGET_INI;
    else if (wcscmp(szExt, L"txt") == 0)
        return IO_TARGET_TXT;
    else if (wcscmp(szExt, L"csv") == 0)
        return IO_TARGET_CSV;
    else if (wcscmp(szExt, L"rtf") == 0)
        return IO_TARGET_RTF;

    return IO_TARGET_HTML;
}

VOID
ReportSaveAll(IN BOOL IsGUI, IN LPWSTR lpszPath, IN BOOL bWithMenu)
{
    StringCbCopy(SettingsInfo.szReportPath,
                 sizeof(SettingsInfo.szReportPath),
                 lpszPath);

    SettingsInfo.IsAddContent = bWithMenu;

    IoSetTarget(GetIoTargetByFileExt(lpszPath));

    ReportCreateThread(IsGUI, TRUE);
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
                IoAddColumnsList(List[Index].ColumnList);
                LoadMUIString(List[Index].StringID, szText, MAX_STR_LEN);

                IoWriteTableTitle(szText,
                                  List[Index].StringID,
                                  FALSE);
                IoWriteBeginTable();
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

    if (!TryEnterCriticalSection(&CriticalSection))
        return;

    OldColumnsCount = IoGetColumnsCount();

    IoSetTarget(GetIoTargetByFileExt(lpszPath));

    StringCbCopy(SettingsInfo.szReportPath,
                 sizeof(SettingsInfo.szReportPath),
                 lpszPath);
    IoCreateReport(SettingsInfo.szReportPath);
    ReportCategoryInfo(PageIndex, RootCategoryList);
    IoCloseReport();

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
        hChild = TreeView_GetNextItem(hTree, hChild, TVGN_NEXT);

        hSubChild = TreeView_GetChild(hTree, hChild);
        if (!hSubChild) continue;

        if (IsChildItemsSelected(hTree, hChild)) continue;

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
GetCheckBoxesState(HWND hDlg)
{
    SettingsInfo.IsAddContent =
       (IsDlgButtonChecked(hDlg, IDC_ADD_CONTENT) == BST_CHECKED) ? TRUE : FALSE;

    SettingsInfo.ELogShowError =
       (IsDlgButtonChecked(hDlg, IDC_FILTER_ELOG_ERROR) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.ELogShowWarning =
       (IsDlgButtonChecked(hDlg, IDC_FILTER_ELOG_WARNING) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.ELogShowInfo =
       (IsDlgButtonChecked(hDlg, IDC_FILTER_ELOG_INFO) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.IEShowFile =
       (IsDlgButtonChecked(hDlg, IDC_FILTER_IE_FILE) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.IEShowHttp =
       (IsDlgButtonChecked(hDlg, IDC_FILTER_IE_HTTP) == BST_CHECKED) ? TRUE : FALSE;
    SettingsInfo.IEShowFtp =
       (IsDlgButtonChecked(hDlg, IDC_FILTER_IE_FTP) == BST_CHECKED) ? TRUE : FALSE;
}

VOID
AddFileTypeToComboBox(HWND hCombo, UINT StringID)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    LoadMUIString(StringID, szText, MAX_STR_LEN);

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
        case IDS_TYPE_JSON:
            szExt = L".jsn";
            break;
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
    saveas.lpstrFilter     = L"HTML File (*.htm)\0*.htm\0RTF File (*.rtf)\0*.rtf\0Text File (*.txt)\0*.txt\0XML File (*.xml)\0*.xml\0INI File (*.ini)\0*.ini\0CSV File (*.csv)\0*.csv\0\0";
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

INT_PTR CALLBACK
ReportDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    HWND hTree = GetDlgItem(hDlg, IDC_CATEGORIES_TREE);
    HWND hCombo = GetDlgItem(hDlg, IDC_FILE_TYPE_COMBO);

    UNREFERENCED_PARAMETER(lParam);

    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            WCHAR szFileName[MAX_PATH];
            DWORD dwSize;

            DebugTrace(L"Dialog init");

            CheckDlgButton(hDlg, IDC_ADD_CONTENT,
                           SettingsInfo.IsAddContent ? BST_CHECKED : BST_UNCHECKED);

            /* Initialize TreeView */
            hReportImageList = ImageList_Create(ParamsInfo.SxSmIcon,
                                                ParamsInfo.SySmIcon,
                                                ILC_MASK | ParamsInfo.SysColorDepth,
                                                1, 1);
            AddTreeViewItems(hTree, RootCategoryList, TVI_ROOT);
            TreeView_SetImageList(hTree, hReportImageList, LVSIL_NORMAL);

            /* Checkbox'es */
            SetCheckStateTreeView(hTree, RootCategoryList);

            /* Set icons for buttons */
            hCheckAllIcon = (HICON)LoadImage(hIconsInst,
                                             MAKEINTRESOURCE(IDI_CHECK_ALL),
                                             IMAGE_ICON,
                                             ParamsInfo.SxSmIcon,
                                             ParamsInfo.SySmIcon,
                                             LR_DEFAULTCOLOR);
            SendMessage(GetDlgItem(hDlg, IDC_SELECT_ALL),
                        BM_SETIMAGE, IMAGE_ICON, (LPARAM)hCheckAllIcon);

            hUnCheckAllIcon = (HICON)LoadImage(hIconsInst,
                                               MAKEINTRESOURCE(IDI_UNCHECK_ALL),
                                               IMAGE_ICON,
                                               ParamsInfo.SxSmIcon,
                                               ParamsInfo.SySmIcon,
                                               LR_DEFAULTCOLOR);
            SendMessage(GetDlgItem(hDlg, IDC_UNSELECT_ALL),
                        BM_SETIMAGE, IMAGE_ICON, (LPARAM)hUnCheckAllIcon);

            /* Set default file path */
            if (SafeStrLen(SettingsInfo.szReportPath) == 0)
            {
                WCHAR szExt[MAX_STR_LEN];

                SHGetSpecialFolderPath(hDlg,
                                       SettingsInfo.szReportPath,
                                       CSIDL_MYDOCUMENTS, FALSE);

                dwSize = MAX_PATH;
                GetComputerName(szFileName, &dwSize);

                StringCbCat(SettingsInfo.szReportPath,
                            sizeof(SettingsInfo.szReportPath), L"\\");
                StringCbCat(SettingsInfo.szReportPath,
                            sizeof(SettingsInfo.szReportPath), szFileName);

                GetReportExtById(SettingsInfo.ReportFileType, szExt, sizeof(szExt));

                StringCbCat(SettingsInfo.szReportPath,
                            sizeof(SettingsInfo.szReportPath), szExt);
            }

            SetWindowText(GetDlgItem(hDlg, IDC_FILEPATH_EDIT),
                          SettingsInfo.szReportPath);

            AddFileTypeToComboBox(hCombo, IDS_TYPE_HTML);
            AddFileTypeToComboBox(hCombo, IDS_TYPE_TEXT);
            AddFileTypeToComboBox(hCombo, IDS_TYPE_CSV);
            AddFileTypeToComboBox(hCombo, IDS_TYPE_JSON);
            AddFileTypeToComboBox(hCombo, IDS_TYPE_INI);
            AddFileTypeToComboBox(hCombo, IDS_TYPE_RTF);

            SetTimer(hTree, IDT_UPDATE_TIMER, 10, UpdateProc);
        }
        break;

        case WM_CLOSE:
        {
            DebugTrace(L"Dialog close");

            GetCheckBoxesState(hDlg);

            RebuildTreeChecks(hTree);
            GetCheckStateTreeView(hTree, RootCategoryList);

            KillTimer(hMainWnd, IDT_UPDATE_TIMER);

            DestroyIcon(hCheckAllIcon);
            DestroyIcon(hUnCheckAllIcon);

            EndDialog(hDlg, LOWORD(wParam));
        }
        break;

        case WM_NOTIFY:
        break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_FILE_TYPE_COMBO:
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        INT Index = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
                        UINT Data = (UINT)SendMessage(hCombo, CB_GETITEMDATA, Index, 0);
                        HWND hEdit = GetDlgItem(hDlg, IDC_FILEPATH_EDIT);
                        WCHAR szPath[MAX_PATH];
                        WCHAR szExt[MAX_STR_LEN];

                        if (Data == CB_ERR) break;

                        SettingsInfo.ReportFileType = Data;

                        GetReportExtById(Data, szExt, sizeof(szExt));

                        GetWindowText(hEdit, szPath, MAX_PATH);
                        for (Index = wcslen(szPath); Index > 0; Index--)
                        {
                            if (szPath[Index] == L'.')
                            {
                                szPath[Index] = L'\0';
                                break;
                            }
                        }
                        StringCbCat(szPath, sizeof(szPath), szExt);
                        SetWindowText(hEdit, szPath);
                        StringCbCopy(SettingsInfo.szReportPath,
                                     sizeof(SettingsInfo.szReportPath),
                                     szPath);
                    }
                }
                break;

                case IDC_SELECT_ALL:
                    SetAllItemsStateTreeView(hTree, TRUE, RootCategoryList);
                    break;

                case IDC_UNSELECT_ALL:
                    SetAllItemsStateTreeView(hTree, FALSE, RootCategoryList);
                    break;

                case IDC_SET_PATH_BTN:
                {
                    if (ReportSaveFileDialog(hDlg,
                                             SettingsInfo.szReportPath,
                                             sizeof(SettingsInfo.szReportPath)))
                    {
                        SetWindowText(GetDlgItem(hDlg, IDC_FILEPATH_EDIT),
                                      SettingsInfo.szReportPath);
                    }
                }
                break;

                case IDOK:
                {
                    DebugTrace(L"Dialog close");

                    GetCheckBoxesState(hDlg);

                    GetCheckStateTreeView(hTree, RootCategoryList);

                    KillTimer(hMainWnd, IDT_UPDATE_TIMER);

                    DestroyIcon(hCheckAllIcon);
                    DestroyIcon(hUnCheckAllIcon);

                    ReportCreateThread(TRUE, FALSE);

                    EndDialog(hDlg, LOWORD(wParam));
                }
                break;

                case IDCANCEL:
                    PostMessage(hDlg, WM_CLOSE, 0, 0);
                    break;
            }
        }
        break;
    }

    return FALSE;
}
