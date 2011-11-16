/*
 * PROJECT:         Aspia
 * FILE:            aspia/dev_report.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"
#include <wininet.h>
#include <urlmon.h>


typedef VOID (CALLBACK *DEVICESENUMPROC)(HWND hList, INT IconIndex, LPWSTR lpName, LPTSTR lpId);

HIMAGELIST hDevImageList = NULL;

static HWND hTopText = NULL;
static HWND hDataList = NULL;
static HWND hCheckBox = NULL;
static HWND hSendBtn = NULL;
static HWND hCloseBtn = NULL;


VOID
SendFileToServer(LPWSTR lpServer, LPWSTR lpFilePath, char *lpFileName)
{
    char hdrs[] = "Content-Type: multipart/form-data, boundary=Jfbvjwj3489078yuyetu";
    static LPSTR accept[2] = { "*/*", NULL };
    HINTERNET hOpenHandle;
    HANDLE hFile;
    DWORD dwFileSize, dwRead;
    LPBYTE pBuf = NULL;
    LPBYTE pDataStart = NULL;
    DWORD dwDataToSend = 0;
    long x;

    char szFnamePrefix[] = "--Jfbvjwj3489078yuyetu\r\nContent-Disposition: form-data; name=\"fname\"\r\n\r\n";
    char szDataPrefix[] = "\r\n--Jfbvjwj3489078yuyetu\r\nContent-Disposition: form-data; name=\"data\"; filename=\"report.ini\"\r\nContent-Type: application/octet-stream\r\n\r\n";
    char szDataPostfix[] = "\r\n--Jfbvjwj3489078yuyetu--";

    hFile = CreateFile(lpFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        return;
    }

    pBuf = (LPBYTE)Alloc(dwFileSize + 2048);
    if (!pBuf)
    {
        CloseHandle(hFile);
        return;
    }

    pDataStart = pBuf;
    x = strlen(szFnamePrefix);
    lstrcpynA((char*)pDataStart, szFnamePrefix, x + 1);
    pDataStart += x;

    x = lstrlenA(lpFileName);
    lstrcpynA((char*)pDataStart, lpFileName, x + 1);
    pDataStart += x;

    x = strlen(szDataPrefix);
    lstrcpynA((char*)pDataStart, szDataPrefix, x + 1);
    pDataStart += x;

    if (!ReadFile(hFile, pDataStart, dwFileSize, &dwRead, NULL))
    {
        Free(pBuf);
        CloseHandle(hFile);
        return;
    }

    pDataStart += dwRead;

    x = strlen(szDataPostfix);
    lstrcpynA((char*)pDataStart, szDataPostfix, x + 1);
    pDataStart += x;

    dwDataToSend = pDataStart - pBuf;

    hOpenHandle = InternetOpen(L"Aspia 0.2.5", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

    if (hOpenHandle)
    {
        HINTERNET hConnectHandle =
            InternetConnect(hOpenHandle, lpServer,
                            INTERNET_DEFAULT_HTTP_PORT,
                            NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);

        if (hConnectHandle)
        {
            HANDLE hResourceHandle =
                HttpOpenRequestA(hConnectHandle, "POST", "/hw_report.php", NULL, NULL, (LPCSTR*)accept, 0, 1);

            if (hResourceHandle)
            {
                HttpSendRequestA(hResourceHandle, hdrs, strlen(hdrs), pBuf, dwDataToSend);
                InternetCloseHandle(hResourceHandle);
            }
            InternetCloseHandle(hConnectHandle);
        }
        InternetCloseHandle(hOpenHandle);
    }

    Free(pBuf);
    CloseHandle(hFile);
}

INT
AddImageListIcon(HIMAGELIST hImgList, UINT IconID)
{
    HICON hIcon;
    INT Index;

    hIcon = (HICON)LoadImage(hIconsInst,
                             MAKEINTRESOURCE(IconID),
                             IMAGE_ICON,
                             ParamsInfo.SxSmIcon,
                             ParamsInfo.SySmIcon,
                             LR_CREATEDIBSECTION);

    if (!hIcon) return -1;

    Index = ImageList_AddIcon(hImgList, hIcon);
    DestroyIcon(hIcon);
    return Index;
}

