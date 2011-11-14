/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/hw.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <intrin.h>

#include "../main.h"
#include "../smart/smart.h"
//#include "scsi/scsi.h"
#include "edid.h"

const BYTE EdidV1DescriptorFlag[] = { 0x00, 0x00 };

VOID
HW_CPUInfo(VOID)
{
    DebugStartReceiving();
    CPUIDInfo();
    DebugEndReceiving();
}

BOOL CALLBACK
EnumSmartDataProc(SMART_RESULT *Result)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    /* Attribute ID */
    StringCbPrintf(szText, sizeof(szText),
                   L"%02X", Result->dwAttrID);
    Index = IoAddItem(0, -1, szText);

    /* Name */
    IoSetItemText(Index, 1, Result->szName);

    /* Threshold */
    StringCbPrintf(szText, sizeof(szText),
                   L"%d", Result->dwWarrantyThreshold);
    IoSetItemText(Index, 2, szText);

    /* Value */
    StringCbPrintf(szText, sizeof(szText),
                   L"%d", Result->bValue);
    IoSetItemText(Index, 3, szText);

    /* Worst */
    StringCbPrintf(szText, sizeof(szText),
                   L"%d", Result->dwWorstValue);
    IoSetItemText(Index, 4, szText);

    /* Data */
    StringCbPrintf(szText, sizeof(szText),
                   L"%d", Result->dwAttrValue);
    IoSetItemText(Index, 5, szText);

    return TRUE;
}

BOOL
GetDiskGeometry(BYTE bDevNumber, DISK_GEOMETRY *DiskGeometry)
{
    WCHAR szPath[MAX_PATH];
    HANDLE hDevice;
    DWORD temp;
    BOOL Result;

    StringCbPrintf(szPath,
                   sizeof(szPath),
                   L"\\\\.\\PhysicalDrive%d",
                   bDevNumber);

    hDevice = CreateFile(szPath, 0,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    Result = DeviceIoControl(hDevice,
                             IOCTL_DISK_GET_DRIVE_GEOMETRY,
                             NULL, 0,
                             DiskGeometry, sizeof(*DiskGeometry),
                             &temp,
                             NULL);

    CloseHandle(hDevice);

    return Result;
}

VOID
HW_HDDATAInfo(VOID)
{
    DISK_GEOMETRY DiskGeometry = {0};
    IDSECTOR DriveInfo = {0};
    WCHAR szText[MAX_STR_LEN], szFormat[MAX_STR_LEN];
    HANDLE hHandle;
    BYTE bIndex;
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_HDD);

    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        hHandle = SMART_Open(bIndex);
        if (!hHandle) continue;

        if (SMART_ReadDriveInformation(hHandle, bIndex, &DriveInfo))
        {
            ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                            sizeof(DriveInfo.sModelNumber));
            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sModelNumber);
            ChopSpaces(szText, sizeof(szText));
            IoAddHeaderString(0, szText, 0);

            Index = IoAddValueName(1, IDS_HDD_ID, 0);
            IoSetItemText(Index, 1, szText);

            Index = IoAddValueName(1, IDS_SERIAL_NUMBER, 0);
            ChangeByteOrder((PCHAR)DriveInfo.sSerialNumber,
                            sizeof(DriveInfo.sSerialNumber));
            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sSerialNumber);
            ChopSpaces(szText, sizeof(szText));
            IoSetItemText(Index, 1, szText);

            Index = IoAddValueName(1, IDS_VERSION, 0);
            ChangeByteOrder((PCHAR)DriveInfo.sFirmwareRev,
                            sizeof(DriveInfo.sFirmwareRev));
            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sFirmwareRev);
            IoSetItemText(Index, 1, szText);

            if (GetDiskGeometry(bIndex, &DiskGeometry))
            {
                ULONGLONG DiskSize;

                Index = IoAddValueName(1, IDS_HDD_PARAMS, 0);
                LoadMUIString(IDS_HDD_PARAMS_FORMAT, szFormat, MAX_STR_LEN);
                StringCbPrintf(szText, sizeof(szText), szFormat,
                               (ULONG)DiskGeometry.Cylinders.QuadPart * (ULONG)DriveInfo.wNumHeads,
                               (ULONG)DriveInfo.wNumHeads,
                               (ULONG)DiskGeometry.SectorsPerTrack,
                               (ULONG)DiskGeometry.BytesPerSector);
                IoSetItemText(Index, 1, szText);

                DiskSize = DiskGeometry.Cylinders.QuadPart * (ULONG)DiskGeometry.TracksPerCylinder *
                           (ULONG)DiskGeometry.SectorsPerTrack * (ULONG)DiskGeometry.BytesPerSector;

                Index = IoAddValueName(1, IDS_HDD_SIZE, 0);
                StringCbPrintf(szText, sizeof(szText),
                               L"%I64d MB (%I64d GB)",
                               DiskSize / (1024 * 1024),
                               DiskSize / (1024 * 1024 * 1024));
                IoSetItemText(Index, 1, szText);
            }

            Index = IoAddValueName(1, IDS_HDD_BUFFER_SIZE, 0);
            StringCbPrintf(szText, sizeof(szText), L"%d MB",
                           (DriveInfo.wBufferSize * 512)/(1024 * 1024));
            IoSetItemText(Index, 1, szText);

            Index = IoAddValueName(1, IDS_HDD_MULTISECTORS, 0);
            StringCbPrintf(szText, sizeof(szText), L"%d",
                           DriveInfo.wMultSectorStuff);
            IoSetItemText(Index, 1, szText);

            Index = IoAddValueName(1, IDS_HDD_ECC_BYTES, 0);
            StringCbPrintf(szText, sizeof(szText), L"%d",
                           DriveInfo.wECCSize);
            IoSetItemText(Index, 1, szText);

            Index = IoAddValueName(1, IDS_TYPE, 0);
            if (DriveInfo.wGenConfig & 0x80)
                StringCbCopy(szText, sizeof(szText), L"Removable");
            else if (DriveInfo.wGenConfig & 0x40)
                StringCbCopy(szText, sizeof(szText), L"Fixed");
            else
                StringCbCopy(szText, sizeof(szText), L"Unknown");
            IoSetItemText(Index, 1, szText);

            IoAddFooter();
        }

        SMART_Close(hHandle);
    }

    DebugEndReceiving();
}

