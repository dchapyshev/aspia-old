/*
 * PROJECT:         Aspia
 * FILE:            aspia/misc.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"
#include "driver.h"


/* Definition for the SetWindowTheme function */
typedef HRESULT (WINAPI *PSWT)(HWND, LPCWSTR, LPCWSTR);


HICON
GetFolderAssocIcon(LPWSTR lpszFolder)
{
    SHFILEINFO sfi = {0};

    SHGetFileInfo(lpszFolder,
                  0,
                  &sfi,
                  sizeof(SHFILEINFO),
                  SHGFI_ICON | SHGFI_SMALLICON);
    return sfi.hIcon;
}

BOOL
GetFileDescription(LPWSTR lpszPath,
                   LPWSTR lpszDesc,
                   SIZE_T Size)
{
    struct LANGANDCODEPAGE
    {
        WORD wLanguage;
        WORD wCodePage;
    } *lpTranslate;

    WCHAR szText[MAX_STR_LEN];
    DWORD dwSize, dwHandle;
    LPVOID pData, pResult;

    dwSize = GetFileVersionInfoSize(lpszPath, &dwHandle);
    if (!dwSize) return FALSE;

    pData = Alloc(dwSize);
    if (!pData)
    {
        DebugAllocFailed();
        return FALSE;
    }

    if (GetFileVersionInfo(lpszPath, dwHandle, dwSize, pData))
    {
        if (VerQueryValue(pData,
                          L"\\VarFileInfo\\Translation",
                          (LPVOID*)&lpTranslate,
                          (PUINT)&dwSize))
        {
            if (lpTranslate)
            {
                StringCbPrintf(szText, sizeof(szText),
                               L"\\StringFileInfo\\%04x%04x\\FileDescription",
                               lpTranslate->wLanguage,
                               lpTranslate->wCodePage);

                if (VerQueryValue(pData, szText, (LPVOID*)&pResult, (PUINT)&dwSize))
                {
                    StringCbCopy(lpszDesc, Size, pResult);
                    Free(pData);
                    return TRUE;
                }
            }
        }
    }
    Free(pData);

    return FALSE;
}

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
ConvertSecondsToString(LONGLONG Seconds,
                       LPWSTR lpszString,
                       SIZE_T Size)
{
    LONGLONG Days  =  (Seconds / 86400);
    LONGLONG Hours = ((Seconds % 86400) / 3600);
    LONGLONG Mins  = ((Seconds % 86400) % 3600) / 60;
    LONGLONG Secs  = ((Seconds % 86400) % 3600) % 60;
    WCHAR szFormat[MAX_STR_LEN];

    LoadMUIString(IDS_SYS_UPTIME_FORMAT, szFormat, MAX_STR_LEN);

    StringCbPrintf(lpszString, Size, szFormat, Days, Hours, Mins, Secs);
}

VOID
ChopSpaces(LPWSTR s, SIZE_T size)
{
    WCHAR szNew[MAX_STR_LEN] = {0};
    SIZE_T i, j;

    for (i = 0, j = 0; i < (SIZE_T)SafeStrLen(s); ++i)
    {
        if (s[i] == L' ' && s[i + 1] == L' ')
        {
            ++i;
            continue;
        }
        else
        {
            if (s[i] == L' ' && j == 0)
                continue;
            szNew[j++] = s[i];
        }
    }

    StringCbCopy(s, size, szNew);
}

BOOL
TimeToString(time_t Time, LPWSTR lpTimeStr, SIZE_T Size)
{
    struct tm *t = localtime(&Time);
    WCHAR *time;
    INT len;

    if (!t) return FALSE;

    time = _wasctime(t);
    if (!time) return FALSE;

    len = SafeStrLen(time);
    if (len == 0) return FALSE;

    time[SafeStrLen(time) - 1] = 0;

    StringCbCopy(lpTimeStr, Size, time);

    return TRUE;
}

