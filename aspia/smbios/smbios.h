/*
 * PROJECT:         Aspia
 * FILE:            aspia/smbios/smbios.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

/* External functions */

#include <windows.h>
#include <wchar.h>

#define MAX_STRING             0x3E8  /* 1K */
#define MAX_CONTAINED_ELEMENTS 255
#define MAX_CONTAINED_RECORD   255

BOOL InitSmBIOSData(VOID);

extern BOOL IsSmBIOSInitialized;

/* Memory Information */
#define MEM_SIZE_MASK    0x07FFF

typedef struct
{                            
    USHORT TotalWidth;
    USHORT DataWidth;
    USHORT Size;
    UCHAR FormFactor;
    WCHAR DeviceLocator[MAX_STRING * sizeof(WCHAR)];
    WCHAR Bank[MAX_STRING * sizeof(WCHAR)];
    UCHAR MemoryType;
    USHORT Speed;
    WCHAR Manufactuer[MAX_STRING * sizeof(WCHAR)];
    WCHAR SerialNumber[MAX_STRING * sizeof(WCHAR)];
    WCHAR PartNumber[MAX_STRING * sizeof(WCHAR)];
}
SMBIOS_MEMINFO;

typedef BOOL (CALLBACK *SMBIOS_MEMENUMPROC)(SMBIOS_MEMINFO Info);
BOOL SMBIOS_EnumMemoryDevices(SMBIOS_MEMENUMPROC lpMemEnumProc);
VOID SMBIOS_FormFactorToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);
VOID SMBIOS_MemoryDeviceToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);

/* Cache Information */
#define SRAM_TYPE_MASK       0x7F
#define CACHE_OTHER          0x01
#define CACHE_UNKNOWN2       0x02
#define CACHE_NON_BURST      0x04
#define CACHE_BURST          0x08
#define CACHE_PIPELINE       0x10 
#define CACHE_SYNCHRONOUS    0x20
#define CACHE_ASYNCHRONOUS   0x40

#define CACHE_SIZE_MASK      0x7FFF

#define CACHE_LEVEL_MASK            0x07
#define CACHE_L1                    0x00
#define CACHE_L2                    0x01
#define CACHE_L3                    0x02
#define CACHE_SOCKETED_MASK         0x08
#define CACHE_SOCKETED              0x01
#define CACHE_NOT_SOCKETED          0x00
#define CACHE_LOCATION_MASK         0x60
#define CACHE_INTERNAL              0x00
#define CACHE_EXTERNAL              0x01
#define CACHE_RESERVED              0x02
#define CACHE_ENABLED_MASK          0x80
#define CACHE_ENABLED               0x01
#define CACHE_NOT_ENABLED           0x00
#define CACHE_OPERATIONAL_MODE_MASK 0x300
#define CACHE_WRITE_THRU            0x00
#define CACHE_WRITE_BACK            0x01
#define CACHE_PER_ADDRESS           0x02
#define CACHE_UNKNOWN4              0x04

typedef struct
{
    WCHAR SocketDesignation[MAX_STRING * sizeof(WCHAR)];
    USHORT Configuration;
    USHORT MaxCacheSize;
    USHORT InstalledSize;
    USHORT SupportedSramType;
    USHORT CurrentSramType;
}
SMBIOS_CACHEINFO;

typedef BOOL (CALLBACK *SMBIOS_CACHEENUMPROC)(SMBIOS_CACHEINFO Info);
BOOL SMBIOS_EnumCacheDevices(SMBIOS_CACHEENUMPROC lpCacheEnumProc);
VOID SMBIOS_SramToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);

/* Processor Information */
#define USE_LEGACY_VOLTAGE_MASK 0x80
#define LEGACY_VOLTAGE_MASK     0x7F
#define VOLTAGE_MASK            0x0F
#define _5_VOLTS                0x01
#define _3_3_VOLTS              0x02
#define _2_9_VOLTS              0x04

typedef struct
{
    WCHAR SocketDesignation[MAX_STRING * sizeof(WCHAR)];
    UCHAR ProcessorType;
    USHORT ProcessorFamily;
    WCHAR Manufacturer[MAX_STRING * sizeof(WCHAR)];
    WCHAR Version[MAX_STRING * sizeof(WCHAR)];
    UCHAR Voltage;
    USHORT ExternalClock;
    USHORT MaxSpeed;
    USHORT CurentSpeed;
    UCHAR Status;
    UCHAR Upgrade;
}
SMBIOS_PROCESSORINFO;

typedef BOOL (CALLBACK *SMBIOS_PROCESSORENUMPROC)(SMBIOS_PROCESSORINFO Info);
BOOL SMBIOS_EnumProcessorDevices(SMBIOS_PROCESSORENUMPROC lpProcessorEnumProc);
VOID SMBIOS_ProcessorTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);
VOID SMBIOS_ProcessorFamilyToText(USHORT Form, LPWSTR lpszText, SIZE_T Size);
VOID SMBIOS_ProcessorStatusToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);
VOID SMBIOS_ProcessorUpgradeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);

