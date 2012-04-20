/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/smbios.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

/*
   This file partialy based on DmiDecode for Windows project
   http://gnuwin32.sourceforge.net/packages/dmidecode.htm
 */

/* System Management BIOS Specification:
   http://dmtf.org/standards/smbios
   Last version: 2.7.1
   Date: 1 Feb 2011
*/

#include "aspia.h"
#include "aspia_dll.h"
#include "smbios.h"

#pragma warning(disable: 4996)

#define WORD(x) (WORD)(*(const WORD *)(x))
#define DWORD64(x) (*(const DWORD64 *)(x))
#define DWORD(x) (DWORD)(*(const DWORD *)(x))


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

/* Definition for the GetSystemFirmwareTable function */
typedef BOOL (WINAPI *PGSFT)(DWORD, DWORD, PVOID, DWORD);

RAWSMBIOSDATA SmbiosRawData;
BOOL IsSmBIOSInitialized = FALSE;
INT StructuresCount = 0;

BYTE SMBIOSMajorVersion = 0;
BYTE SMBIOSMinorVersion = 0;


BOOL
IsSmBiosWorks(VOID)
{
    if (!IsSmBIOSInitialized)
        InitSmBIOSData();

    return IsSmBIOSInitialized;
}

BOOL
InitSmBIOSDataFromFirmwareTable(RAWSMBIOSDATA *RawData, DWORD dwSize)
{
    PGSFT GetSystemFirmwareTable;
    RAWSMBIOSDATA Data = {0};
    HINSTANCE hDLL;
    UINT Ret;

    hDLL = LoadLibrary(L"KERNEL32.DLL");
    if (!hDLL) return FALSE;

    GetSystemFirmwareTable = (PGSFT)GetProcAddress(hDLL, "GetSystemFirmwareTable");
    if (!GetSystemFirmwareTable)
    {
        DebugTrace(L"GetProcAddress(GetSystemFirmwareTable) failed!");
        FreeLibrary(hDLL);
        return FALSE;
    }

    dwSize = sizeof(Data);

    Ret = GetSystemFirmwareTable('RSMB', 'PCAF', RawData, dwSize);
    if (!Ret)
    {
        DebugTrace(L"GetSystemFirmwareTable() failed!");
        FreeLibrary(hDLL);
        return FALSE;
    }

    FreeLibrary(hDLL);

    return TRUE;
}

/*BOOL
GetSmbiosDataFromRegistry(RAWSMBIOSDATA *RawData, DWORD dwSize)
{
    DWORD dwType = REG_BINARY;
    HKEY hKey;

    //if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\services\\mssmbios\\Data", 0, KEY_READ, &hKey) ==
    //    ERROR_SUCCESS)
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\mssmbios\\Data", 0, KEY_READ, &hKey) ==
        ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey,
                            L"SMBiosData",
                            NULL,
                            &dwType,
                            (LPBYTE)RawData,
                            &dwSize) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return TRUE;
        }

        RegCloseKey(hKey);
    }

    return FALSE;
}*/

static INT
GetSmbiosStructuresCount(const BYTE *Buffer, DWORD Length)
{
    INT Count = 0; /* counts the strutures */
    BYTE *Offset = (BYTE *)Buffer; /* points to the actual address in the buff that's been checked */
    DMI_HEADER *Header = NULL; /* header of the struct been read to get the length to increase the offset */

    /* searches structures on the whole SMBIOS Table */
    while (Offset  < (Buffer + Length))
    {
        /* get the header to read te length and to increase the offset */
        Header = (DMI_HEADER*)Offset;
        Offset += Header->Length;

        Count++;

        /*
         * increses the offset to point to the next header that's
         * after the strings at the end of the structure.
         */
        while((*(WORD *)Offset != 0)  &&  (Offset < (Buffer + Length)))
        {
            Offset++;
        }

        /*
         * Points to the next stucture thas after two null BYTEs
         * at the end of the strings.
         */
        Offset += 2;
    }

    return Count;
}

BOOL
InitSmBIOSData(VOID)
{
    PSMBIOS_ENTRY Entry;
    char *buf;
    DWORD dwSize;

    /* For testing only */
    /*if (GetSmbiosDataFromRegistry(&SmbiosRawData, sizeof(RAWSMBIOSDATA)))
    {
        StructuresCount = GetSmbiosStructuresCount(&SmbiosRawData.SMBIOSTableData[0],
                                                   SmbiosRawData.Length);
        goto Success;
    }*/

    buf = GetSmbiosData(&dwSize);
    if (buf)
    {
        Entry = (PSMBIOS_ENTRY)buf;

        CopyMemory(&SmbiosRawData.SMBIOSTableData, buf + sizeof(SMBIOS_ENTRY), Entry->StructureTableLength);
        SmbiosRawData.Length = Entry->StructureTableLength;
        SmbiosRawData.SMBIOSMajorVersion = Entry->MajorVer;
        SmbiosRawData.SMBIOSMinorVersion = Entry->MinorVer;
        StructuresCount = Entry->NumberOfSMBIOSStructures;

        VirtualFree(buf, 0, MEM_RELEASE);

        goto Success;
    }

    if (InitSmBIOSDataFromFirmwareTable(&SmbiosRawData, sizeof(RAWSMBIOSDATA)))
    {
        StructuresCount = GetSmbiosStructuresCount(&SmbiosRawData.SMBIOSTableData[0],
                                                   SmbiosRawData.Length);
        goto Success;
    }

    goto Failed;

Success:
    if (StructuresCount > 0)
    {
        IsSmBIOSInitialized = TRUE;
        return TRUE;
    }
Failed:
    IsSmBIOSInitialized = FALSE;
    return FALSE;
}

