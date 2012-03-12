/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/systray.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"


#define IDT_SENSORS_UPDATE_TIMER 1326
#define MAX_HDD_COUNT 32
#define MAINWND_ICON_ID 0
#define CPU_ICON_ID 1


typedef struct _HDD_DRIVES_
{
    UINT IconID;
    HANDLE hDrive;
    WCHAR szName[MAX_STR_LEN];
    struct _HDD_DRIVES_ *Next;
} HDD_DRIVES, *PHDD_DRIVES;

static INT HddIconsCount = 0;
static PHDD_DRIVES HddDrivesInfo = NULL;
static WCHAR szIniPath[MAX_PATH] = {0};
BOOL IsMainWindowHiden = FALSE;


HICON
CreateTrayIcon(LPWSTR szText, COLORREF crBackground, COLORREF crFont)
{
    HDC hdc, hdcsrc;
    HBITMAP hBitmap, hBmpNew, hBmpOld;
    RECT rect = {0};
    HFONT hFontOld;
    ICONINFO IconInfo;
    HICON hIcon = NULL;

    hdcsrc = GetDC(NULL);
    hdc = CreateCompatibleDC(hdcsrc);
    hBitmap = CreateCompatibleBitmap(hdcsrc,
                                     ParamsInfo.SxSmIcon,
                                     ParamsInfo.SySmIcon);
    ReleaseDC(NULL, hdcsrc);

    if (hdc && hBitmap)
    {
        hBmpNew = (HBITMAP)CreateBitmap(ParamsInfo.SxSmIcon,
                                        ParamsInfo.SySmIcon,
                                        1, 1, NULL);
        if (hBmpNew)
        {
            hBmpOld = (HBITMAP)SelectObject(hdc, hBitmap);
            rect.right = ParamsInfo.SxSmIcon;
            rect.bottom = ParamsInfo.SySmIcon;

            SetBkColor(hdc, crBackground);
            SetTextColor(hdc, crFont);

            ExtTextOut(hdc, rect.left, rect.top, ETO_OPAQUE, &rect, L"", 0, NULL);
            hFontOld = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
            DrawText(hdc, szText, 2, &rect, DT_SINGLELINE|DT_CENTER|DT_VCENTER);

            SelectObject(hdc, hBmpNew);
            PatBlt(hdc, 0, 0,
                   ParamsInfo.SxSmIcon,
                   ParamsInfo.SySmIcon,
                   BLACKNESS);
            SelectObject(hdc, hBmpOld);
            SelectObject(hdc, hFontOld);

            IconInfo.hbmColor = hBitmap;
            IconInfo.hbmMask = hBmpNew;
            IconInfo.fIcon = TRUE;

            hIcon = CreateIconIndirect(&IconInfo);

            DeleteObject(hBmpNew);
            DeleteObject(hBmpOld);
        }
    }

    DeleteDC(hdc);
    DeleteObject(hBitmap);

    return hIcon;
}

VOID
AddTrayIcon(HWND hWnd, UINT uID, UINT Msg, HICON hIcon, LPWSTR lpszToolTip)
{
    NOTIFYICONDATA tnid = {0};

    DebugTrace(L"hwnd = 0x%x, uid = %d, msg = %d, hicon = 0x%x, tooltip = '%s'",
               hWnd, uID, Msg, hIcon, lpszToolTip);

    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd = hWnd;
    tnid.uID = uID;
    tnid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    tnid.uCallbackMessage = Msg;
    tnid.hIcon = hIcon;

    SafeStrCpyN(tnid.szTip, lpszToolTip, sizeof(tnid.szTip));

    Shell_NotifyIcon(NIM_ADD, &tnid);
}

VOID
DeleteTrayIcon(HWND hWnd, UINT uID)
{
    NOTIFYICONDATA tnid = {0};

    DebugTrace(L"hwnd = 0x%x, uid = %d", hWnd, uID);

    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd = hWnd;
    tnid.uID = uID;

    Shell_NotifyIcon(NIM_DELETE, &tnid);
}

