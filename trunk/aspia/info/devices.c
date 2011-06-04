#include "../main.h"

#include <cfgmgr32.h>


BOOL
IsPCIDevice(LPWSTR lpszDevice)
{
    if (wcsncmp(lpszDevice, L"pci\\", 4) == 0)
        return TRUE;
    return FALSE;
}

BOOL
GetPCIVendorID(LPWSTR lpszDevice, LPWSTR lpszVendorID, SIZE_T Size)
{
    WCHAR *p, dev[MAX_PATH];

    if (lpszVendorID) lpszVendorID[0] = 0;

    StringCbCopy(dev, sizeof(dev), lpszDevice);

    p = wcsstr(dev, L"ven_");
    if (!p) return FALSE;

    p += 4, p[4] = 0;
    StringCbCopy(lpszVendorID, Size, p);
    return TRUE;
}

BOOL
GetPCIDeviceID(LPWSTR lpszDevice, LPWSTR lpszDeviceID, SIZE_T Size)
{
    WCHAR *p, dev[MAX_PATH];

    if (lpszDeviceID) lpszDeviceID[0] = 0;

    StringCbCopy(dev, sizeof(dev), lpszDevice);

    p = wcsstr(dev, L"dev_");
    if (!p) return FALSE;

    p += 4, p[4] = 0;
    StringCbCopy(lpszDeviceID, Size, p);
    return TRUE;
}

BOOL
IsUSBDevice(LPWSTR lpszDevice)
{
    if (wcsncmp(lpszDevice, L"usb\\", 4) == 0)
        return TRUE;
    return FALSE;
}

BOOL
GetUSBVendorID(LPWSTR lpszDevice, LPWSTR lpszVendorID, SIZE_T Size)
{
    WCHAR *p, dev[MAX_PATH];

    if (lpszVendorID) lpszVendorID[0] = 0;

    StringCbCopy(dev, sizeof(dev), lpszDevice);

    p = wcsstr(dev, L"vid_");
    if (!p) return FALSE;

    p += 4, p[4] = 0;
    StringCbCopy(lpszVendorID, Size, p);
    return TRUE;
}

BOOL
GetUSBDeviceID(LPWSTR lpszDevice, LPWSTR lpszDeviceID, SIZE_T Size)
{
    WCHAR *p, dev[MAX_PATH];

    if (lpszDeviceID) lpszDeviceID[0] = 0;

    StringCbCopy(dev, sizeof(dev), lpszDevice);

    p = wcsstr(dev, L"pid_");
    if (!p) return FALSE;

    p += 4, p[4] = 0;
    StringCbCopy(lpszDeviceID, Size, p);
    return TRUE;
}

BOOL
IsMonitorDevice(LPWSTR lpszDevice)
{
    if (wcsncmp(lpszDevice, L"display\\", 8) == 0)
        return TRUE;
    return FALSE;
}

BOOL
GetMonitorID(LPWSTR lpDevice, LPWSTR lpID, SIZE_T Size)
{
    WCHAR *p, dev[MAX_PATH];

    if (lpID) lpID[0] = 0;

    StringCbCopy(dev, sizeof(dev), lpDevice);

    p = wcsstr(dev, L"display\\");
    if (!p) return FALSE;

    p += 8, p[7] = 0;
    StringCbCopy(lpID, Size, p);
    return TRUE;
}