static VOID
ToDmiHeader(DMI_HEADER *Header, BYTE *Data)
{
    Header->Type = Data[0];
    Header->Length = Data[1];
    Header->Handle = WORD(Data + 2);
    Header->Data = Data;
}

static BOOL
GetDmiString(BYTE *pData, BYTE s, BYTE Length,
             WCHAR *pString, SIZE_T Size)
{
    CHAR *bp = (CHAR *)pData;

    if (s == 0)
        return FALSE;

    bp += Length;
    while (s > 1 && *bp)
    {
        bp += strlen(bp);
        bp++;
        s--;
    }

    if (!*bp)
        return FALSE;

    StringCbPrintf(pString, Size, L"%S", bp);
    ChopSpaces(pString, Size);

    if (pString[0] == 0)
        return FALSE;

    return TRUE;
}

BOOL
EnumDMITablesByType(BYTE Type, SMBIOS_TABLE_ENUMPROC lpEnumProc)
{
    BYTE *Buffer = (BYTE *)(&SmbiosRawData.SMBIOSTableData[0]);
    DWORD Length = SmbiosRawData.Length;
    BYTE *Data = Buffer;
    BOOL Result = FALSE;
    INT i = 0;

    DebugTrace(L"EnumDMITablesByType(Type = %d)", Type);

    /* 4 is the length of an SMBIOS structure header */
    while (i < StructuresCount && Data + 4 <= Buffer + Length)
    {
        DMI_HEADER Header;
        BYTE *Next;

        ToDmiHeader(&Header, Data);

        /*
         * If a short entry is found (less than 4 bytes), not only it
         * is invalid, but we cannot reliably locate the next entry
         * Better stop at this point, and let the user know his/her
         * table is broken.
         */
        if (Header.Length < 4)
        {
            DebugTrace(L"Invalid entry length (%u). DMI table is broken!",
                       Header.Length);
            break;
        }

        /* In quiet mode, stop decoding at end of table marker */
        if (Header.Type == 127)
            break;

        /* look for the next handle */
        Next = Data + Header.Length;
        while (Next - Buffer + 1 < Length && (Next[0] != 0 || Next[1] != 0))
            Next++;
        Next += 2;

        if (Next - Buffer <= Length && Header.Type == Type)
        {
            DebugTrace(L"SMBIOS table: Type = %d, Length = %d, Handle = 0x04X",
                       Header.Type, Header.Length, Header.Handle);
            lpEnumProc(Header.Data, Header.Length);
            Result = TRUE;
        }

        Data = Next;
        i++;
    }

    if (i != StructuresCount)
    {
        DebugTrace(L"Wrong DMI structures count: %d announced, only %d decoded",
                   StructuresCount, i);
    }

    if (Data - Buffer != Length)
    {
        DebugTrace(L"Wrong DMI structures length: %d bytes announced, structures occupy %d bytes",
                   Length, (unsigned int)(Data - Buffer));
    }

    return Result;
}

VOID
AddDMIFooter(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    INT IconIndex;

    if (IoGetTarget() != IO_TARGET_LISTVIEW ||
        ListView_GetItemCount(DllParams.hListView) == 0)
        return;

    IconIndex = IoAddIcon(IDI_BANG);
    IoAddFooter();

    IoAddValueName(0, IconIndex, IDS_DMI_HEADER_TITLE);

    LoadMUIString(IDS_DMI_HEADER, szText, MAX_STR_LEN);
    IoSetItemText(szText);
}

VOID
SMBIOS_FormFactorToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (FormFactorList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, FormFactorList[Index].lpszString);
            return;
        }
    }
    while (FormFactorList[++Index].dwValue != 0);
}

VOID
SMBIOS_MemoryDeviceToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (MemDevicesList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, MemDevicesList[Index].lpszString);
            return;
        }
    }
    while (MemDevicesList[++Index].dwValue != 0);
}

VOID CALLBACK
RAMInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    /* Device Locator */
    if (GetDmiString(pBuf, pBuf[0x10], Length, szText, sizeof(szText)))
    {
        IoAddHeaderString(0, 0, szText);
    }
    else
    {
        IoAddHeaderString(0, 0, L"Unknown");
    }

    /* Size */
    if ((WORD(pBuf + 0x0C) & MEM_SIZE_MASK) > 0)
    {
        IoAddValueName(1, 0, IDS_MEM_SIZE);
        IoSetItemText(L"%d MB", (WORD(pBuf + 0x0C) & MEM_SIZE_MASK));
    }

    /* Type */
    if (pBuf[0x12] != 0x02)
    {
        SMBIOS_MemoryDeviceToText(pBuf[0x12], szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_TYPE);
            IoSetItemText(szText);
        }
    }

    /* Speed */
    if (WORD(pBuf + 0x15) > 0)
    {
        IoAddValueName(1, 0, IDS_MEM_SPEED);
        IoSetItemText(L"%d MHz", WORD(pBuf + 0x15));
    }

    /* Form factor */
    SMBIOS_FormFactorToText(pBuf[0x0E], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_MEM_FORMFACTOR);
        IoSetItemText(szText);
    }

    /* Serial number */
    if (GetDmiString(pBuf, pBuf[0x18], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_SERIAL_NUMBER);
        IoSetItemText(szText);
    }

    /* Part Number */
    if (GetDmiString(pBuf, pBuf[0x1A], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_MEM_PARTNUMBER);
        IoSetItemText(szText);
    }

    /* Manufacturer */
    if (GetDmiString(pBuf, pBuf[0x17], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_MANUFACTURER);
        IoSetItemText(szText);
    }

    /* Bank */
    if (GetDmiString(pBuf, pBuf[0x11], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_MEM_BANK);
        IoSetItemText(szText);
    }

    /* Total width */
    if (WORD(pBuf + 0x08) > 0)
    {
        IoAddValueName(1, 0, IDS_MEM_TOTALWIDTH);
        IoSetItemText(L"%d bit", WORD(pBuf + 0x08));
    }

    /* Data width */
    if (WORD(pBuf + 0x0A))
    {
        IoAddValueName(1, 0, IDS_MEM_DATAWIDTH);
        IoSetItemText(L"%d bit", WORD(pBuf + 0x0A));
    }
}

