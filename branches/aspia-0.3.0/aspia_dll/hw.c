/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/hw.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"
#include "edid.h"

const BYTE EdidV1DescriptorFlag[] = { 0x00, 0x00 };

VOID
HW_CPUInfo(VOID)
{
    DebugStartReceiving();
    CPUIDInfo();
    DebugEndReceiving();
}

VOID CALLBACK
EnumSmartDataProc(SMART_RESULT *Result)
{
    /* Attribute ID */
    if (Result->IsCritical)
    {
        INT IconIndex;

        if (Result->dwWarrantyThreshold > 0 && Result->dwAttrValue < Result->dwWarrantyThreshold &&
            Result->dwAttrValue > 0)
            IconIndex = 2;
        else
            IconIndex = 1;

        IoAddItem(0, IconIndex, L"%02X", Result->dwAttrID);
    }
    else
    {
        IoAddItem(0, 1, L"%02X", Result->dwAttrID);
    }

    /* Name */
    IoSetItemText(Result->szName);

    /* Threshold */
    IoSetItemText(L"%d", Result->dwWarrantyThreshold);

    /* Value */
    IoSetItemText(L"%d", Result->bValue);

    /* Worst */
    IoSetItemText(L"%d", Result->dwWorstValue);

    /* Data */
    IoSetItemText(L"%d", Result->dwAttrValue);
}

#define TRANSFER_MODE_UNKNOWN        0x00
#define TRANSFER_MODE_PIO            0x01
#define TRANSFER_MODE_PIO_DMA        0x02
#define TRANSFER_MODE_ULTRA_DMA_16   0x03
#define TRANSFER_MODE_ULTRA_DMA_25   0x04
#define TRANSFER_MODE_ULTRA_DMA_33   0x05
#define TRANSFER_MODE_ULTRA_DMA_44   0x06
#define TRANSFER_MODE_ULTRA_DMA_66   0x07
#define TRANSFER_MODE_ULTRA_DMA_100  0x08
#define TRANSFER_MODE_ULTRA_DMA_133  0x09
#define TRANSFER_MODE_SATA_150       0x0A
#define TRANSFER_MODE_SATA_300       0x0B
#define TRANSFER_MODE_SATA_600       0x0C

BOOL
IsSATADrive(IDSECTOR DriveInfo)
{
    if (DriveInfo.wSATACapabilities != 0x0000 &&
        DriveInfo.wSATACapabilities != 0xFFFF)
        return TRUE;

    return FALSE;
}

BYTE
GetMaxTransferMode(IDSECTOR DriveInfo)
{
    BYTE Mode = TRANSFER_MODE_PIO;

    if (DriveInfo.wMultiWordDMA & 0x0700)
    {
        Mode = TRANSFER_MODE_PIO_DMA;
    }

    if (DriveInfo.wUltraDMAMode & 0x40)
        Mode = TRANSFER_MODE_ULTRA_DMA_133;
    else if (DriveInfo.wUltraDMAMode & 0x20)
        Mode = TRANSFER_MODE_ULTRA_DMA_100;
    else if (DriveInfo.wUltraDMAMode & 0x10)
        Mode = TRANSFER_MODE_ULTRA_DMA_66;
    else if (DriveInfo.wUltraDMAMode & 0x08)
        Mode = TRANSFER_MODE_ULTRA_DMA_44;
    else if (DriveInfo.wUltraDMAMode & 0x04)
        Mode = TRANSFER_MODE_ULTRA_DMA_33;
    else if (DriveInfo.wUltraDMAMode & 0x02)
        Mode = TRANSFER_MODE_ULTRA_DMA_25;
    else if (DriveInfo.wUltraDMAMode & 0x01)
        Mode = TRANSFER_MODE_ULTRA_DMA_16;

    if (IsSATADrive(DriveInfo))
    {
        Mode = TRANSFER_MODE_SATA_150;

        if (DriveInfo.wSATACapabilities & 0x10)
            Mode = TRANSFER_MODE_UNKNOWN;
        else if (DriveInfo.wSATACapabilities & 0x8)
            Mode = TRANSFER_MODE_SATA_600;
        else if (DriveInfo.wSATACapabilities & 0x4)
            Mode = TRANSFER_MODE_SATA_300;
        else if (DriveInfo.wSATACapabilities & 0x2)
            Mode = TRANSFER_MODE_SATA_150;
    }

    return Mode;
}