/* BIOS Information */
/* BIOS Characteristics */
#define BIOS_RESERVED1                            0x01 
#define BIOS_RESERVED2                            0x02 
#define BIOS_UNKNOWN                              0x04 
#define BIOS_NOT_SUPPORTED8                       0x08 
#define BIOS_ISA_SUPPORTED                        0x10 
#define BIOS_MCS_SUPPORTED                        0x20 
#define BIOS_ESIA_SUPPORTED                       0x40 
#define BIOS_PCI_SUPPORTED                        0x80 
#define BIOS_PCMCIA_SUPPORTED                     0x100
#define BIOS_PNP_SUPPORTED                        0x200 
#define BIOS_APM_SUPPORTED                        0x400
#define BIOS_FLASHABLE                            0x800
#define BIOS_SHADOW_SUPPORTED                     0x1000
#define BIOS_VL_VESA_SUPPORTED                    0x2000
#define BIOS_ESCD_SUPPORTED                       0x4000
#define BIOS_CD_BOOT_SUPPORTED                    0x8000 
#define BIOS_SELECT_BOOT_SUPPORTED                0x10000
#define BIOS_ROM_SOCKETED                         0x20000
#define BIOS_PCCARD_BOOT_SUPPORTED                0x40000
#define BIOS_EDD_SUPPORTED                        0x80000
#define BIOS_INT_13H_JAP_FLOPPY_NEC_SUPPORTED     0x100000
#define BIOS_INT_13H_JAP_FLOPPY_TOSHIBA_SUPPORTED 0x200000
#define BIOS_INT_13H_525_360KB_FLOPPY_SUPPORTED   0x400000
#define BIOS_INT_13H_525_12MB_FLOPPY_SUPPORTED    0x800000
#define BIOS_INT_13H_35_720KB_FLOPPY_SUPPORTED    0x1000000 
#define BIOS_INT_13H_35_288MB_FLOPPY_SUPPORTED    0x2000000
#define BIOS_INT_5H_PRINT_SCREEN_SUPPORTED        0x4000000
#define BIOS_INT_9H_842_KEYBD_SUPPORTED           0x8000000
#define BIOS_INT_14H_SERIAL_SUPPORTED             0x10000000 
#define BIOS_INT_17H_PRINTER_SUPPORTED            0x20000000 
#define BIOS_INT_10H_CGA_MONO_VIDEO_SUPPORTED     0x40000000 
#define BIOS_NEC_PC_98                            0x80000000

/* BIOS characteristics extension byte 1 */
#define BIOS_ACPI_SUPPORTED           0x01 
#define BIOS_USB_LEGACY_SUPPORTED     0x02 
#define BIOS_AGP_SUPPORTED            0x04 
#define BIOS_I20_SUPPORTED            0x08 
#define BIOS_LS120_SUPPORTED          0x10 
#define BIOS_ATAPI_ZIP_BOOT_SUPPORTED 0x20
#define BIOS_BOOT_1394_SUPPORTED      0x40 
#define BIOS_SMART_BATTERY_SUPPORTED  0x80 

/* BIOS characteristics extension byte 2 */
#define BIOS_BOOT_SPEC_SUPPORTED              0x01 
#define BIOS_FUNC_KEY_INIT_NET_BOOT_SUPPORTED 0x02 
#define BIOS_TARGET_CONTENT_DIST_SUPPORTED    0x04 

typedef struct
{
    WCHAR Vendor[MAX_STRING * sizeof(WCHAR)];
    WCHAR BiosVersion[MAX_STRING * sizeof(WCHAR)];
    WCHAR BiosReleaseDate[MAX_STRING * sizeof(WCHAR)];
    UCHAR BiosRomSize;
    ULONG BiosCharacteristics;
    UCHAR BiosExtension1;
    UCHAR BiosExtension2;
}
SMBIOS_BIOSINFO;

BOOL SMBIOS_GetBiosInformation(SMBIOS_BIOSINFO *BiosInfo);

/* System Information */
typedef struct
{
    ULONG TimeLow;
    USHORT TimeMid;
    USHORT TimeHiAndVersion;
    UCHAR ClockSeqHiAndReserved;
    UCHAR ClockLowSeq;
    UCHAR Node[0x06];
} SystemUuid;

typedef struct
{
    WCHAR Manufacturer[MAX_STRING * sizeof(WCHAR)];
    WCHAR ProductName[MAX_STRING * sizeof(WCHAR)];
    WCHAR Version[MAX_STRING * sizeof(WCHAR)];
    WCHAR SerialNumber[MAX_STRING * sizeof(WCHAR)];
    SystemUuid Uuid;
    UCHAR Wakeup;
}
SMBIOS_SYSINFO;

BOOL SMBIOS_GetSystemInformation(SMBIOS_SYSINFO *SysInfo);
VOID SMBIOS_WakeupTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);

/* Enclusure Information */
typedef struct
{
    WCHAR Manufacturer[MAX_STRING * sizeof(WCHAR)];
    UCHAR Type;
    WCHAR Version[MAX_STRING * sizeof(WCHAR)];
    WCHAR SerialNumber[MAX_STRING * sizeof(WCHAR)];
    UCHAR BootUpState;
    UCHAR PowerSupplyState;
    UCHAR ThermalState;
    UCHAR SecurityStatus;
    UCHAR Height;
}
SMBIOS_ENCLINFO;