VOID
HW_CDInfo(VOID)
{
    SP_DEVINFO_DATA DeviceInfoData = {0};
    WCHAR szDeviceName[MAX_STR_LEN];
    HDEVINFO hDevInfo;
    INT DeviceIndex = 0;

    DebugStartReceiving();

    IoAddIcon(IDI_CD);

    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_CDROM,
                                   0, 0,
                                   DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return;

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(hDevInfo,
                                 DeviceIndex,
                                 &DeviceInfoData))
    {
        ++DeviceIndex;

        if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                              &DeviceInfoData,
                                              SPDRP_FRIENDLYNAME,
                                              0,
                                              (BYTE*)szDeviceName,
                                              MAX_STR_LEN,
                                              NULL))
        {
            if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_DEVICEDESC,
                                                  0,
                                                  (BYTE*)szDeviceName,
                                                  MAX_STR_LEN,
                                                  NULL))
            {
                LoadMUIString(IDS_DEVICE_UNKNOWN_DEVICE,
                              szDeviceName, MAX_STR_LEN);
            }
        }

        MessageBox(0, szDeviceName, 0, 0);
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    DebugEndReceiving();
}

VOID
HW_HDDSCSIInfo(VOID)
{
    //WCHAR szText[MAX_STR_LEN];
    //IDSECTOR IdSector;
    //BYTE bDevNumber;
    //HANDLE hHandle;
    //INT Index;

    IoAddIcon(IDI_HDD);
/*
    for (bDevNumber = 0; bDevNumber < 32; ++bDevNumber)
    {
    hHandle = SCSI_Open(bDevNumber);
    if (hHandle == INVALID_HANDLE_VALUE)
        continue;

    ZeroMemory(&IdSector, sizeof(IdSector));
    if (!SCSI_ReadDriveInformation(hHandle, bDevNumber, &IdSector))
    {
        SCSI_Close(hHandle);
        continue;
    }

    ChangeByteOrder((PCHAR)IdSector.sModelNumber, sizeof(IdSector.sModelNumber));
    StringCbPrintf(szText, sizeof(szText), L"%S", IdSector.sModelNumber);
    Index = IoAddItem(0, szText);

    SCSI_Close(hHandle);
    }

    SCSI_Close(hHandle);*/
}