VOID
DMI_RAMInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_HW);

    if (!IsSmBiosWorks()) return;

    if (EnumDMITablesByType(MEMORY_DEVICES_INFO, RAMInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
SMBIOS_SramToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (SramTypeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, SramTypeList[Index].lpszString);
            return;
        }
    }
    while (SramTypeList[++Index].dwValue != 0);
}

VOID CALLBACK
CacheInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR *p, szText[MAX_STR_LEN];
    WORD Config = 0;
    BYTE SramType;

    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddHeaderString(0, 0, szText);
    }
    else
    {
        IoAddHeaderString(0, 0, L"Unknown");
    }

    /* Cache type */
    p = NULL;
    Config = (WORD(pBuf + 0x05) & CACHE_LOCATION_MASK) >> 5;
    if (Config == CACHE_INTERNAL)
        p = L"Internal";
    if (Config == CACHE_EXTERNAL)
        p = L"External";
    if (Config == CACHE_RESERVED)
        p = L"Reserved";
    if (Config == CACHE_UNKNOWN4)
        p = L"Unknown type";
    if (p)
    {
        IoAddValueName(1, 0, IDS_TYPE);
        IoSetItemText(p);
    }

    /* Cache status */
    p = NULL;
    Config = (WORD(pBuf + 0x05) & CACHE_ENABLED_MASK) >> 7;
    if (Config == CACHE_ENABLED)
        p = L"Enabled";
    if (Config == CACHE_NOT_ENABLED)
        p = L"Disabled";
    if (p)
    {
        IoAddValueName(1, 0, IDS_STATUS);
        IoSetItemText(p);
    }

    /* Cache mode */
    p = NULL;
    Config = (WORD(pBuf + 0x05) & CACHE_OPERATIONAL_MODE_MASK) >> 8;
    if (Config == CACHE_WRITE_THRU)
        p = L"Write Thru";
    if (Config == CACHE_WRITE_BACK)
        p = L"Write Back";
    if (Config == CACHE_PER_ADDRESS)
        p = L"Write varies w/ address";
    if (Config == CACHE_UNKNOWN4)
        p = L"Unknown write capability";
    if (p)
    {
        IoAddValueName(1, 0, IDS_DMI_CACHE_MODE);
        IoSetItemText(p);
    }

    /* Cache level */
    p = NULL;
    Config = WORD(pBuf + 0x05) & CACHE_LEVEL_MASK;
    if (Config == CACHE_L1)
        p = L"L1";
    if (Config == CACHE_L2)
        p = L"L2";
    if (Config == CACHE_L3)
        p = L"L3";
    if (p)
    {
        IoAddValueName(1, 0, IDS_DMI_CACHE_LEVEL);
        IoSetItemText(p);
    }

    /* Max cache size */
    if ((WORD(pBuf + 0x07) & CACHE_SIZE_MASK) > 0)
    {
        IoAddValueName(1, 0, IDS_DMI_CACHE_MAX_SIZE);
        IoSetItemText(L"%d KB",
                      WORD(pBuf + 0x07) & CACHE_SIZE_MASK);
    }

    /* Max cache size */
    if ((WORD(pBuf + 0x09) & CACHE_SIZE_MASK) > 0)
    {
        IoAddValueName(1, 0, IDS_DMI_CACHE_CURRENT_SIZE);
        IoSetItemText(L"%d KB",
                      WORD(pBuf + 0x09) & CACHE_SIZE_MASK);
    }

    /* Supported SRAM type */
    SramType = WORD(pBuf + 0x0B) & SRAM_TYPE_MASK;
    IoAddValueName(1, 0, IDS_DMI_CACHE_SRAM_TYPES);

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
    szText[wcslen(szText) - 2] = 0;
    IoSetItemText(szText);

    /* Current SRAM type */
    SMBIOS_SramToText(WORD(pBuf + 0x0D) & SRAM_TYPE_MASK,
                      szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_CACHE_SRAM_CURRENT);
        IoSetItemText(szText);
    }
}

VOID
DMI_CacheInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_SENSOR);

    if (EnumDMITablesByType(CACHE_INFO, CacheInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
SMBIOS_ProcessorTypeToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (ProcessorTypeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, ProcessorTypeList[Index].lpszString);
            return;
        }
    }
    while (ProcessorTypeList[++Index].dwValue != 0);
}

