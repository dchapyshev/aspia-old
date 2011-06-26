/*
 * PROJECT:         Aspia
 * FILE:            aspia/report/ini.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"


static FILE *hReport = NULL;


BOOL
IniAppendStringToFile(LPWSTR lpszString)
{
    DWORD dwLen = SafeStrLen(lpszString);

    if (!hReport) return FALSE;

    fwrite(lpszString, sizeof(WCHAR), dwLen, hReport);

    return TRUE;
}

BOOL
IniCreateReport(LPWSTR lpszFile)
{
    if (_wfopen_s(&hReport, lpszFile, L"wt+,ccs=UTF-8") != 0)
        return FALSE;

    if (!hReport) return FALSE;

    return TRUE;
}

VOID
IniCloseReport(VOID)
{
    fclose(hReport);
    hReport = NULL;
}

VOID
IniWriteValueString(LPWSTR lpszString)
{
    WCHAR szText[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText), L"%s;", lpszString);
    IniAppendStringToFile(szText);
}

VOID
IniWriteItemString(LPWSTR lpszString)
{
    WCHAR szText[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText), L"\n%s;", lpszString);
    IniAppendStringToFile(szText);
}

VOID
IniTableTitle(LPWSTR lpszTitle)
{
    WCHAR szText[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText), L"\n[%s]\n", lpszTitle);
    IniAppendStringToFile(szText);
}

VOID
IniEndTable(VOID)
{
    IniAppendStringToFile(L"\n");
}
