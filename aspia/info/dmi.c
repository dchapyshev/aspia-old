/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/dmi.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"


VOID
AddDMIFooter(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    if (IoGetTarget() != IO_TARGET_LISTVIEW ||
        ListView_GetItemCount(hListView) == 0)
        return;

    IoAddIcon(IDI_BANG);
    IoAddFooter();

    Index = IoAddValueName(0, IDS_DMI_HEADER_TITLE, 1);

    LoadMUIString(IDS_DMI_HEADER, szText,
                  sizeof(szText)/sizeof(WCHAR));
    IoSetItemText(Index, 1, szText);
}

static BOOL CALLBACK
EnumProcessorsProc(SMBIOS_PROCESSORINFO Info)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;
    FLOAT Voltage;

    ChopSpaces(Info.Version, sizeof(Info.Version));
    IoAddHeaderString(0, Info.Version, 0);

    /* Manufacturer */
    Index = IoAddValueName(0, IDS_MANUFACTURER, -1);
    IoSetItemText(Index, 1, Info.Manufacturer);

    /* Family */
    Index = IoAddValueName(0, IDS_CPU_FAMILY, -1);
    SMBIOS_ProcessorFamilyToText(Info.ProcessorFamily, szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    /* Type */
    Index = IoAddValueName(0, IDS_TYPE, -1);
    SMBIOS_ProcessorTypeToText(Info.ProcessorType, szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    /* Status */
    Index = IoAddValueName(0, IDS_STATUS, -1);
    SMBIOS_ProcessorStatusToText(Info.Status, szText, sizeof(szText));
    if (SafeStrLen(szText) == 0)
    {
        LoadMUIString(IDS_UNKNOWN, szText, MAX_STR_LEN);
    }
    IoSetItemText(Index, 1, szText);

    /* Socket */
    Index = IoAddValueName(0, IDS_CPU_SOCKET, -1);
    IoSetItemText(Index, 1, Info.SocketDesignation);

    /* Upgrade */
    Index = IoAddValueName(0, IDS_CPU_UPGRADE, -1);
    SMBIOS_ProcessorUpgradeToText(Info.Upgrade, szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    /* External clock */
    Index = IoAddValueName(0, IDS_CPU_EXTCLOCK, -1);
    StringCbPrintf(szText, sizeof(szText),
                   L"%d MHz", Info.ExternalClock);
    IoSetItemText(Index, 1, szText);

    /* Current speed */
    Index = IoAddValueName(0, IDS_CPU_CURRENTSPEED, -1);
    StringCbPrintf(szText, sizeof(szText),
                   L"%d MHz", Info.CurentSpeed);
    IoSetItemText(Index, 1, szText);

    /* Max speed */
    Index = IoAddValueName(0, IDS_CPU_MAXSPEED, -1);
    StringCbPrintf(szText, sizeof(szText),
                   L"%d MHz", Info.MaxSpeed);
    IoSetItemText(Index, 1, szText);
            
    if ((Info.Voltage & USE_LEGACY_VOLTAGE_MASK))
    {
        Voltage = (FLOAT) (Info.Voltage & LEGACY_VOLTAGE_MASK);
        Voltage = (FLOAT) Voltage / (FLOAT) 10;
    }
    else
    {
        UCHAR tmp = Info.Voltage & VOLTAGE_MASK;
        if (tmp & _5_VOLTS) 
        {
            Voltage = 5.00;
        }
        else if (tmp & _3_3_VOLTS)
        {
            Voltage = (FLOAT)3.03;
        }
        else if (tmp & _2_9_VOLTS)
        {
            Voltage = (FLOAT)2.09;
        }
        else
        {
            Voltage = (FLOAT)0.00;
        }
    }

    /* Voltage */
    Index = IoAddValueName(0, IDS_CPU_VOLTAGE, -1);
    StringCbPrintf(szText, sizeof(szText),
                   L"%2.2f Volts", (FLOAT)Voltage);
    IoSetItemText(Index, 1, szText);

    return TRUE;
}

VOID
DMI_CPUInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;
    IoAddIcon(IDI_CPU);
    SMBIOS_EnumProcessorDevices(EnumProcessorsProc);
    AddDMIFooter();

    DebugEndReceiving();
}

static BOOL CALLBACK
EnumMemoryDevicesProc(SMBIOS_MEMINFO Info)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    IoAddHeaderString(0, Info.DeviceLocator, 0);

    /* Size */
    Index = IoAddValueName(0, IDS_MEM_SIZE, -1);
    StringCbPrintf(szText, sizeof(szText),
                   L"%d MB", (Info.Size & MEM_SIZE_MASK));
    IoSetItemText(Index, 1, szText);

    /* Type */
    if (Info.MemoryType != 0x02)
    {
        Index = IoAddValueName(0, IDS_TYPE, -1);
        SMBIOS_MemoryDeviceToText(Info.MemoryType, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);
    }

    /* Speed */
    if (Info.Speed > 0)
    {
        Index = IoAddValueName(0, IDS_MEM_SPEED, -1);
        StringCbPrintf(szText, sizeof(szText), L"%d MHz", Info.Speed);
        IoSetItemText(Index, 1, szText);
    }

    /* Form factor */
    Index = IoAddValueName(0, IDS_MEM_FORMFACTOR, -1);
    SMBIOS_FormFactorToText(Info.FormFactor, szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    /* Serial number */
    ChopSpaces(Info.SerialNumber, sizeof(Info.SerialNumber));
    if (SafeStrLen(Info.SerialNumber) > 0)
    {
        Index = IoAddValueName(0, IDS_SERIAL_NUMBER, -1);
        IoSetItemText(Index, 1, Info.SerialNumber);
    }

    /* Part Number */
    ChopSpaces(Info.PartNumber, sizeof(Info.PartNumber));
    if (SafeStrLen(Info.PartNumber) > 0)
    {
        Index = IoAddValueName(0, IDS_MEM_PARTNUMBER, -1);
        IoSetItemText(Index, 1, Info.PartNumber);
    }

    /* Manufacturer */
    ChopSpaces(Info.Manufactuer, sizeof(Info.Manufactuer));
    if (SafeStrLen(Info.Manufactuer) > 0)
    {
        Index = IoAddValueName(0, IDS_MANUFACTURER, -1);
        IoSetItemText(Index, 1, Info.Manufactuer);
    }

    /* Bank */
    Index = IoAddValueName(0, IDS_MEM_BANK, -1);
    IoSetItemText(Index, 1, Info.Bank);

    /* Total width */
    Index = IoAddValueName(0, IDS_MEM_TOTALWIDTH, -1);
    StringCbPrintf(szText, sizeof(szText), L"%d bit", Info.TotalWidth);
    IoSetItemText(Index, 1, szText);

    /* Data width */
    Index = IoAddValueName(0, IDS_MEM_DATAWIDTH, -1);
    StringCbPrintf(szText, sizeof(szText), L"%d bit", Info.DataWidth);
    IoSetItemText(Index, 1, szText);

    return TRUE;
}

VOID
DMI_RAMInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;
    IoAddIcon(IDI_HW);
    SMBIOS_EnumMemoryDevices(EnumMemoryDevicesProc);
    AddDMIFooter();

    DebugEndReceiving();
}

