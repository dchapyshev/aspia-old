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

INT ListViewAddItem(INT Indent, INT IconIndex, LPWSTR lpText);
INT ListViewAddHeaderString(INT Indent, LPWSTR lpszText, INT IconIndex);
VOID ListViewAddHeader(UINT StringID, INT IconIndex);
VOID ListViewSetItemText(INT i, INT iSubItem, LPWSTR pszText);
INT ListViewAddValueName(LPWSTR lpszName, INT IconIndex);
INT ListViewAddImageListIcon(UINT IconID);
VOID ListViewAddColumn(SIZE_T Index, INT Width, LPWSTR lpszText);

BOOL CsvAppendStringToFile(LPWSTR lpszString);
BOOL CsvCreateReport(LPWSTR lpszFile);
VOID CsvCloseReport(VOID);
VOID CsvWriteValueString(LPWSTR lpszString);
VOID CsvWriteItemString(LPWSTR lpszString);
VOID CsvWriteColumnString(LPWSTR lpszString);
VOID CsvTableTitle(LPWSTR lpszTitle);
VOID CsvEndTable(VOID);

BOOL IniAppendStringToFile(LPWSTR lpszString);
BOOL IniCreateReport(LPWSTR lpszFile);
VOID IniCloseReport(VOID);
VOID IniWriteValueString(LPWSTR lpszString);
VOID IniWriteItemString(LPWSTR lpszString);
VOID IniTableTitle(LPWSTR lpszTitle);
VOID IniEndTable(VOID);

BOOL TextAppendStringToFile(LPWSTR lpszString);
BOOL TextCreateReport(LPWSTR lpszFile);
VOID TextCloseReport(VOID);
VOID TextWriteValueString(LPWSTR lpszString);
VOID TextWriteItemString(LPWSTR lpszString);
VOID TextTableTitle(LPWSTR lpszTitle);
VOID TextEndTable(VOID);

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
IoAddHeaderString(INT Indent, LPWSTR lpszText, INT IconIndex)
{
    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            return ListViewAddHeaderString(Indent, lpszText, IconIndex);

        case IO_TARGET_HTML:
            HtmlWriteItemString(lpszText, TRUE);
            HtmlAppendStringToFile(L"<td>&nbsp;</td></tr>");
            break;

        case IO_TARGET_CSV:
            CsvAppendStringToFile(L"\n");
            CsvWriteItemString(lpszText);
            break;

        case IO_TARGET_TXT:
            TextAppendStringToFile(L"\n");
            TextWriteItemString(lpszText);
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
            IniAppendStringToFile(L"\n");
            IniWriteItemString(lpszText);
            break;
    }

    return -1;
}

VOID
IoAddHeader(INT Indent, UINT StringID, INT IconIndex)
{
    WCHAR szText[MAX_STR_LEN];

    LoadMUIString(StringID, szText, MAX_STR_LEN);
    IoAddHeaderString(Indent, szText, IconIndex);
}

INT
IoAddItem(INT Indent, INT IconIndex, LPWSTR lpText)
{
    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            return ListViewAddItem(Indent, IconIndex, lpText);

        case IO_TARGET_HTML:
            HtmlWriteItemString(lpText, FALSE);
            break;

        case IO_TARGET_CSV:
            CsvWriteItemString(lpText);
            break;

        case IO_TARGET_TXT:
            TextWriteItemString(lpText);
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
            IniWriteItemString(lpText);
            break;
    }

    return -1;
}

INT
IoAddValueName(INT Indent, UINT ValueID, INT IconIndex)
{
    WCHAR szText[MAX_STR_LEN];

    LoadMUIString(ValueID, szText, MAX_STR_LEN);
    return IoAddItem(Indent, IconIndex, szText);
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

        case IO_TARGET_CSV:
            CsvWriteValueString(pszText);
            break;

        case IO_TARGET_TXT:
            TextWriteValueString(pszText);
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
            IniWriteValueString(pszText);
            break;
    }
}

VOID
IoAddFooter(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            ListViewAddItem(0, -1, L" ");
            break;
        case IO_TARGET_HTML:
            HtmlWriteItemString(L"&nbsp;", TRUE);
            HtmlAppendStringToFile(L"<td>&nbsp;</td></tr>");
            break;

        case IO_TARGET_CSV:
            CsvAppendStringToFile(L"\n");
            break;

        case IO_TARGET_TXT:
            TextAppendStringToFile(L"\n");
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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

        case IO_TARGET_CSV:
            break;

        case IO_TARGET_TXT:
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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

        case IO_TARGET_TXT:
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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

        case IO_TARGET_CSV:
            CsvWriteColumnString(lpszString);
            break;

        case IO_TARGET_TXT:
            TextWriteColumnString(lpszString);
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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
            case IO_TARGET_CSV:
            case IO_TARGET_TXT:
            case IO_TARGET_XML:
            case IO_TARGET_INI:
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

        case IO_TARGET_CSV:
            return CsvCreateReport(lpszFile);

        case IO_TARGET_TXT:
            return TextCreateReport(lpszFile);

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
            return IniCreateReport(lpszFile);
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

        case IO_TARGET_CSV:
            CsvCloseReport();
            break;

        case IO_TARGET_TXT:
            TextCloseReport();
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
            IniCloseReport();
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

        case IO_TARGET_CSV:
            CsvWriteItemString(lpszString);
            break;

        case IO_TARGET_TXT:
            TextWriteItemString(lpszString);
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
            IniWriteItemString(lpszString);
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

        case IO_TARGET_CSV:
            CsvTableTitle(lpszTitle);
            break;

        case IO_TARGET_TXT:
            TextTableTitle(lpszTitle);
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
            IniTableTitle(lpszTitle);
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

        case IO_TARGET_CSV:
            break;

        case IO_TARGET_TXT:
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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

        case IO_TARGET_CSV:
            break;

        case IO_TARGET_TXT:
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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

        case IO_TARGET_CSV:
            break;

        case IO_TARGET_TXT:
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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

        case IO_TARGET_CSV:
            break;

        case IO_TARGET_TXT:
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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

        case IO_TARGET_CSV:
            break;

        case IO_TARGET_TXT:
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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

        case IO_TARGET_CSV:
            break;

        case IO_TARGET_TXT:
            break;

        case IO_TARGET_XML:
            break;

        case IO_TARGET_INI:
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