BOOL
EnumUnknownDevices(HWND hList, DEVICESENUMPROC lpEnumProc)
{
    SP_DEVINFO_DATA DeviceInfoData = {0};
    WCHAR DeviceID[MAX_STR_LEN], DeviceName[MAX_STR_LEN];
    WCHAR szDeviceID[MAX_STR_LEN], szVendorID[MAX_STR_LEN];
    WCHAR szDeviceName[MAX_STR_LEN];
    WCHAR szPCIIniPath[MAX_PATH], szUSBIniPath[MAX_PATH], szMonIniPath[MAX_PATH];
    HDEVINFO hDevInfo;
    INT DeviceIndex = 0;
    BOOL Result = FALSE;

    DebugStartReceiving();

    StringCbPrintf(szPCIIniPath, sizeof(szPCIIniPath),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
                   L"pci_dev.ini");
    StringCbPrintf(szUSBIniPath, sizeof(szUSBIniPath),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
                   L"usb_dev.ini");
    StringCbPrintf(szMonIniPath, sizeof(szMonIniPath),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
                   L"mon_dev.ini");

    hDevInfo = SetupDiGetClassDevs(NULL, NULL, NULL,
                                   DIGCF_ALLCLASSES | DIGCF_PRESENT);

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(hDevInfo,
                                 DeviceIndex,
                                 &DeviceInfoData))
    {
        ++DeviceIndex;

        if (!SetupDiGetDeviceInstanceId(hDevInfo,
                                        &DeviceInfoData,
                                        DeviceID,
                                        MAX_PATH,
                                        NULL))
        {
            continue;
        }

        /* get the device's friendly name */
        if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                              &DeviceInfoData,
                                              SPDRP_FRIENDLYNAME,
                                              0,
                                              (BYTE*)DeviceName,
                                              MAX_STR_LEN,
                                              NULL))
        {
            /* if the friendly name fails, try the description instead */
            if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_DEVICEDESC,
                                                  0,
                                                  (BYTE*)DeviceName,
                                                  MAX_STR_LEN,
                                                  NULL))
            {
                continue;
            }
        }

        _wcslwr_s(DeviceID, MAX_STR_LEN);

        if (IsPCIDevice(DeviceID))
        {
            if (!GetPCIVendorID(DeviceID, szVendorID, sizeof(szVendorID)) ||
                !GetPCIDeviceID(DeviceID, szDeviceID, sizeof(szDeviceID)))
            {
                continue;
            }

            StringCbCat(szVendorID, sizeof(szVendorID), L"-");
            StringCbCat(szVendorID, sizeof(szVendorID), szDeviceID);
            GetPrivateProfileString(L"devices", szVendorID, L"\0",
                                    szDeviceName, MAX_STR_LEN, szPCIIniPath);
            if (szDeviceName[0] == L'\0')
            {
                if (lpEnumProc) lpEnumProc(hList, 2, DeviceName, szVendorID);
                Result = TRUE;
            }
        }
        else if (IsUSBDevice(DeviceID))
        {
            if (!GetUSBVendorID(DeviceID, szVendorID, sizeof(szVendorID)) ||
                !GetUSBDeviceID(DeviceID, szDeviceID, sizeof(szDeviceID)))
            {
                continue;
            }

            StringCbCat(szVendorID, sizeof(szVendorID), L"-");
            StringCbCat(szVendorID, sizeof(szVendorID), szDeviceID);
            GetPrivateProfileString(L"devices", szVendorID, L"\0",
                                    szDeviceName, MAX_STR_LEN, szUSBIniPath);
            if (szDeviceName[0] == L'\0')
            {
                if (lpEnumProc) lpEnumProc(hList, 1, DeviceName, szVendorID);
                Result = TRUE;
            }
        }
        else if (IsMonitorDevice(DeviceID))
        {
            if (!GetMonitorID(DeviceID, szDeviceID, sizeof(szDeviceID)))
                continue;

            GetPrivateProfileString(L"devices", szDeviceID, L"\0",
                                    szDeviceName, MAX_STR_LEN, szMonIniPath);
            if (szDeviceName[0] == L'\0')
            {
                if (lpEnumProc) lpEnumProc(hList, 0, DeviceName, szDeviceID);
                Result = TRUE;
            }
        }
    }

    if (hDevInfo)
    {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    return Result;
}