VOID
HW_HDDSMARTInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    IDSECTOR DriveInfo;
    HANDLE hHandle;
    INT Index;
    BYTE bIndex;

    DebugStartReceiving();

    IoAddIcon(IDI_HDD);

    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        hHandle = SMART_Open(bIndex);
        if (!hHandle) continue;

        if (SMART_ReadDriveInformation(hHandle, bIndex, &DriveInfo))
        {
            Index = IoAddHeaderString(0, L"\0", 0);
            ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                            sizeof(DriveInfo.sModelNumber));
            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sModelNumber);
            ChopSpaces(szText, sizeof(szText));
            IoSetItemText(Index, 1, szText);
            IoSetItemText(Index, 2, L"\0");
            IoSetItemText(Index, 3, L"\0");
            IoSetItemText(Index, 4, L"\0");
            IoSetItemText(Index, 5, L"\0");
        }

        if (SMART_EnumData(hHandle, bIndex, EnumSmartDataProc))
            IoAddFooter();
        SMART_Close(hHandle);
    }

    DebugEndReceiving();
}

WCHAR*
GetEdidVendorSign(BYTE const *Block)
{
    static WCHAR sign[4];
    USHORT h;

/*
    08h WORD big-endian manufacturer ID (see #00136)
    bits 14-10: first letter (01h='A', 02h='B', etc.)
    bits 9-5: second letter
    bits 4-0: third letter
*/
    h = COMBINE_HI_8LO(Block[0], Block[1]);
    sign[0] = ((h >> 10) & 0x1f) + L'A' - 1;
    sign[1] = ((h >> 5) & 0x1f) + L'A' - 1;
    sign[2] = (h & 0x1f) + L'A' - 1;
    sign[3] = 0;
    return sign;
}

WCHAR*
GetMonitorName(BYTE const *Block)
{
    BYTE const *ptr = Block + DESCRIPTOR_DATA;
    static WCHAR szName[13];
    UINT Index;

    for (Index = 0; Index < 13; Index++, ptr++)
    {
        if (*ptr == 0xa)
        {
            szName[Index] = 0;
            return szName;
        }

        szName[Index] = *ptr;
    }

    return szName;
}

INT
EdidBlockType(BYTE* Block)
{
    if (!wcsncmp((WCHAR*)EdidV1DescriptorFlag, (WCHAR*)Block, 2))
    {
        /* descriptor */
        if (Block[2] != 0)
            return UNKNOWN_DESCRIPTOR;
        return Block[3];
    }
    else
    {
        /* detailed timing block */
        return DETAILED_TIMING_BLOCK;
    }
}