VOID
HW_DevicesInfo(VOID)
{
    WCHAR szClassRootPath[] = L"SYSTEM\\ControlSet001\\Control\\Class\\";
    SP_DEVINFO_DATA DeviceInfoData = {0};
    WCHAR DeviceID[MAX_STR_LEN], ClassName[MAX_STR_LEN],
          DeviceName[MAX_STR_LEN], KeyPath[MAX_PATH],
          DriverName[MAX_STR_LEN];
    WCHAR szDeviceID[MAX_STR_LEN], szVendorID[MAX_STR_LEN];
    WCHAR szVendorName[MAX_STR_LEN], szDeviceName[MAX_STR_LEN];
    WCHAR szPCIIniPath[MAX_PATH], szUSBIniPath[MAX_PATH];
    SP_CLASSIMAGELIST_DATA ImageListData;
    HDEVINFO hDevInfo;
    INT DeviceIndex = 0;
    INT ItemIndex, IconIndex = -1;
    HKEY KeyClass;

    DebugStartReceiving();

    StringCbPrintf(szPCIIniPath, sizeof(szPCIIniPath),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
                   L"pci_dev.ini");
    StringCbPrintf(szUSBIniPath, sizeof(szUSBIniPath),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
                   L"usb_dev.ini");

    if (IoGetTarget() == IO_TARGET_LISTVIEW)
    {
        ImageListData.cbSize = sizeof(ImageListData);
        SetupDiGetClassImageList(&ImageListData);

        ImageList_Destroy(hListViewImageList);
        hListViewImageList = ImageList_Duplicate(ImageListData.ImageList);

        ListView_SetImageList(hListView,
                              hListViewImageList,
                              LVSIL_SMALL);
    }

    hDevInfo = SetupDiGetClassDevs(NULL,
                                   NULL,
                                   NULL,
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
                LoadMUIString(IDS_DEVICE_UNKNOWN_DEVICE,
                              DeviceName, MAX_STR_LEN);
            }
        }

        if (!SetupDiGetClassImageIndex(&ImageListData,
                                       &DeviceInfoData.ClassGuid,
                                       &IconIndex))
        {
            IconIndex = 24;
        }

        ItemIndex = IoAddItem(IconIndex, DeviceName);

        KeyClass = SetupDiOpenClassRegKeyEx(&DeviceInfoData.ClassGuid,
                                            MAXIMUM_ALLOWED,
                                            DIOCR_INSTALLER,
                                            NULL,
                                            0);
        if (KeyClass != INVALID_HANDLE_VALUE)
        {
            LONG dwSize = MAX_STR_LEN;

            if (RegQueryValue(KeyClass,
                              NULL,
                              ClassName,
                              &dwSize) == ERROR_SUCCESS)
            {
                IoSetItemText(ItemIndex, 1, ClassName);
            }
            RegCloseKey(KeyClass);
        }
        else
        {
            IoSetItemText(ItemIndex, 1, L"Other Devices");
        }

        if (SetupDiGetDeviceRegistryProperty(hDevInfo,
                                             &DeviceInfoData,
                                             SPDRP_DRIVER,
                                             0,
                                             (BYTE*)DriverName,
                                             MAX_STR_LEN,
                                             NULL))
        {
            StringCbPrintf(KeyPath, sizeof(KeyPath),
                           L"%s%s", szClassRootPath, DriverName);

            if (!GetStringFromRegistry(HKEY_LOCAL_MACHINE,
                                       KeyPath, L"DriverVersion",
                                       DriverName,
                                       MAX_STR_LEN))
            {
                 DriverName[0] = 0;
            }
            IoSetItemText(ItemIndex, 2, DriverName);

            if (!GetStringFromRegistry(HKEY_LOCAL_MACHINE,
                                       KeyPath, L"DriverDate",
                                       DriverName,
                                       MAX_STR_LEN))
            {
                 DriverName[0] = 0;
            }
            IoSetItemText(ItemIndex, 3, DriverName);

            if (!GetStringFromRegistry(HKEY_LOCAL_MACHINE,
                                       KeyPath, L"ProviderName",
                                       DriverName,
                                       MAX_STR_LEN))
            {
                 DriverName[0] = 0;
            }
            IoSetItemText(ItemIndex, 4, DriverName);
        }
        else
        {
            INT i;

            for (i = 2; i <= 4; ++i)
                IoSetItemText(ItemIndex, i, L"-");
        }

        _wcslwr_s(DeviceID, MAX_STR_LEN);

        if (IsPCIDevice(DeviceID))
        {
            GetPCIVendorID(DeviceID, szVendorID, sizeof(szVendorID));
            GetPCIDeviceID(DeviceID, szDeviceID, sizeof(szDeviceID));

            GetPrivateProfileString(L"vendors", szVendorID, L"-",
                                    szVendorName, MAX_STR_LEN, szPCIIniPath);

            StringCbCat(szVendorID, sizeof(szVendorID), L"-");
            StringCbCat(szVendorID, sizeof(szVendorID), szDeviceID);
            GetPrivateProfileString(L"devices", szVendorID, L"-",
                                    szDeviceName, MAX_STR_LEN, szPCIIniPath);
        }
        else if (IsUSBDevice(DeviceID))
        {
            GetUSBVendorID(DeviceID, szVendorID, sizeof(szVendorID));
            GetUSBDeviceID(DeviceID, szDeviceID, sizeof(szDeviceID));

            GetPrivateProfileString(L"vendors", szVendorID, L"-",
                                    szVendorName, MAX_STR_LEN, szUSBIniPath);

            StringCbCat(szVendorID, sizeof(szVendorID), L"-");
            StringCbCat(szVendorID, sizeof(szVendorID), szDeviceID);
            GetPrivateProfileString(L"devices", szVendorID, L"-",
                                    szDeviceName, MAX_STR_LEN, szUSBIniPath);
        }
        else
        {
            StringCbCopy(szVendorID, sizeof(szVendorID), L"-");
            StringCbCopy(szVendorName, sizeof(szVendorName), L"-");
            StringCbCopy(szDeviceName, sizeof(szDeviceName), L"-");
        }
        IoSetItemText(ItemIndex, 5, szVendorID);
        IoSetItemText(ItemIndex, 6, szVendorName);
        IoSetItemText(ItemIndex, 7, szDeviceName);
    }

    if (hDevInfo)
    {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    if (IoGetTarget() == IO_TARGET_LISTVIEW)
    {
        SetupDiDestroyClassImageList(&ImageListData);
    }

    DebugEndReceiving();
}

