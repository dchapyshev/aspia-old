/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/misc.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"


/***************************************************************************/
/* FUNCTIONS ***************************************************************/
/***************************************************************************/

BOOL
IsDebugModeEnabled(VOID)
{
    WCHAR szPath[MAX_PATH];

    if (!GetCurrentPath(szPath, MAX_PATH))
        return FALSE;

    StringCbCat(szPath, sizeof(szPath), L"\\debug");

    if (GetFileAttributes(szPath) == INVALID_FILE_ATTRIBUTES)
        return FALSE;

    return TRUE;
}

BOOL
IsPortable(VOID)
{
    WCHAR szPath[MAX_PATH];

    if (!GetCurrentPath(szPath, MAX_PATH))
        return FALSE;

    StringCbCat(szPath, sizeof(szPath), L"\\portable");

    if (GetFileAttributes(szPath) == INVALID_FILE_ATTRIBUTES)
        return FALSE;

    return TRUE;
}

/* Функция для установки привилегий для текущего процесса */
VOID
SetProcessPrivilege(LPCTSTR lpName)
{
    HANDLE hToken;

    if (OpenProcessToken(GetCurrentProcess(),
                         TOKEN_ALL_ACCESS,
                         &hToken))
    {
        TOKEN_PRIVILEGES tkp;
        LUID Value;

        if (LookupPrivilegeValue(NULL, lpName, &Value))
        {
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Luid = Value;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL);
        }

        CloseHandle(hToken);
    }
}

INT
StringLoad(UINT ResourceId, LPWSTR Buffer, INT BufLen)
{
    HGLOBAL hMem;
    HRSRC hRsrc;
    WCHAR *p;
    INT string_num;
    INT i;

    if (!Buffer) return 0;

    hRsrc = FindResourceEx(GlobalVars.hInstance,
                           (LPWSTR)RT_STRING,
                           MAKEINTRESOURCE((LOWORD(ResourceId) >> 4) + 1),
                           GlobalVars.wLanguage);
    if (!hRsrc) return 0;

    hMem = LoadResource(GlobalVars.hInstance, hRsrc);
    if (!hMem) return 0;

    p = LockResource(hMem);
    string_num = ResourceId & 0x000f;
    for (i = 0; i < string_num; i++)
        p += *p + 1;

    if (BufLen == 0)
    {
        *((LPWSTR*)Buffer) = p + 1;
        return *p;
    }

    i = min(BufLen - 1, *p);
    if (i > 0)
    {
        memcpy(Buffer, p + 1, i * sizeof(WCHAR));
        Buffer[i] = 0;
    }
    else
    {
        if (BufLen > 1)
        {
            Buffer[0] = 0;
            return 0;
        }
    }

    return i;
}

HMENU
MenuLoad(LPCWSTR lpMenuName)
{
    HANDLE Resource =
        FindResourceEx(GlobalVars.hInstance, RT_MENU,
                       lpMenuName, GlobalVars.wLanguage);

    if (!Resource) return NULL;

    return (LoadMenuIndirect((PVOID)LoadResource(GlobalVars.hInstance, Resource)));
}

INT_PTR
DialogLoad(LPCWSTR lpDialogName, HWND hWndParent, DLGPROC lpDialogFunc)
{
    HANDLE Resource =
        FindResourceEx(GlobalVars.hInstance, RT_DIALOG,
                       lpDialogName, GlobalVars.wLanguage);

    if (!Resource) return NULL;

    return (DialogBoxIndirectParam(GlobalVars.hInstance,
        (PVOID)LoadResource(GlobalVars.hInstance, Resource), hWndParent, lpDialogFunc, 0));
}
