/*
 * PROJECT:         Aspia
 * FILE:            aspia/smbios/smbios_internal.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

/* Internal Stuff */

#include <windows.h>
#include <wchar.h>

#include "smbios.h"

#define MAX_DATA               0xFA00 /* 64K */
#define MAX_STRING_TABLE       0x19   /* 25 */
#define SMALL_STRING           0x64   /* 100 */

typedef struct
{
    BYTE  Used20CallingMethod;
    BYTE  SMBIOSMajorVersion;
    BYTE  SMBIOSMinorVersion;
    BYTE  DmiRevision;
    DWORD Length;
    BYTE  SMBIOSTableData[MAX_DATA];
} RAWSMBIOSDATA;

typedef struct _SMBIOS_ENTRY
{
    CHAR  Anchor[4];
    BYTE  EntryPointCrc;
    BYTE  EntryPointLength;
    BYTE  MajorVer;
    BYTE  MinorVer;
    WORD  MaximumStructureSize;
    BYTE  EntryPointRevision;
    CHAR  FormattedArea[5];
    CHAR  IntermediateAnchor[5];
    BYTE  IntermediateCrc;
    WORD  StructureTableLength;
    DWORD StructureTableAddress;
    WORD  NumberOfSMBIOSStructures;
    BYTE  SMBIOSBCDRevision;
} SMBIOS_ENTRY, *PSMBIOS_ENTRY;

typedef struct
{
    UCHAR dwValue;
    LPWSTR lpszString;
} INFO_STRUCT;

typedef struct
{
    USHORT dwValue;
    LPWSTR lpszString;
} INFO_STRUCT2;

UCHAR SmbiosTableData[MAX_DATA];

/* Information types */
#define BIOS_INFO               0x00 /* Type 0 */
#define SYSTEM_INFO             0x01 /* Type 1 */
#define BOARD_INFO              0x02 /* Type 2 */
#define ENCLOSURE_INFO          0x03 /* Type 3 */
#define PROCESSOR_INFO          0x04 /* Type 4 */
#define CACHE_INFO              0x07 /* Type 7 */
#define PORTS_INFO              0x08 /* Type 8 */
#define SLOTS_INFO              0x09 /* Type 9 */
#define ONBOARD_INFO            0x0A /* Type 10 */
#define OEM_INFO                0x0B /* Type 11 */
#define SYSCONFIG_INFO          0x0C /* Type 12 */
#define PHYS_MEM_ARRAY_INFO     0x10 /* Type 16 */
#define MEMORY_DEVICES_INFO     0x11 /* Type 17 */
#define MEM_ARRAY_MAP_ADDR_INFO 0x13 /* Type 19 */
#define BATTERY_INFO            0x16 /* Type 22 */
#define VOLTAGE_INFO            0x1A /* Type 26 */
#define COOLING_INFO            0x1B /* Type 27 */
#define TEMPERATURE_INFO        0x1C /* Type 28 */
#define SYSTEM_BOOT_INFO        0x20 /* Type 32 */
#define ONBOARD_EXT_INFO        0x29 /* Type 41 */

/* System Information */

/* 7.2.2 System Information - Wake-up Type */
const INFO_STRUCT WakeupTypeList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"APM Timer" },
    { 0x04, L"Modem Ring" },
    { 0x05, L"LAN Remote" },
    { 0x06, L"Power Switch" },
    { 0x07, L"PCI PME#" },
    { 0x08, L"AC Power Restored" },
    { 0 }
};

/* 7.5.1 Processor Information - Processor Type */
const INFO_STRUCT ProcessorTypeList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Central" },
    { 0x04, L"Math" },
    { 0x05, L"DSP" },
    { 0x06, L"Video" },
    { 0 }
};

