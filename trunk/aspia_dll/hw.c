/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/hw.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"
#include <intrin.h>
#include <gl/gl.h>
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
    if (Result->IsCritical)
    {
        INT IconIndex;

        if (Result->dwWarrantyThreshold > 0 && Result->dwAttrValue < Result->dwWarrantyThreshold &&
            Result->dwAttrValue > 0)
            IconIndex = 2;
        else
            IconIndex = 1;

        Index = IoAddItem(0, IconIndex, szText);
    }
    else
    {
        Index = IoAddItem(0, 1, szText);
    }

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
        hHandle = OpenSmart(bIndex);
        if (hHandle == INVALID_HANDLE_VALUE) continue;

        if (ReadSmartInfo(hHandle, bIndex, &DriveInfo))
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

            if (GetSmartDiskGeometry(bIndex, &DiskGeometry))
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

        CloseSmart(hHandle);
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
    IoAddIcon(IDI_INFOICO);
    IoAddIcon(IDI_BANG);

    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        hHandle = OpenSmart(bIndex);
        if (hHandle == INVALID_HANDLE_VALUE) continue;

        if (ReadSmartInfo(hHandle, bIndex, &DriveInfo))
        {
            Index = IoAddItem(0, 0, L"\t\0");
            ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                            sizeof(DriveInfo.sModelNumber));
            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sModelNumber);
            ChopSpaces(szText, sizeof(szText));
            IoSetItemText(Index, 1, szText);
            IoSetItemText(Index, 2, L"\t\0");
            IoSetItemText(Index, 3, L"\t\0");
            IoSetItemText(Index, 4, L"\t\0");
            IoSetItemText(Index, 5, L"\t\0");
        }

        if (EnumSmartData(hHandle, bIndex, EnumSmartDataProc))
            IoAddFooter();
        CloseSmart(hHandle);
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
                   DllParams.szCurrentPath,
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
            ConvertSecondsToString(DllParams.hLangInst,
                                   PowerStatus.BatteryFullLifeTime,
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
            ConvertSecondsToString(DllParams.hLangInst,
                                   PowerStatus.BatteryLifeTime,
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

#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB  0x851C
#define GL_MAX_3D_TEXTURE_SIZE_EXT        0x8073
#define GL_MAX_TEXTURE_LOD_BIAS_EXT       0x84FD
#define GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB 0x84F8
#define GL_MAX_DRAW_BUFFERS_ATI           0x8824
#define GL_MAX_TEXTURE_UNITS              0x84E2

typedef struct
{
    LPWSTR lpExtName;
    BOOL IsSupported;
}
OPENGL_EXTENSIONS;

OPENGL_EXTENSIONS OpenglExtensions[] =
{
    { L"GL_3DFX_multisample", FALSE },
    { L"GL_3DFX_tbuffer", FALSE },
    { L"GL_3DFX_texture_compression_FXT1", FALSE },
    { L"GL_3DL_direct_texture_access2", FALSE },
    { L"GL_3Dlabs_multisample_transparency_id", FALSE },
    { L"GL_3Dlabs_multisample_transparency_range", FALSE },
    { L"GL_AMD_performance_monitor", FALSE },
    { L"GL_AMD_texture_texture4", FALSE },
    { L"GL_AMDX_vertex_shader_tessellator", FALSE },
    { L"GL_APPLE_aux_depth_stencil", FALSE },
    { L"GL_APPLE_client_storage", FALSE },
    { L"GL_APPLE_element_array", FALSE },
    { L"GL_APPLE_fence", FALSE },
    { L"GL_APPLE_float_pixels", FALSE },
    { L"GL_APPLE_flush_buffer_range", FALSE },
    { L"GL_APPLE_flush_render", FALSE },
    { L"GL_APPLE_packed_pixel", FALSE },
    { L"GL_APPLE_packed_pixels", FALSE },
    { L"GL_APPLE_pixel_buffer", FALSE },
    { L"GL_APPLE_specular_vector", FALSE },
    { L"GL_APPLE_texture_range", FALSE },
    { L"GL_APPLE_transform_hint", FALSE },
    { L"GL_APPLE_vertex_array_object", FALSE },
    { L"GL_APPLE_vertex_array_range", FALSE },
    { L"GL_APPLE_vertex_program_evaluators", FALSE },
    { L"GL_APPLE_ycbcr_422", FALSE },
    { L"GL_ARB_color_buffer_float", FALSE },
    { L"GL_ARB_depth_buffer_float", FALSE },
    { L"GL_ARB_depth_texture", FALSE },
    { L"GL_ARB_draw_buffers", FALSE },
    { L"GL_ARB_draw_instanced", FALSE },
    { L"GL_ARB_fragment_program", FALSE },
    { L"GL_ARB_fragment_program_shadow", FALSE },
    { L"GL_ARB_fragment_shader", FALSE },
    { L"GL_ARB_framebuffer_object", FALSE },
    { L"GL_ARB_framebuffer_sRGB", FALSE },
    { L"GL_ARB_geometry_shader4", FALSE },
    { L"GL_ARB_half_float_pixel", FALSE },
    { L"GL_ARB_half_float_vertex", FALSE },
    { L"GL_ARB_imaging", FALSE },
    { L"GL_ARB_instanced_arrays", FALSE },
    { L"GL_ARB_map_buffer_range", FALSE },
    { L"GL_ARB_matrix_palette", FALSE },
    { L"GL_ARB_multisample", FALSE },
    { L"GL_ARB_multitexture", FALSE },
    { L"GL_ARB_occlusion_query", FALSE },
    { L"GL_ARB_pixel_buffer_object", FALSE },
    { L"GL_ARB_point_parameters", FALSE },
    { L"GL_ARB_point_sprite", FALSE },
    { L"GL_ARB_shader_objects", FALSE },
    { L"GL_ARB_shader_texture_lod", FALSE },
    { L"GL_ARB_shading_language_100", FALSE },
    { L"GL_ARB_shadow", FALSE },
    { L"GL_ARB_shadow_ambient", FALSE },
    { L"GL_ARB_texture_border_clamp", FALSE },
    { L"GL_ARB_texture_buffer_object", FALSE },
    { L"GL_ARB_texture_compression", FALSE },
    { L"GL_ARB_texture_compression_rgtc", FALSE },
    { L"GL_ARB_texture_cube_map", FALSE },
    { L"GL_ARB_texture_env_add", FALSE },
    { L"GL_ARB_texture_env_combine", FALSE },
    { L"GL_ARB_texture_env_crossbar", FALSE },
    { L"GL_ARB_texture_env_dot3", FALSE },
    { L"GL_ARB_texture_float", FALSE },
    { L"GL_ARB_texture_mirrored_repeat", FALSE },
    { L"GL_ARB_texture_non_power_of_two", FALSE },
    { L"GL_ARB_texture_rectangle", FALSE },
    { L"GL_ARB_texture_rg", FALSE },
    { L"GL_ARB_transpose_matrix", FALSE },
    { L"GL_ARB_vertex_array_object", FALSE },
    { L"GL_ARB_vertex_blend", FALSE },
    { L"GL_ARB_vertex_buffer_object", FALSE },
    { L"GL_ARB_vertex_program", FALSE },
    { L"GL_ARB_vertex_shader", FALSE },
    { L"GL_ARB_window_pos", FALSE },
    { L"GL_ATI_array_rev_comps_in_4_bytes", FALSE },
    { L"GL_ATI_blend_equation_separate", FALSE },
    { L"GL_ATI_blend_weighted_minmax", FALSE },
    { L"GL_ATI_draw_buffers", FALSE },
    { L"GL_ATI_element_array", FALSE },
    { L"GL_ATI_envmap_bumpmap", FALSE },
    { L"GL_ATI_fragment_shader", FALSE },
    { L"GL_ATI_lock_texture", FALSE },
    { L"GL_ATI_map_object_buffer", FALSE },
    { L"GL_ATI_meminfo", FALSE },
    { L"GL_ATI_pixel_format_float", FALSE },
    { L"GL_ATI_pn_triangles", FALSE },
    { L"GL_ATI_point_cull_mode", FALSE },
    { L"GL_ATI_separate_stencil", FALSE },
    { L"GL_ATI_shader_texture_lod", FALSE },
    { L"GL_ATI_text_fragment_shader", FALSE },
    { L"GL_ATI_texture_compression_3dc", FALSE },
    { L"GL_ATI_texture_env_combine3", FALSE },
    { L"GL_ATI_texture_float", FALSE },
    { L"GL_ATI_texture_mirror_once", FALSE },
    { L"GL_ATI_vertex_array_object", FALSE },
    { L"GL_ATI_vertex_attrib_array_object", FALSE },
    { L"GL_ATI_vertex_blend", FALSE },
    { L"GL_ATI_vertex_shader", FALSE },
    { L"GL_ATI_vertex_streams", FALSE },
    { L"GL_ATIX_pn_triangles", FALSE },
    { L"GL_ATIX_texture_env_combine3", FALSE },
    { L"GL_ATIX_texture_env_route", FALSE },
    { L"GL_ATIX_vertex_shader_output_point_size", FALSE },
    { L"GL_Autodesk_facet_normal", FALSE },
    { L"GL_Autodesk_valid_back_buffer_hint", FALSE },
    { L"GL_DIMD_YUV", FALSE },
    { L"GL_EXT_422_pixels", FALSE },
    { L"GL_EXT_abgr", FALSE },
    { L"GL_EXT_bgra", FALSE },
    { L"GL_EXT_bindable_uniform", FALSE },
    { L"GL_EXT_blend_color", FALSE },
    { L"GL_EXT_blend_equation_separate", FALSE },
    { L"GL_EXT_blend_func_separate", FALSE },
    { L"GL_EXT_blend_logic_op", FALSE },
    { L"GL_EXT_blend_minmax", FALSE },
    { L"GL_EXT_blend_subtract", FALSE },
    { L"GL_EXT_Cg_shader", FALSE },
    { L"GL_EXT_clip_volume_hint", FALSE },
    { L"GL_EXT_cmyka", FALSE },
    { L"GL_EXT_color_matrix", FALSE },
    { L"GL_EXT_color_subtable", FALSE },
    { L"GL_EXT_color_table", FALSE },
    { L"GL_EXT_compiled_vertex_array", FALSE },
    { L"GL_EXT_convolution", FALSE },
    { L"GL_EXT_convolution_border_modes", FALSE },
    { L"GL_EXT_coordinate_frame", FALSE },
    { L"GL_EXT_copy_texture", FALSE },
    { L"GL_EXT_cull_vertex", FALSE },
    { L"GL_EXT_depth_bounds_test", FALSE },
    { L"GL_EXT_depth_buffer_float", FALSE },
    { L"GL_EXT_direct_state_access", FALSE },
    { L"GL_EXT_draw_buffers2", FALSE },
    { L"GL_EXT_draw_instanced", FALSE },
    { L"GL_EXT_draw_range_elements", FALSE },
    { L"GL_EXT_fog_coord", FALSE },
    { L"GL_EXT_fog_function", FALSE },
    { L"GL_EXT_fog_offset", FALSE },
    { L"GL_EXT_fragment_lighting", FALSE },
    { L"GL_EXT_framebuffer_blit", FALSE },
    { L"GL_EXT_framebuffer_multisample", FALSE },
    { L"GL_EXT_framebuffer_object", FALSE },
    { L"GL_EXT_framebuffer_sRGB", FALSE },
    { L"GL_EXT_generate_mipmap", FALSE },
    { L"GL_EXT_geometry_shader4", FALSE },
    { L"GL_EXT_gpu_program_parameters", FALSE },
    { L"GL_EXT_gpu_shader4", FALSE },
    { L"GL_EXT_histogram", FALSE },
    { L"GL_EXT_index_array_formats", FALSE },
    { L"GL_EXT_index_func", FALSE },
    { L"GL_EXT_index_material", FALSE },
    { L"GL_EXT_index_texture", FALSE },
    { L"GL_EXT_interlace", FALSE },
    { L"GL_EXT_light_texture", FALSE },
    { L"GL_EXT_misc_attribute", FALSE },
    { L"GL_EXT_multi_draw_arrays", FALSE },
    { L"GL_EXT_multisample", FALSE },
    { L"GL_EXT_packed_depth_stencil", FALSE },
    { L"GL_EXT_packed_float", FALSE },
    { L"GL_EXT_packed_pixels", FALSE },
    { L"GL_EXT_packed_pixels_12", FALSE },
    { L"GL_EXT_paletted_texture", FALSE },
    { L"GL_EXT_pixel_buffer_object", FALSE },
    { L"GL_EXT_pixel_format", FALSE },
    { L"GL_EXT_pixel_texture", FALSE },
    { L"GL_EXT_pixel_transform", FALSE },
    { L"GL_EXT_pixel_transform_color_table", FALSE },
    { L"GL_EXT_point_parameters", FALSE },
    { L"GL_EXT_polygon_offset", FALSE },
    { L"GL_EXT_rescale_normal", FALSE },
    { L"GL_EXT_scene_marker", FALSE },
    { L"GL_EXT_secondary_color", FALSE },
    { L"GL_EXT_separate_specular_color", FALSE },
    { L"GL_EXT_shadow_funcs", FALSE },
    { L"GL_EXT_shared_texture_palette", FALSE },
    { L"GL_EXT_stencil_clear_tag", FALSE },
    { L"GL_EXT_stencil_two_side", FALSE },
    { L"GL_EXT_stencil_wrap", FALSE },
    { L"GL_EXT_subtexture", FALSE },
    { L"GL_EXT_swap_control", FALSE },
    { L"GL_EXT_texgen_reflection", FALSE },
    { L"GL_EXT_texture", FALSE },
    { L"GL_EXT_texture_array", FALSE },
    { L"GL_EXT_texture_border_clamp", FALSE },
    { L"GL_EXT_texture_buffer_object", FALSE },
    { L"GL_EXT_texture_color_table", FALSE },
    { L"GL_EXT_texture_compression_dxt1", FALSE },
    { L"GL_EXT_texture_compression_latc", FALSE },
    { L"GL_EXT_texture_compression_rgtc", FALSE },
    { L"GL_EXT_texture_compression_s3tc", FALSE },
    { L"GL_EXT_texture_cube_map", FALSE },
    { L"GL_EXT_texture_edge_clamp", FALSE },
    { L"GL_EXT_texture_env", FALSE },
    { L"GL_EXT_texture_env_add", FALSE },
    { L"GL_EXT_texture_env_combine", FALSE },
    { L"GL_EXT_texture_env_dot3", FALSE },
    { L"GL_EXT_texture_filter_anisotropic", FALSE },
    { L"GL_EXT_texture_integer", FALSE },
    { L"GL_EXT_texture_lod", FALSE },
    { L"GL_EXT_texture_lod_bias", FALSE },
    { L"GL_EXT_texture_mirror_clamp", FALSE },
    { L"GL_EXT_texture_object", FALSE },
    { L"GL_EXT_texture_perturb_normal", FALSE },
    { L"GL_EXT_texture_rectangle", FALSE },
    { L"GL_EXT_texture_shared_exponent", FALSE },
    { L"GL_EXT_texture_sRGB", FALSE },
    { L"GL_EXT_texture_swizzle", FALSE },
    { L"GL_EXT_texture3D", FALSE },
    { L"GL_EXT_texture4D", FALSE },
    { L"GL_EXT_timer_query", FALSE },
    { L"GL_EXT_transform_feedback", FALSE },
    { L"GL_EXT_vertex_array", FALSE },
    { L"GL_EXT_vertex_array_bgra", FALSE },
    { L"GL_EXT_vertex_shader", FALSE },
    { L"GL_EXT_vertex_weighting", FALSE },
    { L"GL_EXTX_framebuffer_mixed_formats", FALSE },
    { L"GL_EXTX_packed_depth_stencil", FALSE },
    { L"GL_FGL_lock_texture", FALSE },
    { L"GL_GL2_geometry_shader", FALSE },
    { L"GL_GREMEDY_frame_terminator", FALSE },
    { L"GL_GREMEDY_string_marker", FALSE },
    { L"GL_HP_convolution_border_modes", FALSE },
    { L"GL_HP_image_transform", FALSE },
    { L"GL_HP_occlusion_test", FALSE },
    { L"GL_HP_texture_lighting", FALSE },
    { L"GL_I3D_argb", FALSE },
    { L"GL_I3D_color_clamp", FALSE },
    { L"GL_I3D_interlace_read", FALSE },
    { L"GL_IBM_clip_check", FALSE },
    { L"GL_IBM_cull_vertex", FALSE },
    { L"GL_IBM_load_named_matrix", FALSE },
    { L"GL_IBM_multi_draw_arrays", FALSE },
    { L"GL_IBM_multimode_draw_arrays", FALSE },
    { L"GL_IBM_occlusion_cull", FALSE },
    { L"GL_IBM_pixel_filter_hint", FALSE },
    { L"GL_IBM_rasterpos_clip", FALSE },
    { L"GL_IBM_rescale_normal", FALSE },
    { L"GL_IBM_static_data", FALSE },
    { L"GL_IBM_texture_clamp_nodraw", FALSE },
    { L"GL_IBM_texture_mirrored_repeat", FALSE },
    { L"GL_IBM_vertex_array_lists", FALSE },
    { L"GL_IBM_YCbCr", FALSE },
    { L"GL_INGR_blend_func_separate", FALSE },
    { L"GL_INGR_color_clamp", FALSE },
    { L"GL_INGR_interlace_read", FALSE },
    { L"GL_INGR_multiple_palette", FALSE },
    { L"GL_INTEL_parallel_arrays", FALSE },
    { L"GL_INTEL_texture_scissor", FALSE },
    { L"GL_KTX_buffer_region", FALSE },
    { L"GL_MESA_pack_invert", FALSE },
    { L"GL_MESA_resize_buffers", FALSE },
    { L"GL_MESA_window_pos", FALSE },
    { L"GL_MESA_ycbcr_texture", FALSE },
    { L"GL_MESAX_texture_stack", FALSE },
    { L"GL_MTX_fragment_shader", FALSE },
    { L"GL_MTX_precision_dpi", FALSE },
    { L"GL_NV_blend_square", FALSE },
    { L"GL_NV_centroid_sample", FALSE },
    { L"GL_NV_conditional_render", FALSE },
    { L"GL_NV_copy_depth_to_color", FALSE },
    { L"GL_NV_depth_buffer_float", FALSE },
    { L"GL_NV_depth_clamp", FALSE },
    { L"GL_NV_depth_range_unclamped", FALSE },
    { L"GL_NV_evaluators", FALSE },
    { L"GL_NV_explicit_multisample", FALSE },
    { L"GL_NV_fence", FALSE },
    { L"GL_NV_float_buffer", FALSE },
    { L"GL_NV_fog_distance", FALSE },
    { L"GL_NV_fragment_program", FALSE },
    { L"GL_NV_fragment_program_option", FALSE },
    { L"GL_NV_fragment_program2", FALSE },
    { L"GL_NV_fragment_program4", FALSE },
    { L"GL_NV_framebuffer_multisample_coverage", FALSE },
    { L"GL_NV_framebuffer_multisample_ex", FALSE },
    { L"GL_NV_geometry_program4", FALSE },
    { L"GL_NV_geometry_shader4", FALSE },
    { L"GL_NV_gpu_program4", FALSE },
    { L"GL_NV_half_float", FALSE },
    { L"GL_NV_light_max_exponent", FALSE },
    { L"GL_NV_multisample_coverage", FALSE },
    { L"GL_NV_multisample_filter_hint", FALSE },
    { L"GL_NV_occlusion_query", FALSE },
    { L"GL_NV_packed_depth_stencil", FALSE },
    { L"GL_NV_parameter_buffer_object", FALSE },
    { L"GL_NV_pixel_buffer_object", FALSE },
    { L"GL_NV_pixel_data_range", FALSE },
    { L"GL_NV_point_sprite", FALSE },
    { L"GL_NV_present_video", FALSE },
    { L"GL_NV_primitive_restart", FALSE },
    { L"GL_NV_register_combiners", FALSE },
    { L"GL_NV_register_combiners2", FALSE },
    { L"GL_NV_texgen_emboss", FALSE },
    { L"GL_NV_texgen_reflection", FALSE },
    { L"GL_NV_texture_compression_latc", FALSE },
    { L"GL_NV_texture_compression_vtc", FALSE },
    { L"GL_NV_texture_env_combine4", FALSE },
    { L"GL_NV_texture_expand_normal", FALSE },
    { L"GL_NV_texture_rectangle", FALSE },
    { L"GL_NV_texture_shader", FALSE },
    { L"GL_NV_texture_shader2", FALSE },
    { L"GL_NV_texture_shader3", FALSE },
    { L"GL_NV_timer_query", FALSE },
    { L"GL_NV_transform_feedback", FALSE },
    { L"GL_NV_transform_feedback2", FALSE },
    { L"GL_NV_vertex_array_range", FALSE },
    { L"GL_NV_vertex_array_range2", FALSE },
    { L"GL_NV_vertex_program", FALSE },
    { L"GL_NV_vertex_program1_1", FALSE },
    { L"GL_NV_vertex_program2", FALSE },
    { L"GL_NV_vertex_program2_option", FALSE },
    { L"GL_NV_vertex_program3", FALSE },
    { L"GL_NV_vertex_program4", FALSE },
    { L"GL_NVX_conditional_render", FALSE },
    { L"GL_NVX_flush_hold", FALSE },
    { L"GL_NVX_ycrcb", FALSE },
    { L"GL_OES_byte_coordinates", FALSE },
    { L"GL_OES_compressed_paletted_texture", FALSE },
    { L"GL_OES_fixed_point", FALSE },
    { L"GL_OES_query_matrix", FALSE },
    { L"GL_OES_read_format", FALSE },
    { L"GL_OES_single_precision", FALSE },
    { L"GL_OML_interlace", FALSE },
    { L"GL_OML_resample", FALSE },
    { L"GL_OML_subsample", FALSE },
    { L"GL_PGI_misc_hints", FALSE },
    { L"GL_PGI_vertex_hints", FALSE },
    { L"GL_REND_screen_coordinates", FALSE },
    { L"GL_S3_performance_analyzer", FALSE },
    { L"GL_S3_s3tc", FALSE },
    { L"GL_SGI_color_matrix", FALSE },
    { L"GL_SGI_color_table", FALSE },
    { L"GL_SGI_compiled_vertex_array", FALSE },
    { L"GL_SGI_cull_vertex", FALSE },
    { L"GL_SGI_index_array_formats", FALSE },
    { L"GL_SGI_index_func", FALSE },
    { L"GL_SGI_index_material", FALSE },
    { L"GL_SGI_index_texture", FALSE },
    { L"GL_SGI_make_current_read", FALSE },
    { L"GL_SGI_texture_add_env", FALSE },
    { L"GL_SGI_texture_color_table", FALSE },
    { L"GL_SGI_texture_edge_clamp", FALSE },
    { L"GL_SGI_texture_lod", FALSE },
    { L"GL_SGIS_color_range", FALSE },
    { L"GL_SGIS_detail_texture", FALSE },
    { L"GL_SGIS_fog_function", FALSE },
    { L"GL_SGIS_generate_mipmap", FALSE },
    { L"GL_SGIS_multisample", FALSE },
    { L"GL_SGIS_multitexture", FALSE },
    { L"GL_SGIS_pixel_texture", FALSE },
    { L"GL_SGIS_point_line_texgen", FALSE },
    { L"GL_SGIS_sharpen_texture", FALSE },
    { L"GL_SGIS_texture_border_clamp", FALSE },
    { L"GL_SGIS_texture_color_mask", FALSE },
    { L"GL_SGIS_texture_edge_clamp", FALSE },
    { L"GL_SGIS_texture_filter4", FALSE },
    { L"GL_SGIS_texture_lod", FALSE },
    { L"GL_SGIS_texture_select", FALSE },
    { L"GL_SGIS_texture4D", FALSE },
    { L"GL_SGIX_async", FALSE },
    { L"GL_SGIX_async_histogram", FALSE },
    { L"GL_SGIX_async_pixel", FALSE },
    { L"GL_SGIX_blend_alpha_minmax", FALSE },
    { L"GL_SGIX_clipmap", FALSE },
    { L"GL_SGIX_convolution_accuracy", FALSE },
    { L"GL_SGIX_depth_pass_instrument", FALSE },
    { L"GL_SGIX_depth_texture", FALSE },
    { L"GL_SGIX_flush_raster", FALSE },
    { L"GL_SGIX_fog_offset", FALSE },
    { L"GL_SGIX_framezoom", FALSE },
    { L"GL_SGIX_instruments", FALSE },
    { L"GL_SGIX_interlace", FALSE },
    { L"GL_SGIX_ir_instrument1", FALSE },
    { L"GL_SGIX_list_priority", FALSE },
    { L"GL_SGIX_pbuffer", FALSE },
    { L"GL_SGIX_pixel_texture", FALSE },
    { L"GL_SGIX_pixel_texture_bits", FALSE },
    { L"GL_SGIX_reference_plane", FALSE },
    { L"GL_SGIX_resample", FALSE },
    { L"GL_SGIX_shadow", FALSE },
    { L"GL_SGIX_shadow_ambient", FALSE },
    { L"GL_SGIX_sprite", FALSE },
    { L"GL_SGIX_subsample", FALSE },
    { L"GL_SGIX_tag_sample_buffer", FALSE },
    { L"GL_SGIX_texture_add_env", FALSE },
    { L"GL_SGIX_texture_coordinate_clamp", FALSE },
    { L"GL_SGIX_texture_lod_bias", FALSE },
    { L"GL_SGIX_texture_multi_buffer", FALSE },
    { L"GL_SGIX_texture_range", FALSE },
    { L"GL_SGIX_texture_scale_bias", FALSE },
    { L"GL_SGIX_vertex_preclip", FALSE },
    { L"GL_SGIX_vertex_preclip_hint", FALSE },
    { L"GL_SGIX_ycrcb", FALSE },
    { L"GL_SGIX_ycrcb_subsample", FALSE },
    { L"GL_SUN_convolution_border_modes", FALSE },
    { L"GL_SUN_global_alpha", FALSE },
    { L"GL_SUN_mesh_array", FALSE },
    { L"GL_SUN_multi_draw_arrays", FALSE },
    { L"GL_SUN_slice_accum", FALSE },
    { L"GL_SUN_triangle_list", FALSE },
    { L"GL_SUN_vertex", FALSE },
    { L"GL_SUNX_constant_data", FALSE },
    { L"GL_WGL_ARB_extensions_string", FALSE },
    { L"GL_WGL_EXT_extensions_string", FALSE },
    { L"GL_WGL_EXT_swap_control", FALSE },
    { L"GL_WIN_phong_shading", FALSE },
    { L"GL_WIN_specular_fog", FALSE },
    { L"GL_WIN_swap_hint", FALSE },
    { L"GLU_EXT_nurbs_tessellator", FALSE },
    { L"GLU_EXT_object_space_tess", FALSE },
    { L"GLU_SGI_filter4_parameters", FALSE },
    { L"GLX_ARB_create_context", FALSE },
    { L"GLX_ARB_fbconfig_float", FALSE },
    { L"GLX_ARB_framebuffer_sRGB", FALSE },
    { L"GLX_ARB_get_proc_address", FALSE },
    { L"GLX_ARB_multisample", FALSE },
    { L"GLX_EXT_fbconfig_packed_float", FALSE },
    { L"GLX_EXT_framebuffer_sRGB", FALSE },
    { L"GLX_EXT_import_context", FALSE },
    { L"GLX_EXT_scene_marker", FALSE },
    { L"GLX_EXT_texture_from_pixmap", FALSE },
    { L"GLX_EXT_visual_info", FALSE },
    { L"GLX_EXT_visual_rating", FALSE },
    { L"GLX_MESA_agp_offset", FALSE },
    { L"GLX_MESA_copy_sub_buffer", FALSE },
    { L"GLX_MESA_pixmap_colormap", FALSE },
    { L"GLX_MESA_release_buffers", FALSE },
    { L"GLX_MESA_set_3dfx_mode", FALSE },
    { L"GLX_NV_present_video", FALSE },
    { L"GLX_NV_swap_group", FALSE },
    { L"GLX_NV_video_output", FALSE },
    { L"GLX_OML_swap_method", FALSE },
    { L"GLX_OML_sync_control", FALSE },
    { L"GLX_SGI_cushion", FALSE },
    { L"GLX_SGI_make_current_read", FALSE },
    { L"GLX_SGI_swap_control", FALSE },
    { L"GLX_SGI_video_sync", FALSE },
    { L"GLX_SGIS_blended_overlay", FALSE },
    { L"GLX_SGIS_color_range", FALSE },
    { L"GLX_SGIS_multisample", FALSE },
    { L"GLX_SGIX_dm_buffer", FALSE },
    { L"GLX_SGIX_fbconfig", FALSE },
    { L"GLX_SGIX_hyperpipe", FALSE },
    { L"GLX_SGIX_pbuffer", FALSE },
    { L"GLX_SGIX_swap_barrier", FALSE },
    { L"GLX_SGIX_swap_group", FALSE },
    { L"GLX_SGIX_video_resize", FALSE },
    { L"GLX_SGIX_video_source", FALSE },
    { L"GLX_SGIX_visual_select_group", FALSE },
    { L"GLX_SUN_get_transparent_index", FALSE },
    { L"GLX_SUN_video_resize", FALSE },
    { L"WGL_3DFX_gamma_control", FALSE },
    { L"WGL_3DFX_multisample", FALSE },
    { L"WGL_3DL_stereo_control", FALSE },
    { L"WGL_ARB_buffer_region", FALSE },
    { L"WGL_ARB_create_context", FALSE },
    { L"WGL_ARB_extensions_string", FALSE },
    { L"WGL_ARB_framebuffer_sRGB", FALSE },
    { L"WGL_ARB_make_current_read", FALSE },
    { L"WGL_ARB_multisample", FALSE },
    { L"WGL_ARB_pbuffer", FALSE },
    { L"WGL_ARB_pixel_format", FALSE },
    { L"WGL_ARB_pixel_format_float", FALSE },
    { L"WGL_ARB_render_texture", FALSE },
    { L"WGL_ATI_pbuffer_memory_hint", FALSE },
    { L"WGL_ATI_pixel_format_float", FALSE },
    { L"WGL_ATI_render_texture_rectangle", FALSE },
    { L"WGL_EXT_buffer_region", FALSE },
    { L"WGL_EXT_depth_float", FALSE },
    { L"WGL_EXT_display_color_table", FALSE },
    { L"WGL_EXT_extensions_string", FALSE },
    { L"WGL_EXT_framebuffer_sRGB", FALSE },
    { L"WGL_EXT_framebuffer_sRGBWGL_ARB_create_context", FALSE },
    { L"WGL_EXT_gamma_control", FALSE },
    { L"WGL_EXT_make_current_read", FALSE },
    { L"WGL_EXT_multisample", FALSE },
    { L"WGL_EXT_pbuffer", FALSE },
    { L"WGL_EXT_pixel_format", FALSE },
    { L"WGL_EXT_pixel_format_packed_float", FALSE },
    { L"WGL_EXT_render_texture", FALSE },
    { L"WGL_EXT_swap_control", FALSE },
    { L"WGL_EXT_swap_interval", FALSE },
    { L"WGL_I3D_digital_video_control", FALSE },
    { L"WGL_I3D_gamma", FALSE },
    { L"WGL_I3D_genlock", FALSE },
    { L"WGL_I3D_image_buffer", FALSE },
    { L"WGL_I3D_swap_frame_lock", FALSE },
    { L"WGL_I3D_swap_frame_usage", FALSE },
    { L"WGL_MTX_video_preview", FALSE },
    { L"WGL_NV_float_buffer", FALSE },
    { L"WGL_NV_gpu_affinity", FALSE },
    { L"WGL_NV_multisample_coverage", FALSE },
    { L"WGL_NV_present_video", FALSE },
    { L"WGL_NV_render_depth_texture", FALSE },
    { L"WGL_NV_render_texture_rectangle", FALSE },
    { L"WGL_NV_swap_group", FALSE },
    { L"WGL_NV_video_output", FALSE },
    { L"WGL_OML_sync_control", FALSE },
    {0}
};

VOID
HW_OpenGlInfo(VOID)
{
    PIXELFORMATDESCRIPTOR PixelFormatDesc = {0};
    WCHAR szText[MAX_STR_LEN], szSupported[MAX_STR_LEN],
          szUnsupported[MAX_STR_LEN];
    INT Index, iPixelFormat, i_data, len;
    HGLRC hRC;
    HDC hDC = GetDC(DllParams.hMainWnd);
    char *data;

    DebugStartReceiving();

    IoAddIcon(IDI_OPENGL);
    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);

    LoadMUIString(IDS_CPUID_SUPPORTED, szSupported, MAX_STR_LEN);
    LoadMUIString(IDS_CPUID_UNSUPPORTED, szUnsupported, MAX_STR_LEN);

    /* OpenGL Initializing */
    PixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    PixelFormatDesc.nVersion = 1;
    PixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    PixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
    PixelFormatDesc.iLayerType = PFD_MAIN_PLANE;
    PixelFormatDesc.cDepthBits = 16;
    iPixelFormat = ChoosePixelFormat(hDC, &PixelFormatDesc);
    SetPixelFormat(hDC, iPixelFormat, &PixelFormatDesc);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    IoAddHeader(0, IDS_OPENGL_PROP, 0);

    data = (char *)glGetString(GL_VENDOR);
    if (data)
    {
        Index = IoAddValueName(1, IDS_OPENGL_VENDOR, 0);
        StringCbPrintf(szText, sizeof(szText), L"%S", data);
        IoSetItemText(Index, 1, szText);
    }

    data = (char *)glGetString(GL_RENDERER);
    if (data)
    {
        Index = IoAddItem(1, 0, L"Renderer");
        StringCbPrintf(szText, sizeof(szText), L"%S", data);
        IoSetItemText(Index, 1, szText);
    }

    data = (char *)glGetString(GL_VERSION);
    if (data)
    {
        Index = IoAddValueName(1, IDS_OPENGL_VERSION, 0);
        StringCbPrintf(szText, sizeof(szText), L"%S", data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Multitexture Texture Units");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data, i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_SUBPIXEL_BITS, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Sub-Pixel Precision");
        StringCbPrintf(szText, sizeof(szText), L"%d-bit", i_data, i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Viewport Size");
        StringCbPrintf(szText, sizeof(szText), L"%d x %d", i_data, i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Cube Map Texture Size");
        StringCbPrintf(szText, sizeof(szText), L"%d x %d", i_data, i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Rectangle Texture Size");
        StringCbPrintf(szText, sizeof(szText), L"%d x %d", i_data, i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_EXT, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max 3D Texture Size");
        StringCbPrintf(szText, sizeof(szText), L"%d x %d x %d",
                       i_data, i_data, i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_CLIP_PLANES, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Clipping Planes");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_LIST_NESTING, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Display-List Nesting Level");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_DRAW_BUFFERS_ATI, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Draw Buffers");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_EVAL_ORDER, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Evaluator Order");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_LIGHTS, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Light Sources");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_PIXEL_MAP_TABLE, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Pixel Map Table Size");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_TEXTURE_LOD_BIAS_EXT, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Texture LOD Bias");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();
    IoAddHeaderString(0, L"Max Stack Depth", 0);

    glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Attribute Stack");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Client Attribute Stack");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Modelview Matrix Stack");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_NAME_STACK_DEPTH, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Name Stack");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Projection Matrix Stack");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Texture Matrix Stack");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();
    IoAddHeaderString(0, L"Draw Range Elements", 0);

    glGetIntegerv(GL_MAX_ELEMENTS_INDICES_WIN, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Index Count");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES_WIN, &i_data);
    if (i_data)
    {
        Index = IoAddItem(1, 0, L"Max Vertex Count");
        StringCbPrintf(szText, sizeof(szText), L"%d", i_data);
        IoSetItemText(Index, 1, szText);
    }

    IoAddFooter();
    IoAddHeader(0, IDS_OPENGL_EXTENSIONS, 0);

    data = (char *)glGetString(GL_EXTENSIONS);
    if (data)
    {
        WCHAR szExts[MAX_STR_LEN * 5];
        SIZE_T i, j, k;

        StringCbPrintf(szExts, sizeof(szExts), L"%S", data);

        for (i = 0, j = 0, len = wcslen(szExts); i < len; i++, j++)
        {
            if (szExts[i] == L' ')
            {
                WCHAR *p = szText;

                szText[j] = 0;
                j = 0, k = 0;
                if (i > 0) ++p;

                do
                {
                    if (wcscmp(OpenglExtensions[k].lpExtName, p) == 0)
                    {
                        OpenglExtensions[k].IsSupported = TRUE;
                        break;
                    }
                }
                while (OpenglExtensions[++k].lpExtName != NULL);
            }

            szText[j] = szExts[i];
        }

        k = 0;
        do
        {
            Index = IoAddItem(1, OpenglExtensions[k].IsSupported ? 1 : 2,
                              OpenglExtensions[k].lpExtName);
            IoSetItemText(Index, 1, OpenglExtensions[k].IsSupported ? szSupported : szUnsupported);
        }
        while (OpenglExtensions[++k].lpExtName != NULL);
    }

    /* Cleanup */
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(DllParams.hMainWnd, hDC);

    DebugEndReceiving();
}