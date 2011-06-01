/*
 * PROJECT:         Aspia
 * FILE:            aspia/report/html.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"


static FILE *hReport = NULL;

#define lpszHtmlHeader L"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>\
<html xmlns='http://www.w3.org/1999/xhtml'><head>\
<meta http-equiv='content-type' content='text/html; charset=utf-8' />\
<meta name='rights' content='Aspia Software, Dmitry Chapyshev' />\
<meta name='generator' content='Aspia' />\
<title>Aspia</title>\
<style type='text/css'>\
body{\
color:#353535;\
font-family:Tahoma,Arial,Verdana;\
}\
table{\
background-color:#F9F9F9;\
border:1px solid #E2E2E0;\
font-size:12px;\
}\
td{\
padding:5px;\
margin:5px;\
height:20px;\
border-bottom:1px solid #E2E2E0;\
}\
.h{\
color:#000;\
font-weight:bold;\
}\
.c{\
color:#fff;\
background-color:#383637;\
font-weight:bold;\
}\
#f{\
font-size:12px;\
text-align:center;\
}\
a{\
color:#4475bf;\
}\
h1 a{\
font-size:18px;\
}\
h2 a{\
font-size:16px;\
}\
h3 a{\
font-size:13px;\
}\
h1{\
font-size:18px;\
}\
h2{\
font-size:16px;\
}\
h3{\
font-size:13px;\
}\
</style>\
</head><body>"


BOOL
HtmlAppendStringToFile(LPWSTR lpszString)
{
    DWORD dwLen = SafeStrLen(lpszString);

    if (!hReport) return FALSE;

    fwrite(lpszString, sizeof(WCHAR), dwLen, hReport);

    return TRUE;
}

BOOL
HtmlCreateReport(LPWSTR lpszFile)
{
    if (_wfopen_s(&hReport, lpszFile, L"wt+,ccs=UTF-8") != 0)
        return FALSE;

    if (!hReport) return FALSE;

    HtmlAppendStringToFile(lpszHtmlHeader);
    return TRUE;
}

VOID
HtmlCloseReport(VOID)
{
    HtmlAppendStringToFile(L"<br /><div id='f'>Aspia ");
    HtmlAppendStringToFile(VER_FILEVERSION_STR);
    HtmlAppendStringToFile(L"<br />&copy; 2011 <a href='http://www.aspia.ru'>Aspia Software</a></div></body></html>");

    fclose(hReport);
    hReport = NULL;
}

VOID
HtmlBeginColumn(VOID)
{
    HtmlAppendStringToFile(L"<tr>");
}

VOID
HtmlEndColumn(VOID)
{
    HtmlAppendStringToFile(L"</tr>");
}

VOID
HtmlWriteValueString(LPWSTR lpszString)
{
    WCHAR szText[MAX_STR_LEN * 15];

    StringCbPrintf(szText, sizeof(szText),
                   L"<td>%s</td>",
                   (SafeStrLen(lpszString) == 0) ? L"&nbsp;" : lpszString);

    HtmlAppendStringToFile(szText);
}

VOID
HtmlWriteItemString(LPWSTR lpszString, BOOL bIsHeader)
{
    WCHAR szText[MAX_STR_LEN * 5];

    StringCbPrintf(szText, sizeof(szText),
                   L"<tr>%s%s</td>%s",
                   bIsHeader ? L"<td class='h'>" : L"<td>",
                   lpszString,
                   bIsHeader ? L"" : L"");
    HtmlAppendStringToFile(szText);
}

VOID
HtmlWriteColumnString(LPWSTR lpszString)
{
    WCHAR szText[MAX_STR_LEN * 3];

    StringCbPrintf(szText, sizeof(szText),
                   L"<td class='c'>%s</td>",
                   lpszString);
    HtmlAppendStringToFile(szText);
}

VOID
HtmlTableTitle(LPWSTR lpszTitle, UINT StringID, BOOL WithContentTable)
{
    WCHAR szText[MAX_STR_LEN], szTemp[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText),
                   L"<h2 id='i%ld'>%s",
                   StringID, lpszTitle);
    HtmlAppendStringToFile(szText);

    if (!IsRootCategory(StringID, RootCategoryList))
    {
        if (WithContentTable)
        {
            LoadMUIString(IDS_REPORT_TOP, szTemp, sizeof(szTemp)/sizeof(WCHAR));
            StringCbPrintf(szText, sizeof(szText),
                           L"  (<a href='#top'>%s</a>)</h2>",
                           szTemp);
            HtmlAppendStringToFile(szText);
            return;
        }
    }

    HtmlAppendStringToFile(L"</h2>");
}

VOID
HtmlBeginTable(VOID)
{
    HtmlAppendStringToFile(L"<table cellspacing='0' cellpadding='0'>");
}

VOID
HtmlEndTable(VOID)
{
    HtmlAppendStringToFile(L"</table>");
}

VOID
HtmlBeginContentTable(LPWSTR lpszTitle)
{
    WCHAR szText[MAX_STR_LEN];

    StringCbPrintf(szText, sizeof(szText),
                   L"<h1 id='top' style='font-size:18px'>%s</h1><ul>",
                   lpszTitle);
    HtmlAppendStringToFile(szText);
}

VOID
HtmlEndContentTable(VOID)
{
    HtmlAppendStringToFile(L"</ul>");
}

VOID
HtmlContentTableItem(UINT ID, LPWSTR lpszName, BOOL IsRootItem)
{
    WCHAR szText[MAX_STR_LEN];

    if (!IsRootItem)
    {
        StringCbPrintf(szText, sizeof(szText),
                       L"<li><a href='#i%ld'>%s</a></li>",
                       ID, lpszName);
    }
    else
    {
        StringCbPrintf(szText, sizeof(szText),
                       L"<li>%s<ul>", lpszName);
    }
    HtmlAppendStringToFile(szText);
}

VOID
HtmlContentTableEndRootItem(VOID)
{
    HtmlAppendStringToFile(L"</ul></li>");
}