/* 7.5.2 Processor Information - Processor Family */
const INFO_STRUCT2 ProcessorFamilyList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Intel 8086" },
    { 0x04, L"Intel 80286" },
    { 0x05, L"Intel386" },
    { 0x06, L"Intel486" },
    { 0x07, L"Intel 8087" },
    { 0x08, L"Intel 80287" },
    { 0x09, L"Intel 80387" },
    { 0x0A, L"Intel 80487" },
    { 0x0B, L"Intel Pentium" },
    { 0x0C, L"Intel Pentium Pro" },
    { 0x0D, L"Intel Pentium 2" },
    { 0x0E, L"Pentium with MMX technology" },
    { 0x0F, L"Intel Celeron" },
    { 0x10, L"Intel Pentium 2 Xeon" },
    { 0x11, L"Intel Pentium 3" },
    { 0x12, L"M1 Family" },
    { 0x13, L"M2 Family" },
    { 0x14, L"Intel Celeron M" },
    { 0x15, L"Intel Pentium 4 HT" },
    /* 0x16 - 0x17 Available for assignment */
    { 0x18, L"AMD Duron" },
    { 0x19, L"AMD K5 Family" },
    { 0x1A, L"AMD K6 Family" },
    { 0x1B, L"AMD K6-2" },
    { 0x1C, L"AMD K6-3" },
    { 0x1D, L"AMD Athlon" },
    { 0x1E, L"AMD 29000" },
    { 0x1F, L"AMD K6-2+" },
    { 0x20, L"Power PC" },
    { 0x21, L"Power PC 601" },
    { 0x22, L"Power PC 603" },
    { 0x23, L"Power PC 603+" },
    { 0x24, L"Power PC 604" },
    { 0x25, L"Power PC 620" },
    { 0x26, L"Power PC x704" },
    { 0x27, L"Power PC 750" },
    { 0x28, L"Intel Core Duo" },
    { 0x29, L"Intel Core Duo Mobile" },
    { 0x2A, L"Intel Core Solo Mobile" },
    { 0x2B, L"Intel Atom" },
    /* 0x2C - 0x2F Available for assignment */
    { 0x30, L"Alpha" },
    { 0x31, L"Alpha 21064" },
    { 0x32, L"Alpha 21066" },
    { 0x33, L"Alpha 21164" },
    { 0x34, L"Alpha 21164PC" },
    { 0x35, L"Alpha 21164a" },
    { 0x36, L"Alpha 21264" },
    { 0x37, L"Alpha 21364" },
    { 0x38, L"AMD Turion 2 Ultra Dual-Core Mobile M" },
    { 0x39, L"AMD Turion 2 Dual-Core Mobile M" },
    { 0x3A, L"AMD Athlon 2 Dual-Core M" },
    { 0x3B, L"AMD Opteron 6100 Series" },
    { 0x3C, L"AMD Opteron 4100 Series" },
    { 0x3D, L"AMD Opteron 6200 Series" },
    { 0x3E, L"AMD Opteron 4200 Series" },
    /* 0x3F Available for assignment */
    { 0x40, L"MIPS" },
    { 0x41, L"MIPS R4000" },
    { 0x42, L"MIPS R4200" },
    { 0x43, L"MIPS R4400" },
    { 0x44, L"MIPS R4600" },
    { 0x45, L"MIPS R10000" },
    { 0x46, L"AMD C-Series" },
    { 0x47, L"AMD E-Series" },
    { 0x48, L"AMD S-Series" },
    { 0x49, L"AMD G-Series" },
    /* 0x4A - 0x4F Available for assignment */
    { 0x50, L"SPARC" },
    { 0x51, L"SuperSPARC" },
    { 0x52, L"microSPARC 2" },
    { 0x53, L"microSPARC 2ep" },
    { 0x54, L"UltraSPARC" },
    { 0x55, L"UltraSPARC 2" },
    { 0x56, L"UltraSPARC 2i" },
    { 0x57, L"UltraSPARC 3" },
    { 0x58, L"UltraSPARC 3i" },
    /* 0x59 - 0x5F Available for assignment */
    { 0x60, L"68040" },
    { 0x61, L"68xxx" },
    { 0x62, L"68000" },
    { 0x63, L"68010" },
    { 0x64, L"68020" },
    { 0x65, L"68030" },
    /* 0x66 - 0x6F Available for assignment */
    { 0x70, L"Hobbit" },
    /* 0x71 - 0x77 Available for assignment */
    { 0x78, L"Crusoe TM5000" },
    { 0x79, L"Crusoe TM3000" },
    { 0x7A, L"Efficeon TM8000" },
    /* 0x7B - 0x7F Available for assignment */
    { 0x80, L"Weitek" },
    /* 0x81 Available for assignment */
    { 0x82, L"Itanium" },
    { 0x83, L"AMD Athlon 64" },
    { 0x84, L"AMD Opteron" },
    { 0x85, L"AMD Sempron" },
    { 0x86, L"AMD Turion 64 Mobile" },
    { 0x87, L"AMD Opteron Dual-Core" },
    { 0x88, L"AMD Athlon 64 X2 Dual-Core" },
    { 0x89, L"AMD Turion 64 X2 Mobile" },
    { 0x8A, L"AMD Opteron Quad-Core" },
    { 0x8B, L"Third-Generation AMD Opteron" },
    { 0x8C, L"AMD Phenom FX Quad-Core" },
    { 0x8D, L"AMD Phenom X4 Quad-Core" },
    { 0x8E, L"AMD Phenom X2 Dual-Core" },
    { 0x8F, L"AMD Athlon X2 Dual-Core" },
    { 0x90, L"PA-RISC" },
    { 0x91, L"PA-RISC 8500" },
    { 0x92, L"PA-RISC 8000" },
    { 0x93, L"PA-RISC 7300LC" },
    { 0x94, L"PA-RISC 7200" },
    { 0x95, L"PA-RISC 7100LC" },
    { 0x96, L"PA-RISC 7100" },
    /* 0x97 - 0x9F Available for assignment */
    { 0xA0, L"V30" },
    { 0xA1, L"Intel Xeon Quad-Core 3200" },
    { 0xA2, L"Intel Xeon Dual-Core 3000" },
    { 0xA3, L"Intel Xeon Quad-Core 5300" },
    { 0xA4, L"Intel Xeon Dual-Core 5100" },
    { 0xA5, L"Intel Xeon Dual-Core 5000" },
    { 0xA6, L"Intel Xeon Dual-Core LV" },
    { 0xA7, L"Intel Xeon Dual-Core ULV" },
    { 0xA8, L"Intel Xeon Dual-Core 7100" },
    { 0xA9, L"Intel Xeon Quad-Core 5400" },
    { 0xAA, L"Intel Xeon Quad-Core" },
    { 0xAB, L"Intel Xeon Dual-Core 5200" },
    { 0xAC, L"Intel Xeon Dual-Core 7200" },
    { 0xAD, L"Intel Xeon Quad-Core 7300" },
    { 0xAE, L"Intel Xeon Quad-Core 7400" },
    { 0xAF, L"Intel Xeon Multi-Core 7400" },
    { 0xB0, L"Intel Pentium 3 Xeon" },
    { 0xB1, L"Intel Pentium 3 with SpeedStep Technology" },
    { 0xB2, L"Intel Pentium 4" },
    { 0xB3, L"Intel Xeon" },
    { 0xB4, L"AS400" },
    { 0xB5, L"Intel Xeon MP" },
    { 0xB6, L"AMD Athlon XP" },
    { 0xB7, L"AMD Athlon MP" },
    { 0xB8, L"Intel Itanium 2" },
    { 0xB9, L"Intel Pentium M" },
    { 0xBA, L"Intel Celeron D" },
    { 0xBB, L"Intel Pentium D" },
    { 0xBC, L"Intel Pentium Extreme Edition" },
    { 0xBD, L"Intel Core Solo" },
    /* 0xBE - Reserved */
    { 0xBF, L"Intel Core 2 Duo" },
    { 0xC0, L"Intel Core 2 Solo" },
    { 0xC1, L"Intel Core 2 Extreme" },
    { 0xC2, L"Intel Core 2 Quad" },
    { 0xC3, L"Intel Core 2 Extreme Mobile" },
    { 0xC4, L"Intel Core 2 Duo Mobile" },
    { 0xC5, L"Intel Core 2 Solo Mobile" },
    { 0xC6, L"Intel Core i7" },
    { 0xC7, L"Intel Celeron Dual-Core" },
    { 0xC8, L"IBM390" },
    { 0xC9, L"G4" },
    { 0xCA, L"G5" },
    { 0xCB, L"ESA/390 G6" },
    { 0xCC, L"z/Architectur base" },
    { 0xCD, L"Intel Core i5" },
    { 0xCE, L"Intel Core i3" },
    /* 0xCF - 0xD1 Available for assignment */
    { 0xD2, L"VIA C7-M" },
    { 0xD3, L"VIA C7-D" },
    { 0xD4, L"VIA C7" },
    { 0xD5, L"VIA Eden" },
    { 0xD6, L"Intel Xeon Multi-Core" },
    { 0xD7, L"Intel Xeon Dual-Core 3xxx" },
    { 0xD8, L"Intel Xeon Quad-Core 3xxx" },
    { 0xD9, L"VIA Nano" },
    { 0xDA, L"Intel Xeon Dual-Core 5xxx" },
    { 0xDB, L"Intel Xeon Quad-Core 5xxx" },
    /* 0xDC Available for assignment */
    { 0xDD, L"Intel Xeon Dual-Core 7xxx" },
    { 0xDE, L"Intel Xeon Quad-Core 7xxx" },
    { 0xDF, L"Intel Xeon Multi-Core 7xxx" },
    { 0xE0, L"Intel Xeon Multi-Core 3400" },
    /* 0xE1 - 0xE5 Available for assignment */
    { 0xE6, L"AMD Opteron Quad-Core Embedded" },
    { 0xE7, L"AMD Phenom Triple-Core" },
    { 0xE8, L"AMD Turion Ultra Dual-Core Mobile" },
    { 0xE9, L"AMD Turion Dual-Core Mobile" },
    { 0xEA, L"AMD Athlon Dual-Core" },
    { 0xEB, L"AMD Sempron SI" },
    { 0xEC, L"AMD Phenom 2" },
    { 0xED, L"AMD Athlon 2" },
    { 0xEE, L"AMD Opteron Six-Core" },
    { 0xEF, L"AMD Sempron M" },
    /* 0xF0 - 0xF9 Available for assignment */
    { 0xFA, L"i860" },
    { 0xFB, L"i960" },
    /* 0xFC - 0xFD Available for assignment */
    /* 0xFE - Indicator to obtain the processor family from the Processor Family 2 field */
    /* 0xFF Reserved */
    /* 0x100 - 0x1FF These values are available for assignment, except for the following: */
    { 0x104, L"SH-3" },
    { 0x105, L"SH-4" },
    { 0x118, L"ARM" },
    { 0x119, L"StrongARM" },
    { 0x12C, L"6x86" },
    { 0x12D, L"MediaGX" },
    { 0x12E, L"MII" },
    { 0x140, L"WinChip" },
    { 0x15E, L"DSP" },
    { 0x1F4, L"Video Processor" },
    /* 0x200 - 0xFFFD Available for assignment */
    /* 0xFFFE - 0xFFFF - Reserved */
    { 0 }
};