VOID
ParseAndShowEDID(LPWSTR lpDeviceName, BYTE *Edid)
{
    WCHAR szText[MAX_STR_LEN], szIniPath[MAX_PATH];
    WCHAR szMonitorId[MAX_STR_LEN];
    WCHAR *pVendorSign, *pMonitorModel = NULL;
    UINT Index;
    BYTE Checksum = 0;
    BYTE* Block;
    INT ItemIndex;

    StringCbPrintf(szIniPath, sizeof(szIniPath),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
                   L"mon_dev.ini");

    for (Index = 0; Index < 0x80; Index++)
        Checksum += Edid[Index];
    if (Checksum != 0)
    {
        DebugTrace(L"EDID checksum failed");
        return;
    }

    pVendorSign = GetEdidVendorSign(Edid + ID_MANUFACTURER_NAME);

    Block = Edid + DETAILED_TIMING_DESCRIPTIONS_START;
    for (Index = 0; Index < NO_DETAILED_TIMING_DESCRIPTIONS; Index++,
        Block += DETAILED_TIMING_DESCRIPTION_SIZE)
    {
        if (EdidBlockType(Block) == MONITOR_NAME)
        {
            pMonitorModel = GetMonitorName(Block);
            break;
        }
    }

    StringCbPrintf(szMonitorId, sizeof(szMonitorId), L"%s%02x%02x",
                   pVendorSign, Edid[ID_MODEL + 1], Edid[ID_MODEL]);
    _wcsupr(szMonitorId);

    GetPrivateProfileString(L"devices", szMonitorId, L"",
                            szText, MAX_STR_LEN, szIniPath);
    if (szText[0] == L'\0')
    {
        StringCbPrintf(szText, sizeof(szText), L"%s (NoDB)", lpDeviceName);
    }

    IoAddHeaderString(0, szText, 0);

    /* Monitor ID */
    ItemIndex = IoAddValueName(1, IDS_DISPLAY_ID, 0);
    IoSetItemText(ItemIndex, 1, szMonitorId);

    /* Monitor Model */
    if (pMonitorModel)
    {
        ItemIndex = IoAddValueName(1, IDS_DISPLAY_MODEL, 0);
        IoSetItemText(ItemIndex, 1, pMonitorModel);
    }

    /* Manufacture Date */
    ItemIndex = IoAddValueName(1, IDS_DISPLAY_MANUFACTURE_DATE, 0);
    StringCbPrintf(szText, sizeof(szText), L"%d Week / %d Year",
                   (INT)Edid[MANUFACTURE_WEEK],
                   (INT)Edid[MANUFACTURE_YEAR] + 1990);
    IoSetItemText(ItemIndex, 1, szText);

    /* Max/ Visible Display Size */
    ItemIndex = IoAddValueName(1, IDS_DISPLAY_MAX_VISIBLE_SIZE, 0);

    StringCbPrintf(szText, sizeof(szText), L"%d cm / %d cm (%.1f\")",
                   (INT)Edid[0x15], (INT)Edid[0x16],
                   GetDiagonalSize(Edid[0x15], Edid[0x16]));
    IoSetItemText(ItemIndex, 1, szText);

    Block = Edid + DETAILED_TIMING_DESCRIPTIONS_START;
    for (Index = 0; Index < NO_DETAILED_TIMING_DESCRIPTIONS; Index++,
         Block += DETAILED_TIMING_DESCRIPTION_SIZE)
    {
        if (EdidBlockType(Block) == DETAILED_TIMING_BLOCK)
        {
            /* Max. Resolution */
            ItemIndex = IoAddValueName(1, IDS_DISPLAY_MAX_RESOLUTION, 0);
            StringCbPrintf(szText, sizeof(szText),
                           L"%dx%d",
                           H_ACTIVE, V_ACTIVE);
            IoSetItemText(ItemIndex, 1, szText);
        }
    }

    Block = Edid + DETAILED_TIMING_DESCRIPTIONS_START;
    for(Index = 0; Index < NO_DETAILED_TIMING_DESCRIPTIONS; Index++,
        Block += DETAILED_TIMING_DESCRIPTION_SIZE)
    {
        if (EdidBlockType(Block) == MONITOR_LIMITS)
        {
            ItemIndex = IoAddValueName(1, IDS_DISPLAY_HORIZ_FREQ, 0);
            StringCbPrintf(szText, sizeof(szText),
                           L"%u - %u kHz",
                           Block[7], Block[8]);
            IoSetItemText(ItemIndex, 1, szText);

            ItemIndex = IoAddValueName(1, IDS_DISPLAY_VERT_FREQ, 0);
            StringCbPrintf(szText, sizeof(szText),
                           L"%u - %u Hz",
                           Block[5], Block[6]);
            IoSetItemText(ItemIndex, 1, szText);
        }
    }

    /* Gamma */
    ItemIndex = IoAddValueName(1, IDS_DISPLAY_GAMMA, 0);
    StringCbPrintf(szText, sizeof(szText), L"%.1f",
                   (double)(((double)Edid[23] / 100.0) + 1.0));
    IoSetItemText(ItemIndex, 1, szText);

    /* DPMS Mode Support */
    szText[0] = 0;
    ItemIndex = IoAddValueName(1, IDS_DISPLAY_DPMS_MODE, 0);
    if ((Edid[DPMS_FLAGS] & DPMS_ACTIVE_OFF))
        StringCbCat(szText, sizeof(szText), L"Active-Off, ");
    if (Edid[DPMS_FLAGS] & DPMS_SUSPEND)
        StringCbCat(szText, sizeof(szText), L"Suspend, ");
    if (Edid[DPMS_FLAGS] & DPMS_STANDBY)
        StringCbCat(szText, sizeof(szText), L"Standby, ");
    if (szText[0] != L'\0')
        szText[wcslen(szText) - 2] = 0;
    else
        StringCbCopy(szText, sizeof(szText), L"None");
    IoSetItemText(ItemIndex, 1, szText);

    /* EDID Version */
    ItemIndex = IoAddValueName(1, IDS_DISPLAY_EDID_VERSION, 0);
    StringCbPrintf(szText, sizeof(szText), L"%d.%d",
                   (INT)Edid[EDID_STRUCT_VERSION],
                   (INT)Edid[EDID_STRUCT_REVISION]);
    IoSetItemText(ItemIndex, 1, szText);
}

