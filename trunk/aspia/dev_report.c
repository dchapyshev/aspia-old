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


CHAR*
http_receive(HINTERNET h_req, ULONG *d_size)
{
    ULONG bytes  = sizeof(ULONG);
    ULONG qsize  = 0;
    ULONG readed = 0;
    CHAR  *data   = NULL;
    CHAR   buff[4096];

    if (HttpQueryInfo(h_req, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &qsize, &bytes, NULL) != 0)
    {
        data = malloc(qsize + 1);
    }

    do
    {
        if (InternetReadFile(h_req, buff, sizeof(buff), &bytes) == 0)
            break;

        if ((readed + bytes) > qsize)
        {
            data = realloc(data, readed + bytes + 1);
            if (!data) break;
            qsize += bytes;
        }
        memcpy(data + readed, buff, bytes);
        readed += bytes;
    }
    while (bytes != 0);

    if (data && readed != qsize)
    {
        free(data);
        data = NULL;
    }
    else
    {
        if (d_size) *d_size = readed;
        data[readed] = 0;
    }

    return data;
}

LPVOID
http_get(WCHAR *url, ULONG *d_size)
{
    HINTERNET h_inet = NULL;
    HINTERNET h_req  = NULL;
    CHAR     *replay = NULL;
    
    for (;;)
    {
        h_inet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!h_inet) break;
        
        h_req = InternetOpenUrl(h_inet, url, NULL, 0, INTERNET_FLAGS, 0);
        if (!h_req) break;

        replay = http_receive(h_req, d_size);
    }

    if (h_req) InternetCloseHandle(h_req);
    if (h_inet) InternetCloseHandle(h_inet);

    return replay;
}

LPVOID
http_post(WCHAR *url, LPVOID data, INT size, ULONG *d_size)
{
    URL_COMPONENTS url_cm = {0};
    HINTERNET      h_inet = NULL;
    HINTERNET      h_conn = NULL;
    HINTERNET      h_req  = NULL;
    CHAR          *q_data = NULL;
    CHAR          *replay = NULL;
    WCHAR        host[MAX_PATH];
    WCHAR        path[MAX_PATH];
    UCHAR *p, *d = data;

    for (;;)
    {
        q_data = malloc(size * 3 + 10);
        if (!q_data) break;

        StringCbCopyA(q_data, size * 3 + 10, "data=");
        p = (UCHAR*)(q_data + 5);

        while (size--)
            p += sprintf(p, "%%%0.2x", (ULONG)*d++);

        url_cm.dwStructSize     = sizeof(url_cm);
        url_cm.lpszHostName     = host;
        url_cm.dwHostNameLength = sizeof(host);
        url_cm.lpszUrlPath      = path;
        url_cm.dwUrlPathLength  = sizeof(path);

        if (InternetCrackUrl(url, 0, 0, &url_cm) == 0)
            break;

        h_inet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!h_inet) break;

        h_conn = InternetConnect(h_inet, host, url_cm.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (!h_conn) break;

        h_req = HttpOpenRequest(h_conn, L"POST", path, NULL, NULL, NULL, INTERNET_FLAGS, 0);
        if (!h_req) break;

        HttpAddRequestHeaders(h_req,
                              L"Content-Type: application/x-www-form-urlencoded",
                              47 * 2, HTTP_ADDREQ_FLAG_ADD);

        if (HttpSendRequest(h_req, NULL, 0, q_data, strlen(q_data)) == 0)
            break;

        replay = http_receive(h_req, d_size);
    }

    if (h_req) InternetCloseHandle(h_req);
    if (h_conn) InternetCloseHandle(h_conn);
    if (h_inet) InternetCloseHandle(h_inet);

    if (q_data) free(q_data);

    return replay;
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
                if (lpEnumProc) lpEnumProc(hList, 2, DeviceName, DeviceID);
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
                if (lpEnumProc) lpEnumProc(hList, 1, DeviceName, DeviceID);
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
                if (lpEnumProc) lpEnumProc(hList, 0, DeviceName, DeviceID);
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
    WCHAR *pId = Alloc((SafeStrLen(lpId) + 1) * sizeof(WCHAR));

    if (!pId) return;

    AddItem(hList, IconIndex, lpName, (LPARAM)pId);
}

VOID
FreeItems(HWND hList)
{
    INT Count = ListView_GetItemCount(hList) - 1;
    WCHAR *pDevId;

    while (Count > 0)
    {
        pDevId = (WCHAR*)ListViewGetlParam(hList, Count);
        if (pDevId) Free(pDevId);
        --Count;
    }
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