#define CPU_STATUS_MASK 0x07

const INFO_STRUCT ProcessorStatusList[] =
{
    { 0x01, L"Enabled" },
    { 0x02, L"Disabled by user via BIOS" },
    { 0x04, L"Disabled by BIOS because post error" },
    { 0x08, L"IDLE awaiting to be enabled" },
    { 0 }
};

/* 7.5.5 Processor Information - Processor Upgrade */
const INFO_STRUCT ProcessorUpgradeList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Daughter Board" },
    { 0x04, L"ZIP Socket" },
    { 0x05, L"Replaceable Piggy Back" },
    { 0x06, L"None" },
    { 0x07, L"LIF Socket" },
    { 0x08, L"Slot 1" },
    { 0x09, L"Slot 2" },
    { 0x0A, L"370-pin socket" },
    { 0x0B, L"Slot A" },
    { 0x0C, L"Slot M" },
    { 0x0D, L"Socket 423" },
    { 0x0E, L"Socket A (Socket 462)" },
    { 0x0F, L"Socket 478" },
    { 0x10, L"Socket 754" },
    { 0x11, L"Socket 940" },
    { 0x12, L"Socket 939" },
    { 0x13, L"Socket mPGA604" },
    { 0x14, L"Socket LGA771" },
    { 0x15, L"Socket LGA775" },
    { 0x16, L"Socket S1" },
    { 0x17, L"Socket AM2" },
    { 0x18, L"Socket F(1207)" },
    { 0x19, L"Socket LGA1366" },
    { 0x1A, L"Socket G34" },
    { 0x1B, L"Socket AM3" },
    { 0x1C, L"Socket C32" },
    { 0x1D, L"Socket LGA1156" },
    { 0x1E, L"Socket LGA1567" },
    { 0x1F, L"Socket PGA988A" },
    { 0x20, L"Socket BGA1288" },
    { 0x21, L"Socket rPGA988B" },
    { 0x22, L"Socket BGA1023" },
    { 0x23, L"Socket BGA1224" },
    { 0x24, L"Socket BGA1155" },
    { 0x25, L"Socket LGA1356" },
    { 0x26, L"Socket LGA2011" },
    { 0x27, L"Socket FS1" },
    { 0x28, L"Socket FS2" },
    { 0x29, L"Socket FM1" },
    { 0x2A, L"Socket FM2" },
    { 0 }
};