VOID
SMBIOS_ProcessorFamilyToText(WORD Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (ProcessorFamilyList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, ProcessorFamilyList[Index].lpszString);
            return;
        }
    }
    while (ProcessorFamilyList[++Index].dwValue != 0);
}

VOID
SMBIOS_ProcessorStatusToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    BYTE CpuStatus = Form & CPU_STATUS_MASK;
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (ProcessorStatusList[Index].dwValue == CpuStatus)
        {
            StringCbCopy(lpszText, Size, ProcessorStatusList[Index].lpszString);
            return;
        }
    }
    while (ProcessorStatusList[++Index].dwValue != 0);
}

VOID
SMBIOS_ProcessorUpgradeToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (ProcessorUpgradeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, ProcessorUpgradeList[Index].lpszString);
            return;
        }
    }
    while (ProcessorUpgradeList[++Index].dwValue != 0);
}

VOID CALLBACK
CPUInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];
    FLOAT Voltage;

    /* Version */
    if (GetDmiString(pBuf, pBuf[0x10], Length, szText, sizeof(szText)))
    {
        IoAddHeaderString(0, 0, szText);
    }
    else
    {
        IoAddHeaderString(0, 0, L"Not installed");
    }

    /* Manufacturer */
    if (GetDmiString(pBuf, pBuf[0x07], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_MANUFACTURER);
        IoSetItemText(szText);
    }

    /* Family */
    SMBIOS_ProcessorFamilyToText(pBuf[0x06], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_CPU_FAMILY);
        IoSetItemText(szText);
    }

    /* Type */
    SMBIOS_ProcessorTypeToText(pBuf[0x05], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_TYPE);
        IoSetItemText(szText);
    }

    /* Status */
    SMBIOS_ProcessorStatusToText(pBuf[0x18], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_STATUS);
        IoSetItemText(szText);
    }

    /* Socket */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_CPU_SOCKET);
        IoSetItemText(szText);
    }

    /* Upgrade */
    SMBIOS_ProcessorUpgradeToText(pBuf[0x19], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_CPU_UPGRADE);
        IoSetItemText(szText);
    }

    /* External clock */
    if (WORD(pBuf + 0x12) > 0)
    {
        IoAddValueName(1, 0, IDS_CPU_EXTCLOCK);
        IoSetItemText(L"%d MHz", WORD(pBuf + 0x12));
    }

    /* Current speed */
    if (WORD(pBuf + 0x16) > 0)
    {
        IoAddValueName(1, 0, IDS_CPU_CURRENTSPEED);
        IoSetItemText(L"%d MHz", WORD(pBuf + 0x16));
    }

    /* Max speed */
    if (WORD(pBuf + 0x14) > 0)
    {
        IoAddValueName(1, 0, IDS_CPU_MAXSPEED);
        IoSetItemText(L"%d MHz", WORD(pBuf + 0x14));
    }

    if ((pBuf[0x11] & USE_LEGACY_VOLTAGE_MASK))
    {
        Voltage = (FLOAT) (pBuf[0x11] & LEGACY_VOLTAGE_MASK);
        Voltage = (FLOAT) Voltage / (FLOAT) 10;
    }
    else
    {
        UCHAR tmp = pBuf[0x11] & VOLTAGE_MASK;
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
    if (Voltage > 0.0f)
    {
        IoAddValueName(1, 0, IDS_CPU_VOLTAGE);
        IoSetItemText(L"%2.2f Volts", (FLOAT)Voltage);
    }
}

