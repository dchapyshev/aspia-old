/*
 * PROJECT:         Aspia (Helper DLL)
 * FILE:            helper_dll/debug.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"

#define MAX_STR_LEN 256

static HANDLE hDebugLog = INVALID_HANDLE_VALUE;


VOID
WriteDebugLog(LPSTR lpFile, UINT iLine, LPSTR lpFunc, LPWSTR lpMsg, ...)
{
    LARGE_INTEGER FileSize, MoveTo, NewPos;
    WCHAR szMsg[MAX_STR_LEN * 3];
    WCHAR szText[MAX_STR_LEN * 4], szTime[MAX_STR_LEN];
    DWORD dwBytesWritten;
    va_list args;

    if (!hDebugLog || hDebugLog == INVALID_HANDLE_VALUE)
        return;

    MoveTo.QuadPart = 0;
    if (!SetFilePointerEx(hDebugLog, MoveTo, &NewPos, FILE_END))
        return;

    if (!GetFileSizeEx(hDebugLog, &FileSize))
        return;

    LockFile(hDebugLog, (DWORD_PTR)NewPos.QuadPart, 0, (DWORD_PTR)FileSize.QuadPart, 0);

    GetTimeFormat(LOCALE_USER_DEFAULT,
                  0, NULL, NULL, szTime,
                  MAX_STR_LEN);

    va_start(args, lpMsg);
    StringCbVPrintf(szMsg, sizeof(szMsg), lpMsg, args);
    va_end(args);

    StringCbPrintf(szText, sizeof(szText),
                   L"[%s] %S:%ld %S(): \"%s\"\r\n",
                   szTime, lpFile, iLine, lpFunc, szMsg);

    WriteFile(hDebugLog, szText,
              wcslen(szText) * sizeof(WCHAR),
              &dwBytesWritten, NULL);

    UnlockFile(hDebugLog, (DWORD_PTR)NewPos.QuadPart, 0, (DWORD_PTR)FileSize.QuadPart, 0);
}

BOOL
InitDebugLog(LPWSTR lpLogName, LPWSTR lpVersion)
{
    WCHAR szPath[MAX_PATH], szMsg[MAX_STR_LEN],
          szCurrentPath[MAX_PATH];
    DWORD dwBytesWritten;

    if (!GetCurrentPath(szCurrentPath, MAX_PATH))
        return FALSE;

    StringCbPrintf(szPath, sizeof(szPath),
                   L"%s%s",
                   szCurrentPath,
                   lpLogName);

    hDebugLog = CreateFile(szPath,
                           GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
    if (hDebugLog == INVALID_HANDLE_VALUE)
        return FALSE;
    
    WriteFile(hDebugLog, "\xFF\xFE", 2, &dwBytesWritten, NULL);

    StringCbPrintf(szMsg, sizeof(szMsg),
#ifdef _M_IX86
                   L"Log file created, Aspia %s",
#else
                   L"Log file created, Aspia %s (x64)",
#endif
                   lpVersion);
    DebugTrace(szMsg);

    return TRUE;
}

VOID
CloseDebugLog(VOID)
{
    DebugTrace(L"Log file is closed");

    CloseHandle(hDebugLog);
    hDebugLog = INVALID_HANDLE_VALUE;
}