/* Cache Information */

/* 7.8.2 Cache Information - SRAM Type */

const INFO_STRUCT SramTypeList[] =
{
    { CACHE_OTHER, L"Other" },
    { CACHE_UNKNOWN2, L"Unknown" },
    { CACHE_NON_BURST, L"Non-Burst" },
    { CACHE_BURST, L"Burst" },
    { CACHE_PIPELINE, L"Pipeline Brust" },
    { CACHE_SYNCHRONOUS, L"Synchronous" },
    { CACHE_ASYNCHRONOUS, L"Asynchronous" },
    { 0 }
};

/* Enclosure Information */

/* 7.4.1 System Enclosure or Chassis Types */
const INFO_STRUCT EnclTypesList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Desktop" },
    { 0x04, L"Low Profile Desktop" },
    { 0x05, L"Pizza Box" },
    { 0x06, L"Mini Tower" },
    { 0x07, L"Tower" },
    { 0x08, L"Portable" },
    { 0x09, L"Laptop" },
    { 0x0A, L"Notebook" },
    { 0x0B, L"Hand Held" },
    { 0x0C, L"Docking Station" },
    { 0x0D, L"All in One" },
    { 0x0E, L"Sub Notebook" },
    { 0x0F, L"Space-saving" },
    { 0x10, L"Lunch Box" },
    { 0x11, L"Main Server Chassis" },
    { 0x12, L"Expansion Chassis" },
    { 0x13, L"SubChassis" },
    { 0x14, L"Bus Expansion Chassis" },
    { 0x15, L"Peripheral Chassis" },
    { 0x16, L"RAID Chassis" },
    { 0x17, L"Rack Mound Chassis" },
    { 0x18, L"Sealed-case PC" },
    { 0x19, L"Multi-system chassis" },
    { 0x1A, L"Compact PCI" },
    { 0x1B, L"Advanced TCA" },
    { 0x1C, L"Blade" },
    { 0x1D, L"Blade Enclosure" },
    { 0 }
};

