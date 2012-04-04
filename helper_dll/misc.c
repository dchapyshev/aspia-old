/*
 * PROJECT:         Aspia (Helper DLL)
 * FILE:            helper_dll/misc.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "helper_dll.h"


typedef NTSTATUS (NTAPI* PNQSI) (SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION
{
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

INT
GetCpuUsage(VOID)
{
    static PNQSI pNtQuerySystemInformation = NULL;
    static SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION LastInfo = {0};
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION CurrentInfo = {0};
    double CpuUsage = 0;
    ULONG Size;

    if (pNtQuerySystemInformation == NULL)
    {
        HMODULE hDLL = GetModuleHandle(L"ntdll.dll");
        if (hDLL == NULL) return 0;

        pNtQuerySystemInformation =
            (PNQSI)GetProcAddress(hDLL ,"NtQuerySystemInformation");
        if (pNtQuerySystemInformation == NULL)
            return 0;
    }

    pNtQuerySystemInformation(8, /* SystemProcessorPerformanceInformation */
                              &CurrentInfo, sizeof(CurrentInfo), &Size);

    if (LastInfo.KernelTime.QuadPart != 0 || LastInfo.UserTime.QuadPart != 0)
    {
        CpuUsage = 100.0 - (double)(CurrentInfo.IdleTime.QuadPart - LastInfo.IdleTime.QuadPart) /
        (double)(CurrentInfo.KernelTime.QuadPart - LastInfo.KernelTime.QuadPart +
                 CurrentInfo.UserTime.QuadPart - LastInfo.UserTime.QuadPart) * 100.0;
    }

    LastInfo = CurrentInfo;

    return (INT)CpuUsage;
}

INT
GetSystemColorDepth(VOID)
{
    DEVMODE DevMode = {0};
    INT ColorDepth;

    DevMode.dmSize = sizeof(DEVMODE);
    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DevMode))
    {
        return ILC_COLOR;
    }

    switch (DevMode.dmBitsPerPel)
    {
        case 32: ColorDepth = ILC_COLOR32; break;
        case 24: ColorDepth = ILC_COLOR24; break;
        case 16: ColorDepth = ILC_COLOR16; break;
        case  8: ColorDepth = ILC_COLOR8;  break;
        case  4: ColorDepth = ILC_COLOR4;  break;
        default: ColorDepth = ILC_COLOR;   break;
    }

    return ColorDepth;
}