BYTE
GetCurrentTransferMode(IDSECTOR DriveInfo)
{
    BYTE Mode = TRANSFER_MODE_PIO;

    if (DriveInfo.wMultiWordDMA & 0x0700)
    {
        Mode = TRANSFER_MODE_PIO_DMA;
    }

    if (DriveInfo.wUltraDMAMode & 0x4000)
        Mode = TRANSFER_MODE_ULTRA_DMA_133;
    else if (DriveInfo.wUltraDMAMode & 0x2000)
        Mode = TRANSFER_MODE_ULTRA_DMA_100;
    else if (DriveInfo.wUltraDMAMode & 0x1000)
        Mode = TRANSFER_MODE_ULTRA_DMA_66;
    else if (DriveInfo.wUltraDMAMode & 0x0800)
        Mode = TRANSFER_MODE_ULTRA_DMA_44;
    else if (DriveInfo.wUltraDMAMode & 0x0400)
        Mode = TRANSFER_MODE_ULTRA_DMA_33;
    else if (DriveInfo.wUltraDMAMode & 0x0200)
        Mode = TRANSFER_MODE_ULTRA_DMA_25;
    else if (DriveInfo.wUltraDMAMode & 0x0100)
        Mode = TRANSFER_MODE_ULTRA_DMA_16;

    if (IsSATADrive(DriveInfo))
    {
        Mode = TRANSFER_MODE_SATA_150;

        if (DriveInfo.wSATACapabilities & 0x10)
            Mode = TRANSFER_MODE_UNKNOWN;
        else if (DriveInfo.wSATACapabilities & 0x8)
            Mode = TRANSFER_MODE_SATA_600;
        else if (DriveInfo.wSATACapabilities & 0x4)
            Mode = TRANSFER_MODE_SATA_300;
        else if (DriveInfo.wSATACapabilities & 0x2)
            Mode = TRANSFER_MODE_SATA_150;
    }

    return Mode;
}

VOID
TransferModeToText(BYTE Mode, LPWSTR lpText, SIZE_T Size)
{
    WCHAR *ptr;

    switch (Mode)
    {
        case TRANSFER_MODE_PIO:           ptr = L"PIO";                      break;
        case TRANSFER_MODE_PIO_DMA:       ptr = L"PIO / DMA";                break;
        case TRANSFER_MODE_ULTRA_DMA_133: ptr = L"Ultra DMA/133 (133 MB/s)"; break;
        case TRANSFER_MODE_ULTRA_DMA_100: ptr = L"Ultra DMA/100 (100 MB/s)"; break;
        case TRANSFER_MODE_ULTRA_DMA_66:  ptr = L"Ultra DMA/66 (66 MB/s)";   break;
        case TRANSFER_MODE_ULTRA_DMA_44:  ptr = L"Ultra DMA/44 (44 MB/s)";   break;
        case TRANSFER_MODE_ULTRA_DMA_33:  ptr = L"Ultra DMA/33 (33 MB/s)";   break;
        case TRANSFER_MODE_ULTRA_DMA_25:  ptr = L"Ultra DMA/25 (25 MB/s)";   break;
        case TRANSFER_MODE_ULTRA_DMA_16:  ptr = L"Ultra DMA/16 (16 MB/s)";   break;
        case TRANSFER_MODE_SATA_600:      ptr = L"SATA/600 (600 MB/s)";      break;
        case TRANSFER_MODE_SATA_300:      ptr = L"SATA/300 (300 MB/s)";      break;
        case TRANSFER_MODE_SATA_150:      ptr = L"SATA/150 (150 MB/s)";      break;
        default:                          ptr = L"Unknown";                  break;
    }

    StringCbCopy(lpText, Size, ptr);
}

WORD
GetMajorVersion(IDSECTOR DriveInfo)
{
    WORD result = 0;
    INT i = 14;

    if (DriveInfo.wMajorVersion != 0xFFFF &&
        DriveInfo.wMajorVersion != 0x0)
    {
        while (i > 0)
        {
            if ((DriveInfo.wMajorVersion >> i) & 0x1)
            {
                result = i;
                break;
            }
            --i;
        }
    }

    return result;
}

/*
    BusTypeUnknown = 0x00,
    BusTypeScsi,
    BusTypeAtapi,
    BusTypeAta,
    BusType1394,
    BusTypeSsa,
    BusTypeFibre,
    BusTypeUsb,
    BusTypeRAID,
    BusTypeiScsi,
    BusTypeSas,
    BusTypeSata,
    BusTypeSd,
    BusTypeMmc,
    BusTypeMax,
    BusTypeMaxReserved = 0x7F
 */
STORAGE_BUS_TYPE
GetHardDriveType(HANDLE hHandle)
{
    STORAGE_DEVICE_DESCRIPTOR DeviceDescriptor = {0};
    STORAGE_PROPERTY_QUERY PropertyQuery = {0};
    DWORD bytesReturned;

    if (!DeviceIoControl(hHandle,
                         IOCTL_STORAGE_QUERY_PROPERTY,
                         &PropertyQuery,
                         sizeof(STORAGE_PROPERTY_QUERY),
                         &DeviceDescriptor,
                         sizeof(STORAGE_DEVICE_DESCRIPTOR),
                         &bytesReturned,
                         NULL))
    {
        return 0;
    }

    return DeviceDescriptor.BusType;
}

