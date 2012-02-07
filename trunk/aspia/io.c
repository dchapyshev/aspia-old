/*
 * PROJECT:         Aspia
 * FILE:            aspia/io.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"


INT ListViewAddItem(INT Indent, INT IconIndex, LPWSTR lpText);
INT ListViewAddHeaderString(INT Indent, LPWSTR lpszText, INT IconIndex);
VOID ListViewAddHeader(UINT StringID, INT IconIndex);
VOID ListViewSetItemText(INT i, INT iSubItem, LPWSTR pszText);
INT ListViewAddValueName(LPWSTR lpszName, INT IconIndex);
INT ListViewAddImageListIcon(UINT IconID);
VOID ListViewAddColumn(SIZE_T Index, INT Width, LPWSTR lpszText);

static UINT IoTarget = 0;
static INT ColumnsCount = 0;
static HANDLE hReport = INVALID_HANDLE_VALUE;


#define lpszHtmlHeader L"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>\r\n\
<html xmlns='http://www.w3.org/1999/xhtml'>\r\n\
<head>\r\n\
\t<meta http-equiv='content-type' content='text/html; charset=utf-8' />\r\n\
\t<meta name='rights' content='Aspia Software, Dmitry Chapyshev' />\r\n\
\t<meta name='generator' content='Aspia' />\r\n\
\t<title>Aspia</title>\r\n\
\t<style type='text/css'>\r\n\
\t\tbody{\r\n\
\t\t\tcolor:#353535;\r\n\
\t\t\tfont-family:Tahoma,Arial,Verdana;\r\n\
\t\t}\r\n\
\t\ttable{\r\n\
\t\t\tbackground-color:#F9F9F9;\r\n\
\t\t\tborder:1px solid #E2E2E0;\r\n\
\t\t\tfont-size:12px;\r\n\
\t\t}\r\n\
\t\ttd{\r\n\
\t\t\tpadding:5px;\r\n\
\t\t\tmargin:5px;\r\n\
\t\t\theight:20px;\r\n\
\t\t\tborder-bottom:1px solid #E2E2E0;\r\n\
\t\t}\r\n\
\t\t.h{\r\n\
\t\t\tcolor:#000;\r\n\
\t\t\tfont-weight:bold;\r\n\
\t\t}\r\n\
\t\t.c{\r\n\
\t\t\tcolor:#fff;\r\n\
\t\t\tbackground-color:#383637;\r\n\
\t\t\tfont-weight:bold;\r\n\
\t\t}\r\n\
\t\t#f{\r\n\
\t\t\tfont-size:12px;\r\n\
\t\t\ttext-align:center;\r\n\
\t\t}\r\n\
\t\ta{\r\n\
\t\t\tcolor:#4475bf;\r\n\
\t\t}\r\n\
\t\th1 a{\r\n\
\t\t\tfont-size:18px;\r\n\
\t\t}\r\n\
\t\th2 a{\r\n\
\t\t\tfont-size:16px;\r\n\
\t\t}\r\n\
\t\th3 a{\r\n\
\t\t\tfont-size:13px;\r\n\
\t\t}\r\n\
\t\th1{\r\n\
\t\t\tfont-size:18px;\r\n\
\t\t}\r\n\
\t\th2{\r\n\
\t\t\tfont-size:16px;\r\n\
\t\t}\r\n\
\t\th3{\r\n\
\t\t\tfont-size:13px;\r\n\
\t\t}\r\n\
\t</style>\r\n\
</head>\r\n<body>\r\n"


BOOL
AppendStringToFile(LPWSTR lpszString)
{
    LARGE_INTEGER FileSize, MoveTo, NewPos;
    DWORD dwLen = SafeStrLen(lpszString);
    DWORD dwBytesWritten;
    INT buf_len;
    char *result;

    if (hReport == INVALID_HANDLE_VALUE || dwLen == 0)
        return FALSE;

    buf_len = WideCharToMultiByte(CP_UTF8, 0,
                                  lpszString, dwLen,
                                  NULL, 0, 0, 0);
    result = Alloc(buf_len);
    if (result)
    {
        WideCharToMultiByte(CP_UTF8, 0,
                            lpszString, dwLen,
                            result, buf_len, 0, 0);

        MoveTo.QuadPart = 0;
        if (!SetFilePointerEx(hReport, MoveTo, &NewPos, FILE_END))
        {
            Free(result);
            return FALSE;
        }

        if (!GetFileSizeEx(hReport, &FileSize))
        {
            Free(result);
            return FALSE;
        }

        LockFile(hReport, (DWORD_PTR)NewPos.QuadPart, 0, (DWORD_PTR)FileSize.QuadPart, 0);

        WriteFile(hReport, result, buf_len,
                  &dwBytesWritten, NULL);

        UnlockFile(hReport, (DWORD_PTR)NewPos.QuadPart, 0, (DWORD_PTR)FileSize.QuadPart, 0);

        Free(result);

        return TRUE;
    }

    return FALSE;
}

BOOL
AppendStringToFileA(LPWSTR lpszString)
{
    LARGE_INTEGER FileSize, MoveTo, NewPos;
    DWORD dwLen = SafeStrLen(lpszString);
    DWORD dwBytesWritten;
    INT buf_len;
    char *result;

    if (hReport == INVALID_HANDLE_VALUE || dwLen == 0)
        return FALSE;

    buf_len = WideCharToMultiByte(CP_ACP, 0,
                                  lpszString, dwLen,
                                  NULL, 0, 0, 0);
    result = Alloc(buf_len);
    if (result)
    {
        WideCharToMultiByte(CP_ACP, 0,
                            lpszString, dwLen,
                            result, buf_len, 0, 0);

        MoveTo.QuadPart = 0;
        if (!SetFilePointerEx(hReport, MoveTo, &NewPos, FILE_END))
        {
            Free(result);
            return FALSE;
        }

        if (!GetFileSizeEx(hReport, &FileSize))
        {
            Free(result);
            return FALSE;
        }

        LockFile(hReport, (DWORD_PTR)NewPos.QuadPart, 0, (DWORD_PTR)FileSize.QuadPart, 0);

        WriteFile(hReport, result, buf_len,
                  &dwBytesWritten, NULL);

        UnlockFile(hReport, (DWORD_PTR)NewPos.QuadPart, 0, (DWORD_PTR)FileSize.QuadPart, 0);

        Free(result);

        return TRUE;
    }

    return FALSE;
}

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
    WCHAR szText[MAX_STR_LEN * 5];

    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            return ListViewAddHeaderString(Indent, lpszText, IconIndex);

        case IO_TARGET_HTML:
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"\t<tr><td class='h'>%s</td><td>&nbsp;</td></tr>\r\n",
                           lpszText);
        }
        break;

        case IO_TARGET_CSV:
            StringCbPrintf(szText, sizeof(szText), L"\r\n\r\n%s", lpszText);
            break;

        case IO_TARGET_TXT:
            StringCbPrintf(szText, sizeof(szText), L"\r\n\r\n%s", lpszText);
            break;

        case IO_TARGET_INI:
            StringCbPrintf(szText, sizeof(szText), L"\r\n\r\n%s=", lpszText);
            break;

        case IO_TARGET_RTF:
            StringCbPrintf(szText, sizeof(szText), L"\\par\n{\\b %s}", lpszText);
            AppendStringToFileA(szText);
            return -1;

        case IO_TARGET_JSON:
            break;

        default:
            return -1;
    }

    AppendStringToFile(szText);

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
    WCHAR szText[MAX_STR_LEN];

    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            return ListViewAddItem(Indent, IconIndex, lpText);

        case IO_TARGET_HTML:
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"\r\n\t<tr><td>%s</td>",
                           lpText);
        }
        break;

        case IO_TARGET_CSV:
            StringCbPrintf(szText, sizeof(szText), L"\r\n%s;", lpText);
            break;

        case IO_TARGET_TXT:
            StringCbPrintf(szText, sizeof(szText), L"\r\n%s ", lpText);
            break;

        case IO_TARGET_INI:
            StringCbPrintf(szText, sizeof(szText), L"\r\n%s=", lpText);
            break;

        case IO_TARGET_RTF:
            StringCbPrintf(szText, sizeof(szText), L"\\par\n%s ", lpText);
            AppendStringToFileA(szText);
            return -1;

        case IO_TARGET_JSON:
            StringCbPrintf(szText, sizeof(szText), L"\n    \"%s\": ", lpText);
            break;

        default:
            return -1;
    }

    AppendStringToFile(szText);

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
    WCHAR szText[MAX_STR_LEN * 15];

    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            ListViewSetItemText(Index, iSubItem, pszText);
            return;

        case IO_TARGET_HTML:
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"<td>%s</td>",
                           (SafeStrLen(pszText) == 0) ? L"&nbsp;" : pszText);

            AppendStringToFile(szText);

            if (IoGetColumnsCount() == iSubItem + 1)
                AppendStringToFile(L"</tr>");
            return;
        }

        case IO_TARGET_CSV:
            StringCbPrintf(szText, sizeof(szText),
                           (IoGetColumnsCount() == iSubItem + 1) ? L"%s" : L"%s;", pszText);
            break;

        case IO_TARGET_TXT:
            StringCbPrintf(szText, sizeof(szText),
                           (IoGetColumnsCount() == iSubItem + 1) ? L"%s" : L"%s ", pszText);
            break;

        case IO_TARGET_INI:
            StringCbPrintf(szText, sizeof(szText),
                           (IoGetColumnsCount() == iSubItem + 1) ? L"%s" : L"%s,", pszText);
            break;

        case IO_TARGET_RTF:
            StringCbPrintf(szText, sizeof(szText),
                           (IoGetColumnsCount() == iSubItem + 1) ? L"%s" : L"%s ", pszText);
            AppendStringToFileA(szText);
            return;

        case IO_TARGET_JSON:
            StringCbPrintf(szText, sizeof(szText),
                           (IoGetColumnsCount() == iSubItem + 1) ? L"\"%s\"," : L"\"%s\"", pszText);
            break;

        default:
            break;
    }

    AppendStringToFile(szText);
}

VOID
IoAddFooter(VOID)
{
    LPWSTR lpString = NULL;

    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            ListViewAddItem(0, -1, L" ");
            return;

        case IO_TARGET_HTML:
            lpString = L"\r\n\t<tr><td class='h'>&nbsp;</td><td>&nbsp;</td></tr>\r\n";
            break;

        case IO_TARGET_CSV:
            lpString = L"\r\n";
            break;

        case IO_TARGET_TXT:
            lpString = L"\r\n";
            break;

        case IO_TARGET_RTF:
            AppendStringToFileA(L"\\par\n");
            return;

        default:
            return;
    }

    AppendStringToFile(lpString);
}

VOID
IoReportBeginColumn(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            AppendStringToFile(L"\r\n\t<tr>");
            break;
    }
}

VOID
IoReportEndColumn(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            AppendStringToFile(L"</tr>\r\n");
            break;
    }
}

VOID
IoReportWriteColumnString(LPWSTR lpszString)
{
    WCHAR szText[MAX_STR_LEN * 3];

    switch (IoTarget)
    {
        case IO_TARGET_HTML:
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"<td class='c'>%s</td>",
                           lpszString);
        }
        break;

        case IO_TARGET_CSV:
            StringCbPrintf(szText, sizeof(szText), L"%s;", lpszString);
            break;

        case IO_TARGET_TXT:
            StringCbPrintf(szText, sizeof(szText), L"%s ", lpszString);
            break;

        case IO_TARGET_RTF:
            StringCbPrintf(szText, sizeof(szText), L"{\\b %s} ", lpszString);
            AppendStringToFileA(szText);
            return;

        default:
            return;
    }

    AppendStringToFile(szText);
}

VOID
IoAddColumnsList(COLUMN_LIST *List, LPWSTR lpCategoryName, LPWSTR lpIniPath)
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
            {
                WCHAR szCol[3];
                INT Width;

                StringCbPrintf(szCol, sizeof(szCol), L"%d", Index);
                Width = GetPrivateProfileInt(lpCategoryName, szCol, 0, lpIniPath);

                ListViewAddColumn(Index + 1, (Width > 0) ? Width : List[Index].Width, szText);
            }
            break;

            case IO_TARGET_HTML:
            case IO_TARGET_CSV:
            case IO_TARGET_TXT:
            case IO_TARGET_INI:
            case IO_TARGET_RTF:
            case IO_TARGET_JSON:
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
    DWORD dwBytesWritten;

    hReport = CreateFile(lpszFile,
                         GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
    if (hReport == INVALID_HANDLE_VALUE)
        return FALSE;

    if (IoTarget != IO_TARGET_RTF)
    {
        WriteFile(hReport, "\xEF\xBB\xBF", 3, &dwBytesWritten, NULL);
    }

    if (IoTarget == IO_TARGET_HTML)
    {
        AppendStringToFile(lpszHtmlHeader);
    }
    else if (IoTarget == IO_TARGET_RTF)
    {
        AppendStringToFileA(L"{\\rtf1");
    }
    else if (IoTarget == IO_TARGET_JSON)
    {
        AppendStringToFile(L"{\n");
    }

    return TRUE;
}

VOID
IoCloseReport(VOID)
{
    if (IoTarget == IO_TARGET_HTML)
    {
        AppendStringToFile(L"\r\n<br /><div id='f'>Aspia ");
        AppendStringToFile(VER_FILEVERSION_STR);
        AppendStringToFile(L"<br />&copy; 2011 <a href='http://www.aspia.ru'>Aspia Software</a></div>\r\n</body>\r\n</html>");
    }
    else if (IoTarget == IO_TARGET_RTF)
    {
        AppendStringToFileA(L"\n}");
    }
    else if (IoTarget == IO_TARGET_JSON)
    {
        AppendStringToFile(L"\n}");
    }

    CloseHandle(hReport);
    hReport = NULL;
}

VOID
IoReportWriteItemString(LPWSTR lpszString, BOOL bIsHeader)
{
    WCHAR szText[MAX_STR_LEN * 5];

    switch (IoTarget)
    {
        case IO_TARGET_HTML:
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"\r\n<tr>%s%s</td>",
                           bIsHeader ? L"<td class='h'>" : L"<td>",
                           lpszString);
        }
        break;

        case IO_TARGET_CSV:
            StringCbPrintf(szText, sizeof(szText), L"\r\n%s;", lpszString);
            break;

        case IO_TARGET_TXT:
            StringCbPrintf(szText, sizeof(szText), L"\r\n%s ", lpszString);
            break;

        case IO_TARGET_INI:
            StringCbPrintf(szText, sizeof(szText), L"\r\n%s=", lpszString);
            break;

        case IO_TARGET_JSON:
            StringCbPrintf(szText, sizeof(szText), L"\n\"%s\":", lpszString);
            break;

        default:
            return;
    }

    AppendStringToFile(szText);
}

VOID
IoWriteTableTitle(LPWSTR lpszTitle, UINT StringID, BOOL WithContentTable)
{
    WCHAR szText[MAX_STR_LEN] = {0};

    switch (IoTarget)
    {
        case IO_TARGET_HTML:
        {
            WCHAR szTemp[MAX_STR_LEN];

            StringCbPrintf(szText, sizeof(szText),
                           L"\r\n<h2 id='i%ld'>%s",
                           StringID, lpszTitle);
            AppendStringToFile(szText);

            if (!IsRootCategory(StringID, RootCategoryList))
            {
                if (WithContentTable)
                {
                    LoadMUIString(IDS_REPORT_TOP, szTemp, sizeof(szTemp)/sizeof(WCHAR));
                    StringCbPrintf(szText, sizeof(szText),
                                   L"  (<a href='#top'>%s</a>)</h2>",
                                   szTemp);
                    AppendStringToFile(szText);
                    return;
                }
            }

            AppendStringToFile(L"</h2>\r\n");
            return;
        }
        break;

        case IO_TARGET_CSV:
            StringCbPrintf(szText, sizeof(szText), L"\r\n%s\r\n", lpszTitle);
            break;

        case IO_TARGET_TXT:
            StringCbPrintf(szText, sizeof(szText), L"\r\n%s\r\n", lpszTitle);
            break;

        case IO_TARGET_INI:
            StringCbPrintf(szText, sizeof(szText), L"\r\n[%s]\r\n", lpszTitle);
            break;

        case IO_TARGET_RTF:
            StringCbPrintf(szText, sizeof(szText), L"\n\\par{\\b\\ul %s}\\par\n", lpszTitle);
            AppendStringToFileA(szText);
            return;

        case IO_TARGET_JSON:
            StringCbPrintf(szText, sizeof(szText), L"\n\"%s\": {\n", lpszTitle);
            break;

        default:
            return;
    }

    AppendStringToFile(szText);
}

VOID
IoWriteBeginTable(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            AppendStringToFile(L"\r\n<table cellspacing='0' cellpadding='0'>");
            break;
    }
}

VOID
IoWriteEndTable(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            AppendStringToFile(L"\r\n</table>");
            break;

        case IO_TARGET_JSON:
            AppendStringToFile(L"\n    }");
            break;
    }
}

VOID
IoWriteBeginContentTable(LPWSTR lpszTitle)
{
    WCHAR szText[MAX_STR_LEN];

    switch (IoTarget)
    {
        case IO_TARGET_HTML:
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"<h1 id='top' style='font-size:18px'>%s</h1>\r\n<ul>",
                           lpszTitle);
            AppendStringToFile(szText);
        }
        break;

        case IO_TARGET_RTF:
            StringCbPrintf(szText, sizeof(szText),
                L"{\\b %s}\\par",
                           lpszTitle);
            AppendStringToFileA(szText);
            break;
    }
}

VOID
IoWriteEndContentTable(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            AppendStringToFile(L"</ul>");
            break;
    }
}

VOID
IoWriteContentTableItem(UINT ID, LPWSTR lpszName, BOOL IsRootItem)
{
    WCHAR szText[MAX_STR_LEN];

    switch (IoTarget)
    {
        case IO_TARGET_HTML:
        {
            if (!IsRootItem)
            {
                StringCbPrintf(szText, sizeof(szText),
                               L"\r\n\t<li><a href='#i%ld'>%s</a></li>",
                               ID, lpszName);
            }
            else
            {
                StringCbPrintf(szText, sizeof(szText),
                               L"<li>%s<ul>", lpszName);
            }
            AppendStringToFile(szText);
        }
        break;

        case IO_TARGET_RTF:
            StringCbPrintf(szText, sizeof(szText),
                           L"%s\\par", lpszName);
            AppendStringToFileA(szText);
            break;
    }
}

VOID
IoWriteContentTableEndRootItem(VOID)
{
    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            AppendStringToFile(L"</ul></li>");
            break;
    }
}

VOID
IoRunInfoFunc(UINT Category, CATEGORY_LIST *List)
{
    SIZE_T Index = 0;

    IsCanceled = FALSE;

    do
    {
        if (List[Index].StringID == Category)
        {
            WCHAR szIniPath[MAX_PATH] = {0}, szName[15] = {0};

            if (IoTarget == IO_TARGET_LISTVIEW)
            {
                CurrentMenu      = List[Index].MenuID;
                IsSortingAllowed = List[Index].Sorting;
                InfoFreeFunction = List[Index].FreeFunc;

                GetIniFilePath(szIniPath, MAX_PATH);
                StringCbPrintf(szName, sizeof(szName), L"col-%d", Category);
            }

            IoAddColumnsList(List[Index].ColumnList, szName, szIniPath);

            List[Index].InfoFunc();
            break;
        }
        if (List[Index].Child)
            IoRunInfoFunc(Category, List[Index].Child);
    }
    while (List[++Index].StringID != 0);

    IsCanceled = FALSE;
}
