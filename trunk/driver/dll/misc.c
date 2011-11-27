#include <windows.h>
#include <commctrl.h>
#include "driver.h"


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
drv_get_cpu_usage(VOID)
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
drv_get_system_color_depth(VOID)
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
drv_create_screenshot(HWND hwnd)
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
drv_center_window(HWND hWnd, HWND hWndCenter)
{
    DWORD dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
    HMONITOR hMonitor = NULL;
    RECT rcDlg;
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;
    BOOL bResult;
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
        bResult = GetMonitorInfo(hMonitor, &minfo);
        
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
