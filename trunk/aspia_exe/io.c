/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/io.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"


INT ListViewAddItem(INT Indent, INT IconIndex, LPWSTR lpText);
INT ListViewAddHeaderString(INT Indent, LPWSTR lpszText, INT IconIndex);
VOID ListViewAddHeader(UINT StringID, INT IconIndex);
VOID ListViewSetItemText(INT i, INT iSubItem, LPWSTR pszText);
INT ListViewAddValueName(LPWSTR lpszName, INT IconIndex);
INT ListViewAddImageListIcon(UINT IconID);
VOID ListViewAddColumn(SIZE_T Index, INT Width, LPWSTR lpszText);

static UINT IoTarget = IO_TARGET_LISTVIEW;
static INT ColumnsCount = 0;
static INT CurrentColumn = 0;
static INT CurrentItemIndex = 0;
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
    {
        DebugTrace(L"AppendStringToFile() failed!");
        return FALSE;
    }

    buf_len = WideCharToMultiByte(CP_UTF8, 0,
                                  lpszString, dwLen,
                                  NULL, 0, 0, 0);
    if (buf_len == 0)
    {
        DebugTrace(L"WideCharToMultiByte() failed!");
        return FALSE;
    }

    result = Alloc(buf_len);
    if (result)
    {
        if (WideCharToMultiByte(CP_UTF8, 0,
                                lpszString, dwLen,
                                result, buf_len, 0, 0) == 0)
        {
            Free(result);
            DebugTrace(L"WideCharToMultiByte() failed! Error code = %d",
                       GetLastError());
            return FALSE;
        }

        MoveTo.QuadPart = 0;
        if (!SetFilePointerEx(hReport, MoveTo, &NewPos, FILE_END))
        {
            Free(result);
            DebugTrace(L"SetFilePointerEx() failed! Error code = %d",
                       GetLastError());
            return FALSE;
        }

        if (!GetFileSizeEx(hReport, &FileSize))
        {
            Free(result);
            DebugTrace(L"GetFileSizeEx() failed! Error code = %d",
                       GetLastError());
            return FALSE;
        }

        LockFile(hReport, (DWORD_PTR)NewPos.QuadPart, 0, (DWORD_PTR)FileSize.QuadPart, 0);

        if (!WriteFile(hReport, result, buf_len,
                       &dwBytesWritten, NULL))
        {
            DebugTrace(L"WriteFile() failed! Error code = %d",
                       GetLastError());
        }

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
    {
        DebugTrace(L"AppendStringToFileA() failed!");
        return FALSE;
    }

    buf_len = WideCharToMultiByte(CP_ACP, 0,
                                  lpszString, dwLen,
                                  NULL, 0, 0, 0);
    if (buf_len == 0)
    {
        DebugTrace(L"WideCharToMultiByte() failed!");
        return FALSE;
    }

    result = Alloc(buf_len);
    if (result)
    {
        if (WideCharToMultiByte(CP_ACP, 0,
                                lpszString, dwLen,
                                result, buf_len, 0, 0) == 0)
        {
            Free(result);
            DebugTrace(L"WideCharToMultiByte() failed! Error code = %d",
                       GetLastError());
            return FALSE;
        }

        MoveTo.QuadPart = 0;
        if (!SetFilePointerEx(hReport, MoveTo, &NewPos, FILE_END))
        {
            Free(result);
            DebugTrace(L"SetFilePointerEx() failed! Error code = %d",
                       GetLastError());
            return FALSE;
        }

        if (!GetFileSizeEx(hReport, &FileSize))
        {
            Free(result);
            DebugTrace(L"GetFileSizeEx() failed! Error code = %d",
                       GetLastError());
            return FALSE;
        }

        LockFile(hReport, (DWORD_PTR)NewPos.QuadPart, 0, (DWORD_PTR)FileSize.QuadPart, 0);

        if (!WriteFile(hReport, result, buf_len,
                       &dwBytesWritten, NULL))
        {
            DebugTrace(L"WriteFile() failed! Error code = %d",
                       GetLastError());
        }

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
IoGetCurrentItemIndex(VOID)
{
    return CurrentItemIndex;
}

__inline VOID
WriteRTFRowHeader(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    INT i, width = 11000 / (ColumnsCount + 1);

    AppendStringToFileA(L"\\trowd\\trgaph50\r\n");

    for (i = 0; i < ColumnsCount; i++)
    {
        StringCbPrintf(szText, sizeof(szText),
                       L"\\clbrdrt\\brdrs\\clbrdrl\\brdrs\\clbrdrb\\brdrs\\clbrdrr\\brdrs\\cellx%d\r\n",
                       (i + 1) * width);
        AppendStringToFileA(szText);
    }

    AppendStringToFileA(L"\\pard\\intbl\r\n");
}

WCHAR*
TextToRTFText(WCHAR *text)
{
    WCHAR *buffer;
    LONG len = SafeStrLen(text);
    SIZE_T size;
    INT i = 0;

    if (len == 0)
    {
        WCHAR temp[] = L"\\'2d"; /* "-" */
        buffer = (WCHAR*)Alloc(sizeof(temp));
        wcscpy(buffer, temp);

        return buffer;
    }

    size = len * 5 * sizeof(WCHAR);
    buffer = (WCHAR*)Alloc(size);
    if (buffer == NULL)
    {
        return NULL;
    }

    ZeroMemory(buffer, size);

    while (len)
    {
        BOOL unknown = FALSE;
        WCHAR w_letter[2];
        char a_letter[3];
        WCHAR code[8];
        INT count;

        w_letter[0] = text[i];
        w_letter[1] = 0;

        count = WideCharToMultiByte(CP_ACP, 0,
                                    w_letter, 1,
                                    a_letter, 3,
                                    NULL, &unknown);

        if (unknown)
        {
            StringCbPrintf(code, sizeof(code), L"\\u%d?", (short)text[i]);
            StringCbCat(buffer, size, code);
        }
        else
        {
            INT j;

            for (j = 0; j < count; j++)
            {
                StringCbPrintf(code, sizeof(code), L"\\'%02x", (BYTE)a_letter[j]);
                StringCbCat(buffer, size, code);
            }
        }

        ++i;
        --len;
    }

    return buffer;
}

VOID
IoAddHeaderString(INT Indent, INT IconIndex, LPWSTR pText, ...)
{
    WCHAR *ptr;
    SIZE_T size;
    va_list args;

    va_start(args, pText);
    size = (_vscwprintf(pText, args) + 1) * sizeof(WCHAR);

    ptr = (WCHAR*)Alloc(size);
    if (!ptr)
    {
        DebugTrace(L"Memory allocation failed!");
        return;
    }

    StringCbVPrintf(ptr, size, pText, args);
    va_end(args);

    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            ListViewAddHeaderString(Indent, ptr, IconIndex);
            break;

        case IO_TARGET_HTML:
            AppendStringToFile(L"\t<tr><td class='h'>");
            AppendStringToFile(ptr);
            AppendStringToFile(L"</td><td>&nbsp;</td></tr>\r\n");
            break;

        case IO_TARGET_CSV:
            AppendStringToFile(L"\r\n\r\n");
            AppendStringToFile(ptr);
            break;

        case IO_TARGET_TXT:
            AppendStringToFile(L"\r\n\r\n");
            AppendStringToFile(ptr);
            break;

        case IO_TARGET_INI:
            AppendStringToFile(L"\r\n\r\n");
            AppendStringToFile(ptr);
            AppendStringToFile(L"=");
            break;

        case IO_TARGET_RTF:
        {
            WCHAR *text = TextToRTFText(ptr);

            if (text != NULL)
            {
                WriteRTFRowHeader();
                AppendStringToFileA(L"{\\b ");
                AppendStringToFileA(ptr);
                AppendStringToFileA(L"}\\cell \t\\cell\r\n\\row\r\n");

                Free(text);
            }
        }
        break;
    }

    Free(ptr);
}

VOID
IoAddHeader(INT Indent, INT IconIndex, UINT StringID)
{
    WCHAR szText[MAX_STR_LEN];

    LoadMUIStringF(hLangInst, StringID, szText, MAX_STR_LEN);
    IoAddHeaderString(Indent, IconIndex, szText);
}

VOID
IoAddItem(INT Indent, INT IconIndex, LPWSTR pText, ...)
{
    SIZE_T size;
    WCHAR *ptr;
    va_list args;

    va_start(args, pText);
    size = (_vscwprintf(pText, args) + 1) * sizeof(WCHAR);

    ptr = (WCHAR*)Alloc(size);
    if (!ptr)
    {
        DebugTrace(L"Memory allocation failed!");
        return;
    }

    StringCbVPrintf(ptr, size, pText, args);
    va_end(args);

    CurrentColumn = 0;

    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            CurrentItemIndex = ListViewAddItem(Indent, IconIndex, ptr);
            break;

        case IO_TARGET_HTML:
            AppendStringToFile(L"\r\n\t<tr><td>");
            AppendStringToFile(ptr);
            AppendStringToFile(L"</td>");
            break;

        case IO_TARGET_CSV:
            AppendStringToFile(L"\r\n");
            AppendStringToFile(ptr);
            AppendStringToFile(L";");
            break;

        case IO_TARGET_TXT:
            AppendStringToFile(L"\r\n");
            AppendStringToFile(ptr);
            AppendStringToFile(L" ");
            break;

        case IO_TARGET_INI:
            AppendStringToFile(L"\r\n");
            AppendStringToFile(ptr);
            AppendStringToFile(L"=");
            break;

        case IO_TARGET_RTF:
        {
            WCHAR *text = TextToRTFText(ptr);

            if (text != NULL)
            {
                WriteRTFRowHeader();
                AppendStringToFileA(text);
                AppendStringToFileA(L"\\cell\r\n");

                Free(text);
            }
        }
        break;
    }

    Free(ptr);
}