VOID
DMI_CPUInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;

    IoAddIcon(IDI_CPU);

    if (EnumDMITablesByType(PROCESSOR_INFO, CPUInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
BiosCharacteristics(DWORD64 Characteristics)
{
    WCHAR szSupported[MAX_STR_LEN], szUnsupported[MAX_STR_LEN];
    WCHAR *szCharacteristics[] =
    {
        L"BIOS characteristics not supported", /* 3 */
        L"ISA", /* 4 */
        L"MCA",
        L"EISA",
        L"PCI",
        L"PC Card (PCMCIA)",
        L"PNP",
        L"APM",
        L"BIOS is upgradeable",
        L"BIOS shadowing",
        L"VLB",
        L"ESCD",
        L"Boot from CD",
        L"Selectable boot",
        L"BIOS ROM is socketed",
        L"Boot from PC Card (PCMCIA)",
        L"EDD",
        L"Japanese floppy for NEC 9800 1.2 MB (int 13h)",
        L"Japanese floppy for Toshiba 1.2 MB (int 13h)",
        L"5.25\"/360 kB floppy (int 13h)",
        L"5.25\"/1.2 MB floppy (int 13h)",
        L"3.5\"/720 kB floppy (int 13h)",
        L"3.5\"/2.88 MB floppy (int 13h)",
        L"Print screen (int 5h)",
        L"8042 keyboard (int 9h)",
        L"Serial (int 14h)",
        L"Printer (int 17h)",
        L"CGA/mono video (int 10h)",
        L"NEC PC-98" /* 31 */
    };
    INT i;

    if (Characteristics & (1 << 3))
        return;

    LoadMUIString(IDS_CPUID_SUPPORTED, szSupported, MAX_STR_LEN);
    LoadMUIString(IDS_CPUID_UNSUPPORTED, szUnsupported, MAX_STR_LEN);

    for (i = 4; i <= 31; i++)
    {
        BOOL IsSupported = (Characteristics & (1 << i)) ? TRUE : FALSE;

        IoAddItem(2, IsSupported ? 1 : 2, szCharacteristics[i - 3]);
        IoSetItemText(IsSupported ? szSupported : szUnsupported);
    }
}

VOID
BiosCharacteristics1(BYTE Characteristics)
{
    WCHAR szSupported[MAX_STR_LEN], szUnsupported[MAX_STR_LEN];
    WCHAR *szCharacteristics[] =
    {
        L"ACPI is supported", /* 0 */
        L"USB legacy is supported",
        L"AGP is supported",
        L"I2O boot is supported",
        L"LS-120 boot is supported",
        L"ATAPI Zip drive boot is supported",
        L"IEEE 1394 boot is supported",
        L"Smart battery is supported" /* 7 */
    };
    INT i;

    LoadMUIString(IDS_CPUID_SUPPORTED, szSupported, MAX_STR_LEN);
    LoadMUIString(IDS_CPUID_UNSUPPORTED, szUnsupported, MAX_STR_LEN);

    for (i = 0; i <= 7; i++)
    {
        BOOL IsSupported = (Characteristics & (1 << i)) ? TRUE : FALSE;

        IoAddItem(2, IsSupported ? 1 : 2, szCharacteristics[i]);
        IoSetItemText(IsSupported ? szSupported : szUnsupported);
    }
}

VOID
BiosCharacteristics2(BYTE Characteristics)
{
    WCHAR szSupported[MAX_STR_LEN], szUnsupported[MAX_STR_LEN];
    WCHAR *szCharacteristics[] =
    {
        L"BIOS boot specification is supported", /* 0 */
        L"Function key-initiated network boot is supported",
        L"Targeted content distribution is supported" /* 2 */
    };
    INT i;

    LoadMUIString(IDS_CPUID_SUPPORTED, szSupported, MAX_STR_LEN);
    LoadMUIString(IDS_CPUID_UNSUPPORTED, szUnsupported, MAX_STR_LEN);

    for (i = 0; i <= 2; i++)
    {
        BOOL IsSupported = (Characteristics & (1 << i)) ? TRUE : FALSE;

        IoAddItem(2, IsSupported ? 1 : 2, szCharacteristics[i]);
        IoSetItemText(IsSupported ? szSupported : szUnsupported);
    }
}

VOID CALLBACK
BiosInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    IoAddHeader(0, 0, IDS_CAT_HW_DMI_BIOS);

    /* Manufacturer */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_MANUFACTURER);
        IoSetItemText(szText);
    }

    /* Version */
    if (GetDmiString(pBuf, pBuf[0x05], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_VERSION);
        IoSetItemText(szText);
    }

    /* Date */
    if (GetDmiString(pBuf, pBuf[0x08], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_BIOS_DATE);
        IoSetItemText(szText);
    }

    /* Size */
    if (((pBuf[0x09] + 1) << 6) > 0)
    {
        IoAddValueName(1, 0, IDS_BIOS_SIZE);
        StringCbPrintf(szText, sizeof(szText), L"%u KB", ((pBuf[0x09] + 1) << 6));
        IoSetItemText(szText);
    }

    /* Fatures */
    IoAddHeader(1, 0, IDS_BIOS_FATURES);

    BiosCharacteristics(DWORD64(pBuf + 0x0A));

    if (Length >= 0x13)
    {
        BiosCharacteristics1(pBuf[0x12]);
    }

    if (Length >= 14)
    {
        BiosCharacteristics2(pBuf[0x13]);
    }
}

VOID
DMI_BIOSInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;

    IoAddIcon(IDI_CPU);
    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);

    if (EnumDMITablesByType(BIOS_INFO, BiosInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
SMBIOS_WakeupTypeToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (WakeupTypeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, WakeupTypeList[Index].lpszString);
            return;
        }
    }
    while (WakeupTypeList[++Index].dwValue != 0);
}

BOOL
DmiSystemUuid(const BYTE *p, WCHAR *pUuid, SIZE_T Size)
{
    INT only0xFF = 1, only0x00 = 1;
    INT i;

    for (i = 0; i < 16 && (only0x00 || only0xFF); i++)
    {
        if (p[i] != 0x00) only0x00 = 0;
        if (p[i] != 0xFF) only0xFF = 0;
    }

    if (only0xFF || only0x00)
    {
        return FALSE;
    }

    /*
     * As off version 2.6 of the SMBIOS specification, the first 3
     * fields of the UUID are supposed to be encoded on little-endian.
     * The specification says that this is the defacto standard,
     * however I've seen systems following RFC 4122 instead and use
     * network byte order, so I am reluctant to apply the byte-swapping
     * for older versions.
     */

    if ((SmbiosRawData.SMBIOSMajorVersion << 8) + SmbiosRawData.SMBIOSMinorVersion >= 0x0206)
    {
        StringCbPrintf(pUuid, Size,
            L"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            p[3], p[2], p[1], p[0], p[5], p[4], p[7], p[6],
            p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    }
    else
    {
        StringCbPrintf(pUuid, Size,
            L"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
            p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
    }

    return TRUE;
}

VOID CALLBACK
SystemInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    IoAddHeader(0, 0, IDS_CAT_HW_DMI_SYSTEM);

    /* Vendor */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_MANUFACTURER);
        IoSetItemText(szText);
    }

    /* Product */
    if (GetDmiString(pBuf, pBuf[0x05], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_PRODUCT);
        IoSetItemText(szText);
    }

    /* Version */
    if (GetDmiString(pBuf, pBuf[0x06], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_VERSION);
        IoSetItemText(szText);
    }

    /* Serial number */
    if (GetDmiString(pBuf, pBuf[0x07], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_SERIAL_NUMBER);
        IoSetItemText(szText);
    }

    if (Length >= 0x19)
    {
        /* UUID */
        if (DmiSystemUuid(pBuf + 0x08, szText, sizeof(szText)))
        {
            IoAddValueName(1, 0, IDS_SYS_ID);
            IoSetItemText(szText);
        }

        /* Wakeup type */
        SMBIOS_WakeupTypeToText(pBuf[0x18], szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_SYS_WAKEUP);
            IoSetItemText(szText);
        }
    }
}