BOOL
CreateScreenshot(HWND hwnd)
{
    WCHAR szFilter[] = L"24 bit Bitmap (*.bmp,*.dib)\0*.bmp\0";
    WCHAR szFileName[MAX_PATH];
    HDC hBitmapDC, hWindowDC;
    LPBITMAPINFO lpBitmapInfo;
    BITMAPFILEHEADER FileHeader;
    OPENFILENAME ofn = {0};
    INT width, height;
    HBITMAP hBitmap;
    BITMAP Bitmap;
    WORD cClrBits;
    LPVOID lpvBits;
    HANDLE hFile;
    RECT rect;

    /* get window resolution */
    GetClientRect(hwnd, &rect);

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    /* get a DC for the window */
    hWindowDC = GetDC(hwnd);

    /* get a bitmap handle for the screen
     * needed to convert to a DIB */
    hBitmap = CreateCompatibleBitmap(hWindowDC,
                                     width,
                                     height);
    if (hBitmap == NULL)
    {
        ReleaseDC(hwnd, hWindowDC);
        return FALSE;
    }

    /* get a DC compatable with the window DC */
    hBitmapDC = CreateCompatibleDC(hWindowDC);
    if (hBitmapDC == NULL)
    {
        ReleaseDC(hwnd, hWindowDC);
        return FALSE;
    }

    /* select the bitmap into the DC */
    SelectObject(hBitmapDC, hBitmap);

    /* copy the window DC to the bitmap */
    BitBlt(hBitmapDC,
           0,
           0,
           width,
           height,
           hWindowDC,
           0,
           0,
           SRCCOPY);

    /* we're finished with the screen DC */
    ReleaseDC(hwnd, hWindowDC);

    if (!GetObjectW(hBitmap,
                    sizeof(BITMAP),
                    (LPWSTR)&Bitmap))
    {
        return FALSE;
    }

    cClrBits = (WORD)(Bitmap.bmPlanes * Bitmap.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

    if (cClrBits != 24)
    {
        lpBitmapInfo =
            (LPBITMAPINFO)HeapAlloc(GetProcessHeap(), 0,
                                    sizeof(BITMAPINFOHEADER) +
                                    sizeof(RGBQUAD) * (1 << cClrBits));
    }
    else
    {
        lpBitmapInfo =
            (LPBITMAPINFO)HeapAlloc(GetProcessHeap(), 0,
                                    sizeof(BITMAPINFOHEADER));
    }

    if (lpBitmapInfo == NULL)
        return FALSE;

    lpBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpBitmapInfo->bmiHeader.biWidth = Bitmap.bmWidth;
    lpBitmapInfo->bmiHeader.biHeight = Bitmap.bmHeight;
    lpBitmapInfo->bmiHeader.biPlanes = Bitmap.bmPlanes;
    lpBitmapInfo->bmiHeader.biBitCount = Bitmap.bmBitsPixel;

    if (cClrBits < 24)
        lpBitmapInfo->bmiHeader.biClrUsed = (1 << cClrBits);

    lpBitmapInfo->bmiHeader.biCompression = BI_RGB;
    lpBitmapInfo->bmiHeader.biSizeImage = ((lpBitmapInfo->bmiHeader.biWidth * cClrBits +31) & ~31) /8
                                           * lpBitmapInfo->bmiHeader.biHeight;

    lpBitmapInfo->bmiHeader.biClrImportant = 0;

    lpvBits = HeapAlloc(GetProcessHeap(), 0, lpBitmapInfo->bmiHeader.biSizeImage);
    if (lpvBits == NULL) return FALSE;

    /* convert the DDB to a DIB */
    if (!GetDIBits(hBitmapDC,
                   hBitmap,
                   0,
                   height,
                   lpvBits,
                   lpBitmapInfo,
                   DIB_RGB_COLORS))
    {
        return FALSE;
    }

    wcscpy(szFileName, L"screenshot.bmp");
    ofn.lStructSize   = sizeof(OPENFILENAME);
    ofn.hwndOwner     = hwnd;
    ofn.nMaxFile      = MAX_PATH;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.lpstrDefExt   = L"bmp";
    ofn.lpstrFilter   = szFilter;
    ofn.lpstrFile     = szFileName;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn))
    {
        DWORD dwBytesWritten;
        BOOL bSuccess;

        hFile = CreateFile(ofn.lpstrFile,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile == INVALID_HANDLE_VALUE)
            return FALSE;

        /* write the BITMAPFILEHEADER to file */
        FileHeader.bfType = *(WORD*)"BM";  /* 0x4D 0x42 */
        FileHeader.bfReserved1 = 0;
        FileHeader.bfReserved2 = 0;

        bSuccess = WriteFile(hFile,
                             &FileHeader,
                             sizeof(FileHeader),
                             &dwBytesWritten,
                             NULL);
        if ((!bSuccess) || (dwBytesWritten < sizeof(FileHeader)))
            goto Cleanup;

        /* write the BITMAPINFOHEADER to file */
        bSuccess = WriteFile(hFile,
                             &lpBitmapInfo->bmiHeader,
                             sizeof(BITMAPINFOHEADER),
                             &dwBytesWritten,
                             NULL);
        if ((!bSuccess) || (dwBytesWritten < sizeof(BITMAPINFOHEADER)))
            goto Cleanup;

        /* save the current file position at the final file size */
        FileHeader.bfOffBits = SetFilePointer(hFile, 0, 0, FILE_CURRENT);

        /* write the bitmap bits to file */
        bSuccess = WriteFile(hFile,
                             lpvBits,
                             lpBitmapInfo->bmiHeader.biSizeImage,
                             &dwBytesWritten,
                             NULL);
        if ((!bSuccess) || (dwBytesWritten < lpBitmapInfo->bmiHeader.biSizeImage))
            goto Cleanup;

        /* save the current file position at the final file size */
        FileHeader.bfSize = SetFilePointer(hFile, 0, 0, FILE_CURRENT);

        /* rewrite the updated file headers */
        SetFilePointer(hFile, 0, 0, FILE_BEGIN);
        bSuccess = WriteFile(hFile,
                             &FileHeader,
                             sizeof(FileHeader),
                             &dwBytesWritten,
                             NULL);
        if ((!bSuccess) || (dwBytesWritten < sizeof(FileHeader)))
            goto Cleanup;

        CloseHandle(hFile);
        return TRUE;

Cleanup:
        CloseHandle(hFile);
        DeleteFile(ofn.lpstrFile);
    }

    return FALSE;
}