VOID CALLBACK
DevicesEnumProc(HWND hList, INT IconIndex, LPWSTR lpName, LPWSTR lpId)
{
    SIZE_T Size = (SafeStrLen(lpId) + 1) * sizeof(WCHAR);
    WCHAR *pId = Alloc(Size);

    if (!pId) return;

    StringCbCopy(pId, Size, lpId);
    AddItem(hList, IconIndex, lpName, (LPARAM)pId);
}

VOID
FreeItems(HWND hList)
{
    INT Count = ListView_GetItemCount(hList) - 1;
    WCHAR *pDevId;

    while (Count >= 0)
    {
        pDevId = (WCHAR*)ListViewGetlParam(hList, Count);
        if (pDevId) Free(pDevId);
        --Count;
    }
}

VOID
SaveDevReportFile(HWND hList)
{
    INT Count = ListView_GetItemCount(hList) - 1;
    WCHAR szPath[MAX_PATH], szName[MAX_STR_LEN], *pDevId;
    WCHAR szResult[MAX_STR_LEN * 2];
    char *lpSend;
    LARGE_INTEGER FileSize, MoveTo, NewPos;
    DWORD dwBytesWritten;
    LVITEM Item = {0};
    HANDLE hDB;
    INT len, buf_len;

    if (!GetTempPath(MAX_PATH, szPath))
        return;

    StringCbCat(szPath, sizeof(szPath), L"report.ini");

    DeleteFile(szPath);

    hDB = CreateFile(szPath,
                     GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     NULL);
    if (hDB == INVALID_HANDLE_VALUE)
        return;

    while (Count >= 0)
    {
        ZeroMemory(&Item, sizeof(LVITEM));

        Item.mask = LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;
        Item.cchTextMax = MAX_STR_LEN;
        Item.pszText = szName;
        Item.iItem = Count;

        if (ListView_GetItem(hList, &Item))
        {
            pDevId = (WCHAR*)Item.lParam;

            if (pDevId && szName[0] != L'\0')
            {
                MoveTo.QuadPart = 0;
                if (!SetFilePointerEx(hDB, MoveTo, &NewPos, FILE_END))
                    break;

                if (!GetFileSizeEx(hDB, &FileSize))
                    break;

                LockFile(hDB, (DWORD_PTR)NewPos.QuadPart, 0,
                         (DWORD_PTR)FileSize.QuadPart, 0);

                switch (Item.iImage)
                {
                    case 0:
                        StringCbPrintf(szResult, sizeof(szResult),
                                       L"MON::%s::%s\r\n",
                                       pDevId, szName);
                        break;
                    case 1:
                        StringCbPrintf(szResult, sizeof(szResult),
                                       L"USB::%s::%s\r\n",
                                       pDevId, szName);
                        break;
                    case 2:
                        StringCbPrintf(szResult, sizeof(szResult),
                                       L"PCI::%s::%s\r\n",
                                       pDevId, szName);
                        break;
                }

                buf_len = WideCharToMultiByte(CP_UTF8, 0,
                                              szResult, wcslen(szResult),
                                              NULL, 0, 0, 0);
                lpSend = Alloc(buf_len);
                if (lpSend)
                {
                    len = WideCharToMultiByte(CP_UTF8, 0,
                                              szResult, wcslen(szResult),
                                              lpSend, buf_len, 0, 0);

                    WriteFile(hDB, lpSend, len,
                              &dwBytesWritten, NULL);
                    Free(lpSend);
                }

                UnlockFile(hDB, (DWORD_PTR)NewPos.QuadPart, 0,
                           (DWORD_PTR)FileSize.QuadPart, 0);
            }
        }

        --Count;
    }
    CloseHandle(hDB);

    SendFileToServer(L"aspia.ru", szPath, "report.ini");

    DeleteFile(szPath);
}