VOID
HW_MonitorInfo(VOID)
{
    SP_DEVINFO_DATA DeviceInfoData = {0};
    WCHAR szDevPath[MAX_PATH], szKeyPath[MAX_PATH],
          szDeviceName[MAX_STR_LEN];
    HDEVINFO hDevInfo;
    INT DeviceIndex = 0;
    BYTE Edid[0x80];

    DebugStartReceiving();

    IoAddIcon(IDI_MONITOR);

    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_MONITOR,
                                   0, 0,
                                   DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return;

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(hDevInfo,
                                 DeviceIndex,
                                 &DeviceInfoData))
    {
        ++DeviceIndex;

        if (!SetupDiGetDeviceInstanceId(hDevInfo,
                                        &DeviceInfoData,
                                        szDevPath,
                                        MAX_PATH,
                                        NULL))
        {
            continue;
        }

        if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                              &DeviceInfoData,
                                              SPDRP_FRIENDLYNAME,
                                              0,
                                              (BYTE*)szDeviceName,
                                              MAX_STR_LEN,
                                              NULL))
        {
            if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_DEVICEDESC,
                                                  0,
                                                  (BYTE*)szDeviceName,
                                                  MAX_STR_LEN,
                                                  NULL))
            {
                LoadMUIString(IDS_DEVICE_UNKNOWN_DEVICE,
                              szDeviceName, MAX_STR_LEN);
            }
        }

        StringCbPrintf(szKeyPath, sizeof(szKeyPath),
                       L"SYSTEM\\CurrentControlSet\\Enum\\%s\\Device Parameters",
                       szDevPath);

        if (GetBinaryFromRegistry(HKEY_LOCAL_MACHINE,
                                  szKeyPath, L"EDID",
                                  (LPBYTE)&Edid, sizeof(Edid)))
        {
            ParseAndShowEDID(szDeviceName, Edid);
            IoAddFooter();
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    DebugEndReceiving();
}