VOID
IoAddValueName(INT Indent, INT IconIndex, UINT ValueID)
{
    WCHAR szText[MAX_STR_LEN * 2];

    LoadMUIStringF(hLangInst, ValueID, szText, MAX_STR_LEN);
    IoAddItem(Indent, IconIndex, szText);
}

VOID
IoSetItemText(LPWSTR pText, ...)
{
    WCHAR *ptr;
    va_list args;
    SIZE_T size;

    va_start(args, pText);
    size = (_vscwprintf(pText, args) + 1) * sizeof(WCHAR);

    ptr = (WCHAR*)Alloc(size);
    if (!ptr)
    {
        DebugTrace(L"Memory allocation failed!");
        return;
    }

    StringCbVPrintf(ptr, size, pText, args);
    va_end(args);

    CurrentColumn++;

    switch (IoTarget)
    {
        case IO_TARGET_LISTVIEW:
            ListViewSetItemText(CurrentItemIndex, CurrentColumn, ptr);
            break;

        case IO_TARGET_HTML:
        {
            AppendStringToFile(L"<td>");
            AppendStringToFile(ptr);
            AppendStringToFile(L"</td>");

            if (IoGetColumnsCount() == CurrentColumn + 1)
                AppendStringToFile(L"</tr>");
        }
        break;

        case IO_TARGET_CSV:
        {
            AppendStringToFile(ptr);

            if (IoGetColumnsCount() == CurrentColumn + 1)
                AppendStringToFile(L";");
        }
        break;

        case IO_TARGET_TXT:
        {
            AppendStringToFile(ptr);

            if (IoGetColumnsCount() == CurrentColumn + 1)
                AppendStringToFile(L" ");
        }
        break;

        case IO_TARGET_INI:
        {
            AppendStringToFile(ptr);

            if (IoGetColumnsCount() == CurrentColumn + 1)
                AppendStringToFile(L",");
        }
        break;

        case IO_TARGET_RTF:
        {
            WCHAR *text = TextToRTFText(ptr);

            if (text != NULL)
            {
                AppendStringToFileA(text);

                if (IoGetColumnsCount() == CurrentColumn + 1)
                    AppendStringToFileA(L"\\cell\r\n\\row\r\n");
                else
                    AppendStringToFileA(L"\\cell\r\n");

                Free(text);
            }
        }
        break;
    }

    Free(ptr);
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
            //AppendStringToFileA(L"\\par\n");
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

        case IO_TARGET_RTF:
            AppendStringToFileA(L"\r\n\\row\r\n");
            break;
    }
}