/* 7.4.2 System Enclosure or Chassis States */
const INFO_STRUCT EnclStatusList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Safe" },
    { 0x04, L"Warning" },
    { 0x05, L"Critical" },
    { 0x06, L"Non-recoverable" },
    { 0 }
};

/* 7.4.2 System Enclosure or Chassis Security Status */
const INFO_STRUCT EnclSecStatusList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"None" },
    { 0x04, L"External interface locked out" },
    { 0x05, L"External interface enabled" },
    { 0 }
};

/* 7.18.1 Memory Device - Form Factor, Table 72 */
const INFO_STRUCT FormFactorList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"SIMM" },
    { 0x04, L"SIP" },
    { 0x05, L"Chip" },
    { 0x06, L"DIP" },
    { 0x07, L"ZIP" },
    { 0x08, L"Proprietary Card" },
    { 0x09, L"DIMM" },
    { 0x0A, L"TSOP" },
    { 0x0B, L"Row of chips" },
    { 0x0C, L"RIMM" },
    { 0x0D, L"SODIMM" },
    { 0x0E, L"SRIMM" },
    { 0x0F, L"FB-DIMM" },
    { 0 }
};

/* 7.18.2 Memory Device - Type, Table 73 */
const INFO_STRUCT MemDevicesList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"DRAM" },
    { 0x04, L"EDRAM" },
    { 0x05, L"VRAM" },
    { 0x06, L"SRAM" },
    { 0x07, L"RAM" },
    { 0x08, L"ROM" },
    { 0x09, L"Flash" },
    { 0x0A, L"EEPROM" },
    { 0x0B, L"FEPROM" },
    { 0x0C, L"EPROM" },
    { 0x0D, L"CDRAM" },
    { 0x0E, L"3DRAM" },
    { 0x0F, L"SDRAM" },
    { 0x10, L"SGRAM" },
    { 0x11, L"RDRAM" },
    { 0x12, L"DDR" },
    { 0x13, L"DDR2" },
    { 0x14, L"DDR2 FB-DIMM" },
    /* 0x15 - 0x17 Reserved */
    { 0x18, L"DDR3" },
    { 0x19, L"FBD2" },
    { 0 }
};

