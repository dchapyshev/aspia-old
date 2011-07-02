/*
 * PROJECT:         Aspia
 * FILE:            aspia/report/text.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"

static FILE *hReport = NULL;


BOOL
TextAppendStringToFile(LPWSTR lpszString)
{
    DWORD dwLen = SafeStrLen(lpszString);

    if (!hReport) return FALSE;

    fwrite(lpszString, sizeof(WCHAR), dwLen, hReport);

    return TRUE;
}

BOOL
TextCreateReport(LPWSTR lpszFile)
{
    if (_wfopen_s(&hReport, lpszFile, L"wt+,ccs=UTF-8") != 0)
        return FALSE;

    if (!hReport) return FALSE;

    return TRUE;
}

VOID
TextCloseReport(VOID)
{
    fclose(hReport);
    hReport = NULL;
}

VOID
TextWriteValueString(LPWSTR lpszString)
{
    WCHAR szText[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText), L"%s ", lpszString);
    TextAppendStringToFile(szText);
}

VOID
TextWriteItemString(LPWSTR lpszString)
{
    WCHAR szText[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText), L"\n%s ", lpszString);
    TextAppendStringToFile(szText);
}

VOID
TextTableTitle(LPWSTR lpszTitle)
{
    WCHAR szText[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText), L"\n%s\n", lpszTitle);
    TextAppendStringToFile(szText);
}

VOID
TextEndTable(VOID)
{
    TextAppendStringToFile(L"\n");
}

VOID
TextWriteColumnString(LPWSTR lpszString)
{
    WCHAR szText[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText), L"%s ", lpszString);
    TextAppendStringToFile(szText);
}