VOID
DevReportWindowOnSize(LPARAM lParam)
{
    HDWP hdwp = BeginDeferWindowPos(5);

#define TOP_TEXT_HEIGHT          50
#define CHECKBOX_HEIGHT          20
#define BUTTON_WIDTH             100
#define BUTTON_HEIGHT            25

#define ITEMS_DIVIDER 6

    /*
     * HIWORD(lParam) - Height of main window
     * LOWORD(lParam) - Width of main window
     */

    /* Size top text */
    DeferWindowPos(hdwp,
                   hTopText,
                   0,
                   ITEMS_DIVIDER,
                   ITEMS_DIVIDER,
                   LOWORD(lParam) - (ITEMS_DIVIDER * 2),
                   TOP_TEXT_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size ListView */
    DeferWindowPos(hdwp,
                   hDataList,
                   0,
                   ITEMS_DIVIDER,
                   TOP_TEXT_HEIGHT + (ITEMS_DIVIDER * 2),
                   LOWORD(lParam) - (ITEMS_DIVIDER * 2),
                   HIWORD(lParam) - (ITEMS_DIVIDER * 4) - TOP_TEXT_HEIGHT - CHECKBOX_HEIGHT - BUTTON_HEIGHT - 10,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size checkbox */
    DeferWindowPos(hdwp,
                   hCheckBox,
                   0,
                   ITEMS_DIVIDER * 2,
                   HIWORD(lParam) - CHECKBOX_HEIGHT - BUTTON_HEIGHT - (ITEMS_DIVIDER) - 10,
                   LOWORD(lParam) - (ITEMS_DIVIDER * 3),
                   CHECKBOX_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "Save" button */
    DeferWindowPos(hdwp,
                   hSendBtn,
                   0,
                   LOWORD(lParam) - (BUTTON_WIDTH * 2) - (ITEMS_DIVIDER * 2),
                   HIWORD(lParam) - BUTTON_HEIGHT - ITEMS_DIVIDER,
                   BUTTON_WIDTH,
                   BUTTON_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    /* Size "Close" button */
    DeferWindowPos(hdwp,
                   hCloseBtn,
                   0,
                   LOWORD(lParam) - BUTTON_WIDTH - ITEMS_DIVIDER,
                   HIWORD(lParam) - BUTTON_HEIGHT - ITEMS_DIVIDER,
                   BUTTON_WIDTH,
                   BUTTON_HEIGHT,
                   SWP_NOZORDER|SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);
}

VOID
DevReportInitControls(HWND hwnd)
{
    WCHAR szText[MAX_STR_LEN];

    hTopText = CreateWindow(L"STATIC", L"",
                            WS_CHILD | WS_VISIBLE,
                            0, 0, 0, 0,
                            hwnd, 0, hInstance, NULL);
    if (!hTopText)
    {
        DebugTrace(L"Unable to create window!");
        return;
    }
    LoadMUIString(IDS_DEV_REPORT_TEXT, szText, MAX_STR_LEN);
    SetWindowText(hTopText, szText);
    SendMessage(hTopText, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

    /* Initialize ListView */
    hDataList = CreateWindowEx(WS_EX_CLIENTEDGE,
                               WC_LISTVIEW,
                               L"",
                               WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT |
                               LVS_SORTASCENDING | LVS_NOCOLUMNHEADER,
                               0, 0, 0, 0,
                               hwnd,
                               NULL,
                               hInstance,
                               NULL);

    if (!hDataList)
    {
        DebugTrace(L"Unable to create TreeView control!");
        return;
    }

    /* Try to set theme for ListView */
    IntSetWindowTheme(hDataList);

    hDevImageList =
       ImageList_Create(ParamsInfo.SxSmIcon,
                        ParamsInfo.SySmIcon,
                        ILC_MASK | ParamsInfo.SysColorDepth,
                        1, 1);

    AddImageListIcon(hDevImageList, IDI_MONITOR); /* Icon for monitors */
    AddImageListIcon(hDevImageList, IDI_USB_DEV); /* Icon for USB */
    AddImageListIcon(hDevImageList, IDI_HW); /* Icon for PCI */

    ListView_SetImageList(hDataList, hDevImageList, LVSIL_SMALL);

    AddColumn(hDataList, 0, 400, L"");

    EnumUnknownDevices(hDataList, DevicesEnumProc);

    /* Initialize checkbox */
    hCheckBox = CreateWindow(L"Button",
                             L"",
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                             0, 0, 0, 0,
                             hwnd, 0, hInstance, NULL);

    if (!hCheckBox)
    {
        DebugTrace(L"Unable to create CheckBox control!");
        return;
    }

    /* Set checkbox text and font */
    SendMessage(hCheckBox, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
    LoadMUIString(IDS_NEVER_SEND_REPORT, szText, MAX_STR_LEN);
    SetWindowText(hCheckBox, szText);

    /* Set checkbox state */
    SendMessage(hCheckBox, BM_SETCHECK,
                (SettingsInfo.SendDevReport ? BST_UNCHECKED : BST_CHECKED), 0);

    /* "Send" button */
    hSendBtn = CreateWindow(L"Button", L"",
                            WS_CHILD | WS_VISIBLE,
                            0, 0, 0, 0,
                            hwnd, 0, hInstance, NULL);
    if (!hSendBtn)
    {
        DebugTrace(L"Unable to create button!");
        return;
    }
    LoadMUIString(IDS_SEND_BTN, szText, MAX_STR_LEN);
    SetWindowText(hSendBtn, szText);
    SendMessage(hSendBtn, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

    /* "Close" button */
    hCloseBtn = CreateWindow(L"Button", L"",
                             WS_CHILD | WS_VISIBLE,
                             0, 0, 0, 0,
                             hwnd, 0, hInstance, NULL);
    if (!hCloseBtn)
    {
        DebugTrace(L"Unable to create button!");
        return;
    }
    LoadMUIString(IDS_CLOSE_BTN, szText, MAX_STR_LEN);
    SetWindowText(hCloseBtn, szText);
    SendMessage(hCloseBtn, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
}

VOID
DevReportWindowOnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    if (HIWORD(wParam) == BN_CLICKED)
    {
        if (lParam == (LPARAM)hSendBtn)
        {
            SaveDevReportFile(hDataList);
            SettingsInfo.SendDevReport = TRUE;
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        else if (lParam == (LPARAM)hCloseBtn)
        {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
    }
}

LRESULT CALLBACK
DevReportWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_CREATE:
            DevReportInitControls(hwnd);
            break;

        case WM_COMMAND:
            DevReportWindowOnCommand(hwnd, wParam, lParam);
            break;

        case WM_SIZE:
            DevReportWindowOnSize(lParam);
            break;

        case WM_SIZING:
        {
            LPRECT pRect = (LPRECT)lParam;

            if (pRect->right - pRect->left < 400)
                pRect->right = pRect->left + 400;

            if (pRect->bottom - pRect->top < 300)
                pRect->bottom = pRect->top + 300;
        }
        break;

        case WM_DESTROY:
        {
            SettingsInfo.SendDevReport =
                (SendMessage(hCheckBox, BM_GETCHECK, 0, 0) == BST_CHECKED) ? FALSE : TRUE;

            ImageList_Destroy(hDevImageList);
            FreeItems(hDataList);

            PostQuitMessage(0);
        }
        return 0;
    }

    return DefWindowProc(hwnd, Msg, wParam, lParam);
}

VOID
DetectUnknownDevices(VOID)
{
    WNDCLASSEX WndClass = {0};
    WCHAR szWindowClass[] = L"ASPIAISDEVREPORT";
    WCHAR szWindowName[MAX_STR_LEN];
    HWND hWnd;
    MSG Msg;

    if (SettingsInfo.SendDevReport)
        return;

    /* Create the window */
    WndClass.cbSize        = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = szWindowClass;
    WndClass.lpfnWndProc   = DevReportWindowProc;
    WndClass.hInstance     = hInstance;
    WndClass.style         = CS_HREDRAW | CS_VREDRAW;
    WndClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.lpszMenuName  = NULL;

    if (RegisterClassEx(&WndClass) == (ATOM)0)
        return;

    LoadMUIString(IDS_DEV_REPORT_TITLE, szWindowName, MAX_STR_LEN);

    /* Создаем главное окно программы */
    hWnd = CreateWindowEx(WS_EX_WINDOWEDGE,
                          szWindowClass,
                          szWindowName,
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                          22, 16, 430, 490,
                          NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        UnregisterClass(szWindowClass, hInstance);
        return;
    }

	CenterWindow(hWnd, NULL);
    /* Show it */
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    /* Message Loop */
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    UnregisterClass(szWindowClass, hInstance);
}
