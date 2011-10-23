/*
 * PROJECT:         Aspia
 * FILE:            aspia/debug.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"


static HANDLE hDebugLog = INVALID_HANDLE_VALUE;


VOID
DebugWriteLog(LPSTR lpFile, UINT iLine, LPSTR lpFunc, LPWSTR lpMsg, ...)
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
              SafeStrLen(szText) * sizeof(WCHAR),
              &dwBytesWritten, NULL);

    UnlockFile(hDebugLog, (DWORD_PTR)NewPos.QuadPart, 0, (DWORD_PTR)FileSize.QuadPart, 0);
}

BOOL
DebugCreateLog(VOID)
{
    WCHAR szPath[MAX_PATH], szMsg[MAX_STR_LEN];
    DWORD dwBytesWritten;

    StringCbPrintf(szPath, sizeof(szPath),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
                   L"debug.log");

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
                   L"Log file created, Aspia %s",
                   VER_FILEVERSION_STR);
    DebugTrace(szMsg);

    return TRUE;
}

VOID
DebugCloseLog(VOID)
{
    DebugTrace(L"Log file is closed");

    CloseHandle(hDebugLog);
    hDebugLog = INVALID_HANDLE_VALUE;
}