VOID
DMI_BIOSInfo(VOID)
{
    SMBIOS_BIOSINFO Info;
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;

    IoAddIcon(IDI_CPU);

    SMBIOS_GetBiosInformation(&Info);

    IoAddHeader(0, IDS_CAT_HW_DMI_BIOS, 0);

    /* Manufacturer */
    Index = IoAddValueName(0, IDS_MANUFACTURER, -1);
    IoSetItemText(Index, 1, Info.Vendor);

    /* Version */
    Index = IoAddValueName(0, IDS_VERSION, -1);
    IoSetItemText(Index, 1, Info.BiosVersion);

    /* Date */
    Index = IoAddValueName(0, IDS_BIOS_DATE, -1);
    IoSetItemText(Index, 1, Info.BiosReleaseDate);

    /* Size */
    Index = IoAddValueName(0, IDS_BIOS_SIZE, -1);
    StringCbPrintf(szText, sizeof(szText),
                   L"0x%02x", Info.BiosRomSize);
    IoSetItemText(Index, 1, szText);

    /* Boot devices */
    StringCbCopy(szText, sizeof(szText), L"");
    if (Info.BiosCharacteristics & BIOS_CD_BOOT_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"CD-ROM, ");
    if (Info.BiosCharacteristics & BIOS_PCCARD_BOOT_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"PCCARD, ");
    if (Info.BiosExtension1 & BIOS_ATAPI_ZIP_BOOT_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"ATAPI ZIP, ");
    if (Info.BiosExtension1 & BIOS_BOOT_1394_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"IEEE1394, ");
    szText[SafeStrLen(szText) - 2] = L'\0';
    if (SafeStrLen(szText) > 0)
    {
        Index = IoAddValueName(0, IDS_BIOS_BOOT_DEVICES, -1);
        IoSetItemText(Index, 1, szText);
    }

    /* Fatures */
    Index = IoAddValueName(0, IDS_BIOS_FATURES, -1);

    StringCbCopy(szText, sizeof(szText), L"");
    if (Info.BiosCharacteristics & BIOS_FLASHABLE)
        StringCbCat(szText, sizeof(szText), L"Flash BIOS, ");
    if (Info.BiosCharacteristics & BIOS_SHADOW_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"Shadow BIOS, ");
    if (Info.BiosCharacteristics & BIOS_SELECT_BOOT_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"Select boot device, ");
    if (Info.BiosCharacteristics & BIOS_ROM_SOCKETED)
        StringCbCat(szText, sizeof(szText), L"Socketed BIOS, ");
    if (Info.BiosExtension2 & BIOS_BOOT_SPEC_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"BBS, ");
    szText[SafeStrLen(szText) - 2] = L'\0';
    IoSetItemText(Index, 1, szText);

    /* Supported standards */
    Index = IoAddValueName(0, IDS_BIOS_STANDATDS, -1);

    StringCbCopy(szText, sizeof(szText), L"");
    if (Info.BiosCharacteristics & BIOS_APM_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"APM, ");
    if (Info.BiosCharacteristics & BIOS_PNP_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"PnP, ");
    if (Info.BiosCharacteristics & BIOS_ESCD_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"ESCD, ");
    if (Info.BiosExtension1 & BIOS_ACPI_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"ACPI, ");
    szText[SafeStrLen(szText) - 2] = L'\0';
    IoSetItemText(Index, 1, szText);

    /* Slots */
    Index = IoAddValueName(0, IDS_BIOS_SLOTS, -1);

    StringCbCopy(szText, sizeof(szText), L"");
    if (Info.BiosCharacteristics & BIOS_ISA_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"ISA, ");
    if (Info.BiosCharacteristics & BIOS_MCS_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"MCS, ");
    if (Info.BiosCharacteristics & BIOS_ESIA_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"ESIA, ");
    if (Info.BiosCharacteristics & BIOS_PCI_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"PCI, ");
    if (Info.BiosCharacteristics & BIOS_PCMCIA_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"PCMCIA, ");
    if (Info.BiosExtension1 & BIOS_AGP_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"AGP, ");
    if (Info.BiosExtension1 & BIOS_USB_LEGACY_SUPPORTED)
        StringCbCat(szText, sizeof(szText), L"USB, ");
    szText[SafeStrLen(szText) - 2] = L'\0';
    IoSetItemText(Index, 1, szText);

    AddDMIFooter();

    DebugEndReceiving();
}