VOID
HW_WinVideoInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    DWORD dwValue, dwDevIndex = 0;
    DISPLAY_DEVICE DispDevice, DispDevice2;
    SP_DEVINFO_DATA DevInfoData;
    HDEVINFO hDevInfo;
    DEVMODE DevMode;
    DWORD dwOffset = 0;
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_MONITOR);

    for (;;)
    {
        ZeroMemory(&DispDevice, sizeof(DISPLAY_DEVICE));
        DispDevice.cb = sizeof(DISPLAY_DEVICE);

        if (!EnumDisplayDevices(NULL, dwDevIndex + dwOffset, &DispDevice, 0))
            break;

        if (!(DispDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) ||
            (DispDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
        {
            ++dwOffset;
            continue;
        }

        /* Monitor name */
        ZeroMemory(&DispDevice2, sizeof(DISPLAY_DEVICE));
        DispDevice2.cb = sizeof(DISPLAY_DEVICE);
        if (!EnumDisplayDevices(DispDevice.DeviceName, 0, &DispDevice2, 0))
            IoAddHeaderString(0, L"Mirror Monitor", 0);
        else
            IoAddHeaderString(0, DispDevice2.DeviceString, 0);

        /* Adapter name */
        if (SafeStrLen(DispDevice.DeviceString) > 0)
        {
            Index = IoAddValueName(1, IDS_MONITOR_ADAPTER, 0);
            IoSetItemText(Index, 1, DispDevice.DeviceString);
        }

        /* Chip type */
        if (GetStringFromRegistry(TRUE,
                                  HKEY_LOCAL_MACHINE,
                                  &DispDevice.DeviceKey[18],
                                  L"HardwareInformation.ChipType",
                                  szText, MAX_STR_LEN) &&
            szText[0] != 0)
        {
            Index = IoAddValueName(1, IDS_MONITOR_CHIP_TYPE, 0);
            IoSetItemText(Index, 1, szText);
        }

        /* DAC type */
        if (GetStringFromRegistry(TRUE,
                                  HKEY_LOCAL_MACHINE,
                                  &DispDevice.DeviceKey[18],
                                  L"HardwareInformation.DacType",
                                  szText, MAX_STR_LEN) &&
            szText[0] != 0)
        {
            Index = IoAddValueName(1, IDS_MONITOR_DAC_TYPE, 0);
            IoSetItemText(Index, 1, szText);
        }

        /* Memory size */
        if (GetBinaryFromRegistry(HKEY_LOCAL_MACHINE,
                                  &DispDevice.DeviceKey[18],
                                  L"HardwareInformation.MemorySize",
                                  (LPBYTE)&dwValue,
                                  sizeof(dwValue)) &&
            dwValue > 0)
        {
            Index = IoAddValueName(1, IDS_MONITOR_MEM_SIZE, 0);
            StringCbPrintf(szText, sizeof(szText), L"%ld MB",
                           dwValue / (1024 * 1024));
            IoSetItemText(Index, 1, szText);
        }

        /* retrieve current display mode */
        Index = IoAddValueName(1, IDS_MONITOR_MODE, 0);
        DevMode.dmSize = sizeof(DEVMODE);
        if (EnumDisplaySettings(DispDevice.DeviceName,
                                ENUM_CURRENT_SETTINGS,
                                &DevMode))
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"%Iu x %Iu (%Iu bit) (%Iu Hz)",
                           DevMode.dmPelsWidth,
                           DevMode.dmPelsHeight,
                           DevMode.dmBitsPerPel,
                           DevMode.dmDisplayFrequency);
            IoSetItemText(Index, 1, szText);
        }

        hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_DISPLAY,
                                       NULL, NULL,
                                       DIGCF_PRESENT | DIGCF_PROFILE);
        if (hDevInfo == INVALID_HANDLE_VALUE)
        {
            ++dwDevIndex;
            continue;
        }

        ZeroMemory(&DevInfoData, sizeof(SP_DEVINFO_DATA));
        DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        if (SetupDiEnumDeviceInfo(hDevInfo, dwDevIndex, &DevInfoData))
        {
            /* Driver vendor */
            if (SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                 &DevInfoData,
                                                 SPDRP_MFG,
                                                 NULL,
                                                 (PBYTE)szText,
                                                 sizeof(szText),
                                                 NULL))
            {
                Index = IoAddValueName(1, IDS_MONITOR_DRIVER_VENDOR, 0);
                IoSetItemText(Index, 1, szText);
            }
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);

        IoAddFooter();
        ++dwDevIndex;
    }

    DebugEndReceiving();
}

VOID
HW_PowerInfo(VOID)
{
    SYSTEM_POWER_STATUS PowerStatus = {0};
    WCHAR szText[MAX_STR_LEN], szResult[MAX_STR_LEN];
    UINT StringID;
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_POWERICO);
    IoAddIcon(IDI_BATTERY);

    IoAddHeader(0, IDS_POWER_MANAGMT_PROP, 0);

    if (GetSystemPowerStatus(&PowerStatus))
    {
        Index = IoAddValueName(1, IDS_CURRENT_POWER_SOURCE, 0);
        switch (PowerStatus.ACLineStatus)
        {
            case 0:
                StringID = IDS_POWER_ON_DC_BATTERY;
                break;
            case 1:
                StringID = IDS_POWER_ON_AC_LINE;
                break;
            default:
                StringID = IDS_UNKNOWN;
                break;
        }
        LoadMUIString(StringID, szText, MAX_STR_LEN);
        IoSetItemText(Index, 1, szText);

        Index = IoAddValueName(1, IDS_BATTERY_STATUS, 0);
        switch (PowerStatus.BatteryFlag)
        {
            case 1:
                StringID = IDS_POWER_HIGH_BATTERY;
                break;
            case 2:
                StringID = IDS_POWER_LOW_BATTERY;
                break;
            case 4:
                StringID = IDS_POWER_CRITICAL_BATTERY;
                break;
            case 8:
                StringID = IDS_POWER_CHARGING;
                break;
            case 128:
                StringID = IDS_POWER_NO_BATTERY;
                break;
            default:
                StringID = IDS_UNKNOWN;
                break;
        }
        LoadMUIString(StringID, szText, MAX_STR_LEN);
        if (PowerStatus.BatteryFlag == 128 ||
            StringID == IDS_UNKNOWN)
        {
            IoSetItemText(Index, 1, szText);
        }
        else
        {
            StringCbPrintf(szResult, sizeof(szResult), L"%ld%% (%s)",
                           PowerStatus.BatteryLifePercent, szText);
            IoSetItemText(Index, 1, szResult);
        }

        Index = IoAddValueName(1, IDS_FULL_BATTERY_LIFETIME, 0);
        if (PowerStatus.BatteryFullLifeTime != -1)
        {
            ConvertSecondsToString(PowerStatus.BatteryFullLifeTime,
                                   szText, sizeof(szText));
        }
        else
        {
            LoadMUIString(IDS_UNKNOWN,
                       szText, MAX_STR_LEN);
        }
        IoSetItemText(Index, 1, szText);

        Index = IoAddValueName(1, IDS_REMAINING_BAT_LIFETIME, 0);
        if (PowerStatus.BatteryLifeTime != -1)
        {
            ConvertSecondsToString(PowerStatus.BatteryLifeTime,
                                   szText, sizeof(szText));
        }
        else
        {
            LoadMUIString(IDS_UNKNOWN, szText, MAX_STR_LEN);
        }
        IoSetItemText(Index, 1, szText);

        if (PowerStatus.BatteryFlag == 128 ||
            PowerStatus.BatteryFlag == 255)
        {
            return;
        }
    }
    IoAddFooter();

    IoAddHeader(0, IDS_BAT_BATTERYS, 1);
    GetBatteryInformation();

    DebugEndReceiving();
}