/* 7.10 System Slots (Type 9) */
const INFO_STRUCT SlotTypeList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"ISA" },
    { 0x04, L"MCA" },
    { 0x05, L"EISA" },
    { 0x06, L"PCI" },
    { 0x07, L"PC Card (PCMCIA)" },
    { 0x08, L"VL-VESA" },
    { 0x09, L"Proprietary" },
    { 0x0A, L"Processor Card Slot" },
    { 0x0B, L"Proprietary Memory Card Slot" },
    { 0x0C, L"I/O Riser Card Slot" },
    { 0x0D, L"NuBus" },
    { 0x0E, L"PCI - 66MHz Capable" },
    { 0x0F, L"AGP" },
    { 0x10, L"AGP 2X" },
    { 0x11, L"AGP 4X" },
    { 0x12, L"PCI-X" },
    { 0x13, L"AGP 8X" },
    { 0xA0, L"PC-98/C20" },
    { 0xA1, L"PC-98/C24" },
    { 0xA2, L"PC-98/E" },
    { 0xA3, L"PC-98/Local Bus" },
    { 0xA4, L"PC-98/Card" },
    { 0xA5, L"PCI Express" },
    { 0xA6, L"PCI Express x1" },
    { 0xA7, L"PCI Express x2" },
    { 0xA8, L"PCI Express x4" },
    { 0xA9, L"PCI Express x8" },
    { 0xAA, L"PCI Express x16" },
    { 0xAB, L"PCI Express Gen 2" },
    { 0xAC, L"PCI Express Gen 2 x1" },
    { 0xAD, L"PCI Express Gen 2 x2" },
    { 0xAE, L"PCI Express Gen 2 x4" },
    { 0xAF, L"PCI Express Gen 2 x8" },
    { 0xB0, L"PCI Express Gen 2 x16" },
    { 0xB1, L"PCI Express Gen 3" },
    { 0xB2, L"PCI Express Gen 3 x1" },
    { 0xB3, L"PCI Express Gen 3 x2" },
    { 0xB4, L"PCI Express Gen 3 x4" },
    { 0xB5, L"PCI Express Gen 3 x8" },
    { 0xB6, L"PCI Express Gen 3 x16" },
    { 0 }
};

const INFO_STRUCT BusWidthList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"8 bit" },
    { 0x04, L"16 bit" },
    { 0x05, L"32 bit" },
    { 0x06, L"64 bit" },
    { 0x07, L"128 bit" },
    { 0x08, L"1x or x1" },
    { 0x09, L"2x or x2" },
    { 0x0A, L"4x or x4" },
    { 0x0B, L"8x or x8" },
    { 0x0C, L"12x or x12" },
    { 0x0D, L"16x or x16" },
    { 0x0E, L"32x or x32" },
    { 0 }
};

const INFO_STRUCT SlotLengthList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Short Length" },
    { 0x04, L"Long Length" },
    { 0 }
};

