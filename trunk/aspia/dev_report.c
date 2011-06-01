/*
 * PROJECT:         Aspia
 * FILE:            aspia/dev_report.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"

typedef VOID (CALLBACK *DEVICESENUMPROC)(HWND hList, INT IconIndex, LPWSTR lpName, LPTSTR lpId);

HIMAGELIST hDevImageList = NULL;


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

        _wcslwr(DeviceID);

        if (IsPCIDevice(DeviceID))
        {
            GetPCIVendorID(DeviceID, szVendorID, sizeof(szVendorID));
            GetPCIDeviceID(DeviceID, szDeviceID, sizeof(szDeviceID));

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
            GetUSBVendorID(DeviceID, szVendorID, sizeof(szVendorID));
            GetUSBDeviceID(DeviceID, szDeviceID, sizeof(szDeviceID));

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
            GetMonitorID(DeviceID, szDeviceID, sizeof(szDeviceID));

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
    if (!SettingsInfo.SendDevReport)
        return;

    if (EnumUnknownDevices(NULL, NULL))
    {
        DialogBox(hLangInst,
                  MAKEINTRESOURCE(IDD_UNKNOWN_DEV_REPORT),
                  hMainWnd,
                  DevReportDlgProc);
    }
}