VOID
HW_UnknownDevicesInfo(VOID)
{
    WCHAR szDeviceID[MAX_STR_LEN], szVendorID[MAX_STR_LEN];
    WCHAR szVendorName[MAX_STR_LEN], szDeviceName[MAX_STR_LEN];
    WCHAR szPCIIniPath[MAX_PATH], szUSBIniPath[MAX_PATH];
    SP_DEVINFO_DATA DeviceInfoData = {0};
    WCHAR DeviceID[MAX_STR_LEN];
    HDEVINFO hDevInfo;
    INT DeviceIndex = 0;
    INT ItemIndex;
    HKEY KeyClass;

    DebugStartReceiving();

    IoAddIcon(IDI_DISABLED_HW);

    StringCbPrintf(szPCIIniPath, sizeof(szPCIIniPath),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
                   L"pci_dev.ini");
    StringCbPrintf(szUSBIniPath, sizeof(szUSBIniPath),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
                   L"usb_dev.ini");

    hDevInfo = SetupDiGetClassDevs(NULL,
                                   NULL,
                                   NULL,
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
                                        MAX_STR_LEN,
                                        NULL))
        {
            continue;
        }

        KeyClass = SetupDiOpenClassRegKeyEx(&DeviceInfoData.ClassGuid,
                                            MAXIMUM_ALLOWED,
                                            DIOCR_INSTALLER,
                                            NULL,
                                            0);
        if (KeyClass != INVALID_HANDLE_VALUE)
        {
            RegCloseKey(KeyClass);
            continue;
        }
        else
        {
            ItemIndex = IoAddItem(0, DeviceID);
        }

        _wcslwr_s(DeviceID, MAX_STR_LEN);

        if (IsPCIDevice(DeviceID))
        {
            GetPCIVendorID(DeviceID, szVendorID, sizeof(szVendorID));
            GetPCIDeviceID(DeviceID, szDeviceID, sizeof(szDeviceID));

            GetPrivateProfileString(L"vendors", szVendorID, L"-",
                                    szVendorName, MAX_STR_LEN, szPCIIniPath);

            StringCbCat(szVendorID, sizeof(szVendorID), L"-");
            StringCbCat(szVendorID, sizeof(szVendorID), szDeviceID);
            GetPrivateProfileString(L"devices", szVendorID, L"-",
                                    szDeviceName, MAX_STR_LEN, szPCIIniPath);
        }
        else if (IsUSBDevice(DeviceID))
        {
            GetUSBVendorID(DeviceID, szVendorID, sizeof(szVendorID));
            GetUSBDeviceID(DeviceID, szDeviceID, sizeof(szDeviceID));

            GetPrivateProfileString(L"vendors", szVendorID, L"-",
                                    szVendorName, MAX_STR_LEN, szUSBIniPath);

            StringCbCat(szVendorID, sizeof(szVendorID), L"-");
            StringCbCat(szVendorID, sizeof(szVendorID), szDeviceID);
            GetPrivateProfileString(L"devices", szVendorID, L"-",
                                    szDeviceName, MAX_STR_LEN, szUSBIniPath);
        }
        else
        {
            StringCbCopy(szVendorID, sizeof(szVendorID), L"-");
            StringCbCopy(szVendorName, sizeof(szVendorName), L"-");
            StringCbCopy(szDeviceName, sizeof(szDeviceName), L"-");
        }
        IoSetItemText(ItemIndex, 1, szVendorID);
        IoSetItemText(ItemIndex, 2, szVendorName);
        IoSetItemText(ItemIndex, 3, szDeviceName);
    }

    if (hDevInfo)
    {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    DebugEndReceiving();
}