VOID
DMI_SystemInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    SMBIOS_SYSINFO Info;
    INT Index;

    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;

    IoAddIcon(IDI_COMPUTER);

    SMBIOS_GetSystemInformation(&Info);

    IoAddHeader(0, IDS_CAT_HW_DMI_SYSTEM, 0);

    /* Vendor */
    Index = IoAddValueName(0, IDS_MANUFACTURER, -1);
    IoSetItemText(Index, 1, Info.Manufacturer);

    /* Product */
    Index = IoAddValueName(0, IDS_PRODUCT, -1);
    IoSetItemText(Index, 1, Info.ProductName);

    /* Version */
    ChopSpaces(Info.Version, sizeof(Info.Version));
    if (SafeStrLen(Info.Version) > 0)
    {
        Index = IoAddValueName(0, IDS_VERSION, -1);
        IoSetItemText(Index, 1, Info.Version);
    }

    /* Serial number */
    ChopSpaces(Info.SerialNumber, sizeof(Info.SerialNumber));
    if (SafeStrLen(Info.SerialNumber) > 0)
    {
        Index = IoAddValueName(0, IDS_SERIAL_NUMBER, -1);
        IoSetItemText(Index, 1, Info.SerialNumber);
    }

    /* ID */
    if (Info.Uuid.TimeLow && Info.Uuid.TimeHiAndVersion && Info.Uuid.TimeMid)
    {
        Index = IoAddValueName(0, IDS_SYS_ID, -1);
        StringCbPrintf(szText,
                       sizeof(szText),
                       L"%08x-%04x%04x-%02x%02x%02x%02x-%02x%02x%02x%02x",
                       Info.Uuid.TimeLow, 
                       Info.Uuid.TimeHiAndVersion, 
                       Info.Uuid.TimeMid, 
                       Info.Uuid.Node[0x01],
                       Info.Uuid.Node[0x00],
                       Info.Uuid.ClockLowSeq,
                       Info.Uuid.ClockSeqHiAndReserved,
                       Info.Uuid.Node[0x05],
                       Info.Uuid.Node[0x04],
                       Info.Uuid.Node[0x03],
                       Info.Uuid.Node[0x02]);
        IoSetItemText(Index, 1, szText);
    }

    /* Wakeup type */
    Index = IoAddValueName(0, IDS_SYS_WAKEUP, -1);
    SMBIOS_WakeupTypeToText(Info.Wakeup, szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    AddDMIFooter();

    DebugEndReceiving();
}