BOOL
CenterWindow(HWND hWnd, HWND hWndCenter)
{
    DWORD dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
    HMONITOR hMonitor = NULL;
    RECT rcDlg;
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;
    MONITORINFO minfo;
    INT DlgWidth;
    INT DlgHeight;
    INT xLeft;
    INT yTop;

    if (!hWndCenter)
    {
        if (dwStyle & WS_CHILD)
            hWndCenter = GetParent(hWnd);
        else
            hWndCenter = GetWindow(hWnd, GW_OWNER);
    }

    /* get coordinates of the window relative to its parent */
    GetWindowRect(hWnd, &rcDlg);
    if (!(dwStyle & WS_CHILD))
    {
        /* don't center against invisible or minimized windows */
        if (hWndCenter)
        {
            DWORD dwStyleCenter = GetWindowLongPtr(hWndCenter, GWL_STYLE);
            if (!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
                hWndCenter = NULL;
        }

        /* center within screen coordinates */
        if (hWndCenter)
        {
            hMonitor = MonitorFromWindow(hWndCenter, MONITOR_DEFAULTTONEAREST);
        }
        else
        {
            hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        }
        
        minfo.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(hMonitor, &minfo);
        
        rcArea = minfo.rcWork;

        if (!hWndCenter)
            rcCenter = rcArea;
        else
            GetWindowRect(hWndCenter, &rcCenter);
    }
    else
    {
        /* center within parent client coordinates */
        hWndParent = GetParent(hWnd);

        GetClientRect(hWndParent, &rcArea);
        GetClientRect(hWndCenter, &rcCenter);
        MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
    }

    DlgWidth = rcDlg.right - rcDlg.left;
    DlgHeight = rcDlg.bottom - rcDlg.top;

    /* find dialog's upper left based on rcCenter */
    xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    /* if the dialog is outside the screen, move it inside */
    if (xLeft + DlgWidth > rcArea.right)
        xLeft = rcArea.right - DlgWidth;
    if (xLeft < rcArea.left)
        xLeft = rcArea.left;

    if (yTop + DlgHeight > rcArea.bottom)
        yTop = rcArea.bottom - DlgHeight;
    if (yTop < rcArea.top)
        yTop = rcArea.top;

    /* map screen coordinates to child coordinates */
    return SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

VOID
ChangeByteOrder(PCHAR pString, USHORT StrSize)
{
    USHORT pos, index;
    char temp;

    for (pos = 0; pos < StrSize; pos += 2)
    {
        temp = pString[pos];
        pString[pos] = pString[pos + 1];
        pString[pos + 1] = temp;
    }

    pString[pos] = '\0';

    for (index = pos - 1; index > 0 && pString[index] == ' '; --index)
        pString[index] = '\0';
}

BOOL
GetCurrentPath(LPWSTR lpszPath, SIZE_T PathLen)
{
    SIZE_T Index;

    if (!GetModuleFileName(hInst, lpszPath, PathLen))
        return FALSE;

    for (Index = SafeStrLen(lpszPath); Index > 0; Index--)
    {
        if (lpszPath[Index] == L'\\')
        {
            lpszPath[Index + 1] = 0;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IsWin64System(VOID)
{
#ifdef _M_IX86
    typedef BOOL (WINAPI *IW64P)(HANDLE, PBOOL);

    IW64P IsWow64Process;
    HINSTANCE hDLL;
    BOOL Result;

    hDLL = LoadLibrary(L"KERNEL32.DLL");
    if (hDLL)
    {
        IsWow64Process = (IW64P)GetProcAddress(hDLL, "IsWow64Process");
        if (IsWow64Process)
        {
            if (IsWow64Process(GetCurrentProcess(), &Result))
            {
                FreeLibrary(hDLL);
                return Result;
            }
        }
    }

    FreeLibrary(hDLL);
    return FALSE;
#else
    return TRUE;
#endif
}

INT
SafeStrLen(LPCWSTR lpString)
{
    INT Ret = 0;

    __try
    {
        Ret = wcslen(lpString);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return Ret;
}

LPWSTR
SafeStrCpyN(LPWSTR lpString1, LPCTSTR lpString2, INT iMaxLength)
{
    LPWSTR d = lpString1;
    LPCTSTR s = lpString2;
    UINT count = iMaxLength;
    LPWSTR Ret = NULL;

    __try
    {
        while ((count > 1) && *s)
        {
            --count;
            *d++ = *s++;
        }

        if (count)
            *d = 0;

        Ret = lpString1;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return Ret;
}

INT
SafeStrCmp(LPCTSTR lpString1, LPCTSTR lpString2)
{
    INT Result;

    if (lpString1 == lpString2)
        return 0;
    if (lpString1 == NULL)
        return -1;
    if (lpString2 == NULL)
        return 1;

    Result = CompareString(GetThreadLocale(), 0, lpString1, -1, lpString2, -1);
    if (Result)
        Result -= 2;

    return Result;
}

BOOL
IsUserAdmin(VOID)
{
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup = NULL;
    BOOL bIsAdmin;

    bIsAdmin = AllocateAndInitializeSid(&NtAuthority,
                                        2,
                                        SECURITY_BUILTIN_DOMAIN_RID,
                                        DOMAIN_ALIAS_RID_ADMINS,
                                        0, 0, 0, 0, 0, 0,
                                        &AdministratorsGroup);
    if (bIsAdmin)
    {
        if (!CheckTokenMembership(NULL, AdministratorsGroup, &bIsAdmin))
        {
            bIsAdmin = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }

    return bIsAdmin;
}

static SIZE_T
MyPow(SIZE_T x, SIZE_T y)
{
    SIZE_T i, r = 1;

    for (i = 0; i < y; ++i)
        r = r * x;
    return r;
}

SIZE_T
StrToHex(LPWSTR lpszStr, SIZE_T StrLen)
{
    SIZE_T dwVal, dwRes = 0;
    SIZE_T Index = 0;
    SIZE_T Pos = SafeStrLen(lpszStr) - 1;
    INT len;

    if (!lpszStr || Pos == 0)
        return 0;

    _wcsupr_s(lpszStr, StrLen);

    if (lpszStr[0] == L'0' && lpszStr[1] == L'X')
    {
        Index = 2;
        Pos -= 2;
    }

    for (len = SafeStrLen(lpszStr); Index < (SIZE_T)len; ++Index)
    {
        switch (lpszStr[Index])
        {
            case L'0': dwVal = 0x0; break;
            case L'1': dwVal = 0x1; break;
            case L'2': dwVal = 0x2; break;
            case L'3': dwVal = 0x3; break;
            case L'4': dwVal = 0x4; break;
            case L'5': dwVal = 0x5; break;
            case L'6': dwVal = 0x6; break;
            case L'7': dwVal = 0x7; break;
            case L'8': dwVal = 0x8; break;
            case L'9': dwVal = 0x9; break;
            case L'A': dwVal = 0xA; break;
            case L'B': dwVal = 0xB; break;
            case L'C': dwVal = 0xC; break;
            case L'D': dwVal = 0xD; break;
            case L'E': dwVal = 0xE; break;
            case L'F': dwVal = 0xF; break;
            default: dwVal = 0; break;
        }
        dwRes += (dwVal * MyPow(16, Pos));
        Pos -= 1;
    }
    return dwRes;
}

BOOL
IsWindows2000(VOID)
{
    DWORD dwVersion = GetVersion();

    if (LOBYTE(LOWORD(dwVersion)) == 5 &&
        HIBYTE(LOWORD(dwVersion)) == 0)
        return TRUE;
    return FALSE;
}

BOOL
GetStringFromRegistry(BOOL Is64KeyRequired,
                      HKEY hRootKey,
                      LPWSTR lpszPath,
                      LPWSTR lpszKeyName,
                      LPWSTR lpszValue,
                      DWORD dwSize)
{
    REGSAM samDesired = KEY_READ;
    HKEY hKey;

    /* Check Win2000 (KEY_WOW64_64KEY - not supported) */
    if (!IsWindows2000() && Is64KeyRequired)
    {
        samDesired |= KEY_WOW64_64KEY;
    }

    if (RegOpenKeyEx(hRootKey, lpszPath, 0, samDesired, &hKey) ==
        ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey,
                            lpszKeyName,
                            NULL, NULL,
                            (LPBYTE)lpszValue,
                            &dwSize) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return TRUE;
        }
        RegCloseKey(hKey);
    }

    return FALSE;
}

BOOL
GetBinaryFromRegistry(HKEY hRootKey,
                      LPWSTR lpszPath,
                      LPWSTR lpszKeyName,
                      LPBYTE lpdwValue,
                      DWORD dwSize)
{
    REGSAM samDesired = KEY_READ;
    DWORD dwType = REG_BINARY;
    HKEY hKey;

    /* Check Win2000 (KEY_WOW64_64KEY - not supported) */
    if (!IsWindows2000())
    {
        samDesired |= KEY_WOW64_64KEY;
    }

    if (RegOpenKeyEx(hRootKey, lpszPath, 0, samDesired, &hKey) ==
        ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey,
                            lpszKeyName,
                            NULL,
                            &dwType,
                            lpdwValue,
                            &dwSize) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return TRUE;
        }

        RegCloseKey(hKey);
    }

    if (lpdwValue) *lpdwValue = 0;

    return FALSE;
}

INT
AddIconToImageList(HINSTANCE hInst, HIMAGELIST hImageList, UINT IconIndex)
{
    HICON hIcon = NULL;
    INT Index;

    hIcon = (HICON)LoadImage(hInst,
                             MAKEINTRESOURCE(IconIndex),
                             IMAGE_ICON,
                             24,
                             24,
                             LR_CREATEDIBSECTION);

    if (!hIcon) return -1;

    Index = ImageList_AddIcon(hImageList, hIcon);
    DestroyIcon(hIcon);
    return Index;
}

BOOL
KillProcess(DWORD pid, BOOL KillTree)
{
    HANDLE hProcess;

    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess)
    {
        if (!TerminateProcess(hProcess, KillTree ? 0 : 1))
        {
            DebugTrace(L"TerminateProcess() failed. Error code = %d",
                       GetLastError());
            CloseHandle(hProcess);

            return FALSE;
        }
        CloseHandle(hProcess);

        return TRUE;
    }

    return FALSE;
}

INT
LoadMUIStringF(HINSTANCE hLangInst, UINT ResID, LPWSTR Buffer, INT BufLen)
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

VOID
ChopSpaces(LPWSTR s, SIZE_T size)
{
    WCHAR szNew[MAX_STR_LEN] = {0};
    SIZE_T i, j;
    INT len;

    for (i = 0, j = 0, len = SafeStrLen(s); i < (SIZE_T)len; ++i)
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

VOID
ConvertSecondsToString(HINSTANCE hLangInst,
                       LONGLONG Seconds,
                       LPWSTR lpszString,
                       SIZE_T Size)
{
    LONGLONG Days  =  (Seconds / 86400);
    LONGLONG Hours = ((Seconds % 86400) / 3600);
    LONGLONG Mins  = ((Seconds % 86400) % 3600) / 60;
    LONGLONG Secs  = ((Seconds % 86400) % 3600) % 60;
    WCHAR szFormat[MAX_STR_LEN];

    LoadMUIStringF(hLangInst, IDS_SYS_UPTIME_FORMAT, szFormat, MAX_STR_LEN);

    StringCbPrintf(lpszString, Size, szFormat, Days, Hours, Mins, Secs);
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
GetFileExt(LPWSTR lpFileName, LPWSTR lpExt, SIZE_T ExtSize)
{
    INT i, j = 0, len = wcslen(lpFileName);
    INT dot_pos = -1;
    WCHAR szExt[MAX_PATH] = {0};

    if (len < 2)
        return FALSE;

    for (i = len; i > 0; i--)
    {
        if (lpFileName[i] == L'.')
        {
            dot_pos = i + 1;
            break;
        }
    }

    if (dot_pos <= 0)
        return FALSE;

    for (i = dot_pos; i < len; i++, j++)
    {
        szExt[j] = lpFileName[i];
    }

    StringCbPrintf(lpExt, ExtSize, szExt);

    return TRUE;
}

WCHAR*
EscapePercentSymbols(WCHAR *pIn)
{
    INT i, j, len = SafeStrLen(pIn);
    INT count = 0;
    WCHAR *buf;

    if (len == 0) return NULL;

    /* Подсчитываем количество знаков процента в строке */
    for (i = 0; i < len; i++)
    {
        if (pIn[i] == L'%')
            count++;
    }

    /* Вычисляем количество памяти, которое необходимо для новой строки
       и выделяем память */
    buf = (WCHAR*)Alloc((len + count + 1) * sizeof(WCHAR));
    if (!buf) return NULL;

    /* Копируем строку посимвольно, добавляя после каждого знака
       процента еще один знак процента */
    for (i = 0, j = 0; i < len; i++, j++)
    {
        buf[j] = pIn[i];
        if (pIn[i] == L'%')
            buf[++j] = L'%';
    }

    buf[j] = 0;

    return buf;
}