VOID
HW_HDDATAInfo(VOID)
{
    DISK_GEOMETRY DiskGeometry = {0};
    IDSECTOR DriveInfo = {0};
    WCHAR szText[MAX_STR_LEN], szFormat[MAX_STR_LEN], *pText;
    WCHAR szSupported[MAX_STR_LEN], szUnsupported[MAX_STR_LEN],
          szSupEnabled[MAX_STR_LEN], szSupDisabled[MAX_STR_LEN];
    BOOL IsSupported, IsEnabled;
    WORD wMajorVersion;
    HANDLE hHandle;
    BYTE bIndex, Mode;

    DebugStartReceiving();

    IoAddIcon(IDI_HDD);
    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);

    LoadMUIString(IDS_CPUID_SUPPORTED,    szSupported,   MAX_STR_LEN);
    LoadMUIString(IDS_CPUID_UNSUPPORTED,  szUnsupported, MAX_STR_LEN);
    LoadMUIString(IDS_SUPPORTED_ENABLED,  szSupEnabled,  MAX_STR_LEN);
    LoadMUIString(IDS_SUPPORTED_DISABLED, szSupDisabled, MAX_STR_LEN);

    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        hHandle = OpenSmart(bIndex);
        if (hHandle == INVALID_HANDLE_VALUE)
        {
            continue;
        }

        if (!ReadSmartInfo(hHandle, bIndex, &DriveInfo))
        {
            DebugTrace(L"ReadSmartInfo() failed!");

            if (!ScsiOverAtaReadSmartInfo(hHandle, bIndex, &DriveInfo))
            {
                DebugTrace(L"ScsiOverAtaReadSmartInfo() failed!");
                continue;
            }
        }

        ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                        sizeof(DriveInfo.sModelNumber));
        StringCbPrintf(szText, sizeof(szText),
                       L"%S", DriveInfo.sModelNumber);
        ChopSpaces(szText, sizeof(szText));
        IoAddHeaderString(0, 0, (szText[0] != 0) ? szText : L"Unknown Hard Drive");

        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_HDD_ID);
            IoSetItemText(szText);
        }

        ChangeByteOrder((PCHAR)DriveInfo.sSerialNumber,
                        sizeof(DriveInfo.sSerialNumber));
        StringCbPrintf(szText, sizeof(szText),
                       L"%S", DriveInfo.sSerialNumber);
        ChopSpaces(szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_SERIAL_NUMBER);
            IoSetItemText(szText);
        }

        ChangeByteOrder((PCHAR)DriveInfo.sFirmwareRev,
                        sizeof(DriveInfo.sFirmwareRev));
        if (DriveInfo.sFirmwareRev[0] != 0)
        {
            IoAddValueName(1, 0, IDS_VERSION);
            IoSetItemText(L"%S", DriveInfo.sFirmwareRev);
        }

        IoAddValueName(1, 0, IDS_HDD_INTERFACE);
        if (IsSATADrive(DriveInfo))
        {
            if (GetHardDriveType(hHandle) == BusTypeUsb)
                pText = L"USB (SATA)";
            else
                pText = L"SATA";
        }
        else
        {
            if (GetHardDriveType(hHandle) == BusTypeUsb)
                pText = L"USB (IDE)";
            else
                pText = L"IDE";
        }
        IoSetItemText(pText);

        IoAddValueName(1, 0, IDS_HDD_CURRENT_TRANSFER_MODE);
        Mode = GetCurrentTransferMode(DriveInfo);
        TransferModeToText(Mode, szText, sizeof(szText));
        IoSetItemText(szText);

        if (!IsSATADrive(DriveInfo))
        {
            IoAddValueName(1, 0, IDS_HDD_MAX_TRANSFER_MODE);
            Mode = GetCurrentTransferMode(DriveInfo);
            TransferModeToText(Mode, szText, sizeof(szText));
            IoSetItemText(szText);
        }

        if (DriveInfo.wRotationRate < 65535 &&
            DriveInfo.wRotationRate >= 1025)
        {
            IoAddValueName(1, 0, IDS_HDD_ROTATION_RATE);
            IoSetItemText(L"%d RPM", DriveInfo.wRotationRate);
        }

        if (GetSmartDiskGeometry(bIndex, &DiskGeometry))
        {
            ULONGLONG DiskSize;

            if ((DiskGeometry.Cylinders.QuadPart * (ULONG)DriveInfo.wNumHeads) > 0 &&
                DriveInfo.wNumHeads > 0)
            {
                IoAddValueName(1, 0, IDS_HDD_PARAMS);
                LoadMUIString(IDS_HDD_PARAMS_FORMAT, szFormat, MAX_STR_LEN);
                IoSetItemText(szFormat,
                              (ULONG)DiskGeometry.Cylinders.QuadPart * (ULONG)DriveInfo.wNumHeads,
                              (ULONG)DriveInfo.wNumHeads,
                              (ULONG)DiskGeometry.SectorsPerTrack,
                              (ULONG)DiskGeometry.BytesPerSector);
            }

            DiskSize = DiskGeometry.Cylinders.QuadPart * (ULONG)DiskGeometry.TracksPerCylinder *
                       (ULONG)DiskGeometry.SectorsPerTrack * (ULONG)DiskGeometry.BytesPerSector;

            if (DiskSize > 0)
            {
                IoAddValueName(1, 0, IDS_HDD_SIZE);
                IoSetItemText(L"%I64d MB (%I64d GB)",
                              DiskSize / (1024 * 1024),
                              DiskSize / (1024 * 1024 * 1024));
            }
        }

        if (DriveInfo.wBufferSize > 0)
        {
            IoAddValueName(1, 0, IDS_HDD_BUFFER_SIZE);
            IoSetItemText(L"%d MB",
                          (DriveInfo.wBufferSize * 512)/(1024 * 1024));
        }

        if (DriveInfo.wMultSectorStuff > 0)
        {
            IoAddValueName(1, 0, IDS_HDD_MULTISECTORS);
            IoSetItemText(L"%d", DriveInfo.wMultSectorStuff);
        }

        if (DriveInfo.wECCSize > 0)
        {
            IoAddValueName(1, 0, IDS_HDD_ECC_BYTES);
            IoSetItemText(L"%d", DriveInfo.wECCSize);
        }

        szText[0] = 0;
        if (DriveInfo.wGenConfig & 0x80)
            StringCbCopy(szText, sizeof(szText), L"Removable");
        else if (DriveInfo.wGenConfig & 0x40)
            StringCbCopy(szText, sizeof(szText), L"Fixed");
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_TYPE);
            IoSetItemText(szText);
        }

        wMajorVersion = GetMajorVersion(DriveInfo);

        IoAddHeader(1, 0, IDS_HDD_ATA_FEATURES);

        /* 48-bit LBA */
        IsSupported = (wMajorVersion >= 5 && DriveInfo.wCommandSetSupport2 & (1 << 10)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"48-bit LBA");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* Advanced Power Management */
        IsSupported = (wMajorVersion >= 3 && DriveInfo.wCommandSetSupport2 & (1 << 3)) ? TRUE : FALSE;
        IsEnabled = (IsSupported && DriveInfo.wCommandSetEnabled2 & (1 << 3)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Advanced Power Management");
        IoSetItemText(IsSupported ? (IsEnabled ? szSupEnabled : szSupDisabled) : szUnsupported);

        /* Automatic Acoustic Management */
        IsSupported = (wMajorVersion >= 5 && DriveInfo.wCommandSetSupport2 & (1 << 9)) ? TRUE : FALSE;
        IsEnabled = (IsSupported && DriveInfo.wCommandSetEnabled2 & (1 << 9)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Automatic Acoustic Management");
        IoSetItemText(IsSupported ? (IsEnabled ? szSupEnabled : szSupDisabled) : szUnsupported);

        /* SMART */
        IsSupported = (wMajorVersion >= 3 && DriveInfo.wCommandSetSupport1 & (1 << 0)) ? TRUE : FALSE;
        IsEnabled = (IsSupported && DriveInfo.wCommandSetEnabled1 & (1 << 0)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"SMART");
        IoSetItemText(IsSupported ? (IsEnabled ? szSupEnabled : szSupDisabled) : szUnsupported);

        /* SMART Error Logging */
        IsSupported = (DriveInfo.wCommandSetSupport3 & (1 << 0)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"SMART Error Logging");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* SMART Self-Test */
        IsSupported = (DriveInfo.wCommandSetSupport3 & (1 << 1)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"SMART Self-Test");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* Streaming */
        IsSupported = (DriveInfo.wCommandSetSupport3 & (1 << 4)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Streaming");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* General Purpose Logging */
        IsSupported = (DriveInfo.wCommandSetSupport3 & (1 << 5)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"General Purpose Logging");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* Security Mode */
        IsSupported = (DriveInfo.wCommandSetSupport1 & (1 << 1)) ? TRUE : FALSE;
        IsEnabled = (IsSupported && DriveInfo.wCommandSetEnabled1 & (1 << 1)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Security Mode");
        IoSetItemText(IsSupported ? (IsEnabled ? szSupEnabled : szSupDisabled) : szUnsupported);

        /* Power Management */
        IsSupported = (DriveInfo.wCommandSetSupport1 & (1 << 3)) ? TRUE : FALSE;
        IsEnabled = (IsSupported && DriveInfo.wCommandSetEnabled1 & (1 << 3)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Power Management");
        IoSetItemText(IsSupported ? (IsEnabled ? szSupEnabled : szSupDisabled) : szUnsupported);

        /* Write Cache */
        IsSupported = (DriveInfo.wCommandSetSupport1 & (1 << 5)) ? TRUE : FALSE;
        IsEnabled = (IsSupported && DriveInfo.wCommandSetEnabled1 & (1 << 5)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Write Cache");
        IoSetItemText(IsSupported ? (IsEnabled ? szSupEnabled : szSupDisabled) : szUnsupported);

        /* Read Look-Ahead */
        IsSupported = (DriveInfo.wCommandSetSupport1 & (1 << 6)) ? TRUE : FALSE;
        IsEnabled = (IsSupported && DriveInfo.wCommandSetEnabled1 & (1 << 6)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Read Look-Ahead");
        IoSetItemText(IsSupported ? (IsEnabled ? szSupEnabled : szSupDisabled) : szUnsupported);

        /* Host Protected Area */
        IsSupported = (DriveInfo.wCommandSetSupport1 & (1 << 10)) ? TRUE : FALSE;
        IsEnabled = (IsSupported && DriveInfo.wCommandSetEnabled1 & (1 << 10)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Host Protected Area");
        IoSetItemText(IsSupported ? (IsEnabled ? szSupEnabled : szSupDisabled) : szUnsupported);

        /* Release Interrupt */
        IsSupported = (DriveInfo.wCommandSetSupport1 & (1 << 7)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Release Interrupt");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* Power-Up In Standby */
        IsSupported = (DriveInfo.wCommandSetSupport2 & (1 << 5)) ? TRUE : FALSE;
        IsEnabled = (IsSupported && DriveInfo.wCommandSetEnabled2 & (1 << 5)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Power-Up In Standby");
        IoSetItemText(IsSupported ? (IsEnabled ? szSupEnabled : szSupDisabled) : szUnsupported);

        /* Device Configuration Overlay */
        IsSupported = (DriveInfo.wCommandSetSupport2 & (1 << 11)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Device Configuration Overlay");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* Service Interrupt */
        IsSupported = (DriveInfo.wCommandSetSupport1 & (1 << 8)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Service Interrupt");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* Native Command Queuing (NCQ) */
        IsSupported = (wMajorVersion >= 6 && DriveInfo.wSATACapabilities & (1 << 8)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"Native Command Queuing");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        /* TRIM */
        IsSupported = (wMajorVersion >= 7 && DriveInfo.wDataSetManagement & (1 << 0)) ? TRUE : FALSE;
        IoAddItem(2, (IsSupported ? 1 : 2), L"TRIM");
        IoSetItemText(IsSupported ? szSupported : szUnsupported);

        IoAddFooter();

        CloseSmart(hHandle);
    }

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
    BYTE bIndex;
    BOOL ScsiOverAta = FALSE;

    DebugStartReceiving();

    IoAddIcon(IDI_HDD);
    IoAddIcon(IDI_INFOICO);
    IoAddIcon(IDI_BANG);

    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        ScsiOverAta = FALSE;

        hHandle = OpenSmart(bIndex);
        if (hHandle == INVALID_HANDLE_VALUE) continue;

        if (!ReadSmartInfo(hHandle, bIndex, &DriveInfo))
        {
            DebugTrace(L"ReadSmartInfo() failed!");

            if (!ScsiOverAtaReadSmartInfo(hHandle, bIndex, &DriveInfo))
            {
                DebugTrace(L"ScsiOverAtaReadSmartInfo() failed!");
                continue;
            }
            else
            {
                ScsiOverAta = TRUE;
            }
        }

        ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                        sizeof(DriveInfo.sModelNumber));
        StringCbPrintf(szText, sizeof(szText),
                       L"%S", DriveInfo.sModelNumber);
        ChopSpaces(szText, sizeof(szText));

        if (szText[0] == 0)
        {
            CloseSmart(hHandle);
            continue;
        }

        IoAddItem(0, 0, L"-\0");

        IoSetItemText(szText);
        IoSetItemText(L"-\0");
        IoSetItemText(L"-\0");
        IoSetItemText(L"-\0");
        IoSetItemText(L"-\0");

        if (!ScsiOverAta)
        {
            if (EnumSmartData(hHandle, bIndex, EnumSmartDataProc))
                IoAddFooter();
        }
        else
        {
            if (ScsiOverAtaEnumSmartData(hHandle, bIndex, EnumSmartDataProc))
                IoAddFooter();
        }
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

    IoAddHeaderString(0, 0, szText);

    /* Monitor ID */
    IoAddValueName(1, 0, IDS_DISPLAY_ID);
    IoSetItemText(szMonitorId);

    /* Monitor Model */
    if (pMonitorModel)
    {
        IoAddValueName(1, 0, IDS_DISPLAY_MODEL);
        IoSetItemText(pMonitorModel);
    }

    /* Manufacture Date */
    IoAddValueName(1, 0, IDS_DISPLAY_MANUFACTURE_DATE);
    IoSetItemText(L"%d Week / %d Year",
                  (INT)Edid[MANUFACTURE_WEEK],
                  (INT)Edid[MANUFACTURE_YEAR] + 1990);

    /* Max/ Visible Display Size */
    IoAddValueName(1, 0, IDS_DISPLAY_MAX_VISIBLE_SIZE);
    IoSetItemText(L"%d cm / %d cm (%.1f\")",
                  (INT)Edid[0x15], (INT)Edid[0x16],
                  GetDiagonalSize(Edid[0x15], Edid[0x16]));

    Block = Edid + DETAILED_TIMING_DESCRIPTIONS_START;
    for (Index = 0; Index < NO_DETAILED_TIMING_DESCRIPTIONS; Index++,
         Block += DETAILED_TIMING_DESCRIPTION_SIZE)
    {
        if (EdidBlockType(Block) == DETAILED_TIMING_BLOCK)
        {
            /* Max. Resolution */
            IoAddValueName(1, 0, IDS_DISPLAY_MAX_RESOLUTION);
            IoSetItemText(L"%dx%d",
                          H_ACTIVE, V_ACTIVE);
        }
    }

    Block = Edid + DETAILED_TIMING_DESCRIPTIONS_START;
    for(Index = 0; Index < NO_DETAILED_TIMING_DESCRIPTIONS; Index++,
        Block += DETAILED_TIMING_DESCRIPTION_SIZE)
    {
        if (EdidBlockType(Block) == MONITOR_LIMITS)
        {
            IoAddValueName(1, 0, IDS_DISPLAY_HORIZ_FREQ);
            IoSetItemText(L"%u - %u kHz",
                          Block[7], Block[8]);

            IoAddValueName(1, 0, IDS_DISPLAY_VERT_FREQ);
            IoSetItemText(L"%u - %u Hz",
                          Block[5], Block[6]);
        }
    }

    /* Gamma */
    IoAddValueName(1, 0, IDS_DISPLAY_GAMMA);
    IoSetItemText(L"%.1f",
                  (double)(((double)Edid[23] / 100.0) + 1.0));

    /* DPMS Mode Support */
    szText[0] = 0;
    IoAddValueName(1, 0, IDS_DISPLAY_DPMS_MODE);
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
    IoSetItemText(szText);

    /* EDID Version */
    IoAddValueName(1, 0, IDS_DISPLAY_EDID_VERSION);
    IoSetItemText(L"%d.%d",
                  (INT)Edid[EDID_STRUCT_VERSION],
                  (INT)Edid[EDID_STRUCT_REVISION]);
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
    {
        DebugTrace(L"SetupDiGetClassDevs() falied. Error code = %x",
                   GetLastError());
        goto Cleanup;
    }

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

Cleanup:
    if (hDevInfo != INVALID_HANDLE_VALUE)
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

    DebugStartReceiving();

    IoAddIcon(IDI_MONITOR);
    IoAddIcon(IDI_CPU);
    IoAddIcon(IDI_HW);

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
            IoAddHeaderString(0, 0, L"Mirror Monitor");
        else
            IoAddHeaderString(0, 0, DispDevice2.DeviceString);

        /* Adapter name */
        if (SafeStrLen(DispDevice.DeviceString) > 0)
        {
            IoAddValueName(1, 0, IDS_MONITOR_ADAPTER);
            IoSetItemText(DispDevice.DeviceString);
        }

        /* Chip type */
        if (GetStringFromRegistry(TRUE,
                                  HKEY_LOCAL_MACHINE,
                                  &DispDevice.DeviceKey[18],
                                  L"HardwareInformation.ChipType",
                                  szText, MAX_STR_LEN) &&
            szText[0] != 0)
        {
            IoAddValueName(1, 1, IDS_MONITOR_CHIP_TYPE);
            IoSetItemText(szText);
        }

        /* DAC type */
        if (GetStringFromRegistry(TRUE,
                                  HKEY_LOCAL_MACHINE,
                                  &DispDevice.DeviceKey[18],
                                  L"HardwareInformation.DacType",
                                  szText, MAX_STR_LEN) &&
            szText[0] != 0)
        {
            IoAddValueName(1, 1, IDS_MONITOR_DAC_TYPE);
            IoSetItemText(szText);
        }

        /* Memory size */
        if (GetBinaryFromRegistry(HKEY_LOCAL_MACHINE,
                                  &DispDevice.DeviceKey[18],
                                  L"HardwareInformation.MemorySize",
                                  (LPBYTE)&dwValue,
                                  sizeof(dwValue)) &&
            dwValue > 0)
        {
            IoAddValueName(1, 0, IDS_MONITOR_MEM_SIZE);
            IoSetItemText(L"%ld MB",
                          dwValue / (1024 * 1024));
        }

        /* retrieve current display mode */
        IoAddValueName(1, 0, IDS_MONITOR_MODE);
        DevMode.dmSize = sizeof(DEVMODE);
        if (EnumDisplaySettings(DispDevice.DeviceName,
                                ENUM_CURRENT_SETTINGS,
                                &DevMode))
        {
            IoSetItemText(L"%Iu x %Iu (%Iu bit) (%Iu Hz)",
                          DevMode.dmPelsWidth,
                          DevMode.dmPelsHeight,
                          DevMode.dmBitsPerPel,
                          DevMode.dmDisplayFrequency);
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
                IoAddValueName(1, 2, IDS_MONITOR_DRIVER_VENDOR);
                IoSetItemText(szText);
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
    WCHAR szText[MAX_STR_LEN];
    UINT StringID;

    DebugStartReceiving();

    IoAddIcon(IDI_POWERICO);
    IoAddIcon(IDI_BATTERY);

    IoAddHeader(0, 0, IDS_POWER_MANAGMT_PROP);

    if (GetSystemPowerStatus(&PowerStatus))
    {
        IoAddValueName(1, 0, IDS_CURRENT_POWER_SOURCE);
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
        IoSetItemText(szText);

        IoAddValueName(1, 0, IDS_BATTERY_STATUS);
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
            IoSetItemText(szText);
        }
        else
        {
            IoSetItemText(L"%ld%% (%s)",
                          PowerStatus.BatteryLifePercent, szText);
        }

        IoAddValueName(1, 0, IDS_FULL_BATTERY_LIFETIME);
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
        IoSetItemText(szText);

        IoAddValueName(1, 0, IDS_REMAINING_BAT_LIFETIME);
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
        IoSetItemText(szText);

        if (PowerStatus.BatteryFlag == 128 ||
            PowerStatus.BatteryFlag == 255)
        {
            goto Cleanup;
        }
    }
    IoAddFooter();

    IoAddHeader(0, 1, IDS_BAT_BATTERYS);
    GetBatteryInformation();

Cleanup:
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
        IoAddHeaderString(0, 0, pPrinterInfo[dwIndex].pPrinterName);

        /* Default? */
        IoAddValueName(1, 0, IDS_PRINTER_DEFAULT);
        if (SafeStrCmp(szDefPrinter, pPrinterInfo[dwIndex].pPrinterName) == 0)
            LoadMUIString(IDS_YES, szText, MAX_STR_LEN);
        else
            LoadMUIString(IDS_NO, szText, MAX_STR_LEN);

        IoSetItemText(szText);

        /* Shared? */
        IoAddValueName(1, 4, IDS_PRINTER_SHARED);
        if (pPrinterInfo[dwIndex].Attributes & PRINTER_ATTRIBUTE_SHARED)
            LoadMUIString(IDS_YES, szText, MAX_STR_LEN);
        else
            LoadMUIString(IDS_NO, szText, MAX_STR_LEN);

        IoSetItemText(szText);

        /* Share name */
        if (SafeStrLen(pPrinterInfo[dwIndex].pShareName) > 1)
        {
            IoAddValueName(1, 4, IDS_PRINTER_SHARENAME);
            IoSetItemText(pPrinterInfo[dwIndex].pShareName);
        }

        /* Port name */
        if (SafeStrLen(pPrinterInfo[dwIndex].pPortName) > 1)
        {
            IoAddValueName(1, 2, IDS_PRINTER_PORT);
            IoSetItemText(pPrinterInfo[dwIndex].pPortName);
        }

        /* Driver name */
        if (SafeStrLen(pPrinterInfo[dwIndex].pDriverName) > 1)
        {
            IoAddValueName(1, 3, IDS_PRINTER_DRIVER);
            IoSetItemText(pPrinterInfo[dwIndex].pDriverName);
        }

        /* Device name */
        if (SafeStrLen(pDevMode->dmDeviceName) > 1)
        {
            IoAddValueName(1, 3, IDS_PRINTER_DEVICENAME);
            IoSetItemText(pDevMode->dmDeviceName);
        }

        /* Print processor */
        if (SafeStrLen(pPrinterInfo[dwIndex].pPrintProcessor) > 1)
        {
            IoAddValueName(1, 0, IDS_PRINTER_PROCESSOR);
            IoSetItemText(pPrinterInfo[dwIndex].pPrintProcessor);
        }

        /* Data type */
        if (SafeStrLen(pPrinterInfo[dwIndex].pDatatype) > 1)
        {
            IoAddValueName(1, 0, IDS_PRINTER_DATATYPE);
            IoSetItemText(pPrinterInfo[dwIndex].pDatatype);
        }

        /* Server name */
        if (pPrinterInfo[dwIndex].pServerName &&
            SafeStrLen(pPrinterInfo[dwIndex].pServerName) > 1)
        {
            IoAddValueName(1, 0, IDS_PRINTER_SERVER);
            IoSetItemText(pPrinterInfo[dwIndex].pServerName);
        }

        /* Location */
        if (SafeStrLen(pPrinterInfo[dwIndex].pLocation) > 1)
        {
            IoAddValueName(1, 0, IDS_PRINTER_LOCATION);
            IoSetItemText(pPrinterInfo[dwIndex].pLocation);
        }

        /* Comment */
        if (SafeStrLen(pPrinterInfo[dwIndex].pComment) > 1)
        {
            IoAddValueName(1, 0, IDS_PRINTER_COMMENT);
            IoSetItemText(pPrinterInfo[dwIndex].pComment);
        }

        /* Jobs count */
        IoAddValueName(1, 0, IDS_PRINTER_JOBS_COUNT);
        IoSetItemText(L"%d",
                      pPrinterInfo[dwIndex].cJobs);

        if (pDevMode)
        {
            /* Paper size */
            if (pDevMode->dmPaperWidth && pDevMode->dmPaperLength)
            {
                IoAddValueName(1, 1, IDS_PRINTER_PAPER_SIZE);
                IoSetItemText(L"%ld x %ld mm",
                              pDevMode->dmPaperWidth / 10,
                              pDevMode->dmPaperLength / 10);
            }

            /* Quality */
            if (pDevMode->dmPrintQuality)
            {
                IoAddValueName(1, 1, IDS_PRINTER_QUALITY);
                IoSetItemText(L"%ld x %ld dpi",
                              pDevMode->dmPrintQuality,
                              pDevMode->dmPrintQuality);
            }

            /* Orientation */
            IoAddValueName(1, 1, IDS_PRINTER_ORIENTATION);
            if (pDevMode->dmOrientation == DMORIENT_PORTRAIT)
                LoadMUIString(IDS_PRINTER_PORTRAIT, szText, MAX_STR_LEN);
            else
                LoadMUIString(IDS_PRINTER_LANDSCAPE, szText, MAX_STR_LEN);

            IoSetItemText(szText);
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
    IoAddIcon(IDI_TEXT);
    IoAddIcon(IDI_PORT);
    IoAddIcon(IDI_HW);
    IoAddIcon(IDI_SHARED);

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
    INT iPixelFormat, i_data, len;
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
    PixelFormatDesc.cColorBits = 32;
    PixelFormatDesc.iLayerType = PFD_MAIN_PLANE;
    PixelFormatDesc.cDepthBits = 32;

    iPixelFormat = ChoosePixelFormat(hDC, &PixelFormatDesc);
    if (iPixelFormat == 0)
    {
        DebugTrace(L"ChoosePixelFormat() failed! Error code = %x",
                   GetLastError());
        goto Cleanup;
    }

    if (!SetPixelFormat(hDC, iPixelFormat, &PixelFormatDesc))
    {
        DebugTrace(L"SetPixelFormat() failed! Error code = %x",
                   GetLastError());
        goto Cleanup;
    }

    hRC = wglCreateContext(hDC);
    if (hRC == NULL)
    {
        DebugTrace(L"wglCreateContext() failed! Error code = %x",
                   GetLastError());
        goto Cleanup;
    }

    if (!wglMakeCurrent(hDC, hRC))
    {
        DebugTrace(L"wglMakeCurrent() failed! Error code = %x",
                   GetLastError());
        goto Cleanup;
    }

    IoAddHeader(0, 0, IDS_OPENGL_PROP);

    data = (char *)glGetString(GL_VENDOR);
    if (data)
    {
        IoAddValueName(1, 0, IDS_OPENGL_VENDOR);
        IoSetItemText(L"%S", data);
    }

    data = (char *)glGetString(GL_RENDERER);
    if (data)
    {
        IoAddItem(1, 0, L"Renderer");
        IoSetItemText(L"%S", data);
    }

    data = (char *)glGetString(GL_VERSION);
    if (data)
    {
        IoAddValueName(1, 0, IDS_OPENGL_VERSION);
        IoSetItemText(L"%S", data);
    }

    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Multitexture Texture Units");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_SUBPIXEL_BITS, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Sub-Pixel Precision");
        IoSetItemText(L"%d-bit", i_data);
    }

    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Viewport Size");
        IoSetItemText(L"%d x %d", i_data, i_data);
    }

    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Cube Map Texture Size");
        IoSetItemText(L"%d x %d", i_data, i_data);
    }

    glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Rectangle Texture Size");
        IoSetItemText(L"%d x %d", i_data, i_data);
    }

    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE_EXT, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max 3D Texture Size");
        IoSetItemText(L"%d x %d x %d",
                      i_data, i_data, i_data);
    }

    glGetIntegerv(GL_MAX_CLIP_PLANES, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Clipping Planes");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_LIST_NESTING, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Display-List Nesting Level");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_DRAW_BUFFERS_ATI, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Draw Buffers");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_EVAL_ORDER, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Evaluator Order");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_LIGHTS, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Light Sources");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_PIXEL_MAP_TABLE, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Pixel Map Table Size");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_TEXTURE_LOD_BIAS_EXT, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Texture LOD Bias");
        IoSetItemText(L"%d", i_data);
    }

    IoAddFooter();
    IoAddHeaderString(0, 0, L"Max Stack Depth");

    glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Attribute Stack");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Client Attribute Stack");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Modelview Matrix Stack");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_NAME_STACK_DEPTH, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Name Stack");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Projection Matrix Stack");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Texture Matrix Stack");
        IoSetItemText(L"%d", i_data);
    }

    IoAddFooter();
    IoAddHeaderString(0, 0, L"Draw Range Elements");

    glGetIntegerv(GL_MAX_ELEMENTS_INDICES_WIN, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Index Count");
        IoSetItemText(L"%d", i_data);
    }

    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES_WIN, &i_data);
    if (i_data)
    {
        IoAddItem(1, 0, L"Max Vertex Count");
        IoSetItemText(L"%d", i_data);
    }

    IoAddFooter();
    IoAddHeader(0, 0, IDS_OPENGL_EXTENSIONS);

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
            IoAddItem(1, OpenglExtensions[k].IsSupported ? 1 : 2,
                      OpenglExtensions[k].lpExtName);
            IoSetItemText(OpenglExtensions[k].IsSupported ? szSupported : szUnsupported);
        }
        while (OpenglExtensions[++k].lpExtName != NULL);
    }

Cleanup:
    /* Cleanup */
    wglMakeCurrent(NULL, NULL);
    if (hRC != NULL) wglDeleteContext(hRC);

    ReleaseDC(DllParams.hMainWnd, hDC);

    DebugEndReceiving();
}