VOID
DMI_BoardInfo(VOID)
{
    SMBIOS_BOARDINFO Info;
    INT Index;

    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;

    IoAddIcon(IDI_HW);
    IoAddHeader(0, IDS_CAT_HW_DMI_BOARD, 0);

    SMBIOS_GetBoardInformation(&Info);

    /* Vendor */
    Index = IoAddValueName(0, IDS_MANUFACTURER, -1);
    IoSetItemText(Index, 1, Info.Manufacturer);

    /* Product */
    Index = IoAddValueName(0, IDS_PRODUCT, -1);
    IoSetItemText(Index, 1, Info.Product);

    /* Version */
    Index = IoAddValueName(0, IDS_VERSION, -1);
    IoSetItemText(Index, 1, Info.Version);

    /* Serial number */
    ChopSpaces(Info.SerialNumber, sizeof(Info.SerialNumber));
    if (SafeStrLen(Info.SerialNumber) != 0)
    {
        Index = IoAddValueName(0, IDS_SERIAL_NUMBER, -1);
        IoSetItemText(Index, 1, Info.SerialNumber);
    }

    AddDMIFooter();

    DebugEndReceiving();
}

static BOOL CALLBACK
EnumCacheDevicesProc(SMBIOS_CACHEINFO Info)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;
    UCHAR SramType = Info.SupportedSramType & SRAM_TYPE_MASK;
    USHORT Config = 0;

    IoAddHeaderString(0, Info.SocketDesignation, 0);

    /* Cache type */
    Index = IoAddValueName(0, IDS_TYPE, -1);

    Config = (Info.Configuration & CACHE_LOCATION_MASK) >> 5;
    if (Config == CACHE_INTERNAL)
        StringCbCopy(szText, sizeof(szText), L"Internal");
    if (Config == CACHE_EXTERNAL)
        StringCbCopy(szText, sizeof(szText), L"External");
    if (Config == CACHE_RESERVED)
        StringCbCopy(szText, sizeof(szText), L"Reserved");
    if (Config == CACHE_UNKNOWN4)
        StringCbCopy(szText, sizeof(szText), L"Unknown type");
    IoSetItemText(Index, 1, szText);

    /* Cache status */
    Index = IoAddValueName(0, IDS_STATUS, -1);

    Config = (Info.Configuration & CACHE_ENABLED_MASK) >> 7;
    if (Config == CACHE_ENABLED)
        StringCbCopy(szText, sizeof(szText), L"Enabled");
    if (Config == CACHE_NOT_ENABLED)
        StringCbCopy(szText, sizeof(szText), L"Disabled");
    IoSetItemText(Index, 1, szText);

    /* Cache mode */
    Index = IoAddValueName(0, IDS_DMI_CACHE_MODE, -1);

    Config = (Info.Configuration & CACHE_OPERATIONAL_MODE_MASK) >> 8;
    if (Config == CACHE_WRITE_THRU)
        StringCbCopy(szText, sizeof(szText), L"Write Thru");
    if (Config == CACHE_WRITE_BACK)
        StringCbCopy(szText, sizeof(szText), L"Write Back");
    if (Config == CACHE_PER_ADDRESS)
        StringCbCopy(szText, sizeof(szText), L"Write varies w/ address");
    if (Config == CACHE_UNKNOWN4)
        StringCbCopy(szText, sizeof(szText), L"Unknown write capability");
    IoSetItemText(Index, 1, szText);

    /* Max cache size */
    if ((Info.MaxCacheSize & CACHE_SIZE_MASK) > 0)
    {
        Index = IoAddValueName(0, IDS_DMI_CACHE_MAX_SIZE, -1);
        StringCbPrintf(szText, sizeof(szText), L"%d KB",
                       Info.MaxCacheSize & CACHE_SIZE_MASK);
        IoSetItemText(Index, 1, szText);
    }

    /* Max cache size */
    if ((Info.InstalledSize & CACHE_SIZE_MASK) > 0)
    {
        Index = IoAddValueName(0, IDS_DMI_CACHE_CURRENT_SIZE, -1);
        StringCbPrintf(szText, sizeof(szText), L"%d KB",
                       Info.InstalledSize & CACHE_SIZE_MASK);
        IoSetItemText(Index, 1, szText);
    }

    /* Supported SRAM type */
    Index = IoAddValueName(0, IDS_DMI_CACHE_SRAM_TYPES, -1);

    szText[0] = 0;
    if (SramType & CACHE_OTHER)
        StringCbCat(szText, sizeof(szText), L"Other, ");
    if (SramType & CACHE_UNKNOWN2)
        StringCbCat(szText, sizeof(szText), L"Unknown, ");
    if (SramType & CACHE_NON_BURST)
        StringCbCat(szText, sizeof(szText), L"Non-burst, ");
    if (SramType & CACHE_BURST)
        StringCbCat(szText, sizeof(szText), L"Burst, ");
    if (SramType & CACHE_PIPELINE)
        StringCbCat(szText, sizeof(szText), L"Pipeline brust, ");
    if (SramType & CACHE_SYNCHRONOUS)
        StringCbCat(szText, sizeof(szText), L"Synchronous, ");
    if (SramType & CACHE_ASYNCHRONOUS)
        StringCbCat(szText, sizeof(szText), L"Asynchronous, ");
    szText[SafeStrLen(szText) - 2] = 0;
    IoSetItemText(Index, 1, szText);

    /* Current SRAM type */
    SMBIOS_SramToText(Info.CurrentSramType & SRAM_TYPE_MASK,
                      szText, sizeof(szText));
    ChopSpaces(szText, sizeof(szText));
    if (SafeStrLen(szText) > 0)
    {
        Index = IoAddValueName(0, IDS_DMI_CACHE_SRAM_CURRENT, -1);
        IoSetItemText(Index, 1, szText);
    }

    /* Cache level */
    Index = IoAddValueName(0, IDS_DMI_CACHE_LEVEL, -1);

    Config = Info.Configuration & CACHE_LEVEL_MASK;
    if (Config == CACHE_L1)
        StringCbCopy(szText, sizeof(szText), L"L1");
    if (Config == CACHE_L2)
        StringCbCopy(szText, sizeof(szText), L"L2");
    if (Config == CACHE_L3)
        StringCbCopy(szText, sizeof(szText), L"L3");
    IoSetItemText(Index, 1, szText);

    return TRUE;
}

