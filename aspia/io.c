/*
 * PROJECT:         Aspia
 * FILE:            aspia/io.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"


BOOL HtmlAppendStringToFile(LPWSTR lpszString);
BOOL HtmlCreateReport(LPWSTR lpszFile);
VOID HtmlCloseReport(VOID);
VOID HtmlBeginColumn(VOID);
VOID HtmlEndColumn(VOID);
VOID HtmlWriteItemString(LPWSTR lpszString, BOOL bIsHeader);
VOID HtmlWriteValueString(LPWSTR lpszString);
VOID HtmlWriteColumnString(LPWSTR lpszString);
VOID HtmlTableTitle(LPWSTR lpszTitle, UINT StringID, BOOL WithContentTable);
VOID HtmlBeginTable(VOID);
VOID HtmlEndTable(VOID);
VOID HtmlBeginContentTable(LPWSTR lpszTitle);
VOID HtmlEndContentTable(VOID);
VOID HtmlContentTableItem(UINT ID, LPWSTR lpszName, BOOL IsRootItem);
VOID HtmlContentTableEndRootItem(VOID);

INT ListViewAddItem(INT IconIndex, LPWSTR lpText);
INT ListViewAddHeaderString(LPWSTR lpszText, INT IconIndex);
VOID ListViewAddHeader(UINT StringID, INT IconIndex);
VOID ListViewSetItemText(INT i, INT iSubItem, LPWSTR pszText);
INT ListViewAddValueName(LPWSTR lpszName, INT IconIndex);
INT ListViewAddImageListIcon(UINT IconID);
VOID ListViewAddColumn(SIZE_T Index, INT Width, LPWSTR lpszText);

static UINT IoTarget = 0;
static INT ColumnsCount = 0;


VOID
IoSetTarget(UINT Target)
{
    IoTarget = Target;
}

UINT
IoGetTarget(VOID)
{
    return IoTarget;
}

INT
IoAddHeaderString(LPWSTR lpszText, INT IconIndex)
{
    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            return ListViewAddHeaderString(lpszText, IconIndex);

        case IO_TARGET_HTML:
            HtmlWriteItemString(lpszText, TRUE);
            HtmlAppendStringToFile(L"<td>&nbsp;</td></tr>");
            return -1;
    }

    return -1;
}

VOID
IoAddHeader(UINT StringID, INT IconIndex)
{
    WCHAR szText[MAX_STR_LEN];

    LoadMUIString(StringID, szText, MAX_STR_LEN);
    IoAddHeaderString(szText, IconIndex);
}

INT
IoAddItem(INT IconIndex, LPWSTR lpText)
{
    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            return ListViewAddItem(IconIndex, lpText);
            break;
        case IO_TARGET_HTML:
            HtmlWriteItemString(lpText, FALSE);
            return -1;
            break;
    }

    return -1;
}

INT
IoAddValueName(UINT ValueID, INT IconIndex)
{
    WCHAR szText[MAX_STR_LEN];

    LoadMUIString(ValueID, szText, MAX_STR_LEN);
    return IoAddItem(IconIndex, szText);
}

VOID
IoSetItemText(INT Index, INT iSubItem, LPWSTR pszText)
{
    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            ListViewSetItemText(Index, iSubItem, pszText);
            break;
        case IO_TARGET_HTML:
            HtmlWriteValueString(pszText);
            if (IoGetColumnsCount() == iSubItem + 1)
                HtmlAppendStringToFile(L"</tr>");
            break;
    }
}

VOID
IoAddFooter(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            ListViewAddItem(-1, L" ");
            break;
        case IO_TARGET_HTML:
            HtmlWriteItemString(L"&nbsp;", TRUE);
            HtmlAppendStringToFile(L"<td>&nbsp;</td></tr>");
            break;
    }
}

VOID
IoReportBeginColumn(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlBeginColumn();
            break;
    }
}

VOID
IoReportEndColumn(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlEndColumn();
            break;
    }
}

VOID
IoReportWriteColumnString(LPWSTR lpszString)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlWriteColumnString(lpszString);
            break;
    }
}

VOID
IoAddColumnsList(COLUMN_LIST *List)
{
    WCHAR szText[MAX_STR_LEN];
    SIZE_T Index = 0;

    IoReportBeginColumn();

    do
    {
        LoadMUIString(List[Index].StringID, szText, MAX_STR_LEN);

        switch (IoTarget)
        {
            case IO_TARGET_LISTVIEW:
                ListViewAddColumn(Index + 1, List[Index].Width, szText);
                break;
            case IO_TARGET_HTML:
                IoReportWriteColumnString(szText);
                break;
        }
    }
    while (List[++Index].StringID != 0);

    IoReportEndColumn();

    ColumnsCount = Index;
}

INT
IoGetColumnsCount(VOID)
{
    return ColumnsCount;
}

VOID
IoSetColumnsCount(INT Count)
{
    ColumnsCount = Count;
}

INT
IoAddIcon(UINT IconID)
{
    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            return ListViewAddImageListIcon(IconID);
            break;
    }

    return -1;
}

BOOL
IoCreateReport(LPWSTR lpszFile)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            return HtmlCreateReport(lpszFile);
            break;
    }

    return FALSE;
}

VOID
IoCloseReport(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlCloseReport();
            break;
    }
}

VOID
IoReportWriteItemString(LPWSTR lpszString, BOOL bIsHeader)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlWriteItemString(lpszString, bIsHeader);
            break;
    }
}

VOID
IoWriteTableTitle(LPWSTR lpszTitle, UINT StringID, BOOL WithContentTable)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlTableTitle(lpszTitle, StringID, WithContentTable);
            break;
    }
}

VOID
IoWriteBeginTable(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlBeginTable();
            break;
    }
}

VOID
IoWriteEndTable(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlEndTable();
            break;
    }
}

VOID
IoWriteBeginContentTable(LPWSTR lpszTitle)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlBeginContentTable(lpszTitle);
            break;
    }
}

VOID
IoWriteEndContentTable(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlEndContentTable();
            break;
    }
}

VOID
IoWriteContentTableItem(UINT ID, LPWSTR lpszName, BOOL IsRootItem)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlContentTableItem(ID, lpszName, IsRootItem);
            break;
    }
}

VOID
IoWriteContentTableEndRootItem(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            HtmlContentTableEndRootItem();
            break;
    }
}

VOID
IoRunInfoFunc(UINT Category, CATEGORY_LIST *List)
{
    SIZE_T Index = 0;

    do
    {
        if (List[Index].StringID == Category)
        {
            IoAddColumnsList(List[Index].ColumnList);
            List[Index].InfoFunc();
            break;
        }
        if (List[Index].Child)
            IoRunInfoFunc(Category, List[Index].Child);
    }
    while (List[++Index].StringID != 0);
}