typedef BOOL (CALLBACK *SMBIOS_ENCLENUMPROC)(SMBIOS_ENCLINFO Info);
BOOL SMBIOS_EnumEnclosureInformation(SMBIOS_ENCLENUMPROC lpEnclEnumProc);
VOID SMBIOS_EnclStateToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);
VOID SMBIOS_EnclTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);
VOID SMBIOS_EnclSecStatusToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);

/* 7.3 Baseboard Information (Type 2) */
typedef struct
{
    WCHAR Manufacturer[MAX_STRING * sizeof(WCHAR)];
    WCHAR Product[MAX_STRING * sizeof(WCHAR)];
    WCHAR Version[MAX_STRING * sizeof(WCHAR)];
    WCHAR SerialNumber[MAX_STRING * sizeof(WCHAR)];
}
SMBIOS_BOARDINFO;

BOOL SMBIOS_GetBoardInformation(SMBIOS_BOARDINFO *BoardInfo);

/* 7.10 System Slots (Type 9) */
typedef struct
{
    WCHAR SlotDesignation[MAX_STRING * sizeof(WCHAR)];
    UCHAR SlotType;
    UCHAR SlotDataBusWidth;
    UCHAR SlotLength;
}
SMBIOS_SLOTINFO;

typedef BOOL (CALLBACK *SMBIOS_SLOTENUMPROC)(SMBIOS_SLOTINFO Info);
BOOL SMBIOS_EnumSlotsInformation(SMBIOS_SLOTENUMPROC lpSlotEnumProc);
VOID SMBIOS_SlotTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);
VOID SMBIOS_BusWidthToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);
VOID SMBIOS_SlotLengthToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);

/* 7.9 Port Connector Information (Type 8) */
typedef struct
{
    WCHAR IntRefDesignation[MAX_STRING * sizeof(WCHAR)]; /* 0x04 */
    UCHAR IntConnectorType; /* 0x05 */
    WCHAR ExtRefDesignation[MAX_STRING * sizeof(WCHAR)]; /* 0x06 */
    UCHAR ExtConnectorType; /* 0x07 */
    UCHAR PortType; /* 0x08 */
}
SMBIOS_PORTINFO;

typedef BOOL (CALLBACK *SMBIOS_PORTENUMPROC)(SMBIOS_PORTINFO Info);
BOOL SMBIOS_EnumPortsInformation(SMBIOS_PORTENUMPROC lpPortEnumProc);
VOID SMBIOS_PortTypesToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);
VOID SMBIOS_PortConnectorToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);

/* 7.11 Onboard Device Information (Type 10) */
typedef struct
{
    UCHAR DeviceType; /* 4+2*(n-1) Bits 6:0 */
    UCHAR DeviceStatus; /* 4+2*(n-1) Bit 7 */
    WCHAR Description[MAX_STRING * sizeof(WCHAR)]; /* 5+2*(n-1) */
    /* n - number of devices */
}
SMBIOS_ONBOARDINFO;

typedef BOOL (CALLBACK *SMBIOS_ONBOARDENUMPROC)(SMBIOS_ONBOARDINFO Info);
BOOL SMBIOS_EnumOnboardInformation(SMBIOS_ONBOARDENUMPROC lpOnboardEnumProc);
VOID SMBIOS_OnboardDeviceTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);

/* 7.23 Portable Battery (Type 22) */
typedef struct
{
    WCHAR Location[MAX_STRING * sizeof(WCHAR)]; /* 0x04 */
    WCHAR Manufacturer[MAX_STRING * sizeof(WCHAR)]; /* 0x05 */
    WCHAR MenufDate[MAX_STRING * sizeof(WCHAR)]; /* 0x06 */
    WCHAR SerialNumber[MAX_STRING * sizeof(WCHAR)]; /* 0x07 */
    WCHAR DeviceName[MAX_STRING * sizeof(WCHAR)]; /* 0x08 */
    UCHAR DeviceChemistry; /* 0x09 */
    DWORD DesignCapacity; /* 0x0A */
    DWORD DesignVoltage; /* 0x0C */
    WCHAR SBDSVersionNumber[MAX_STRING * sizeof(WCHAR)]; /* 0x0E */
    UCHAR MaxErrorInData; /* 0x0F */
    DWORD SBDSSerialNumber; /* 0x10 */
    DWORD SBDSManufDate; /* 0x12 */
    WCHAR SBDSDeviceChemistry[MAX_STRING * sizeof(WCHAR)]; /* 0x14 */
    UCHAR DesignCapacityMultiplier; /* 0x15 */
}
SMBIOS_BATTERYINFO;

typedef BOOL (CALLBACK *SMBIOS_BATTERYENUMPROC)(SMBIOS_BATTERYINFO Info);
BOOL SMBIOS_EnumBatteryInformation(SMBIOS_BATTERYENUMPROC lpBatteryEnumProc);
VOID SMBIOS_BatteryChemistryToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size);