VOID
IoReportWriteColumnString(LPWSTR lpszString)
{
    WCHAR szText[MAX_STR_LEN * 5];
    WCHAR *pFormat;

    switch (IoTarget)
    {
        case IO_TARGET_HTML:
            pFormat = L"<td class='c'>%s</td>";
            break;

        case IO_TARGET_CSV:
            pFormat = L"%s;";
            break;

        case IO_TARGET_TXT:
            pFormat = L"%s ";
            break;

        case IO_TARGET_RTF:
        {
            WCHAR *text = TextToRTFText(lpszString);

            if (text != NULL)
            {
                StringCbPrintf(szText, sizeof(szText), L"{\\b %s}\\cell", text);
                AppendStringToFileA(szText);

                Free(text);
            }
            return;
        }

        default:
            return;
    }

    StringCbPrintf(szText, sizeof(szText), pFormat, lpszString);
    AppendStringToFile(szText);
}

VOID
IoAddColumnsList(COLUMN_LIST *List, LPWSTR lpCategoryName, LPWSTR lpIniPath)
{
    WCHAR szText[MAX_STR_LEN];
    SIZE_T Index = 0;
    INT width;

    if (IoTarget == IO_TARGET_RTF)
    {
        do
        {
        }
        while (List[++Index].StringID != 0);

        width = 11000 / (Index + 1);

        Index = 0;

        AppendStringToFileA(L"\\trowd\\trgaph50\r\n");

        do
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"\\clbrdrt\\brdrs\\clbrdrl\\brdrs\\clbrdrb\\brdrs\\clbrdrr\\brdrs\\cellx%d\r\n",
                           (Index + 1) * width);
            AppendStringToFileA(szText);
        }
        while (List[++Index].StringID != 0);

        AppendStringToFileA(L"\\pard\\intbl\r\n");

        Index = 0;
    }

    IoReportBeginColumn();

    do
    {
        LoadMUIStringF(hLangInst, List[Index].StringID, szText, MAX_STR_LEN);

        switch (IoTarget)
        {
            case IO_TARGET_LISTVIEW:
            {
                WCHAR szCol[3];
                INT Width = 0;

                if (!ParamsInfo.IsPortable)
                {
                    StringCbPrintf(szCol, sizeof(szCol), L"%d", Index);
                    Width = GetPrivateProfileInt(lpCategoryName, szCol, 0, lpIniPath);
                }

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
    DWORD dwBytesWritten, dwRes;

    hReport = CreateFile(lpszFile,
                         GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
    dwRes = GetLastError();

    DebugTrace(L"CreateFile() called. lpszFile = %s, Error code = %d",
               lpszFile, GetLastError());

    if (hReport == INVALID_HANDLE_VALUE)
    {
        WCHAR szText[MAX_STR_LEN];
        UINT id;

        switch (dwRes)
        {
            case ERROR_SHARING_VIOLATION:
                id = IDS_REPORT_SAVE_ERROR1;
                break;
            case ERROR_INVALID_NAME:
                id = IDS_REPORT_SAVE_ERROR2;
                break;
            default:
                id = IDS_REPORT_SAVE_ERROR3;
                break;
        }

        LoadMUIStringF(hLangInst, id, szText, MAX_STR_LEN);
        MessageBox(0, szText, NULL, MB_OK | MB_ICONERROR);

        DebugTrace(L"CreateFile() failed. Error code = %d", dwRes);
        return FALSE;
    }

    if (IoTarget != IO_TARGET_RTF)
    {
        if (!WriteFile(hReport, "\xEF\xBB\xBF", 3, &dwBytesWritten, NULL))
        {
            DebugTrace(L"WriteFile() failed! Error code = %d",
                       GetLastError());
        }
    }

    if (IoTarget == IO_TARGET_HTML)
    {
        AppendStringToFile(lpszHtmlHeader);
    }
    else if (IoTarget == IO_TARGET_RTF)
    {
        WCHAR szText[MAX_STR_LEN], szCodePage[MAX_STR_LEN];

        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE,
                      szCodePage, MAX_STR_LEN);
        StringCbPrintf(szText, sizeof(szText),
                       L"{\\rtf1\\ansi\\ansicpg%s",
                       szCodePage);
        AppendStringToFileA(szText);
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
#ifndef _M_IX86
        AppendStringToFile(L" (x64)");
#endif
        AppendStringToFile(L"<br />&copy; 2011-2012 <a href='http://www.aspia.ru'>Aspia Software</a></div>\r\n</body>\r\n</html>");
    }
    else if (IoTarget == IO_TARGET_RTF)
    {
        AppendStringToFileA(L"\r\n}");
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
    WCHAR szText[MAX_STR_LEN * 5] = {0};

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
                    LoadMUIStringF(hLangInst, IDS_REPORT_TOP, szTemp, sizeof(szTemp)/sizeof(WCHAR));
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
        {
            WCHAR *text = TextToRTFText(lpszTitle);

            if (text != NULL)
            {
                StringCbPrintf(szText, sizeof(szText),
                               L"\r\n\\pard\\sa200\\sl276\\slmult1\\lang9\\f0\\fs22{\\b\\ul %s}\\par\n",
                               text);
                AppendStringToFileA(szText);

                Free(text);
            }
            return;
        }

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
    WCHAR szText[MAX_STR_LEN * 5];

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
        {
            WCHAR *text = TextToRTFText(lpszTitle);

            if (text != NULL)
            {
                StringCbPrintf(szText, sizeof(szText),
                               L"{\\b %s}\\par\r\n",
                               text);
                AppendStringToFileA(szText);

                Free(text);
            }
        }
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

        case IO_TARGET_RTF:
            AppendStringToFileA(L"\\par");
            break;
    }
}

VOID
IoWriteContentTableItem(UINT ID, LPWSTR lpszName, BOOL IsRootItem)
{
    WCHAR szText[MAX_STR_LEN * 5];

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
        {
            WCHAR *text = TextToRTFText(lpszName);

            if (text != NULL)
            {
                StringCbPrintf(szText, sizeof(szText),
                               L"%s\\par", text);
                AppendStringToFileA(szText);

                Free(text);
            }
        }
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

    SetCanceledState(FALSE);

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

    SetCanceledState(FALSE);
}