VOID
DMI_SystemInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;

    IoAddIcon(IDI_COMPUTER);

    if (EnumDMITablesByType(SYSTEM_INFO, SystemInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
SMBIOS_EnclStateToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (EnclStatusList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, EnclStatusList[Index].lpszString);
            return;
        }
    }
    while (EnclStatusList[++Index].dwValue != 0);
}

VOID
SMBIOS_EnclTypeToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (EnclTypesList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, EnclTypesList[Index].lpszString);
            return;
        }
    }
    while (EnclTypesList[++Index].dwValue != 0);
}

VOID
SMBIOS_EnclSecStatusToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (EnclSecStatusList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, EnclSecStatusList[Index].lpszString);
            return;
        }
    }
    while (EnclSecStatusList[++Index].dwValue != 0);
}

VOID CALLBACK
EnclosureInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    IoAddHeader(0, 0, IDS_ENCL_ID);

    /* Vendor */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_MANUFACTURER);
        IoSetItemText(szText);
    }

    /* Version */
    if (GetDmiString(pBuf, pBuf[0x06], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_VERSION);
        IoSetItemText(szText);
    }

    /* Serial number */
    if (GetDmiString(pBuf, pBuf[0x07], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_SERIAL_NUMBER);
        IoSetItemText(szText);
    }

    /* Type */
    SMBIOS_EnclTypeToText(pBuf[0x05], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_ENCL_TYPE);
        IoSetItemText(szText);
    }

    /* OS Load status */
    if (pBuf[0x09] != 0x02)
    {
        SMBIOS_EnclStateToText(pBuf[0x09], szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_ENCL_OSLOAD_STATUS);
            IoSetItemText(szText);
        }
    }

    /* Power source status */
    if (pBuf[0x0a] != 0x02)
    {
        SMBIOS_EnclStateToText(pBuf[0x0a], szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_ENCL_POWER_STATUS);
            IoSetItemText(szText);
        }
    }

    /* Temperature status */
    if (pBuf[0x0b] != 0x02)
    {
        SMBIOS_EnclStateToText(pBuf[0x0b], szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_ENCL_TEMPERATURE_STATUS);
            IoSetItemText(szText);
        }
    }

    /* Security status */
    if (pBuf[0x0c] != 0x02)
    {
        SMBIOS_EnclSecStatusToText(pBuf[0x0c], szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_ENCL_SECURITY_STATUS);
            IoSetItemText(szText);
        }
    }
}

VOID
DMI_EnclosureInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_ENCLOSURE);

    if (EnumDMITablesByType(ENCLOSURE_INFO, EnclosureInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID CALLBACK
BoardInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    IoAddHeader(0, 0, IDS_CAT_HW_DMI_BOARD);

    /* Vendor */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_MANUFACTURER);
        IoSetItemText(szText);
    }

    /* Product */
    if (GetDmiString(pBuf, pBuf[0x05], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_PRODUCT);
        IoSetItemText(szText);
    }

    /* Version */
    if (GetDmiString(pBuf, pBuf[0x06], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_VERSION);
        IoSetItemText(szText);
    }

    /* Serial number */
    if (GetDmiString(pBuf, pBuf[0x07], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_SERIAL_NUMBER);
        IoSetItemText(szText);
    }
}

VOID
DMI_BoardInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;

    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(BOARD_INFO, BoardInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
SMBIOS_SlotTypeToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (SlotTypeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, SlotTypeList[Index].lpszString);
            return;
        }
    }
    while (SlotTypeList[++Index].dwValue != 0);
}

VOID
SMBIOS_BusWidthToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (BusWidthList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, BusWidthList[Index].lpszString);
            return;
        }
    }
    while (BusWidthList[++Index].dwValue != 0);
}

VOID
SMBIOS_SlotLengthToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (SlotLengthList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, SlotLengthList[Index].lpszString);
            return;
        }
    }
    while (SlotLengthList[++Index].dwValue != 0);
}

VOID CALLBACK
SlotInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
        IoAddHeaderString(0, 0, szText);
    else
        IoAddHeaderString(0, 0, L"Unknown");

    /* Type */
    SMBIOS_SlotTypeToText(pBuf[0x05], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_TYPE);
        IoSetItemText(szText);
    }

    /* Bus width */
    SMBIOS_BusWidthToText(pBuf[0x06], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_SLOT_BUS_WIDTH);
        IoSetItemText(szText);
    }

    /* Length */
    SMBIOS_SlotLengthToText(pBuf[0x08], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_SLOT_LENGTH);
        IoSetItemText(szText);
    }
}