VOID
DMI_CacheInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;
    IoAddIcon(IDI_SENSOR);
    SMBIOS_EnumCacheDevices(EnumCacheDevicesProc);
    AddDMIFooter();

    DebugEndReceiving();
}

static BOOL CALLBACK
EnumSlotInformationProc(SMBIOS_SLOTINFO Info)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    IoAddHeaderString(0, Info.SlotDesignation, 0);

    /* Type */
    Index = IoAddValueName(0, IDS_TYPE, -1);
    SMBIOS_SlotTypeToText(Info.SlotType, szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    /* Bus width */
    Index = IoAddValueName(0, IDS_SLOT_BUS_WIDTH, -1);
    SMBIOS_BusWidthToText(Info.SlotDataBusWidth,
                          szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    /* Length */
    Index = IoAddValueName(0, IDS_SLOT_LENGTH, -1);
    SMBIOS_SlotLengthToText(Info.SlotLength, szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    return TRUE;
}

VOID
DMI_SlotInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;
    IoAddIcon(IDI_PORT);
    SMBIOS_EnumSlotsInformation(EnumSlotInformationProc);
    AddDMIFooter();

    DebugEndReceiving();
}

static BOOL CALLBACK
EnumPortsInformationProc(SMBIOS_PORTINFO Info)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    ChopSpaces(Info.ExtRefDesignation, sizeof(Info.ExtRefDesignation));
    if (SafeStrLen(Info.ExtRefDesignation) > 0)
        IoAddHeaderString(0, Info.ExtRefDesignation, 0);
    else
        IoAddHeaderString(0, Info.IntRefDesignation, 0);

    /* Type */
    Index = IoAddValueName(0, IDS_PORT_TYPE, -1);
    SMBIOS_PortTypesToText(Info.PortType,
                           szText, sizeof(szText));
    if (SafeStrLen(szText) < 1)
        LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
    IoSetItemText(Index, 1, szText);

    /* Internal designation */
    Index = IoAddValueName(0, IDS_PORT_INT_DESIGNATION, -1);
    if (SafeStrLen(Info.IntRefDesignation) >= 1)
    {
        IoSetItemText(Index, 1, Info.IntRefDesignation);
    }
    else
    {
        LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
        IoSetItemText(Index, 1, szText);
    }

    /* Internal connector type */
    Index = IoAddValueName(0, IDS_PORT_INT_CONNECTOR, -1);
    SMBIOS_PortConnectorToText(Info.IntConnectorType,
                               szText, sizeof(szText));
    if (SafeStrLen(szText) < 1)
        LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
    IoSetItemText(Index, 1, szText);

    /* External designation */
    Index = IoAddValueName(0, IDS_PORT_EXT_DESIGNATION, -1);
    if (SafeStrLen(Info.ExtRefDesignation) > 0)
    {
        IoSetItemText(Index, 1, Info.ExtRefDesignation);
    }
    else
    {
        LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
        IoSetItemText(Index, 1, szText);
    }

    /* External connector type */
    Index = IoAddValueName(0, IDS_PORT_EXT_CONNECTOR, -1);

    SMBIOS_PortConnectorToText(Info.ExtConnectorType,
                               szText, sizeof(szText));
    if (SafeStrLen(szText) < 1)
        LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
    IoSetItemText(Index, 1, szText);

    return TRUE;
}