VOID
EnumPrintersInfo(DWORD dwFlag)
{
    PPRINTER_INFO_2 pPrinterInfo;
    DWORD cbNeeded, cReturned, dwSize;
    WCHAR szText[MAX_STR_LEN], szDefPrinter[MAX_STR_LEN] = {0};
    DEVMODE *pDevMode;
    SIZE_T dwIndex;
    INT Index;

    if (!EnumPrinters(dwFlag, 0, 2, 0, 0, &cbNeeded, &cReturned))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return;
    }

    pPrinterInfo = (PPRINTER_INFO_2)Alloc(cbNeeded);
    if (!pPrinterInfo)
        return;

    if (!EnumPrinters(dwFlag,
                      NULL, 2,
                      (LPBYTE)pPrinterInfo,
                      cbNeeded,
                      &cbNeeded,
                      &cReturned))
    {
        Free(pPrinterInfo);
        return;
    }

    dwSize = MAX_STR_LEN;
    GetDefaultPrinter(szDefPrinter, &dwSize);

    for (dwIndex = 0; dwIndex < cReturned; ++dwIndex)
    {
        pDevMode = pPrinterInfo[dwIndex].pDevMode;

        /* Printer name */
        IoAddHeaderString(0, pPrinterInfo[dwIndex].pPrinterName, 0);

        /* Default? */
        Index = IoAddValueName(1, IDS_PRINTER_DEFAULT, 0);
        if (SafeStrCmp(szDefPrinter, pPrinterInfo[dwIndex].pPrinterName) == 0)
            LoadMUIString(IDS_YES, szText, MAX_STR_LEN);
        else
            LoadMUIString(IDS_NO, szText, MAX_STR_LEN);

        IoSetItemText(Index, 1, szText);

        /* Shared? */
        Index = IoAddValueName(1, IDS_PRINTER_SHARED, 0);
        if (pPrinterInfo[dwIndex].Attributes & PRINTER_ATTRIBUTE_SHARED)
            LoadMUIString(IDS_YES, szText, MAX_STR_LEN);
        else
            LoadMUIString(IDS_NO, szText, MAX_STR_LEN);

        IoSetItemText(Index, 1, szText);

        /* Share name */
        if (SafeStrLen(pPrinterInfo[dwIndex].pShareName) > 1)
        {
            Index = IoAddValueName(1, IDS_PRINTER_SHARENAME, 0);
            IoSetItemText(Index, 1, pPrinterInfo[dwIndex].pShareName);
        }

        /* Port name */
        if (SafeStrLen(pPrinterInfo[dwIndex].pPortName) > 1)
        {
            Index = IoAddValueName(1, IDS_PRINTER_PORT, 0);
            IoSetItemText(Index, 1, pPrinterInfo[dwIndex].pPortName);
        }

        /* Driver name */
        if (SafeStrLen(pPrinterInfo[dwIndex].pDriverName) > 1)
        {
            Index = IoAddValueName(1, IDS_PRINTER_DRIVER, 0);
            IoSetItemText(Index, 1, pPrinterInfo[dwIndex].pDriverName);
        }

        /* Device name */
        if (SafeStrLen(pDevMode->dmDeviceName) > 1)
        {
            Index = IoAddValueName(1, IDS_PRINTER_DEVICENAME, 0);
            IoSetItemText(Index, 1, pDevMode->dmDeviceName);
        }

        /* Print processor */
        if (SafeStrLen(pPrinterInfo[dwIndex].pPrintProcessor) > 1)
        {
            Index = IoAddValueName(1, IDS_PRINTER_PROCESSOR, 0);
            IoSetItemText(Index, 1, pPrinterInfo[dwIndex].pPrintProcessor);
        }

        /* Data type */
        if (SafeStrLen(pPrinterInfo[dwIndex].pDatatype) > 1)
        {
            Index = IoAddValueName(1, IDS_PRINTER_DATATYPE, 0);
            IoSetItemText(Index, 1, pPrinterInfo[dwIndex].pDatatype);
        }

        /* Server name */
        if (pPrinterInfo[dwIndex].pServerName &&
            SafeStrLen(pPrinterInfo[dwIndex].pServerName) > 1)
        {
            Index = IoAddValueName(1, IDS_PRINTER_SERVER, 0);
            IoSetItemText(Index, 1, pPrinterInfo[dwIndex].pServerName);
        }

        /* Location */
        if (SafeStrLen(pPrinterInfo[dwIndex].pLocation) > 1)
        {
            Index = IoAddValueName(1, IDS_PRINTER_LOCATION, 0);
            IoSetItemText(Index, 1, pPrinterInfo[dwIndex].pLocation);
        }

        /* Comment */
        if (SafeStrLen(pPrinterInfo[dwIndex].pComment) > 1)
        {
            Index = IoAddValueName(1, IDS_PRINTER_COMMENT, 0);
            IoSetItemText(Index, 1, pPrinterInfo[dwIndex].pComment);
        }

        /* Jobs count */
        Index = IoAddValueName(1, IDS_PRINTER_JOBS_COUNT, 0);
        StringCbPrintf(szText, sizeof(szText), L"%d",
                       pPrinterInfo[dwIndex].cJobs);
        IoSetItemText(Index, 1, szText);

        if (pDevMode)
        {
            /* Paper size */
            if (pDevMode->dmPaperWidth && pDevMode->dmPaperLength)
            {
                Index = IoAddValueName(1, IDS_PRINTER_PAPER_SIZE, 0);
                StringCbPrintf(szText, sizeof(szText), L"%ld x %ld mm",
                               pDevMode->dmPaperWidth / 10,
                               pDevMode->dmPaperLength / 10);
                IoSetItemText(Index, 1, szText);
            }

            /* Quality */
            if (pDevMode->dmPrintQuality)
            {
                Index = IoAddValueName(1, IDS_PRINTER_QUALITY, 0);
                StringCbPrintf(szText, sizeof(szText), L"%ld x %ld dpi",
                               pDevMode->dmPrintQuality,
                               pDevMode->dmPrintQuality);
                IoSetItemText(Index, 1, szText);
            }

            /* Orientation */
            Index = IoAddValueName(1, IDS_PRINTER_ORIENTATION, 0);
            if (pDevMode->dmOrientation == DMORIENT_PORTRAIT)
                LoadMUIString(IDS_PRINTER_PORTRAIT, szText, MAX_STR_LEN);
            else
                LoadMUIString(IDS_PRINTER_LANDSCAPE, szText, MAX_STR_LEN);

            IoSetItemText(Index, 1, szText);
        }

        IoAddFooter();

        if (IsCanceled) break;
    }

    Free(pPrinterInfo);
}

VOID
HW_PrintersInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_PRINTER);

    EnumPrintersInfo(PRINTER_ENUM_FAVORITE | PRINTER_ENUM_LOCAL |
                     PRINTER_ENUM_NETWORK);

    DebugEndReceiving();
}