VOID
UpdateTrayIcon(HWND hWnd, UINT uID, UINT Msg, HICON hIcon, LPWSTR lpszToolTip)
{
    NOTIFYICONDATA tnid = {0};

    DebugTrace(L"hwnd = 0x%x, uid = %d, msg = %d, hicon = 0x%x, tooltip = '%s'",
               hWnd, uID, Msg, hIcon, lpszToolTip);

    tnid.cbSize = sizeof(NOTIFYICONDATA);
    tnid.hWnd = hWnd;
    tnid.uID = uID;
    tnid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    tnid.uCallbackMessage = Msg;
    tnid.hIcon = hIcon;

    SafeStrCpyN(tnid.szTip, lpszToolTip, sizeof(tnid.szTip));

    Shell_NotifyIcon(NIM_MODIFY, &tnid);
}

INT
GetCPUTemperature(VOID)
{
    WCHAR szVendor[MAX_STR_LEN];

    GetCPUVendor(szVendor, sizeof(szVendor));
    if (wcscmp(szVendor, L"GenuineIntel") == 0)
    {
        UINT64 Value;
        UINT Temp;

        /* Only for first core */
        ReadMsr(IA32_THERM_STATUS, 0, &Value);

        Temp = ((Value >> 16) & 0xFF);
        if (Temp == 0) return 0;

        return GetTjmaxTemperature(0) - Temp;
    }
    else if (wcscmp(szVendor, L"AuthenticAMD") == 0)
    {
        /*DWORD Value;
        ULONG Device;

        Device = GetPciAddress(0, 24, 3);
        Value = ReadPciConfigDword(Device, 0xA4);
        return ((Value >> 21) & 0x7FF) / 8;*/
        return 0;
    }
    else
    {
        return 0;
    }
}

VOID
AddCPUSensor(BOOL IsUpdate)
{
    WCHAR szCpuName[MAX_STR_LEN], szValue[MAX_STR_LEN];
    HICON hIcon;

    if (GetCPUName(szCpuName, sizeof(szCpuName)))
    {
        if (GetPrivateProfileInt(L"sensors", szCpuName, 0, szIniPath) > 0)
        {
            StringCbPrintf(szValue, sizeof(szValue),
                           L"%ld", GetCPUTemperature());
            hIcon = CreateTrayIcon(szValue, SettingsInfo.CpuBackground, SettingsInfo.CpuFontColor);

            if (IsUpdate)
            {
                UpdateTrayIcon(hMainWnd, CPU_ICON_ID, WM_NOTIFYICONMSG_SENSORS, hIcon, szCpuName);
            }
            else
            {
                AddTrayIcon(hMainWnd, CPU_ICON_ID, WM_NOTIFYICONMSG_SENSORS, hIcon, szCpuName);
            }
            DestroyIcon(hIcon);
        }
    }
}

VOID
AddHDDSensor(BOOL IsUpdate)
{
    WCHAR szValue[MAX_STR_LEN];
    PHDD_DRIVES HddDrives;
    IDSECTOR DriveInfo;
    BYTE bIndex;
    HICON hIcon;

    if (!IsUpdate)
    {
        HddDrivesInfo = (PHDD_DRIVES)Alloc(sizeof(HDD_DRIVES));
        if (!HddDrivesInfo) return;
    }

    HddDrives = HddDrivesInfo;

    for (bIndex = 0; bIndex <= MAX_HDD_COUNT; bIndex++)
    {
        if (!IsUpdate)
        {
            HddDrives->hDrive = OpenSmart(bIndex);
            if (HddDrives->hDrive == INVALID_HANDLE_VALUE)
                continue;

            if (!ReadSmartInfo(HddDrives->hDrive, bIndex, &DriveInfo))
                continue;

            ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                            sizeof(DriveInfo.sModelNumber));
            StringCbPrintf(HddDrives->szName, sizeof(HddDrives->szName),
                           L"%S", DriveInfo.sModelNumber);
            ChopSpaces(HddDrives->szName, sizeof(HddDrives->szName));
        }

        if (GetPrivateProfileInt(L"sensors", HddDrives->szName, 0, szIniPath) > 0)
        {
            StringCbPrintf(szValue, sizeof(szValue), L"%ld",
                           GetSmartTemperature(HddDrives->hDrive, bIndex));
            hIcon = CreateTrayIcon(szValue, SettingsInfo.HddBackground, SettingsInfo.HddFontColor);

            if (IsUpdate)
            {
                UpdateTrayIcon(hMainWnd, HddDrives->IconID,
                               WM_NOTIFYICONMSG_SENSORS,
                               hIcon, HddDrives->szName);
            }
            else
            {
                HddDrives->IconID = bIndex + 2;
                AddTrayIcon(hMainWnd, HddDrives->IconID,
                            WM_NOTIFYICONMSG_SENSORS,
                            hIcon, HddDrives->szName);
                ++HddIconsCount;
            }
            DestroyIcon(hIcon);
        }

        if (!IsUpdate)
        {
            HddDrives->Next = (PHDD_DRIVES)Alloc(sizeof(HDD_DRIVES));
        }
        HddDrives = HddDrives->Next;
        if (!HddDrives) break;
    }
}