VOID
DMI_SlotInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_PORT);

    if (EnumDMITablesByType(SLOTS_INFO, SlotInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
SMBIOS_PortTypesToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (PortTypesList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, PortTypesList[Index].lpszString);
            return;
        }
    }
    while (PortTypesList[++Index].dwValue != 0);
}

VOID
SMBIOS_PortConnectorToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (PortConnectorList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, PortConnectorList[Index].lpszString);
            return;
        }
    }
    while (PortConnectorList[++Index].dwValue != 0);
}

VOID CALLBACK
PortsInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN], szText2[MAX_STR_LEN];

    GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText));
    GetDmiString(pBuf, pBuf[0x06], Length, szText2, sizeof(szText2));

    if (szText2[0] != 0)
        IoAddHeaderString(0, 0, szText2);
    else
        IoAddHeaderString(0, 0, szText);

    /* Internal designation */
    IoAddValueName(1, 0, IDS_PORT_INT_DESIGNATION);
    if (szText[0] == 0)
        LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
    IoSetItemText(szText);

    /* External designation */
    IoAddValueName(1, 0, IDS_PORT_EXT_DESIGNATION);
    if (szText2[0] == 0)
        LoadMUIString(IDS_NO, szText2, MAX_STR_LEN);
    IoSetItemText( szText2);

    /* Type */
    IoAddValueName(1, 0, IDS_PORT_TYPE);
    SMBIOS_PortTypesToText(pBuf[0x08], szText, sizeof(szText));
    if (szText[0] == 0)
        LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
    IoSetItemText(szText);

    /* Internal connector type */
    IoAddValueName(1, 0, IDS_PORT_INT_CONNECTOR);
    SMBIOS_PortConnectorToText(pBuf[0x05], szText, sizeof(szText));
    if (szText[0] == 0)
        LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
    IoSetItemText(szText);

    /* External connector type */
    IoAddValueName(1, 0, IDS_PORT_EXT_CONNECTOR);

    SMBIOS_PortConnectorToText(pBuf[0x07], szText, sizeof(szText));
    if (szText[0] == 0)
        LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
    IoSetItemText(szText);
}

VOID
DMI_PortsInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_PORT);

    if (EnumDMITablesByType(PORTS_INFO, PortsInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
SMBIOS_OnboardDeviceTypeToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (OnboardDeviceTypesList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, OnboardDeviceTypesList[Index].lpszString);
            return;
        }
    }
    while (OnboardDeviceTypesList[++Index].dwValue != 0);
}

VOID CALLBACK
OnboardInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];
    BYTE Count, Type, DeviceType, DeviceStatus;
    BYTE *p;
    INT i;

    Count = (Length - 0x04) / 2;
    p = pBuf + 4;

    if (Count == 0) return;

    for (i = 0; i < Count; i++)
    {
        GetDmiString(pBuf, p[2 * i + 1], Length, szText, sizeof(szText));

        IoAddHeaderString(0, 0, (szText[0] != 0) ? szText : L"Unknown");

        Type = p[2 * i];

        DeviceStatus = GetBitsBYTE(Type, 7, 7); /* Bit 7 */
        DeviceType = GetBitsBYTE(Type, 0, 6);

        /* Desc */
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_DMI_ONBOARD_DESC);
            IoSetItemText(szText);
        }

        /* Type */
        SMBIOS_OnboardDeviceTypeToText(DeviceType, szText, sizeof(szText));
        if (szText[0] != 0)
        {
            IoAddValueName(1, 0, IDS_DMI_ONBOARD_TYPE);
            IoSetItemText(szText);
        }

        /* Status */
        IoAddValueName(1, 0, IDS_DMI_ONBOARD_STATUS);
        IoSetItemText(DeviceStatus ? L"Enabled" : L"Disabled");
    }
}

VOID
DMI_OnboardInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(ONBOARD_INFO, OnboardInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
SMBIOS_BatteryChemistryToText(BYTE Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpszText[0] = 0;
    do
    {
        if (BatteryDeviceChemistryList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, BatteryDeviceChemistryList[Index].lpszString);
            return;
        }
    }
    while (BatteryDeviceChemistryList[++Index].dwValue != 0);
}

