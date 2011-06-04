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
#define INTERNET_FLAGS (INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_COOKIES)

HIMAGELIST hDevImageList = NULL;


VOID
SendFileToServer(LPWSTR lpServer, LPWSTR lpFilePath, LPWSTR lpFileName)
{
    LPWSTR hdrs = L"Content-Type: multipart/form-data, boundary=Jfbvjwj3489078yuyetu";
    static LPWSTR accept[2] = { L"*/*", NULL };
    HINTERNET hOpenHandle;
    HANDLE hFile;
    DWORD dwFileSize, dwRead;
    LPBYTE pBuf = NULL;
    LPBYTE pDataStart = NULL;
    DWORD dwDataToSend = 0;
    long x;

    LPWSTR szFnamePrefix = L"--Jfbvjwj3489078yuyetu\r\nContent-Disposition: form-data; name=\"fname\"\r\n\r\n";
    LPWSTR szDataPrefix = L"\r\n--Jfbvjwj3489078yuyetu\r\nContent-Disposition: form-data; name=\"data\"; filename=\"report.ini\"\r\nContent-Type: application/octet-stream\r\n\r\n";
    LPWSTR szDataPostfix = L"\r\n--Jfbvjwj3489078yuyetu--";

    hFile = CreateFile(lpFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        return;
    }

    pBuf = (LPBYTE)HeapAlloc(hProcessHeap, 0, dwFileSize + 2048);
    if (!pBuf)
    {
        CloseHandle(hFile);
        return;
    }

    pDataStart = pBuf;
    x = lstrlen(szFnamePrefix);
    lstrcpyn((LPWSTR)pDataStart, szFnamePrefix, x + 1);
    pDataStart += x;

    x = lstrlen(lpFileName);
    lstrcpyn((LPWSTR)pDataStart, lpFileName, x + 1);
    pDataStart += x;

    x = lstrlen(szDataPrefix);
    lstrcpyn((LPWSTR)pDataStart, szDataPrefix, x + 1);
    pDataStart += x;

    if (!ReadFile(hFile, pDataStart, dwFileSize, &dwRead, NULL))
    {
        HeapFree(hProcessHeap, 0, pBuf);
        CloseHandle(hFile);
        return;
    }

    pDataStart += dwRead;

    x = lstrlen(szDataPostfix);
    lstrcpyn((LPWSTR)pDataStart, szDataPostfix, x + 1);
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
                HttpOpenRequest(hConnectHandle, L"POST", L"/hw_report.php", NULL, NULL, (LPCWSTR*)accept, 0, 1);

            if (hResourceHandle)
            {
                HttpSendRequest(hResourceHandle, hdrs, wcslen(hdrs), pBuf, dwDataToSend);
                InternetCloseHandle(hResourceHandle);
            }
            InternetCloseHandle(hConnectHandle);
        }
        InternetCloseHandle(hOpenHandle);
    }

    HeapFree(hProcessHeap, 0, pBuf);
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
                             SettingsInfo.SxSmIcon,
                             SettingsInfo.SySmIcon,
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
                   SettingsInfo.szCurrentPath,
                   L"pci_dev.ini");
    StringCbPrintf(szUSBIniPath, sizeof(szUSBIniPath),
                   L"%s%s",
                   SettingsInfo.szCurrentPath,
                   L"usb_dev.ini");
    StringCbPrintf(szMonIniPath, sizeof(szMonIniPath),
                   L"%s%s",
                   SettingsInfo.szCurrentPath,
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
            if (wcslen(szDeviceName) == 0)
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
            if (wcslen(szDeviceName) == 0)
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
            if (wcslen(szDeviceName) == 0)
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
    WCHAR szPath[MAX_PATH],szName[MAX_STR_LEN], *pDevId;
    LVITEM Item = {0};

    if (!GetTempPath(MAX_PATH, szPath))
        return;

    StringCbCat(szPath, sizeof(szPath), L"report.ini");

    DeleteFile(szPath);

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

            if (pDevId && wcslen(szName) > 0)
            {
                switch (Item.iImage)
                {
                    case 0:
                        WritePrivateProfileString(L"mon_dev", pDevId, szName, szPath);
                        break;
                    case 1:
                        WritePrivateProfileString(L"usb_dev", pDevId, szName, szPath);
                        break;
                    case 2:
                        WritePrivateProfileString(L"pci_dev", pDevId, szName, szPath);
                        break;
                }
            }
        }

        --Count;
    }

    SendFileToServer(L"aspia.ru", szPath, L"report.ini");

    DeleteFile(szPath);
}

INT_PTR CALLBACK
DevReportDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            HWND hList = GetDlgItem(hDlg, IDC_REPORT_DATA_LIST);

            CheckDlgButton(hDlg, IDC_NEVER_SEND_REPORT,
                           SettingsInfo.SendDevReport ? BST_UNCHECKED : BST_CHECKED);

            hDevImageList =
                ImageList_Create(SettingsInfo.SxSmIcon,
                                 SettingsInfo.SySmIcon,
                                 ILC_MASK | SettingsInfo.SysColorDepth,
                                 1, 1);

            AddImageListIcon(hDevImageList, IDI_MONITOR); /* Icon for monitors */
            AddImageListIcon(hDevImageList, IDI_USB_DEV); /* Icon for USB */
            AddImageListIcon(hDevImageList, IDI_HW); /* Icon for PCI */

            ListView_SetImageList(hList, hDevImageList, LVSIL_SMALL);

            AddColumn(hList, 0, 400, L"");

            EnumUnknownDevices(hList, DevicesEnumProc);
        }
        break;

        case WM_CLOSE:
        {
            SettingsInfo.SendDevReport =
                (IsDlgButtonChecked(hDlg, IDC_NEVER_SEND_REPORT) == BST_CHECKED) ? FALSE : TRUE;
            FreeItems(GetDlgItem(hDlg, IDC_REPORT_DATA_LIST));
            EndDialog(hDlg, LOWORD(wParam));
        }
        break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDCANCEL:
                    PostMessage(hDlg, WM_CLOSE, 0, 0);
                    break;

                case IDOK:
                    SaveDevReportFile(GetDlgItem(hDlg, IDC_REPORT_DATA_LIST));
                    PostMessage(hDlg, WM_CLOSE, 0, 0);
                    break;
            }
        }
        break;
    }

    return FALSE;
}

VOID
DetectUnknownDevices(VOID)
{
    if (SettingsInfo.SendDevReport)
        return;

    if (EnumUnknownDevices(NULL, NULL))
    {
        DialogBox(hLangInst,
                  MAKEINTRESOURCE(IDD_UNKNOWN_DEV_REPORT),
                  hMainWnd,
                  DevReportDlgProc);
    }
}