VOID
DMI_PortsInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;
    IoAddIcon(IDI_PORT);
    SMBIOS_EnumPortsInformation(EnumPortsInformationProc);
    AddDMIFooter();

    DebugEndReceiving();
}

static BOOL CALLBACK
EnclosureEnumProc(SMBIOS_ENCLINFO Info)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    IoAddHeader(0, IDS_ENCL_ID, 0);

    /* Vendor */
    Index = IoAddValueName(0, IDS_MANUFACTURER, -1);
    IoSetItemText(Index, 1, Info.Manufacturer);

    /* Version */
    ChopSpaces(Info.Version, sizeof(Info.Version));
    if (SafeStrLen(Info.Version) > 0)
    {
        Index = IoAddValueName(0, IDS_VERSION, -1);
        IoSetItemText(Index, 1, Info.Version);
    }

    /* Serial number */
    ChopSpaces(Info.SerialNumber, sizeof(Info.SerialNumber));
    if (SafeStrLen(Info.SerialNumber) != 0)
    {
        Index = IoAddValueName(0, IDS_SERIAL_NUMBER, -1);
        IoSetItemText(Index, 1, Info.SerialNumber);
    }

    /* Type */
    Index = IoAddValueName(0, IDS_ENCL_TYPE, -1);
    SMBIOS_EnclTypeToText(Info.Type, szText, sizeof(szText));
    if (SafeStrLen(szText) == 0)
    {
        LoadMUIString(IDS_UNKNOWN, szText, MAX_STR_LEN);
    }
    IoSetItemText(Index, 1, szText);

    /* OS Load status */
    if (Info.BootUpState != 0x02)
    {
        Index = IoAddValueName(0, IDS_ENCL_OSLOAD_STATUS, -1);
        SMBIOS_EnclStateToText(Info.BootUpState, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);
    }

    /* Power source status */
    if (Info.PowerSupplyState != 0x02)
    {
        Index = IoAddValueName(0, IDS_ENCL_POWER_STATUS, -1);
        SMBIOS_EnclStateToText(Info.PowerSupplyState, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);
    }

    /* Temperature status */
    if (Info.ThermalState != 0x02)
    {
        Index = IoAddValueName(0, IDS_ENCL_TEMPERATURE_STATUS, -1);
        SMBIOS_EnclStateToText(Info.ThermalState, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);
    }

    /* Security status */
    if (Info.SecurityStatus != 0x02)
    {
        Index = IoAddValueName(0, IDS_ENCL_SECURITY_STATUS, -1);
        SMBIOS_EnclSecStatusToText(Info.SecurityStatus, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);
    }

    return TRUE;
}