/* http://social.msdn.microsoft.com/Forums/en-US/vclanguage/thread/348a99cb-758e-4cff-858c-10d09710f784/ */
BOOL
GetMSProductKey(BOOL is64, LPSTR lpszKeyPath, LPWSTR lpszKey, INT iSize)
{
    REGSAM samDesired = KEY_QUERY_VALUE;
    const CHAR *KeyChars[25];
    CHAR *wsResult, *pch, sCDKey[30];
    BYTE *DigitalProductID;
    SIZE_T DataLength;
    BYTE ProductKeyExtract[15];
    INT ByteCounter, k, nCur;
    HKEY hKey;
    BOOL bOk;

    DebugTrace(L"is64 = %d, lpszKeyPath = %S, lpszKey = %s, iSize = %d",
               is64, lpszKeyPath, lpszKey, iSize);

    KeyChars[0] = "B"; KeyChars[1] = "C";
    KeyChars[2] = "D"; KeyChars[3] = "F";
    KeyChars[4] = "G"; KeyChars[5] = "H";
    KeyChars[6] = "J"; KeyChars[7] = "K";
    KeyChars[8] = "M"; KeyChars[9] = "P";
    KeyChars[10] = "Q"; KeyChars[11] = "R";
    KeyChars[12] = "T"; KeyChars[13] = "V";
    KeyChars[14] = "W"; KeyChars[15] = "X";
    KeyChars[16] = "Y"; KeyChars[17] = "2";
    KeyChars[18] = "3"; KeyChars[19] = "4";
    KeyChars[20] = "6"; KeyChars[21] = "7";
    KeyChars[22] = "8"; KeyChars[23] = "9";
    KeyChars[24] = NULL;
    /*KeyChars[25] = {"B","C","D","F","G","H","J","K","M",
                              "P","Q","R","T","V","W","X","Y","2",
                              "3","4","6","7","8","9",NULL};*/

    DigitalProductID = 0;
    bOk = FALSE;

    if (is64)
    {
#define WIN64KEY
    }

#ifdef WIN64KEY
#ifdef KEY_WOW64_64KEY
#else
#define KEY_WOW64_64KEY 0x0100
#endif
    /* Check Win2000 (KEY_WOW64_64KEY - not supported) */
    if (!IsWindows2000() && is64)
    {
        samDesired |= KEY_WOW64_64KEY;
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, lpszKeyPath,
                      REG_OPTION_NON_VOLATILE,
                      samDesired,
                      &hKey) == ERROR_SUCCESS)
#else
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, lpszKeyPath,
                      REG_OPTION_NON_VOLATILE,
                      KEY_ALL_ACCESS,
                      &hKey) == ERROR_SUCCESS)
#endif

    {
        DataLength = 164;
        DigitalProductID = (BYTE*)Alloc(DataLength);
        SecureZeroMemory(DigitalProductID, DataLength);

        if (RegQueryValueExA(hKey,
                             "DigitalProductId",
                             NULL, NULL,
                             DigitalProductID,
                             &DataLength) == ERROR_SUCCESS)
        {
            CopyMemory(ProductKeyExtract, DigitalProductID + 52, 15);
            bOk = TRUE;
        }

        if (DigitalProductID) Free(DigitalProductID);
        RegCloseKey(hKey);
    }
    else return FALSE;

    SecureZeroMemory(sCDKey, sizeof(sCDKey));

    for (ByteCounter = 24; ByteCounter >= 0; --ByteCounter)
    {
        nCur = 0;
        for (k = 14; k >= 0; --k)
        {
            nCur = (nCur * 256) ^ ProductKeyExtract[k];
            ProductKeyExtract[k] = (BYTE)(nCur / 24);
            nCur %= 24;
        }
        StringCbCatA((STRSAFE_LPSTR)sCDKey,
                      sizeof(sCDKey),
                      KeyChars[nCur]);
        if (!(ByteCounter % 5) && ByteCounter)
            StringCbCatA(sCDKey, sizeof(sCDKey), "-");
    }

    _strrev(sCDKey);
    wsResult = sCDKey;

    pch = strstr(wsResult, "-");

    if (pch)
    {
        if (lpszKey)
        {
            MultiByteToWideChar(CP_ACP, 0, wsResult, -1, lpszKey, iSize);
        }
    }
    else
    {
         GetMSProductKey(TRUE, lpszKeyPath, NULL, 0);
    }

#ifdef WIN64KEY
#undef WIN64KEY
#endif
    return TRUE;
}

INT
LoadMUIString(UINT ResID, LPWSTR Buffer, INT BufLen)
{
    HGLOBAL hMem;
    HRSRC hRsrc;
    WCHAR *p;
    int string_num;
    int i;

    if (!Buffer) return 0;

    hRsrc = FindResource(hLangInst,
                         MAKEINTRESOURCE((LOWORD(ResID) >> 4) + 1),
                         (LPWSTR)RT_STRING);
    if (!hRsrc) return 0;

    hMem = LoadResource(hLangInst, hRsrc);
    if (!hMem) return 0;

    p = LockResource(hMem);
    string_num = ResID & 0x000f;
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
            pSetWindowTheme(hwnd, L"Explorer", 0);
        }
        FreeLibrary(hUxThemeDLL);
    }
}

double
Round(double Argument, int Precision)
{
    double div = 1.0;

    if (Precision < 0)
        while(Precision++) div /= 10.0;
    else
        while(Precision--) div *= 10.0;

    return floor(Argument * div + 0.5) / div;
}