VOID CALLBACK
BatteryInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    IoAddHeader(0, 0, IDS_BAT_BATTERYS);

    /* Location */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_LOCATION);
        IoSetItemText(szText);
    }

    /* Manufacturer */
    if (GetDmiString(pBuf, pBuf[0x05], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_MANUFACTURER);
        IoSetItemText(szText);
    }

    /* Manufacture Date */
    if (GetDmiString(pBuf, pBuf[0x06], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_MANUFDATE);
        IoSetItemText(szText);
    }

    /* Serial Number */
    if (GetDmiString(pBuf, pBuf[0x07], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_SERIAL);
        IoSetItemText(szText);
    }

    /* Device Name */
    if (GetDmiString(pBuf, pBuf[0x08], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_DEVICE_NAME);
        IoSetItemText(szText);
    }

    /* Device Chemistry */
    SMBIOS_BatteryChemistryToText(pBuf[0x09], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_DEVICE_CHEMISTRY);
        IoSetItemText(szText);
    }

    /* Design Capacity */
    IoAddValueName(1, 0, IDS_DMI_BATTERY_DESIGN_CAPACITY);
    IoSetItemText(L"%u mWh",
                  pBuf[0x15] ? pBuf[0x0A] * pBuf[0x15] : pBuf[0x0A]);

    /* Design Voltage */
    if (pBuf[0x0C] > 0)
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_DESIGN_VOLTAGE);
        IoSetItemText(L"%u mV", pBuf[0x0C]);
    }

    /* SBDS Version Number */
    if (GetDmiString(pBuf, pBuf[0x0E], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_SBDS_VERSION);
        IoSetItemText(szText);
    }

    /* Max. Error in Battery Data */
    IoAddValueName(1, 0, IDS_DMI_BATTERY_MAXERROR_IN_DATA);
    IoSetItemText(L"%u", pBuf[0x0F]);

    /* SBDS Serial Number */
    IoAddValueName(1, 0, IDS_DMI_BATTERY_SBDS_SERIAL);
    IoSetItemText(L"%u", pBuf[0x10]);

    /* SBDS Manufacture Date */
    IoAddValueName(1, 0, IDS_DMI_BATTERY_SBDS_MANUFDATE);
    IoSetItemText(L"%02u/%02u/%u",
                  GetBitsWORD(WORD(pBuf + 0x12), 0, 4), /* Bits 4:0 */
                  GetBitsWORD(WORD(pBuf + 0x12), 5, 8), /* Bits 8:5 */
                  GetBitsWORD(WORD(pBuf + 0x12), 9, 15) + 1980 /* Bits 15:9 */);

    /* SBDS Device Chemistry */
    if (GetDmiString(pBuf, pBuf[0x14], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_SBDS_DEV_CHEMIST);
        IoSetItemText(szText);
    }
}

VOID
DMI_BatteryInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_BATTERY);

    if (EnumDMITablesByType(BATTERY_INFO, BatteryInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

const INFO_STRUCT PointingDeviceTypesList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Mouse" },
    { 0x04, L"Track Ball" },
    { 0x05, L"Track Point" },
    { 0x06, L"Glide Point" },
    { 0x07, L"Touch Pad" },
    { 0x08, L"Touch Screen" },
    { 0x09, L"Optical Sensor" },
    { 0 }
};

VOID
PointingDevicesTypeToText(BYTE Type, LPWSTR lpText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpText[0] = 0;
    do
    {
        if (PointingDeviceTypesList[Index].dwValue == Type)
        {
            StringCbCopy(lpText, Size, PointingDeviceTypesList[Index].lpszString);
            return;
        }
    }
    while (PointingDeviceTypesList[++Index].dwValue != 0);
}

const INFO_STRUCT PointingInterfaceList[] =
{
    { 0x01, L"Other" },
    { 0x02, L"Unknown" },
    { 0x03, L"Serial" },
    { 0x04, L"PS/2" },
    { 0x05, L"Infrared" },
    { 0x06, L"HP-HIL" },
    { 0x07, L"Bus mouse" },
    { 0x08, L"ADB (Apple Desktop Bus)" },
    { 0xA0, L"Bus mouse DB-9" },
    { 0xA1, L"Bus mouse micro-DIN" },
    { 0xA2, L"USB" },
    { 0 }
};

VOID
PointingInterfaceToText(BYTE Interface, LPWSTR lpText, SIZE_T Size)
{
    SIZE_T Index = 0;

    lpText[0] = 0;
    do
    {
        if (PointingInterfaceList[Index].dwValue == Interface)
        {
            StringCbCopy(lpText, Size, PointingInterfaceList[Index].lpszString);
            return;
        }
    }
    while (PointingInterfaceList[++Index].dwValue != 0);
}

VOID CALLBACK
PointingInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    IoAddHeader(0, 0, IDS_DMI_POINTING_DEVICE);

    PointingDevicesTypeToText(pBuf[0x04], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_POINTING_TYPE);
        IoSetItemText(szText);
    }

    PointingInterfaceToText(pBuf[0x05], szText, sizeof(szText));
    if (szText[0] != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_POINTING_INTERFACE);
        IoSetItemText(szText);
    }

    IoAddValueName(1, 0, IDS_DMI_POINTING_BUTTONS);
    IoSetItemText(L"%d", pBuf[0x06]);
}

VOID
DMI_PointingInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_MOUSE);

    if (EnumDMITablesByType(BUILDIN_POINT_DEV_INF0, PointingInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

WCHAR szMainboardName[MAX_STR_LEN];
WCHAR szMainboardVendor[MAX_STR_LEN];

VOID CALLBACK
BoardNameInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    /* Product */
    GetDmiString(pBuf, pBuf[0x05], Length, szText, sizeof(szText));
    StringCbCopy(szMainboardName, sizeof(szMainboardName), szText);

    /* Vendor */
    GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText));
    StringCbCopy(szMainboardVendor, sizeof(szMainboardVendor), szText);
}

BOOL
SMBIOS_GetMainboardName(LPWSTR lpName, SIZE_T NameSize,
                        LPWSTR lpManuf, SIZE_T ManufSize)
{
    if (!IsSmBiosWorks()) return FALSE;

    if (EnumDMITablesByType(BOARD_INFO, BoardNameInfoEnumProc))
    {
        StringCbCopy(lpName, NameSize, szMainboardName);
        StringCbCopy(lpManuf, ManufSize, szMainboardVendor);
        return TRUE;
    }

    return FALSE;
}