/* 7.9 Port Connector Information (Type 8) */
const INFO_STRUCT PortTypesList[] =
{
    { 0x01, L"Parallel Port XT/AT Compatible" },
    { 0x02, L"Parallel Port PS/2" },
    { 0x03, L"Parallel Port ECP" },
    { 0x04, L"Parallel Port EPP" },
    { 0x05, L"Parallel Port ECP/EPP" },
    { 0x06, L"Serial Port XT/AT Compatible" },
    { 0x07, L"Serial Port 16450 Compatible" },
    { 0x08, L"Serial Port 16550 Compatible" },
    { 0x09, L"Serial Port 16550A Compatible" },
    { 0x0A, L"SCSI Port" },
    { 0x0B, L"MIDI Port" },
    { 0x0C, L"Joy Stick Port" },
    { 0x0D, L"Keyboard Port" },
    { 0x0E, L"Mouse Port" },
    { 0x0F, L"SSA SCSI" },
    { 0x10, L"USB" },
    { 0x11, L"FireWire (IEEE P1394)" },
    { 0x12, L"PCMCIA Type I" },
    { 0x13, L"PCMCIA Type II" },
    { 0x14, L"PCMCIA Type III" },
    { 0x15, L"Cardbus" },
    { 0x16, L"Access Bus Port" },
    { 0x17, L"SCSI II" },
    { 0x18, L"SCSI Wide" },
    { 0x19, L"PC-98" },
    { 0x1A, L"PC-98-Hireso" },
    { 0x1B, L"PC-H98" },
    { 0x1C, L"Video Port" },
    { 0x1D, L"Audio Port" },
    { 0x1E, L"Modem Port" },
    { 0x1F, L"Network Port" },
    { 0x20, L"SATA" },
    { 0x21, L"SAS" },
    { 0xA0, L"8251 Compatible" },
    { 0xA1, L"8251 FIFO Compatible" },
    { 0x0FF, L"Other" },
    { 0 }
};

const INFO_STRUCT PortConnectorList[] =
{
    { 0x01, L"Centronics" },
    { 0x02, L"Mini Centronics" },
    { 0x03, L"Proprietary" },
    { 0x04, L"DB-25 pin male" },
    { 0x05, L"DB-25 pin female" },
    { 0x06, L"DB-15 pin male" },
    { 0x07, L"DB-15 pin female" },
    { 0x08, L"DB-9 pin male" },
    { 0x09, L"DB-9 pin female" },
    { 0x0A, L"RJ-11" },
    { 0x0B, L"RJ-45" },
    { 0x0C, L"50-pin MiniSCSI" },
    { 0x0D, L"Mini-DIN" },
    { 0x0E, L"Micro-DIN" },
    { 0x0F, L"PS/2" },
    { 0x10, L"Infrared" },
    { 0x11, L"HP-HIL" },
    { 0x12, L"Access Bus (USB)" },
    { 0x13, L"SSA SCSI" },
    { 0x14, L"Circular DIN-8 male" },
    { 0x15, L"Circular DIN-8 female" },
    { 0x16, L"On Board IDE" },
    { 0x17, L"On Board Floppy" },
    { 0x18, L"9-pin Dual Inline (pin 10 cut)" },
    { 0x19, L"25-pin Dual Inline (pin 26 cut)" },
    { 0x1A, L"50-pin Dual Inline" },
    { 0x1B, L"68-pin Dual Inline" },
    { 0x1C, L"On Board Sound Input from CD-ROM" },
    { 0x1D, L"Mini-Centronics Type-14" },
    { 0x1E, L"Mini-Centronics Type-26" },
    { 0x1F, L"Mini-jack (headphones)" },
    { 0x20, L"BNC" },
    { 0x21, L"1394" },
    { 0x22, L"SAS/SATA Plug Receptacle" },
    { 0xA0, L"PC-98" },
    { 0xA1, L"PC-98Hireso" },
    { 0xA2, L"PC-H98" },
    { 0xA3, L"PC-98Note" },
    { 0xA4, L"PC-98Full" },
    { 0xFF, L"Other" },
    { 0 }
};

/* 7.11.1 Onboard Device Types */
const INFO_STRUCT OnboardDeviceTypesList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Video" },
    { 0x04, L"SCSI Controller" },
    { 0x05, L"Ethernet" },
    { 0x06, L"Token Ring" },
    { 0x07, L"Sound" },
    { 0x08, L"PATA Controller" },
    { 0x09, L"SATA Controller" },
    { 0x0A, L"SAS Controller" },
    { 0 }
};

/* 7.23.1 Portable Battery - Device Chemistry */
const INFO_STRUCT BatteryDeviceChemistryList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Lead Acid" },
    { 0x04, L"Nickel Cadmium" },
    { 0x05, L"Nickel metal hydride" },
    { 0x06, L"Lithium-ion" },
    { 0x07, L"Zinc air" },
    { 0x08, L"Lithium Polymer" },
    { 0 }
};
