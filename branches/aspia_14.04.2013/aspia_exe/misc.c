/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/misc.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"


/* Definition for the SetWindowTheme function */
typedef HRESULT (WINAPI *PSWT)(HWND, LPCWSTR, LPCWSTR);


VOID
CopyTextToClipboard(LPCTSTR lpszText)
{
    HGLOBAL ClipBuffer;
    WCHAR *Buffer;

    if (!lpszText) return;

    if (OpenClipboard(NULL))
    {
        SIZE_T BufSize = (SafeStrLen(lpszText) + 1) * sizeof(WCHAR);
        EmptyClipboard();

        ClipBuffer = GlobalAlloc(GMEM_DDESHARE, BufSize);
        Buffer = (WCHAR*)GlobalLock(ClipBuffer);

        StringCbCopy(Buffer, BufSize, lpszText);

        GlobalUnlock(ClipBuffer);

        SetClipboardData(CF_UNICODETEXT, ClipBuffer);

        CloseClipboard();
    }
}

VOID
IntSetWindowTheme(HWND hwnd)
{
    HINSTANCE hUxThemeDLL;
    PSWT pSetWindowTheme;

    hUxThemeDLL = LoadLibrary(L"UXTHEME.DLL");
    if (hUxThemeDLL)
    {
        pSetWindowTheme = (PSWT)GetProcAddress(hUxThemeDLL, "SetWindowTheme");
        if (pSetWindowTheme)
        {
            pSetWindowTheme(hwnd, SettingsInfo.ShowWindowStyles ? L"Explorer" : NULL, 0);
        }
        FreeLibrary(hUxThemeDLL);
    }
}