VOID
DMI_EnclosureInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;
    IoAddIcon(IDI_COMPUTER);
    SMBIOS_EnumEnclosureInformation(EnclosureEnumProc);
    AddDMIFooter();

    DebugEndReceiving();
}

static BOOL CALLBACK
OnboardEnumProc(SMBIOS_ONBOARDINFO Info)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    IoAddHeaderString(0, Info.Description, 0);

    /* Desc */
    Index = IoAddValueName(0, IDS_DMI_ONBOARD_DESC, -1);
    IoSetItemText(Index, 1, Info.Description);

    /* Type */
    Index = IoAddValueName(0, IDS_DMI_ONBOARD_TYPE, -1);
    SMBIOS_OnboardDeviceTypeToText(Info.DeviceType, szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    /* Status */
    Index = IoAddValueName(0, IDS_DMI_ONBOARD_STATUS, -1);
    IoSetItemText(Index, 1, Info.DeviceStatus ? L"Enabled" : L"Disabled");

    return TRUE;
}

VOID
DMI_OnboardInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;
    IoAddIcon(IDI_HW);
    SMBIOS_EnumOnboardInformation(OnboardEnumProc);
    AddDMIFooter();

    DebugEndReceiving();
}

static BOOL CALLBACK
BatteryEnumProc(SMBIOS_BATTERYINFO Info)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    IoAddHeader(0, IDS_BAT_BATTERYS, 0);

    /* Location */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_LOCATION, -1);
    IoSetItemText(Index, 1, Info.Location);

    /* Manufacturer */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_MANUFACTURER, -1);
    IoSetItemText(Index, 1, Info.Manufacturer);

    /* Manufacture Date */
    if (SafeStrLen(Info.MenufDate) > 0)
    {
        Index = IoAddValueName(0, IDS_DMI_BATTERY_MANUFDATE, -1);
        IoSetItemText(Index, 1, Info.MenufDate);
    }

    /* Serial Number */
    if (SafeStrLen(Info.SerialNumber) > 0)
    {
        Index = IoAddValueName(0, IDS_DMI_BATTERY_SERIAL, -1);
        IoSetItemText(Index, 1, Info.SerialNumber);
    }

    /* Device Name */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_DEVICE_NAME, -1);
    IoSetItemText(Index, 1, Info.DeviceName);

    /* Device Chemistry */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_DEVICE_CHEMISTRY, -1);
    SMBIOS_BatteryChemistryToText(Info.DeviceChemistry, szText, sizeof(szText));
    IoSetItemText(Index, 1, szText);

    /* Design Capacity */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_DESIGN_CAPACITY, -1);
    StringCbPrintf(szText, sizeof(szText), L"%u mWh",
        Info.DesignCapacityMultiplier ? Info.DesignCapacity * Info.DesignCapacityMultiplier : Info.DesignCapacity);
    IoSetItemText(Index, 1, szText);

    /* Design Voltage */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_DESIGN_VOLTAGE, -1);
    StringCbPrintf(szText, sizeof(szText), L"%u mV", Info.DesignVoltage);
    IoSetItemText(Index, 1, szText);

    /* SBDS Version Number */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_SBDS_VERSION, -1);
    IoSetItemText(Index, 1, Info.SBDSVersionNumber);

    /* Max. Error in Battery Data */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_MAXERROR_IN_DATA, -1);
    StringCbPrintf(szText, sizeof(szText), L"%u", Info.MaxErrorInData);
    IoSetItemText(Index, 1, szText);

    /* SBDS Serial Number */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_SBDS_SERIAL, -1);
    StringCbPrintf(szText, sizeof(szText), L"%u", Info.SBDSSerialNumber);
    IoSetItemText(Index, 1, szText);

    /* SBDS Manufacture Date */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_SBDS_MANUFDATE, -1);
    StringCbPrintf(szText, sizeof(szText), L"%d/%d/%d",
                   (Info.SBDSManufDate & 0x10), /* Bits 4:0 */
                   (Info.SBDSManufDate & 0x100) >> 0x4, /* Bits 8:5 */
                   ((Info.SBDSManufDate & 0x8000) >> 0x8) + 1980 /* Bits 15:9 */);
    IoSetItemText(Index, 1, szText);

    /* SBDS Device Chemistry */
    Index = IoAddValueName(0, IDS_DMI_BATTERY_SBDS_DEV_CHEMIST, -1);
    IoSetItemText(Index, 1, Info.SBDSDeviceChemistry);

    return TRUE;
}

VOID
DMI_BatteryInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBIOSInitialized) return;
    IoAddIcon(IDI_BATTERY);
    SMBIOS_EnumBatteryInformation(BatteryEnumProc);
    AddDMIFooter();

    DebugEndReceiving();
}