static VOID CALLBACK
SensorsUpdateProc(HWND hwnd, UINT msg, UINT id, DWORD systime)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(systime);

    AddCPUSensor(TRUE);
    AddHDDSensor(TRUE);
}

VOID
DeleteTraySensors(VOID)
{
    PHDD_DRIVES HddDrives, Temp;

    KillTimer(hMainWnd, IDT_SENSORS_UPDATE_TIMER);

    HddDrives = HddDrivesInfo;

    while (HddIconsCount)
    {
        CloseSmart(HddDrives->hDrive);
        DeleteTrayIcon(hMainWnd, HddDrives->IconID);

        Temp = HddDrives->Next;
        Free(HddDrives);
        HddDrives = Temp;

        --HddIconsCount;
    }

    DeleteTrayIcon(hMainWnd, CPU_ICON_ID);
}

VOID
AddTraySensors(VOID)
{
    if (!GetIniFilePath(szIniPath, MAX_PATH))
        return;

    AddCPUSensor(FALSE);
    AddHDDSensor(FALSE);

    if (SettingsInfo.SensorsRefreshRate > 0 &&
        SettingsInfo.SensorsRefreshRate < MAX_SENSORS_REFRESH_RATE)
    {
        SetTimer(hMainWnd,
                 IDT_SENSORS_UPDATE_TIMER,
                 SettingsInfo.SensorsRefreshRate * 1000,
                 SensorsUpdateProc);
    }
}

VOID
AddMainWindowToTray(VOID)
{
    HICON hIcon;

    hIcon = (HICON)LoadImage(hInstance,
                             MAKEINTRESOURCE(IDI_MAINICON),
                             IMAGE_ICON,
                             ParamsInfo.SxSmIcon,
                             ParamsInfo.SySmIcon,
                             LR_CREATEDIBSECTION);
    if (!hIcon) return;

    AddTrayIcon(hMainWnd, MAINWND_ICON_ID, WM_NOTIFYICONMSG_WINDOW, hIcon, L"Aspia");
    DestroyIcon(hIcon);
}

VOID
HideMainWindowToTray(VOID)
{
    AddMainWindowToTray();
    ShowWindow(hMainWnd, SW_HIDE);

    IsMainWindowHiden = TRUE;
}

VOID
DeleteMainWindowFromTray(VOID)
{
    DeleteTrayIcon(hMainWnd, MAINWND_ICON_ID);
}

VOID
RestoreMainWindowFromTray(VOID)
{
    if (!SettingsInfo.ShowProgIcon)
        DeleteMainWindowFromTray();

    ShowWindow(hMainWnd, SW_SHOW);

    IsMainWindowHiden = FALSE;
}

VOID
UpdateTrayIcons(VOID)
{
    DeleteTraySensors();
    DeleteMainWindowFromTray();

    if (SettingsInfo.ShowSensorIcons)
        AddTraySensors();

    if (SettingsInfo.ShowProgIcon)
        AddMainWindowToTray();
}
