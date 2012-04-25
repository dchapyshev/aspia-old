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

    if (Length < 0x15) return;

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

    if (EnumDMITablesByType(17, RAMInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiCacheTypes(WORD Code, UINT ValueId)
{
    WCHAR szText[MAX_STR_LEN] = {0};
    WCHAR *szTypes[] = {
        L"Other, ", /* 0 */
        L"Unknown, ",
        L"Non-burst, ",
        L"Burst, ",
        L"Pipeline Burst, ",
        L"Synchronous, ",
        L"Asynchronous, " /* 6 */
    };
    INT i;

    if ((Code & 0x007F) == 0)
        return;

    for (i = 0; i <= 6; i++)
    {
        if (Code & (1 << i))
        {
            StringCbCat(szText, sizeof(szText), szTypes[i]);
        }
    }

    if (szText[0] != 0)
    {
        szText[wcslen(szText) - 2] = 0;

        IoAddValueName(1, 0, ValueId);
        IoSetItemText(szText);
    }
}

VOID
DmiCacheEcType(BYTE Code)
{
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        NULL,
        L"None",
        L"Parity",
        L"Single-bit ECC",
        L"Multi-bit ECC" /* 0x06 */
    };

    if (Code >= 0x01 && Code <= 0x06 && szType[Code - 0x01] != NULL)
    {
        IoAddValueName(1, 0, IDS_DMI_CACHE_ERROR_COR_TYPE);
        IoSetItemText(szType[Code - 0x01]);
    }
}

VOID
DmiCacheType(BYTE Code)
{
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        NULL,
        L"Instruction",
        L"Data",
        L"Unified" /* 0x05 */
    };

    if (Code >= 0x01 && Code <= 0x05 && szType[Code - 0x01] != NULL)
    {
        IoAddValueName(1, 0, IDS_DMI_CACHE_SYSTEM_TYPE);
        IoSetItemText(szType[Code - 0x01]);
    }
}

VOID
DmiCacheAssociativity(BYTE Code)
{
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        NULL,
        L"Direct Mapped",
        L"2-way Set-associative",
        L"4-way Set-associative",
        L"Fully Associative",
        L"8-way Set-associative",
        L"16-way Set-associative",
        L"12-way Set-associative",
        L"24-way Set-associative",
        L"32-way Set-associative",
        L"48-way Set-associative",
        L"64-way Set-associative" /* 0x0D */
    };

    if (Code >= 0x01 && Code <= 0x0D && szType[Code - 0x01] != NULL)
    {
        IoAddValueName(1, 0, IDS_DMI_CACHE_ASSOCIATIVITY);
        IoSetItemText(szType[Code - 0x01]);
    }
}

VOID CALLBACK
CacheInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR *p, szText[MAX_STR_LEN];
    WORD Config = 0;

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
    DmiCacheTypes(WORD(pBuf + 0x0B), IDS_DMI_CACHE_SRAM_TYPES);

    /* Current SRAM type */
    DmiCacheTypes(WORD(pBuf + 0x0D), IDS_DMI_CACHE_SRAM_CURRENT);

    if (Length < 0x13) return;

    /* Speed */
    if (pBuf[0x0F] != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_CACHE_SPPED);
        IoSetItemText(L"%u ns", pBuf[0x0F]);
    }

    /* Error Correction Type */
    DmiCacheEcType(pBuf[0x10]);

    /* System Type */
    DmiCacheType(pBuf[0x11]);

    /* Associativity */
    DmiCacheAssociativity(pBuf[0x12]);
}

VOID
DMI_CacheInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_SENSOR);

    if (EnumDMITablesByType(7, CacheInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiProcessorType(BYTE Code)
{
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"Central Processor",
        L"Math Processor",
        L"DSP Processor",
        L"Video Processor" /* 0x06 */
    };

    if (Code >= 0x01 && Code <= 0x06)
    {
        IoAddValueName(1, 0, IDS_TYPE);
        IoSetItemText(szType[Code - 0x01]);
    }
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
DmiProcessorStatus(BYTE Code)
{
    WCHAR *szStatus[] = {
        L"Unknown", /* 0x00 */
        L"Enabled",
        L"Disabled By User",
        L"Disabled By BIOS",
        L"Idle", /* 0x04 */
        NULL,
        NULL,
        L"Other" /* 0x07 */
    };

    if (szStatus[Code][0] != 0)
    {
        IoAddValueName(1, 0, IDS_STATUS);
        IoSetItemText(szStatus[Code]);
    }
}

VOID
DmiProcessorUpgrade(BYTE Code)
{
    WCHAR *szUpgrade[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"Daughter Board",
        L"ZIF Socket",
        L"Replaceable Piggy Back",
        L"None",
        L"LIF Socket",
        L"Slot 1",
        L"Slot 2",
        L"370-pin Socket",
        L"Slot A",
        L"Slot M",
        L"Socket 423",
        L"Socket A (Socket 462)",
        L"Socket 478",
        L"Socket 754",
        L"Socket 940",
        L"Socket 939",
        L"Socket mPGA604",
        L"Socket LGA771",
        L"Socket LGA775",
        L"Socket S1",
        L"Socket AM2",
        L"Socket F (1207)",
        L"Socket LGA1366",
        L"Socket G34",
        L"Socket AM3",
        L"Socket C32",
        L"Socket LGA1156",
        L"Socket LGA1567",
        L"Socket PGA988A",
        L"Socket BGA1288" /* 0x20 */
    };

    if (Code >= 0x01 && Code <= 0x20)
    {
        IoAddValueName(1, 0, IDS_CPU_UPGRADE);
        IoSetItemText(szUpgrade[Code - 0x01]);
    }
}

VOID
DmiProcessorCharacteristics(WORD Code)
{
    WCHAR szSupported[MAX_STR_LEN], szUnsupported[MAX_STR_LEN];
    WCHAR *szCharacteristics[] = {
        L"64-bit capable", /* 2 */
        L"Multi-Core",
        L"Hardware Thread",
        L"Execute Protection",
        L"Enhanced Virtualization",
        L"Power/Performance Control" /* 7 */
    };
    INT i;

    if ((Code & 0x00FC) == 0)
        return;

    LoadMUIString(IDS_CPUID_SUPPORTED, szSupported, MAX_STR_LEN);
    LoadMUIString(IDS_CPUID_UNSUPPORTED, szUnsupported, MAX_STR_LEN);

    IoAddHeader(1, 0, IDS_DMI_CPU_CHARACTERISTICS);

    for (i = 2; i <= 7; i++)
    {
        BOOL IsSupported = (Code & (1 << i)) ? TRUE : FALSE;

        IoAddItem(2, IsSupported ? 1 : 2, szCharacteristics[i - 2]);
        IoSetItemText(IsSupported ? szSupported : szUnsupported);
    }
}

VOID
DmiProcessorVoltage(BYTE Code, INT Indent, UINT ValueId)
{
    WCHAR szText[MAX_STR_LEN] = {0};
    /* 7.5.4 */
    WCHAR *szVoltage[] = {
        L"5.0 V, ", /* 0 */
        L"3.3 V, ",
        L"2.9 V, " /* 2 */
    };
    INT i;

    if (Code == 0x00)
        return;

    if (Code & 0x80)
    {
        IoAddValueName(Indent, 0, ValueId);
        IoSetItemText(L"%.1f V", (FLOAT)(Code & 0x7F) / 10);
    }
    else
    {
        for (i = 0; i <= 2; i++)
        {
            if (Code & (1 << i))
                StringCbCat(szText, sizeof(szText), szVoltage[i]);
        }

        if (szText[0] != 0)
        {
            szText[wcslen(szText) - 2] = 0;

            IoAddValueName(Indent, 0, ValueId);
            IoSetItemText(szText);
        }
    }
}

VOID CALLBACK
CPUInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x1A) return;

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
    DmiProcessorType(pBuf[0x05]);

    /* Status */
    DmiProcessorStatus(pBuf[0x18] & 0x07);

    /* Socket */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_CPU_SOCKET);
        IoSetItemText(szText);
    }

    /* Upgrade */
    DmiProcessorUpgrade(pBuf[0x19]);

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

    /* Voltage */
    DmiProcessorVoltage(pBuf[0x11], 1, IDS_CPU_VOLTAGE);

    if (Length < 0x20) return;

    /* L1 Cache Handle */
    if (WORD(pBuf + 0x1A) != 0xFFFF)
    {
        IoAddValueName(1, 0, IDS_DMI_CPU_L1_CACHE_HANDLE);
        IoSetItemText(L"0x%04X", WORD(pBuf + 0x1A));
    }

    /* L2 Cache Handle */
    if (WORD(pBuf + 0x1C) != 0xFFFF)
    {
        IoAddValueName(1, 0, IDS_DMI_CPU_L2_CACHE_HANDLE);
        IoSetItemText(L"0x%04X", WORD(pBuf + 0x1C));
    }

    /* L3 Cache Handle */
    if (WORD(pBuf + 0x1E) != 0xFFFF)
    {
        IoAddValueName(1, 0, IDS_DMI_CPU_L3_CACHE_HANDLE);
        IoSetItemText(L"0x%04X", WORD(pBuf + 0x1E));
    }

    if (Length < 0x23) return;

    /* Serial Number */
    if (GetDmiString(pBuf, pBuf[0x20], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_CPU_SERIAL_NUMBER);
        IoSetItemText(szText);
    }

    /* Asset Tag */
    if (GetDmiString(pBuf, pBuf[0x21], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_CPU_ASSET_TAG);
        IoSetItemText(szText);
    }

    /* Part Number */
    if (GetDmiString(pBuf, pBuf[0x22], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_CPU_PART_NUMBER);
        IoSetItemText(szText);
    }

    if (Length < 0x28) return;

    /* Core Count */
    IoAddValueName(1, 0, IDS_DMI_CPU_CORE_COUNT);
    IoSetItemText(L"%u", pBuf[0x23]);

    /* Core Enabled */
    IoAddValueName(1, 0, IDS_DMI_CPU_CORE_ENABLED);
    IoSetItemText(L"%u", pBuf[0x24]);

    /* Thread Count */
    IoAddValueName(1, 0, IDS_DMI_CPU_THREAD_COUNT);
    IoSetItemText(L"%u", pBuf[0x25]);

    /* Characteristics */
    DmiProcessorCharacteristics(WORD(pBuf + 0x26));
}

VOID
DMI_CPUInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;

    IoAddIcon(IDI_CPU);
    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);

    if (EnumDMITablesByType(4, CPUInfoEnumProc))
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
        L"ACPI", /* 0 */
        L"USB legacy",
        L"AGP",
        L"I2O boot",
        L"LS-120 boot",
        L"ATAPI Zip drive boot",
        L"IEEE 1394 boot",
        L"Smart battery" /* 7 */
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
        L"BIOS boot specification", /* 0 */
        L"Function key-initiated network boot",
        L"Targeted content distribution" /* 2 */
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

    /* BIOS Revision */
    if (pBuf[0x14] != 0xFF && pBuf[0x15] != 0xFF)
    {
        IoAddValueName(1, 0, IDS_DMI_BIOS_REVISION);
        IoSetItemText(L"%u.%u", pBuf[0x14], pBuf[0x15]);
    }

    /* Firmware Revision */
    if (pBuf[0x16] != 0xFF && pBuf[0x17] != 0xFF)
    {
        IoAddValueName(1, 0, IDS_DMI_BIOS_FIRMWARE_REV);
        IoSetItemText(L"%u.%u", pBuf[0x16], pBuf[0x17]);
    }

    /* Address */
    if (WORD(pBuf + 0x06) != 0)
    {
        DWORD Code = (0x10000 - WORD(pBuf + 0x06)) << 4;

        IoAddValueName(1, 0, IDS_DMI_BIOS_ADDRESS);
        IoSetItemText(L"0x%04X0", WORD(pBuf + 0x06));

        /* Runtime Size */
        IoAddValueName(1, 0, IDS_DMI_BIOS_RUNTIME_SIZE);
        if (Code & 0x000003FF)
            IoSetItemText(L"%u Bytes", Code);
        else
            IoSetItemText(L"%u KB", Code >> 10);
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

    if (EnumDMITablesByType(0, BiosInfoEnumProc))
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

    if (Length >= 0x1B)
    {
        /* SKU Number */
        if (GetDmiString(pBuf, pBuf[0x19], Length, szText, sizeof(szText)))
        {
            IoAddValueName(1, 0, IDS_DMI_SYSTEM_SKU_NUMBER);
            IoSetItemText(szText);
        }

        /* Family */
        if (GetDmiString(pBuf, pBuf[0x1A], Length, szText, sizeof(szText)))
        {
            IoAddValueName(1, 0, IDS_DMI_SYSTEM_FAMILY);
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

    if (EnumDMITablesByType(1, SystemInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiChassisState(BYTE Code, UINT ValueId)
{
    WCHAR *szState[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"Safe",
        L"Warning",
        L"Critical",
        L"Non-recoverable" /* 0x06 */
    };

    if (Code >= 0x01 && Code <= 0x06)
    {
        IoAddValueName(1, 0, ValueId);
        IoSetItemText(szState[Code - 0x01]);
    }
}

VOID
DmiChassisType(BYTE Code)
{
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"Desktop",
        L"Low Profile Desktop",
        L"Pizza Box",
        L"Mini Tower",
        L"Tower",
        L"Portable",
        L"Laptop",
        L"Notebook",
        L"Hand Held",
        L"Docking Station",
        L"All In One",
        L"Sub Notebook",
        L"Space-saving",
        L"Lunch Box",
        L"Main Server Chassis", /* CIM_Chassis.ChassisPackageType says "Main System Chassis" */
        L"Expansion Chassis",
        L"Sub Chassis",
        L"Bus Expansion Chassis",
        L"Peripheral Chassis",
        L"RAID Chassis",
        L"Rack Mount Chassis",
        L"Sealed-case PC",
        L"Multi-system",
        L"CompactPCI",
        L"AdvancedTCA",
        L"Blade",
        L"Blade Enclosing" /* 0x1D */
    };

    if (Code >= 0x01 && Code <= 0x1D)
    {
        IoAddValueName(1, 0, IDS_ENCL_TYPE);
        IoSetItemText(szType[Code - 0x01]);
    }
}

VOID
DmiChassisSecurityStatus(BYTE Code)
{
    WCHAR *szStatus[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"None",
        L"External Interface Locked Out",
        L"External Interface Enabled" /* 0x05 */
    };

    if (Code >= 0x01 && Code <= 0x05)
    {
        IoAddValueName(1, 0, IDS_ENCL_SECURITY_STATUS);
        IoSetItemText(szStatus[Code - 0x01]);
    }
}

VOID
DmiChassisLock(BYTE Code)
{
    WCHAR *szLock[] = {
        L"Not Present", /* 0x00 */
        L"Present" /* 0x01 */
    };

    IoAddValueName(1, 0, IDS_DMI_CHASSIS_LOCK);
    IoSetItemText(szLock[Code]);
}

VOID CALLBACK
EnclosureInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x09) return;

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

    /* Lock */
    DmiChassisLock(pBuf[0x05] >> 7);

    /* Serial number */
    if (GetDmiString(pBuf, pBuf[0x07], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_SERIAL_NUMBER);
        IoSetItemText(szText);
    }

    /* Asset Tag */
    if (GetDmiString(pBuf, pBuf[0x08], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_CHASSIS_ASSET_TAG);
        IoSetItemText(szText);
    }

    /* Type */
    DmiChassisType(pBuf[0x05] & 0x7F);

    if (Length < 0x0D) return;

    /* OS Load status */
    DmiChassisState(pBuf[0x09], IDS_ENCL_OSLOAD_STATUS);

    /* Power source status */
    DmiChassisState(pBuf[0x0A], IDS_ENCL_POWER_STATUS);

    /* Temperature status */
    DmiChassisState(pBuf[0x0B], IDS_ENCL_TEMPERATURE_STATUS);

    /* Security status */
    DmiChassisSecurityStatus(pBuf[0x0C]);

    if (Length < 0x11) return;

    /* OEM Information */
    IoAddValueName(1, 0, IDS_DMI_CHASSIS_OEM_INFO);
    IoSetItemText(L"0x%08X", DWORD(pBuf + 0x0D));

    if (Length < 0x13) return;

    /* Height */
    if (pBuf[0x11] != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_CHASSIS_HEIGHT);
        IoSetItemText(L"%u U", pBuf[0x11]);
    }

    /* Number Of Power Cords */
    if (pBuf[0x12] != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_CHASSIS_NUM_OF_POWER_CORDS);
        IoSetItemText(L"%u", pBuf[0x12]);
    }

    if (Length < 0x16) return;
    if (Length < 0x16 + pBuf[0x13] * pBuf[0x14])
        return;

    /* SKU Number */
    if (GetDmiString(pBuf, pBuf[0x15 + pBuf[0x13] * pBuf[0x14]], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_CHASSIS_SKU_NUMBER);
        IoSetItemText(szText);
    }
}

VOID
DMI_EnclosureInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_ENCLOSURE);

    if (EnumDMITablesByType(3, EnclosureInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiBaseBoardFeatures(BYTE Code)
{
    WCHAR szSupported[MAX_STR_LEN], szUnsupported[MAX_STR_LEN];
    WCHAR *szFeatures[] = {
        L"Board is a hosting board", /* 0 */
        L"Board requires at least one daughter board",
        L"Board is removable",
        L"Board is replaceable",
        L"Board is hot swappable" /* 4 */
    };
    INT i;

    if ((Code & 0x1F) == 0)
        return;

    IoAddHeader(1, 0, IDS_DMI_BASEBOARD_FEATURES);

    LoadMUIString(IDS_CPUID_SUPPORTED, szSupported, MAX_STR_LEN);
    LoadMUIString(IDS_CPUID_UNSUPPORTED, szUnsupported, MAX_STR_LEN);

    for (i = 0; i <= 4; i++)
    {
        BOOL IsSupported = (Code & (1 << i)) ? TRUE : FALSE;

        IoAddItem(2, IsSupported ? 1 : 2, szFeatures[i]);
        IoSetItemText(IsSupported ? szSupported : szUnsupported);
    }
}

VOID
DmiBaseBoardType(BYTE Code)
{
    WCHAR *szType[] = {
        L"Unknown", /* 0x01 */
        L"Other",
        L"Server Blade",
        L"Connectivity Switch",
        L"System Management Module",
        L"Processor Module",
        L"I/O Module",
        L"Memory Module",
        L"Daughter Board",
        L"Motherboard",
        L"Processor+Memory Module",
        L"Processor+I/O Module",
        L"Interconnect Board" /* 0x0D */
    };

    if (Code >= 0x01 && Code <= 0x0D)
    {
        IoAddValueName(1, 0, IDS_DMI_BASEBOARD_TYPE);
        IoSetItemText(szType[Code - 0x01]);
    }
}

VOID CALLBACK
BoardInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x08) return;

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

    if (Length >= 0x09)
    {
        /* Asset Tag */
        if (GetDmiString(pBuf, pBuf[0x08], Length, szText, sizeof(szText)))
        {
            IoAddValueName(1, 0, IDS_DMI_BASEBOARD_ASSET_TAG);
            IoSetItemText(szText);
        }
    }

    if (Length >= 0x0A)
    {
        /* Features */
        DmiBaseBoardFeatures(pBuf[0x09]);
    }

    if (Length >= 0x0E)
    {
        /* Location In Chassis */
        if (GetDmiString(pBuf, pBuf[0x0A], Length, szText, sizeof(szText)))
        {
            IoAddValueName(1, 0, IDS_DMI_BASEBOARD_LOCATION_IN_CHASSIS);
            IoSetItemText(szText);
        }

        /* Chassis Handle */
        IoAddValueName(1, 0, IDS_DMI_BASEBOARD_CHASSIS_HANDLE);
        IoSetItemText(L"0x%04X", WORD(pBuf + 0x0B));

        /* Type */
        DmiBaseBoardType(pBuf[0x0D]);
    }

    if (Length >= 0x0F)
    {
        if (Length >= 0x0F + pBuf[0x0E] * sizeof(WORD))
        {
            WCHAR szTemp[MAX_STR_LEN];
            BYTE Count = pBuf[0x0E];
            INT i;

            /* Contained Object Handles */
            szText[0] = 0;

            for (i = 0; i < Count; i++)
            {
                StringCbPrintf(szTemp, sizeof(szTemp), L"0x%04X, ",
                               WORD((pBuf + 0x0F) + sizeof(WORD) * i));
                StringCbCat(szText, sizeof(szText), szTemp);
            }

            if (Count > 0)
            {
                IoAddValueName(1, 0, IDS_DMI_BASEBOARD_OBJECT_HANDLES);
                szText[wcslen(szText) - 2] = 0;
                IoSetItemText(szText);
            }
        }
    }
}

VOID
DMI_BoardInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;

    IoAddIcon(IDI_HW);
    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);

    if (EnumDMITablesByType(2, BoardInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiSlotType(BYTE Code)
{
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"ISA",
        L"MCA",
        L"EISA",
        L"PCI",
        L"PC Card (PCMCIA)",
        L"VLB",
        L"Proprietary",
        L"Processor Card",
        L"Proprietary Memory Card",
        L"I/O Riser Card",
        L"NuBus",
        L"PCI-66",
        L"AGP",
        L"AGP 2x",
        L"AGP 4x",
        L"PCI-X",
        L"AGP 8x" /* 0x13 */
    };
    WCHAR *szType_0xA0[] = {
        L"PC-98/C20", /* 0xA0 */
        L"PC-98/C24",
        L"PC-98/E",
        L"PC-98/Local Bus",
        L"PC-98/Card",
        L"PCI Express",
        L"PCI Express x1",
        L"PCI Express x2",
        L"PCI Express x4",
        L"PCI Express x8",
        L"PCI Express x16",
        L"PCI Express 2",
        L"PCI Express 2 x1",
        L"PCI Express 2 x2",
        L"PCI Express 2 x4",
        L"PCI Express 2 x8",
        L"PCI Express 2 x16", /* 0xB0 */
    };

    if (Code >= 0x01 && Code <= 0x13)
    {
        IoAddValueName(1, 0, IDS_TYPE);
        IoSetItemText(szType[Code - 0x01]);
    }

    if (Code >= 0xA0 && Code <= 0xB0)
    {
        IoAddValueName(1, 0, IDS_TYPE);
        IoSetItemText(szType_0xA0[Code - 0xA0]);
    }
}

VOID
DmiSlotWidth(BYTE Code)
{
    WCHAR *szWidth[] = {
        NULL, /* 0x01, "Other" */
        NULL, /* "Unknown" */
        L"8-bit",
        L"16-bit",
        L"32-bit",
        L"64-bit",
        L"128-bit",
        L"x1",
        L"x2",
        L"x4",
        L"x8",
        L"x12",
        L"x16",
        L"x32" /* 0x0E */
    };

    if (Code >= 0x01 && Code <= 0x0E && szWidth[Code - 0x01] != NULL)
    {
        IoAddValueName(1, 0, IDS_SLOT_BUS_WIDTH);
        IoSetItemText(szWidth[Code - 0x01]);
    }
}

VOID
DmiSlotLength(BYTE Code)
{
    WCHAR *szLength[] = {
        L"Other", /* 0x01 */
        NULL, /* Unknown */
        L"Short",
        L"Long" /* 0x04 */
    };

    if (Code >= 0x01 && Code <= 0x04 && szLength[Code - 0x01] != NULL)
    {
        IoAddValueName(1, 0, IDS_SLOT_LENGTH);
        IoSetItemText(szLength[Code - 0x01]);
    }
}

VOID CALLBACK
SlotInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x0C) return;

    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
        IoAddHeaderString(0, 0, szText);
    else
        IoAddHeaderString(0, 0, L"Unknown");

    /* Type */
    DmiSlotType(pBuf[0x05]);

    /* Bus width */
    DmiSlotWidth(pBuf[0x06]);

    /* Length */
    DmiSlotLength(pBuf[0x08]);
}

VOID
DMI_SlotInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_PORT);

    if (EnumDMITablesByType(9, SlotInfoEnumProc))
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

    if (Length < 0x09) return;

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

    if (EnumDMITablesByType(8, PortsInfoEnumProc))
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

    if (EnumDMITablesByType(10, OnboardInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiBatteryChemistry(BYTE Code)
{
    /* 7.22.1 */
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        NULL,
        L"Mouse",
        L"Track Ball",
        L"Track Point",
        L"Glide Point",
        L"Touch Pad",
        L"Touch Screen",
        L"Optical Sensor" /* 0x09 */
    };

    if (Code >= 0x01 && Code <= 0x09 && szType[Code - 0x01] != NULL)
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_DEVICE_CHEMISTRY);
        IoSetItemText(szType[Code - 0x01]);
    }
}

VOID CALLBACK
BatteryInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x10) return;

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
    DmiBatteryChemistry(pBuf[0x09]);

    /* Design Capacity */
    IoAddValueName(1, 0, IDS_DMI_BATTERY_DESIGN_CAPACITY);
    IoSetItemText(L"%u mWh",
                  (Length < 0x16) ? WORD(pBuf + 0x0A) : WORD(pBuf + 0x0A) * pBuf[0x15]);

    /* Design Voltage */
    if (WORD(pBuf + 0x0C) > 0)
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_DESIGN_VOLTAGE);
        IoSetItemText(L"%u mV", WORD(pBuf + 0x0C));
    }

    /* SBDS Version Number */
    if (GetDmiString(pBuf, pBuf[0x0E], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_BATTERY_SBDS_VERSION);
        IoSetItemText(szText);
    }

    /* Max. Error in Battery Data */
    IoAddValueName(1, 0, IDS_DMI_BATTERY_MAXERROR_IN_DATA);
    IoSetItemText(L"%u%%", pBuf[0x0F]);

    if (Length < 0x1A) return;

    /* SBDS Serial Number */
    IoAddValueName(1, 0, IDS_DMI_BATTERY_SBDS_SERIAL);
    IoSetItemText(L"%04X", WORD(pBuf + 0x10));

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

    if (EnumDMITablesByType(22, BatteryInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiPointingDeviceType(BYTE Code)
{
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        NULL,
        L"Mouse",
        L"Track Ball",
        L"Track Point",
        L"Glide Point",
        L"Touch Pad",
        L"Touch Screen",
        L"Optical Sensor" /* 0x09 */
    };

    if (Code >= 0x01 && Code <= 0x09 && szType[Code - 0x01] != NULL)
    {
        IoAddValueName(1, 0, IDS_DMI_POINTING_TYPE);
        IoSetItemText(szType[Code - 0x01]);
    }
}

VOID
DmiPointingDeviceInterface(BYTE Code)
{
    WCHAR *szInterface[] = {
        L"Other", /* 0x01 */
        NULL,
        L"Serial",
        L"PS/2",
        L"Infrared",
        L"HIP-HIL",
        L"Bus Mouse",
        L"ADB (Apple Desktop Bus)" /* 0x08 */
    };
    WCHAR *szInterface_0xA0[] = {
        L"Bus Mouse DB-9", /* 0xA0 */
        L"Bus Mouse Micro DIN",
        L"USB" /* 0xA2 */
    };

    if (Code >= 0x01 && Code <= 0x08 && szInterface[Code - 0x01] != NULL)
    {
        IoAddValueName(1, 0, IDS_DMI_POINTING_INTERFACE);
        IoSetItemText(szInterface[Code - 0x01]);
    }

    if (Code >= 0xA0 && Code <= 0xA2)
    {
        IoAddValueName(1, 0, IDS_DMI_POINTING_INTERFACE);
        IoSetItemText(szInterface_0xA0[Code - 0xA0]);
    }
}

VOID CALLBACK
PointingInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x07) return;

    IoAddHeader(0, 0, IDS_DMI_POINTING_DEVICE);

    /* Device Type */
    DmiPointingDeviceType(pBuf[0x04]);

    /* Device Interface */
    DmiPointingDeviceInterface(pBuf[0x05]);

    /* Buttons Count */
    IoAddValueName(1, 0, IDS_DMI_POINTING_BUTTONS);
    IoSetItemText(L"%u", pBuf[0x06]);
}

VOID
DMI_PointingInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_MOUSE);

    if (EnumDMITablesByType(21, PointingInfoEnumProc))
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

    if (EnumDMITablesByType(2, BoardNameInfoEnumProc))
    {
        StringCbCopy(lpName, NameSize, szMainboardName);
        StringCbCopy(lpManuf, ManufSize, szMainboardVendor);
        return TRUE;
    }

    return FALSE;
}

VOID CALLBACK
OemStringsInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];
    BYTE *p = pBuf + 4;
    BYTE count = p[0x00];
    INT i;

    if (Length < 0x05) return;

    for (i = 1; i <= count; i++)
    {
        if (GetDmiString(pBuf, i, Length, szText, sizeof(szText)))
        {
            IoAddItem(0, 0, L"String #%d", i);
            IoSetItemText(szText);
        }
    }
}

VOID
DMI_OemStringsInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_TEXT);

    if (EnumDMITablesByType(11, OemStringsInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiMemoryControllerEdMethod(BYTE Code)
{
    /* 7.6.1 */
    WCHAR *szMethod[] = {
        L"Other", /* 0x01 */
        NULL,
        L"None",
        L"8-bit Parity",
        L"32-bit ECC",
        L"64-bit ECC",
        L"128-bit ECC",
        L"CRC" /* 0x08 */
    };

    if (Code >= 0x01 && Code <= 0x08 && szMethod[Code - 0x01] != NULL)
    {
        IoAddValueName(0, 0, IDS_DMI_MEM_CTRL_ERROR_DETECT_METHOD);
        IoSetItemText(szMethod[Code - 0x01]);
    }
}

VOID
DmiMemoryControllerEcCapabilities(BYTE *pBuf, BYTE Length)
{
    /* 7.6.2 */
    WCHAR *szCapabilities[] = {
        L"Other", /* 0 */
        NULL,
        L"None",
        L"Single-bit Error Correcting",
        L"Double-bit Error Correcting",
        L"Error Scrubbing" /* 5 */
    };
    INT i;

    if ((pBuf[0x04] & 0x3F) == 0)
        return;

    IoAddHeader(0, 0, IDS_DMI_MEM_CTRL_ERROR_CORRECT_CAP);

    for (i = 0; i <= 5; i++)
    {
        BOOL IsSupported = (pBuf[0x04] & (1 << i)) ? TRUE : FALSE;
        BOOL IsEnabled = FALSE;

        if (szCapabilities[i] != NULL)
        {
            IoAddItem(1, IsSupported ? 1 : 2, szCapabilities[i]);

            if (Length >= 0x10 + pBuf[0x0E] * sizeof(WORD))
            {
                IsEnabled = (pBuf[0x0F + pBuf[0x0E] * sizeof(WORD)] & (1 << i)) ? TRUE : FALSE;

                IoSetItemText(IsSupported ? (IsEnabled ? L"Supported, Enabled" : L"Supported, Disabled") : L"Unsupported");
            }
            else
            {
                IoSetItemText(IsSupported ? L"Supported" : L"Unsupported");
            }
        }
    }
}

VOID
DmiMemoryControllerInterleave(BYTE Code, UINT ValueId)
{
    /* 7.6.3 */
    WCHAR *szInterleave[] = {
        L"Other", /* 0x01 */
        NULL,
        L"One-way Interleave",
        L"Two-way Interleave",
        L"Four-way Interleave",
        L"Eight-way Interleave",
        L"Sixteen-way Interleave" /* 0x07 */
    };

    if (Code >= 0x01 && Code <= 0x07 && szInterleave[Code - 0x01] != NULL)
    {
        IoAddValueName(0, 0, ValueId);
        IoSetItemText(szInterleave[Code - 0x01]);
    }
}

VOID
DmiMemoryControllerSpeeds(WORD Code)
{
    WCHAR szText[MAX_STR_LEN] = {0};
    /* 7.6.4 */
    WCHAR *szSpeeds[] = {
        L"Other, ", /* 0 */
        NULL,
        L"70 ns, ",
        L"60 ns, ",
        L"50 ns, " /* 4 */
    };
    INT i;

    if ((Code & 0x001F) == 0)
        return;

    for (i = 0; i <= 4; i++)
    {
        if ((Code & (1 << i)) && (szSpeeds[i] != NULL))
        {
            StringCbCat(szText, sizeof(szText), szSpeeds[i]);
        }
    }

    if (szText[0] != 0)
    {
        szText[wcslen(szText) - 2] = 0;

        IoAddValueName(0, 0, IDS_DMI_MEM_CTRL_SUPPORTED_SPEEDS);
        IoSetItemText(szText);
    }
}

VOID
DmiMemoryModuleTypes(WORD Code, INT Indent)
{
    WCHAR szText[MAX_STR_LEN] = {0};
    /* 7.7.1 */
    WCHAR *szTypes[] = {
        L"Other, ", /* 0 */
        NULL,
        L"Standard, ",
        L"FPM, ",
        L"EDO, ",
        L"Parity, ",
        L"ECC, ",
        L"SIMM, ",
        L"DIMM, ",
        L"Burst EDO, ",
        L"SDRAM, " /* 10 */
    };
    INT i;

    if ((Code & 0x07FF) == 0)
        return;

    for (i = 0; i <= 10; i++)
    {
        if ((Code & (1 << i)) && (szTypes[i] != NULL))
        {
            StringCbCat(szText, sizeof(szText), szTypes[i]);
        }
    }

    if (szText[0] != 0)
    {
        szText[wcslen(szText) - 2] = 0;

        IoAddValueName(Indent, 0, IDS_DMI_MEM_CTRL_SUP_MEM_TYPES);
        IoSetItemText(szText);
    }
}

VOID CALLBACK
MemCtrlInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x0F) return;

    /* Error Detecting Method */
    DmiMemoryControllerEdMethod(pBuf[0x04]);

    /* Error Correcting Capabilities */
    DmiMemoryControllerEcCapabilities(pBuf, Length);

    /* Supported Interleave */
    DmiMemoryControllerInterleave(pBuf[0x06],
        IDS_DMI_MEM_CTRL_SUP_INTERLEAVE);

    /* Current Interleave */
    DmiMemoryControllerInterleave(pBuf[0x07],
        IDS_DMI_MEM_CTRL_CUR_INTERLEAVE);

    /* Maximum Memory Module Size */
    IoAddValueName(0, 0, IDS_DMI_MEM_CTRL_MAX_MODULE_SIZE);
    IoSetItemText(L"%u MB", 1 << pBuf[0x08]);

    /* Maximum Total Memory Size */
    IoAddValueName(0, 0, IDS_DMI_MEM_CTRL_MAX_TOTAL_SIZE);
    IoSetItemText(L"%u MB", pBuf[0x0E] * (1 << pBuf[0x08]));

    /* Supported Speeds */
    DmiMemoryControllerSpeeds(WORD(pBuf + 0x09));

    /* Supported Memory Types */
    DmiMemoryModuleTypes(WORD(pBuf + 0x0B), 0);

    /* Memory Module Voltage */
    DmiProcessorVoltage(pBuf[0x0D], 0, IDS_DMI_MEM_CTRL_MODULE_VOLTAGE);
}

VOID
DMI_MemCtrlInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);
    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);

    if (EnumDMITablesByType(5, MemCtrlInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiMemoryModuleConnections(BYTE Code)
{
    WCHAR szText[MAX_STR_LEN] = {0};
    WCHAR szTemp[MAX_STR_LEN] = {0};

    if (Code != 0xFF)
    {
        if ((Code & 0xF0) != 0xF0)
        {
            StringCbPrintf(szTemp, sizeof(szTemp), L"%u, ", Code >> 4);
            StringCbCat(szText, sizeof(szText), szTemp);
        }

        if ((Code & 0x0F) != 0x0F)
        {
            StringCbPrintf(szTemp, sizeof(szTemp), L"%u, ", Code & 0x0F);
            StringCbCat(szText, sizeof(szText), szTemp);
        }

        if (szText[0] != 0)
        {
            szText[wcslen(szText) - 2] = 0;

            IoAddValueName(1, 0, IDS_DMI_MEM_MODULE_BANK_CONNECTIONS);
            IoSetItemText(szText);
        }
    }
}

VOID
DmiMemoryModuleSize(BYTE Code, UINT ValueId)
{
    WCHAR szText[MAX_STR_LEN] = {0};

    IoAddValueName(1, 0, ValueId);

    /* 7.7.2 */
    switch (Code & 0x7F)
    {
        case 0x7D:
            StringCbCopy(szText, sizeof(szText), L"Not Determinable");
            break;
        case 0x7E:
            StringCbCopy(szText, sizeof(szText), L"Disabled");
            break;
        case 0x7F:
            IoSetItemText(L"Not Installed");
            return;
        default:
            StringCbPrintf(szText, sizeof(szText),
                           L"%u MB", 1 << (Code & 0x7F));
            break;
    }

    StringCbCat(szText, sizeof(szText),
        (Code & 0x80) ? L" (Double-bank Connection)" : L" (Single-bank Connection)");
    IoSetItemText(szText);
}

VOID
DmiMemoryModuleError(BYTE Code)
{
    WCHAR szText[MAX_STR_LEN] = {0};

    if (Code & (1 << 2))
    {
        IoAddValueName(1, 0, IDS_DMI_MEM_MODULE_ERROR_STATUS);
        IoSetItemText(L"See Event Log");
    }
    else
    {
        if ((Code & 0x03) == 0)
        {
            StringCbCat(szText, sizeof(szText), L"OK, ");
        }
        if (Code & (1 << 0))
        {
            StringCbCat(szText, sizeof(szText), L"Uncorrectable Errors, ");
        }
        if (Code & (1 << 1))
        {
            StringCbCat(szText, sizeof(szText), L"Correctable Errors, ");
        }

        if (szText[0] != 0)
        {
            szText[wcslen(szText) - 2] = 0;

            IoAddValueName(1, 0, IDS_DMI_MEM_MODULE_ERROR_STATUS);
            IoSetItemText(szText);
        }
    }
}

VOID CALLBACK
MemModuleInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x0C) return;

    /* Socket Designation */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddHeaderString(0, 0, szText);
    }
    else
    {
        IoAddHeaderString(0, 0, L"Unknown");
    }

    /* Bank Connections */
    DmiMemoryModuleConnections(pBuf[0x05]);

    /* Current Speed */
    if (pBuf[0x06] != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_MEM_MODULE_CURRENT_SPEED);
        IoSetItemText(L"%u ns", pBuf[0x06]);
    }

    /* Type */
    DmiMemoryModuleTypes(WORD(pBuf + 0x07), 1);

    /* Installed Size */
    DmiMemoryModuleSize(pBuf[0x09], IDS_DMI_MEM_MODULE_INSTALLED_SIZE);

    /* Enabled Size */
    DmiMemoryModuleSize(pBuf[0x0A], IDS_DMI_MEM_MODULE_ENABLED_SIZE);

    /* Error Status */
    DmiMemoryModuleError(pBuf[0x0B]);
}

VOID
DMI_MemModuleInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(6, MemModuleInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
GetMemorySizeWithUnit(DWORD64 MemorySize, BOOL Shift,
                      WCHAR *pOut, SIZE_T OutSize)
{
    DWORD64 Size = Shift ? MemorySize * 1024 : MemorySize;
    DWORD64 Divider;
    LPWSTR pUnit;

    if (Size >= ((DWORD64)1024 * (DWORD64)1024 * (DWORD64)1024 * (DWORD64)1024))
    {
        Divider = ((DWORD64)1024 * (DWORD64)1024 * (DWORD64)1024 * (DWORD64)1024);
        pUnit = L"TB";
    }
    else if (Size >= ((DWORD64)1024 * (DWORD64)1024 * (DWORD64)1024))
    {
        Divider = ((DWORD64)1024 * (DWORD64)1024 * (DWORD64)1024);
        pUnit = L"GB";
    }
    else if (Size >= ((DWORD64)1024 * (DWORD64)1024))
    {
        Divider = ((DWORD64)1024 * (DWORD64)1024);
        pUnit = L"MB";
    }
    else if (Size >= (DWORD64)1024)
    {
        Divider = ((DWORD64)1024);
        pUnit = L"kB";
    }
    else
    {
        Divider = 1;
        pUnit = L"Bytes";
    }

    StringCbPrintf(pOut, OutSize, L"%lu ",
                   (Size / Divider));
    StringCbCat(pOut, OutSize, pUnit);
}

VOID CALLBACK
MemArrayInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x0F) return;

    if (Length >= 0x1F && DWORD(pBuf + 0x04) == 0xFFFFFFFF)
    {
        DWORD64 Start, End;

        Start = DWORD64(pBuf + 0x0F);
        End   = DWORD64(pBuf + 0x17);

        IoAddValueName(0, 0, IDS_DMI_MEM_ARR_MAP_ADR_START_ADDRESS);
        IoSetItemText(L"0x%16X", Start);

        IoAddValueName(0, 0, IDS_DMI_MEM_ARR_MAP_ADR_END_ADDRESS);
        IoSetItemText(L"0x%16X", End);

        IoAddValueName(0, 0, IDS_DMI_MEM_ARR_MAP_ADR_RANGE_SIZE);
        GetMemorySizeWithUnit(End - Start + 1, FALSE, szText, sizeof(szText));
        IoSetItemText(szText);
    }
    else
    {
        IoAddValueName(0, 0, IDS_DMI_MEM_ARR_MAP_ADR_START_ADDRESS);
        IoSetItemText(L"0x%08X%03X",
                      DWORD(pBuf + 0x04) >> 2,
                      (DWORD(pBuf + 0x04) & 0x3) << 10);

        IoAddValueName(0, 0, IDS_DMI_MEM_ARR_MAP_ADR_END_ADDRESS);
        IoSetItemText(L"0x%08X%03X",
                      DWORD(pBuf + 0x08) >> 2,
                      ((DWORD(pBuf + 0x08) & 0x3) << 10) + 0x3FF);

        IoAddValueName(0, 0, IDS_DMI_MEM_ARR_MAP_ADR_RANGE_SIZE);
        GetMemorySizeWithUnit(DWORD(pBuf + 0x08) - DWORD(pBuf + 0x04) + 1,
                              TRUE, szText, sizeof(szText));
        IoSetItemText(szText);
    }

    IoAddValueName(0, 0, IDS_DMI_MEM_ARR_MAP_ADR_HANDLE);
    IoSetItemText(L"0x%04X", WORD(pBuf + 0x0C));

    IoAddValueName(0, 0, IDS_DMI_MEM_ARR_MAP_ADR_WIDTH);
    IoSetItemText(L"%u", pBuf[0x0E]);
}

VOID
DMI_MemArrayInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(19, MemArrayInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiVoltageProbeLocation(BYTE Code)
{
    /* 7.27.1 */
    WCHAR *szLocation[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"Processor",
        L"Disk",
        L"Peripheral Bay",
        L"System Management Module",
        L"Motherboard",
        L"Memory Module",
        L"Processor Module",
        L"Power Unit",
        L"Add-in Card" /* 0x0B */
    };

    if (Code >= 0x01 && Code <= 0x0B)
    {
        IoAddHeaderString(0, 0, szLocation[Code - 0x01]);
    }
    else
    {
        IoAddHeaderString(0, 0, L"Unknown Location");
    }
}

VOID
DmiProbeStatus(BYTE Code)
{
    /* 7.27.1 */
    WCHAR *szStatus[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"OK",
        L"Non-critical",
        L"Critical",
        L"Non-recoverable" /* 0x06 */
    };

    if (Code >= 0x01 && Code <= 0x06)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_STATUS);
        IoSetItemText(szStatus[Code - 0x01]);
    }
}

VOID CALLBACK
VoltageProbeInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x14) return;

    /* Location */
    DmiVoltageProbeLocation(pBuf[0x05] & 0x1F);

    /* Description */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_DESC);
        IoSetItemText(szText);
    }

    /* Status */
    DmiProbeStatus(pBuf[0x05] >> 5);

    /* Maximum Value */
    if (WORD(pBuf + 0x06) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_MAX_VAL);
        IoSetItemText(L"%.3f V", (FLOAT)WORD(pBuf + 0x06) / (FLOAT)1000);
    }

    /* Minimum Value */
    if (WORD(pBuf + 0x08) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_MIN_VAL);
        IoSetItemText(L"%.3f V", (FLOAT)WORD(pBuf + 0x08) / (FLOAT)1000);
    }

    /* Resolution */
    if (WORD(pBuf + 0x0A) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_RESOLUTION);
        IoSetItemText(L"%.1f mV", (FLOAT)WORD(pBuf + 0x0A) / (FLOAT)10);
    }

    /* Tolerance */
    if (WORD(pBuf + 0x0C) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_TOLERANCE);
        IoSetItemText(L"%.3f V", (FLOAT)WORD(pBuf + 0x0C) / (FLOAT)1000);
    }

    /* Accuracy */
    if (WORD(pBuf + 0x0E) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_ACCURACY);
        IoSetItemText(L"%.2f%%", (FLOAT)WORD(pBuf + 0x0E) / (FLOAT)100);
    }

    /* OEM-specific Information */
    if (DWORD(pBuf + 0x10) != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_OEM);
        IoSetItemText(L"0x%08X", DWORD(pBuf + 0x10));
    }

    if (Length < 0x16) return;

    /* Nominal Value */
    if (WORD(pBuf + 0x14) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_NOMINAL);
        IoSetItemText(L"%.3f V", (FLOAT)WORD(pBuf + 0x14) / (FLOAT)1000);
    }
}

VOID
DMI_VoltageProbeInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_VOLTAGE);

    if (EnumDMITablesByType(26, VoltageProbeInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiTemperatureProbeLocation(BYTE Code)
{
    /* 7.29.1 */
    WCHAR *szLocation[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"Processor",
        L"Disk",
        L"Peripheral Bay",
        L"System Management Module",
        L"Motherboard",
        L"Memory Module",
        L"Processor Module",
        L"Power Unit",
        L"Add-in Card",
        L"Front Panel Board",
        L"Back Panel Board",
        L"Power System Board",
        L"Drive Back Plane" /* 0x0F */
    };

    if (Code >= 0x01 && Code <= 0x0F)
    {
        IoAddHeaderString(0, 0, szLocation[Code - 0x01]);
    }
    else
    {
        IoAddHeaderString(0, 0, L"Unknown Location");
    }
}

VOID CALLBACK
TempProbeInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x14) return;

    /* Location */
    DmiTemperatureProbeLocation(pBuf[0x05] & 0x1F);

    /* Description */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_DESC);
        IoSetItemText(szText);
    }

    /* Status */
    DmiProbeStatus(pBuf[0x05] >> 5);

    /* Maximum Value */
    if (WORD(pBuf + 0x06) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_MAX_VAL);
        IoSetItemText(L"%.1f deg C", (FLOAT)WORD(pBuf + 0x06) / (FLOAT)10);
    }

    /* Minimum Value */
    if (WORD(pBuf + 0x08) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_MIN_VAL);
        IoSetItemText(L"%.1f deg C", (FLOAT)WORD(pBuf + 0x08) / (FLOAT)10);
    }

    /* Resolution */
    if (WORD(pBuf + 0x0A) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_RESOLUTION);
        IoSetItemText(L"%.3f deg C", (FLOAT)WORD(pBuf + 0x0A) / (FLOAT)1000);
    }

    /* Tolerance */
    if (WORD(pBuf + 0x0C) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_TOLERANCE);
        IoSetItemText(L"%.1f deg C", (FLOAT)WORD(pBuf + 0x0C) / (FLOAT)10);
    }

    /* Accuracy */
    if (WORD(pBuf + 0x0E) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_ACCURACY);
        IoSetItemText(L"%.2f%%", (FLOAT)WORD(pBuf + 0x0E) / (FLOAT)100);
    }

    /* OEM-specific Information */
    if (DWORD(pBuf + 0x10) != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_OEM);
        IoSetItemText(L"0x%08X", DWORD(pBuf + 0x10));
    }

    if (Length < 0x16) return;

    /* Nominal Value */
    if (WORD(pBuf + 0x14) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_NOMINAL);
        IoSetItemText(L"%.1f deg C", (FLOAT)WORD(pBuf + 0x14) / (FLOAT)10);
    }
}

VOID
DMI_TempProbeInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_TEMPERATURE);

    if (EnumDMITablesByType(28, TempProbeInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID CALLBACK
ElectrProbeInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x14) return;

    /* Location */
    DmiVoltageProbeLocation(pBuf[0x05] & 0x1F);

    /* Description */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_DESC);
        IoSetItemText(szText);
    }

    /* Status */
    DmiProbeStatus(pBuf[0x05] >> 5);

    /* Maximum Value */
    if (WORD(pBuf + 0x06) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_MAX_VAL);
        IoSetItemText(L"%.3f A", (FLOAT)WORD(pBuf + 0x06) / (FLOAT)1000);
    }

    /* Minimum Value */
    if (WORD(pBuf + 0x08) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_MIN_VAL);
        IoSetItemText(L"%.3f A", (FLOAT)WORD(pBuf + 0x08) / (FLOAT)1000);
    }

    /* Resolution */
    if (WORD(pBuf + 0x0A) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_RESOLUTION);
        IoSetItemText(L"%.1f mA", (FLOAT)WORD(pBuf + 0x0A) / (FLOAT)10);
    }

    /* Tolerance */
    if (WORD(pBuf + 0x0C) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_TOLERANCE);
        IoSetItemText(L"%.3f A", (FLOAT)WORD(pBuf + 0x0C) / (FLOAT)10);
    }

    /* Accuracy */
    if (WORD(pBuf + 0x0E) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_ACCURACY);
        IoSetItemText(L"%.2f%%", (FLOAT)WORD(pBuf + 0x0E) / (FLOAT)100);
    }

    /* OEM-specific Information */
    if (DWORD(pBuf + 0x10) != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_OEM);
        IoSetItemText(L"0x%08X", DWORD(pBuf + 0x10));
    }

    if (Length < 0x16) return;

    /* Nominal Value */
    if (WORD(pBuf + 0x14) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_NOMINAL);
        IoSetItemText(L"%.3f A", (FLOAT)WORD(pBuf + 0x14) / (FLOAT)10);
    }
}

VOID
DMI_ElectrProbeInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_VOLTAGE);

    if (EnumDMITablesByType(29, ElectrProbeInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiCoolingDeviceType(BYTE Code)
{
    /* 7.28.1 */
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"Fan",
        L"Centrifugal Blower",
        L"Chip Fan",
        L"Cabinet Fan",
        L"Power Supply Fan",
        L"Heat Pipe",
        L"Integrated Refrigeration" /* 0x09 */
    };
    WCHAR *szType_0x10[] = {
        L"Active Cooling", /* 0x10 */
        L"Passive Cooling" /* 0x11 */
    };

    if (Code >= 0x01 && Code <= 0x09)
    {
        IoAddHeaderString(0, 0, szType[Code - 0x01]);
    }
    else if (Code >= 0x10 && Code <= 0x11)
    {
        IoAddHeaderString(0, 0, szType_0x10[Code - 0x10]);
    }
    else
    {
        IoAddHeaderString(0, 0, L"Unknown");
    }
}

VOID CALLBACK
CoolingDeviceInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x0C) return;

    /* Type */
    DmiCoolingDeviceType(pBuf[0x06] & 0x1F);

    /* Handle */
    if (WORD(pBuf + 0x04) != 0xFFFF)
    {
        IoAddValueName(1, 0, IDS_DMI_COOLING_HANDLE);
        IoSetItemText(L"0x%04X", WORD(pBuf + 0x04));
    }

    /* Status */
    DmiProbeStatus(pBuf[0x06] >> 5);

    /* Unit Group */
    if (pBuf[0x07] != 0x00)
    {
        IoAddValueName(1, 0, IDS_DMI_COOLING_UNIT_GROUP);
        IoSetItemText(L"%u", pBuf[0x07]);
    }

    /* OEM-specific Information */
    if (DWORD(pBuf + 0x08) != 0)
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_OEM);
        IoSetItemText(L"0x%08X", DWORD(pBuf + 0x08));
    }

    if (Length < 0x0E) return;

    /* Nominal Speed */
    if (WORD(pBuf + 0x0C) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_COOLING_NOMINAL_SPEED);
        IoSetItemText(L"%u RPM", DWORD(pBuf + 0x0C));
    }

    if (Length < 0x0F) return;

    /* Description */
    if (GetDmiString(pBuf, pBuf[0x0E], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_PROBE_DESC);
        IoSetItemText(szText);
    }
}

VOID
DMI_CoolingDeviceInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_FAN);

    if (EnumDMITablesByType(27, CoolingDeviceInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiPowerSupplyStatus(BYTE Code)
{
    /* 7.40.1 */
    WCHAR *szStatus[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"OK",
        L"Non-critical",
        L"Critical" /* 0x05 */
    };

    if (Code >= 0x01 && Code <= 0x05)
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_STATUS);
        IoSetItemText(szStatus[Code - 0x01]);
    }
}

VOID
DmiPowerSupplyType(BYTE Code)
{
    /* 7.40.1 */
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"Linear",
        L"Switching",
        L"Battery",
        L"UPS",
        L"Converter",
        L"Regulator" /* 0x08 */
    };

    if (Code >= 0x01 && Code <= 0x08)
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_TYPE);
        IoSetItemText(szType[Code - 0x01]);
    }
}

VOID
DmiPowerSupplyRangeSwitching(BYTE Code)
{
    /* 7.40.1 */
    WCHAR *szSwitching[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"Manual",
        L"Auto-switch",
        L"Wide Range",
        L"N/A" /* 0x06 */
    };

    if (Code >= 0x01 && Code <= 0x06)
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_IN_VOLTAGE_RANGE);
        IoSetItemText(szSwitching[Code - 0x01]);
    }
}

VOID CALLBACK
SysPowerSupplyInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x10) return;

    /* Location */
    if (GetDmiString(pBuf, pBuf[0x05], Length, szText, sizeof(szText)))
    {
        IoAddHeaderString(0, 0, szText);
    }
    else
    {
        IoAddHeaderString(0, 0, L"Unknown Location");
    }

    /* Power Unit Group */
    IoAddValueName(1, 0, IDS_DMI_SPS_UNIT_GROUP);
    IoSetItemText(L"%u", pBuf[0x04]);

    /* Name */
    if (GetDmiString(pBuf, pBuf[0x06], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_NAME);
        IoSetItemText(szText);
    }

    /* Manufacturer */
    if (GetDmiString(pBuf, pBuf[0x07], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_MANUF);
        IoSetItemText(szText);
    }

    /* Serial Number */
    if (GetDmiString(pBuf, pBuf[0x08], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_SERIAL);
        IoSetItemText(szText);
    }

    /* Asset Tag */
    if (GetDmiString(pBuf, pBuf[0x09], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_ASSET_TAG);
        IoSetItemText(szText);
    }

    /* Model Part Number */
    if (GetDmiString(pBuf, pBuf[0x0A], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_MODEL_PART_NUMBER);
        IoSetItemText(szText);
    }

    /* Revision */
    if (GetDmiString(pBuf, pBuf[0x0B], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_REVISION);
        IoSetItemText(szText);
    }

    /* Max Power Capacity */
    if (WORD(pBuf + 0x0C) != 0x8000)
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_MAX_POWER_CAPACITY);
        IoSetItemText(L"%u W", WORD(pBuf + 0x0C));
    }

    /* Status */
    if (WORD(pBuf + 0x0E) & (1 << 1))
    {
        DmiPowerSupplyStatus((WORD(pBuf + 0x0E) >> 7) & 0x07);
    }

    /* Type */
    DmiPowerSupplyType((WORD(pBuf + 0x0E) >> 10) & 0x0F);

    /* Input Voltage Range Switching */
    DmiPowerSupplyRangeSwitching((WORD(pBuf + 0x0E) >> 3) & 0x0F);

    /* Plugged */
    IoAddValueName(1, 0, IDS_DMI_SPS_PLUGGED);
    IoSetItemText(L"%s", WORD(pBuf + 0x0E) & (1 << 2) ? L"No" : L"Yes");

    /* Hot Replaceable */
    IoAddValueName(1, 0, IDS_DMI_SPS_HOT_REPLACEABLE);
    IoSetItemText(L"%s", WORD(pBuf + 0x0E) & (1 << 0) ? L"Yes" : L"No");

    if (Length < 0x16) return;

    /* Input Voltage Probe Handle */
    if (WORD(pBuf + 0x10) != 0xFFFF)
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_IN_VOLTAGE_HANDLE);
        IoSetItemText(L"0x%04X", WORD(pBuf + 0x10));
    }

    /* Cooling Device Handle */
    if (WORD(pBuf + 0x12) != 0xFFFF)
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_COOLING_HANDLE);
        IoSetItemText(L"0x%04X", WORD(pBuf + 0x12));
    }

    /* Input Current Probe Handle */
    if (WORD(pBuf + 0x14) != 0xFFFF)
    {
        IoAddValueName(1, 0, IDS_DMI_SPS_IN_CURRENT_HANDLE);
        IoSetItemText(L"0x%04X", WORD(pBuf + 0x14));
    }
}

VOID
DMI_SysPowerSupplyInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_VOLTAGE);

    if (EnumDMITablesByType(39, SysPowerSupplyInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiBiosLanguages(BYTE *pBuf, BYTE Length)
{
    WCHAR szTemp[MAX_STR_LEN], szText[MAX_STR_LEN] = {0};
    BYTE *p = pBuf + 4;
    BYTE count = p[0x00];
    INT i;

    for (i = 1; i <= count; i++)
    {
        if (GetDmiString(pBuf, i, Length, szTemp, sizeof(szTemp)))
        {
            StringCbCat(szText, sizeof(szText), szTemp);
            StringCbCat(szText, sizeof(szText), L", ");
        }
    }

    if (szText[0] != 0)
    {
        szText[wcslen(szText) - 2] = 0;

        IoAddHeader(0, 0, IDS_DMI_BIOS_LANG_LANGUAGES);
        IoSetItemText(szText);
    }
}

VOID CALLBACK
BiosLanguageInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x16) return;

    /* Current Language */
    if (GetDmiString(pBuf, pBuf[0x15], Length, szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_DMI_BIOS_LANG_CURRENT);
        IoSetItemText(szText);
    }

    /* Languages Count */
    IoAddValueName(1, 0, IDS_DMI_BIOS_LANG_COUNT);
    IoSetItemText(L"%u", pBuf[0x04]);

    /* Languages List */
    DmiBiosLanguages(pBuf, Length);
}

VOID
DMI_BiosLanguageInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_CPU);

    if (EnumDMITablesByType(13, BiosLanguageInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID
DmiEventLogMethod(BYTE Code)
{
    WCHAR *szMethod[] = {
        L"Indexed I/O, one 8-bit index port, one 8-bit data port", /* 0x00 */
        L"Indexed I/O, two 8-bit index ports, one 8-bit data port",
        L"Indexed I/O, one 16-bit index port, one 8-bit data port",
        L"Memory-mapped physical 32-bit address",
        L"General-purpose non-volatile data functions" /* 0x04 */
    };

    if (Code <= 0x04)
    {
        IoAddValueName(0, 0, IDS_DMI_SEL_ACCESS_METHOD);
        IoSetItemText(szMethod[Code]);
    }
    if (Code >= 0x80)
    {
        IoAddValueName(0, 0, IDS_DMI_SEL_ACCESS_METHOD);
        IoSetItemText(L"OEM-specific");
    }
}

VOID
DmiEventLogAddress(BYTE Method, BYTE *p)
{
    IoAddValueName(0, 0, IDS_DMI_SEL_ACCESS_ADDRESS);

    /* 7.16.3 */
    switch (Method)
    {
        case 0x00:
        case 0x01:
        case 0x02:
            IoSetItemText(L"Index 0x%04X, Data 0x%04X", WORD(p), WORD(p + 2));
            break;
        case 0x03:
            IoSetItemText(L"0x%08X", DWORD(p));
            break;
        case 0x04:
            IoSetItemText(L"0x%04X", WORD(p));
            break;
        default:
            IoSetItemText(L"Unknown");
    }
}

VOID
DmiEventLogStatus(BYTE Code)
{
    WCHAR *szValid[] = {
        L"Invalid", /* 0 */
        L"Valid" /* 1 */
    };
    WCHAR *szFull[] = {
        L"Not Full", /* 0 */
        L"Full" /* 1 */
    };

    IoAddValueName(0, 0, IDS_DMI_SEL_STATUS);
    IoSetItemText(L"%s, %s", szValid[(Code >> 0) & 1],
                  szFull[(Code >> 1) & 1]);
}

VOID
DmiEventLogHeaderType(BYTE Code)
{
    WCHAR *szType[] = {
        L"No Header", /* 0x00 */
        L"Type 1" /* 0x01 */
    };

    if (Code <= 0x01)
    {
        IoAddValueName(0, 0, IDS_DMI_SEL_HEADER_FORMAT);
        IoSetItemText(szType[Code]);
    }
    if (Code >= 0x80)
    {
        IoAddValueName(0, 0, IDS_DMI_SEL_HEADER_FORMAT);
        IoSetItemText(L"OEM-specific");
    }
}

WCHAR*
DmiEventLogDescriptorType(BYTE Code)
{
    /* 7.16.6.1 */
    WCHAR *szType[] = {
        NULL, /* 0x00 */
        L"Single-bit ECC memory error",
        L"Multi-bit ECC memory error",
        L"Parity memory error",
        L"Bus timeout",
        L"I/O channel block",
        L"Software NMI",
        L"POST memory resize",
        L"POST error",
        L"PCI parity error",
        L"PCI system error",
        L"CPU failure",
        L"EISA failsafe timer timeout",
        L"Correctable memory log disabled",
        L"Logging disabled",
        NULL, /* 0x0F */
        L"System limit exceeded",
        L"Asynchronous hardware timer expired",
        L"System configuration information",
        L"Hard disk information",
        L"System reconfigured",
        L"Uncorrectable CPU-complex error",
        L"Log area reset/cleared",
        L"System boot" /* 0x17 */
    };

    if (Code <= 0x17 && szType[Code] != NULL)
    {
        return szType[Code];
    }
    if (Code >= 0x80 && Code <= 0xFE)
    {
        return L"OEM-specific";
    }
    if (Code == 0xFF)
    {
        return L"End of log";
    }
    return NULL;
}

WCHAR*
DmiEventLogDescriptorFormat(BYTE Code)
{
    /* 7.16.6.2 */
    WCHAR *szFormat[] = {
        L"None", /* 0x00 */
        L"Handle",
        L"Multiple-event",
        L"Multiple-event handle",
        L"POST results bitmap",
        L"System management",
        L"Multiple-event system management" /* 0x06 */
    };

    if (Code <= 0x06)
    {
        return szFormat[Code];
    }
    if (Code >= 0x80)
    {
        return L"OEM-specific";
    }
    return NULL;
}

VOID
DmiEventLogDescriptors(BYTE Count, BYTE Length, BYTE *p)
{
    /* 7.16.1 */
    INT i;

    for (i = 0; i < Count; i++)
    {
        if (Length >= 0x02)
        {
            WCHAR *Type = DmiEventLogDescriptorType(p[i * Length]);
            WCHAR *Format = DmiEventLogDescriptorFormat(p[i * Length + 1]);

            if (Type != NULL && Format != NULL)
            {
                IoAddItem(1, 0, L"Descriptor #%u", i + 1);
                IoSetItemText(Type);

                IoAddItem(1, 0, L"Data Format #%u", i + 1);
                IoSetItemText(Format);
            }
        }
    }
}

VOID CALLBACK
SysEventLogInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szFormat[MAX_STR_LEN];

    if (Length < 0x14) return;

    /* Area Length */
    IoAddValueName(0, 0, IDS_DMI_SEL_AREA_LENGTH);
    IoSetItemText(L"%u bytes", WORD(pBuf + 0x04));

    /* Header Start Offset */
    IoAddValueName(0, 0, IDS_DMI_SEL_HEADER_START_OFFSET);
    IoSetItemText(L"0x%04X", WORD(pBuf + 0x06));

    /* Header Length */
    if (WORD(pBuf + 0x08) - WORD(pBuf + 0x06))
    {
        IoAddValueName(0, 0, IDS_DMI_SEL_HEADER_LENGTH);
        IoSetItemText(L"%u byte%s", WORD(pBuf + 0x08) - WORD(pBuf + 0x06),
            (WORD(pBuf + 0x08) - WORD(pBuf + 0x06) > 1) ? L"s" : L"");
    }

    /* Data Start Offset */
    IoAddValueName(0, 0, IDS_DMI_SEL_DATA_START_OFFSET);
    IoSetItemText(L"0x%04X", WORD(pBuf + 0x08));

    /* Access Method */
    DmiEventLogMethod(pBuf[0x0A]);

    /* Access Address */
    DmiEventLogAddress(pBuf[0x0A], pBuf + 0x10);

    /* Status */
    DmiEventLogStatus(pBuf[0x0B]);

    /* Change Token */
    IoAddValueName(0, 0, IDS_DMI_SEL_CHANGE_TOKEN);
    IoSetItemText(L"0x%08X", DWORD(pBuf + 0x0C));

    if (Length < 0x17) return;

    /* Header Format */
    DmiEventLogHeaderType(pBuf[0x14]);

    if (Length < 0x17 + pBuf[0x15] * pBuf[0x16])
        return;

    /* Supported Log Type Descriptors */
    LoadMUIString(IDS_DMI_SEL_LOG_TYPE_DESCS, szFormat, MAX_STR_LEN);
    IoAddHeaderString(0, 0, szFormat, pBuf[0x15]);
    DmiEventLogDescriptors(pBuf[0x15], pBuf[0x16], pBuf + 0x17);
}

VOID
DMI_SysEventLogInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_EVENTVWR);

    if (EnumDMITablesByType(15, SysEventLogInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

WCHAR*
DmiSmbiosStructureType(BYTE Code)
{
    WCHAR *szType[] = {
        L"BIOS", /* 0 */
        L"System",
        L"Base Board",
        L"Chassis",
        L"Processor",
        L"Memory Controller",
        L"Memory Module",
        L"Cache",
        L"Port Connector",
        L"System Slots",
        L"On Board Devices",
        L"OEM Strings",
        L"System Configuration Options",
        L"BIOS Language",
        L"Group Associations",
        L"System Event Log",
        L"Physical Memory Array",
        L"Memory Device",
        L"32-bit Memory Error",
        L"Memory Array Mapped Address",
        L"Memory Device Mapped Address",
        L"Built-in Pointing Device",
        L"Portable Battery",
        L"System Reset",
        L"Hardware Security",
        L"System Power Controls",
        L"Voltage Probe",
        L"Cooling Device",
        L"Temperature Probe",
        L"Electrical Current Probe",
        L"Out-of-band Remote Access",
        L"Boot Integrity Services",
        L"System Boot",
        L"64-bit Memory Error",
        L"Management Device",
        L"Management Device Component",
        L"Management Device Threshold Data",
        L"Memory Channel",
        L"IPMI Device",
        L"Power Supply",
        L"Additional Information",
        L"Onboard Device",
        L"Management Controller Host Interface", /* 42 */
    };

    if (Code <= 42)
        return szType[Code];

    return NULL;
}

VOID CALLBACK
GroupAssocInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];
    INT i, count;

    if (Length < 0x05) return;

    /* Name */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(0, 0, IDS_DMI_GROUP_ASSOC_NAME);
        IoSetItemText(szText);
    }

    /* Items */
    count = (Length - 0x05) / 3;

    LoadMUIString(IDS_DMI_GROUP_ASSOC_ITEMS, szText, MAX_STR_LEN);
    IoAddHeaderString(0, 0, szText, count);

    for (i = 0; i < count; i++)
    {
        IoAddItem(1, 0, DmiSmbiosStructureType((pBuf + 0x05)[3 * i]));
        IoSetItemText(L"0x%04X", WORD((pBuf + 0x05) + 3 * i + 1));
    }
}

VOID
DMI_GroupAssocInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_COMPUTER);

    if (EnumDMITablesByType(14, GroupAssocInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID CALLBACK
SysConfigOptInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];
    BYTE *p = pBuf + 4;
    BYTE count = p[0x00];
    INT i;

    if (Length < 0x05) return;

    for (i = 1; i <= count; i++)
    {
        if (GetDmiString(pBuf, i, Length, szText, sizeof(szText)))
        {
            IoAddItem(0, 0, L"Option #%d", i);
            IoSetItemText(szText);
        }
    }
}

VOID
DMI_SysConfigOptInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_COMPUTER);

    if (EnumDMITablesByType(12, SysConfigOptInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

WCHAR*
DmiMemoryArrayLocation(BYTE Code)
{
    /* 7.17.1 */
    WCHAR *szLocation[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"System Board Or Motherboard",
        L"ISA Add-on Card",
        L"EISA Add-on Card",
        L"PCI Add-on Card",
        L"MCA Add-on Card",
        L"PCMCIA Add-on Card",
        L"Proprietary Add-on Card",
        L"NuBus" /* 0x0A */
    };
    WCHAR *szLocation_0xA0[] = {
        L"PC-98/C20 Add-on Card", /* 0xA0 */
        L"PC-98/C24 Add-on Card",
        L"PC-98/E Add-on Card",
        L"PC-98/Local Bus Add-on Card" /* 0xA3 */
    };

    if (Code >= 0x01 && Code <= 0x0A)
        return szLocation[Code - 0x01];
    if (Code >= 0xA0 && Code <= 0xA4)
        return szLocation_0xA0[Code - 0xA0];
    return L"Unknown";
}

WCHAR*
DmiMemoryArrayUse(BYTE Code)
{
    /* 7.17.2 */
    WCHAR *szUse[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"System Memory",
        L"Video Memory",
        L"Flash Memory",
        L"Non-volatile RAM",
        L"Cache Memory" /* 0x07 */
    };

    if (Code >= 0x01 && Code <= 0x07)
        return szUse[Code - 0x01];
    return L"Unknown";
}

WCHAR*
DmiMemoryArrayEcType(BYTE Code)
{
    /* 7.17.3 */
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"None",
        L"Parity",
        L"Single-bit ECC",
        L"Multi-bit ECC",
        L"CRC" /* 0x07 */
    };

    if (Code >= 0x01 && Code <= 0x07)
        return szType[Code - 0x01];
    return L"Unknown";
}

VOID
DmiMemoryArrayMaximumCapacity(BYTE *pBuf, BYTE Length)
{
    WCHAR szSize[MAX_STR_LEN];

    IoAddValueName(1, 0, IDS_DMI_PMA_MAX_CAPACITY);

    if (DWORD(pBuf + 0x07) == 0x80000000)
    {
        if (Length < 0x17)
        {
            IoSetItemText(L"Unknown");
        }
        else
        {
            GetMemorySizeWithUnit(DWORD64(pBuf + 0x0F), FALSE,
                                  szSize, sizeof(szSize));
            IoSetItemText(szSize);
        }
    }
    else
    {
        GetMemorySizeWithUnit(DWORD(pBuf + 0x07), TRUE,
                              szSize, sizeof(szSize));
        IoSetItemText(szSize);
    }
}

VOID
DmiMemoryArrayErrorHandle(WORD Code, INT Indent, UINT ValueId)
{
    IoAddValueName(Indent, 0, ValueId);

    if (Code == 0xFFFE)
        IoSetItemText(L"Not Provided");
    else if (Code == 0xFFFF)
        IoSetItemText(L"No Error");
    else
        IoSetItemText(L"0x%04X", Code);
}

VOID CALLBACK
PhysicMemArrayInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x0F) return;

    /* Location */
    IoAddHeaderString(0, 0, DmiMemoryArrayLocation(pBuf[0x04]));

    /* Use */
    IoAddValueName(1, 0, IDS_DMI_PMA_USE);
    IoSetItemText(DmiMemoryArrayUse(pBuf[0x05]));

    /* Error Correction Type */
    IoAddValueName(1, 0, IDS_DMI_PMA_ERROR_CORRECTION_TYPE);
    IoSetItemText(DmiMemoryArrayEcType(pBuf[0x06]));

    /* Maximum Capacity */
    DmiMemoryArrayMaximumCapacity(pBuf, Length);

    /* Error Information Handle */
    DmiMemoryArrayErrorHandle(WORD(pBuf + 0x0B), 1,
        IDS_DMI_PMA_ERROR_INFO_HANDLE);

    /* Number Of Devices */
    IoAddValueName(1, 0, IDS_DMI_PMA_NUMBER_OF_DEVICES);
    IoSetItemText(L"%u", WORD(pBuf + 0x0D));
}

VOID
DMI_PhysicMemArrayInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(16, PhysicMemArrayInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

/* code is assumed to be a 2-bit value */
WCHAR*
DmiSystemResetBootOption(BYTE Code)
{
    WCHAR *szOption[] = {
        L"Unknown", /* 0x0 */
        L"Operating System", /* 0x1 */
        L"System Utilities",
        L"Do Not Reboot" /* 0x3 */
    };

    return szOption[Code];
}

VOID CALLBACK
SysResetInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x0D) return;

    /* Status */
    IoAddValueName(0, 0, IDS_DMI_SR_STATUS);
    IoSetItemText(L"%s", pBuf[0x04] & (1 << 0) ? L"Enabled" : L"Disabled");

    /* Watchdog Timer */
    IoAddValueName(0, 0, IDS_DMI_SR_WATCHDOG_TIMER);
    IoSetItemText(L"%s", pBuf[0x04] & (1 << 5) ? L"Present" : L"Not Prosent");

    if (!(pBuf[0x04] & (1 << 5)))
        return;

    /* Boot Option */
    IoAddValueName(0, 0, IDS_DMI_SR_BOOT_OPTION);
    IoSetItemText(DmiSystemResetBootOption((pBuf[0x04] >> 1) & 0x3));

    /* Boot Option On Limit */
    IoAddValueName(0, 0, IDS_DMI_SR_BOOT_OPTION_ON_LIMIT);
    IoSetItemText(DmiSystemResetBootOption((pBuf[0x04] >> 3) & 0x3));

    /* Reset Count */
    if (WORD(pBuf + 0x05) != 0xFFFF)
    {
        IoAddValueName(0, 0, IDS_DMI_SR_RESET_COUNT);
        IoSetItemText(L"%u", WORD(pBuf + 0x05));
    }

    /* Reset Limit */
    if (WORD(pBuf + 0x07) != 0xFFFF)
    {
        IoAddValueName(0, 0, IDS_DMI_SR_RESET_LIMIT);
        IoSetItemText(L"%u", WORD(pBuf + 0x07));
    }

    /* Timer Interval */
    if (WORD(pBuf + 0x09) != 0xFFFF)
    {
        IoAddValueName(0, 0, IDS_DMI_SR_TIMER_INTERVAL);
        IoSetItemText(L"%u min", WORD(pBuf + 0x09));
    }

    /* Timeout */
    if (WORD(pBuf + 0x0B) != 0xFFFF)
    {
        IoAddValueName(0, 0, IDS_DMI_SR_TIMEOUT);
        IoSetItemText(L"%u min", WORD(pBuf + 0x0B));
    }
}

VOID
DMI_SysResetInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_COMPUTER);

    if (EnumDMITablesByType(23, SysResetInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

WCHAR*
DmiHardwareSecurityStatus(BYTE Code)
{
    WCHAR *szStatus[] = {
        L"Disabled", /* 0x00 */
        L"Enabled",
        L"Not Implemented",
        L"Unknown" /* 0x03 */
    };

    return szStatus[Code];
}

VOID CALLBACK
HardwareSecInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x05) return;

    /* Power-On Password Status */
    IoAddValueName(0, 0, IDS_DMI_HS_POWER_ON_PASSWORD);
    IoSetItemText(DmiHardwareSecurityStatus(pBuf[0x04] >> 6));

    /* Keyboard Password Status */
    IoAddValueName(0, 0, IDS_DMI_HS_KEYBOARD_PASSWORD);
    IoSetItemText(DmiHardwareSecurityStatus((pBuf[0x04] >> 4) & 0x3));

    /* Administrator Password Status */
    IoAddValueName(0, 0, IDS_DMI_HS_ADMIN_PASSWORD);
    IoSetItemText(DmiHardwareSecurityStatus((pBuf[0x04] >> 2) & 0x3));

    /* Front Panel Reset Status */
    IoAddValueName(0, 0, IDS_DMI_HS_FRONT_PANEL_RESET);
    IoSetItemText(DmiHardwareSecurityStatus(pBuf[0x04] & 0x3));
}

VOID
DMI_HardwareSecInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_SECURITY);

    if (EnumDMITablesByType(24, HardwareSecInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

INT
DmiBcdRange(BYTE Value, BYTE Low, BYTE High)
{
    if (Value > 0x99 || (Value & 0x0F) > 0x09)
        return 0;
    if (Value < Low || Value > High)
        return 0;
    return 1;
}

VOID
DmiPowerControlsPowerOn(BYTE *p)
{
    WCHAR szTime[MAX_STR_LEN] = {0};
    WCHAR szTemp[MAX_STR_LEN];

    IoAddValueName(0, 0, IDS_DMI_PC_POWER_ON);

    /* 7.26.1 */
    if (DmiBcdRange(p[0], 0x01, 0x12))
    {
        StringCbPrintf(szTemp, sizeof(szTemp), L"%02X", p[0]);
        StringCbCat(szTime, sizeof(szTime), szTemp);
    }
    else
    {
        StringCbCat(szTime, sizeof(szTime), L" *");
    }

    if (DmiBcdRange(p[1], 0x01, 0x31))
    {
        StringCbPrintf(szTemp, sizeof(szTemp), L"-%02X", p[1]);
        StringCbCat(szTime, sizeof(szTime), szTemp);
    }
    else
    {
        StringCbCat(szTime, sizeof(szTime), L"-*");
    }

    if (DmiBcdRange(p[2], 0x00, 0x23))
    {
        StringCbPrintf(szTemp, sizeof(szTemp), L" %02X", p[2]);
        StringCbCat(szTime, sizeof(szTime), szTemp);
    }
    else
    {
        StringCbCat(szTime, sizeof(szTime), L" *");
    }

    if (DmiBcdRange(p[3], 0x00, 0x59))
    {
        StringCbPrintf(szTemp, sizeof(szTemp), L":%02X", p[3]);
        StringCbCat(szTime, sizeof(szTime), szTemp);
    }
    else
    {
        StringCbCat(szTime, sizeof(szTime), L":*");
    }

    if (DmiBcdRange(p[4], 0x00, 0x59))
    {
        StringCbPrintf(szTemp, sizeof(szTemp), L":%02X", p[4]);
        StringCbCat(szTime, sizeof(szTime), szTemp);
    }
    else
    {
        StringCbCat(szTime, sizeof(szTime), L":*");
    }

    IoSetItemText(szTime);
}

VOID CALLBACK
SysPowerCtrlsInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x09) return;

    /* Next Scheduled Power-On */
    DmiPowerControlsPowerOn(pBuf + 0x04);
}

VOID
DMI_SysPowerCtrlsInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_COMPUTER);

    if (EnumDMITablesByType(25, SysPowerCtrlsInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID CALLBACK
RemoteAccessInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x06) return;

    /* Manufacturer */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(0, 0, IDS_DMI_OOBRA_MANUFACTURER);
        IoSetItemText(szText);
    }

    /* Inbound Connection */
    IoAddValueName(0, 0, IDS_DMI_OOBRA_INBOUND_CON);
    IoSetItemText(pBuf[0x05] & (1 << 0) ? L"Enabled" : L"Disabled");

    /* Outbound Connection */
    IoAddValueName(0, 0, IDS_DMI_OOBRA_OUTBOUND_CON);
    IoSetItemText(pBuf[0x05] & (1 << 1) ? L"Enabled" : L"Disabled");
}

VOID
DMI_RemoteAccessInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_MONITOR);

    if (EnumDMITablesByType(30, RemoteAccessInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

WCHAR*
DmiSystemBootStatus(BYTE Code)
{
    WCHAR *szStatus[] = {
        L"No errors detected", /* 0 */
        L"No bootable media",
        L"Operating system failed to load",
        L"Firmware-detected hardware failure",
        L"Operating system-detected hardware failure",
        L"User-requested boot",
        L"System security violation",
        L"Previously-requested image",
        L"System watchdog timer expired" /* 8 */
    };

    if (Code <= 8)
        return szStatus[Code];
    if (Code >= 128 && Code <= 191)
        return L"OEM-specific";
    if (Code >= 192)
        return L"Product-specific";
    return L"Out Of Spec";
}

VOID CALLBACK
SysBootInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x0B) return;

    /* Status */
    IoAddValueName(0, 0, IDS_DMI_SBI_STATUS);
    IoSetItemText(DmiSystemBootStatus(pBuf[0x0A]));
}

VOID
DMI_SysBootInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_COMPUTER);

    if (EnumDMITablesByType(32, SysBootInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

WCHAR*
DmiManagementDeviceType(BYTE Code)
{
    /* 7.35.1 */
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"LM75",
        L"LM78",
        L"LM79",
        L"LM80",
        L"LM81",
        L"ADM9240",
        L"DS1780",
        L"MAX1617",
        L"GL518SM",
        L"W83781D",
        L"HT82H791" /* 0x0D */
    };

    if (Code >= 0x01 && Code <= 0x0D)
        return szType[Code - 0x01];
    return L"Out Of Spec";
}

WCHAR*
DmiManagementDeviceAddressType(BYTE Code)
{
    /* 7.35.2 */
    WCHAR *szType[] = {
        L"Other", /* 0x01 */
        L"Unknown",
        L"I/O Port",
        L"Memory",
        L"SMBus" /* 0x05 */
    };

    if (Code >= 0x01 && Code <= 0x05)
        return szType[Code - 0x01];
    return L"Out Of Spec";
}

VOID CALLBACK
ManagDevInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x0B) return;

    /* Description */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(0, 0, IDS_DMI_MD_DESC);
        IoSetItemText(szText);
    }

    /* Type */
    IoAddValueName(0, 0, IDS_DMI_MD_TYPE);
    IoSetItemText(DmiManagementDeviceType(pBuf[0x05]));

    /* Address */
    IoAddValueName(0, 0, IDS_DMI_MD_ADDRESS);
    IoSetItemText(L"0x%08X", DWORD(pBuf + 0x06));

    /* Address Type */
    IoAddValueName(0, 0, IDS_DMI_MD_ADDRESS_TYPE);
    IoSetItemText(DmiManagementDeviceAddressType(pBuf[0x0A]));
}

VOID
DMI_ManagDevInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(34, ManagDevInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID CALLBACK
ManagDevComponentInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    WCHAR szText[MAX_STR_LEN];

    if (Length < 0x0B) return;

    /* Description */
    if (GetDmiString(pBuf, pBuf[0x04], Length, szText, sizeof(szText)))
    {
        IoAddValueName(0, 0, IDS_DMI_MDC_DESC);
        IoSetItemText(szText);
    }

    /* Management Device Handle */
    IoAddValueName(0, 0, IDS_DMI_MDC_MD_HANDLE);
    IoSetItemText(L"0x%04X", WORD(pBuf + 0x05));

    /* Component Handle */
    IoAddValueName(0, 0, IDS_DMI_MDC_COMPONENT_HANDLE);
    IoSetItemText(L"0x%04X", WORD(pBuf + 0x07));

    /* Threshold Handle */
    if (WORD(pBuf + 0x09) != 0xFFFF)
    {
        IoAddValueName(0, 0, IDS_DMI_MDC_THRESHOLD_HANDLE);
        IoSetItemText(L"0x%04X", WORD(pBuf + 0x09));
    }
}

VOID
DMI_ManagDevComponentInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(35, ManagDevComponentInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

VOID CALLBACK
ManagDevThresholdInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x10) return;

    /* Lower Non-critical Threshold */
    if (WORD(pBuf + 0x04) != 0x8000)
    {
        IoAddValueName(0, 0, IDS_DMI_MDTD_LOWER_NON_CRITICAL);
        IoSetItemText(L"%u", WORD(pBuf + 0x04));
    }

    /* Upper Non-critical Threshold */
    if (WORD(pBuf + 0x06) != 0x8000)
    {
        IoAddValueName(0, 0, IDS_DMI_MDTD_UPPER_NON_CRITICAL);
        IoSetItemText(L"%u", WORD(pBuf + 0x06));
    }

    /* Lower Critical Threshold */
    if (WORD(pBuf + 0x08) != 0x8000)
    {
        IoAddValueName(0, 0, IDS_DMI_MDTD_LOWER_CRITICAL);
        IoSetItemText(L"%u", WORD(pBuf + 0x08));
    }

    /* Upper Critical Threshold */
    if (WORD(pBuf + 0x0A) != 0x8000)
    {
        IoAddValueName(0, 0, IDS_DMI_MDTD_UPPER_CRITICAL);
        IoSetItemText(L"%u", WORD(pBuf + 0x0A));
    }

    /* Lower Non-recoverable Threshold */
    if (WORD(pBuf + 0x0C) != 0x8000)
    {
        IoAddValueName(0, 0, IDS_DMI_MDTD_LOWER_NON_RECOVERABLE);
        IoSetItemText(L"%u", WORD(pBuf + 0x0C));
    }

    /* Upper Non-recoverable Threshold */
    if (WORD(pBuf + 0x0E) != 0x8000)
    {
        IoAddValueName(0, 0, IDS_DMI_MDTD_UPPER_NON_RECOVERABLE);
        IoSetItemText(L"%u", WORD(pBuf + 0x0E));
    }
}

VOID
DMI_ManagDevThresholdInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(36, ManagDevThresholdInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

WCHAR*
DmiManagementControllerHostType(BYTE Code)
{
    /* DMTF DSP0239 (MCTP) version 1.1.0 */
    WCHAR *szType[] = {
        L"KCS: Keyboard Controller Style", /* 0x02 */
        L"8250 UART Register Compatible",
        L"16450 UART Register Compatible",
        L"16550/16550A UART Register Compatible",
        L"16650/16650A UART Register Compatible",
        L"16750/16750A UART Register Compatible",
        L"16850/16850A UART Register Compatible" /* 0x08 */
    };

    if (Code >= 0x02 && Code <= 0x08)
        return szType[Code - 0x02];
    if (Code == 0xF0)
        return L"OEM";
    return L"Out Of Spec";
}

VOID CALLBACK
ManagCtrlHostInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x05) return;

    /* Interface Type */
    IoAddValueName(0, 0, IDS_DMI_MCHI_INTERFACE_TYPE);
    IoSetItemText(DmiManagementControllerHostType(pBuf[0x04]));

    if (Length < 0x09) return;

    /* Vendor ID */
    if (pBuf[0x04] == 0xF0)
    {
        IoAddValueName(0, 0, IDS_DMI_MCHI_VENDOR_ID);
        IoSetItemText(L"0x%02X%02X%02X%02X",
                      pBuf[0x05], pBuf[0x06],
                      pBuf[0x07], pBuf[0x08]);
    }
}

VOID
DMI_ManagCtrlHostInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(42, ManagCtrlHostInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}

WCHAR*
DmiIpmiInterfaceType(BYTE Code)
{
    /* 7.39.1 and IPMI 2.0, appendix C1, table C1-2 */
    WCHAR *szType[] = {
        L"Unknown", /* 0x00 */
        L"KCS (Keyboard Control Style)",
        L"SMIC (Server Management Interface Chip)",
        L"BT (Block Transfer)",
        L"SSIF (SMBus System Interface)" /* 0x04 */
    };

    if (Code <= 0x04)
        return szType[Code];
    return L"Out Of Spec";
}

VOID
DmiIpmiBaseAddress(BYTE Type, BYTE *p, BYTE lsb)
{
    IoAddValueName(0, 0, IDS_DMI_IPMI_BASEADDRESS);

    if (Type == 0x04) /* SSIF */
    {
        IoSetItemText(L"0x%02X (SMBus)", (*p) >> 1);
    }
    else
    {
        DWORD high = GetBitsDWORD64(DWORD64(p), 16, 31);
        DWORD low = GetBitsDWORD64(DWORD64(p), 0, 15);

        IoSetItemText(L"0x%08X%08X (%s)", high, (low & ~1) | lsb,
            low & 1 ? L"I/O" : L"Memory-mapped");
    }
}

/* code is assumed to be a 2-bit value */
WCHAR*
DmiIpmiRegisterSpacing(BYTE Code)
{
    /* IPMI 2.0, appendix C1, table C1-1 */
    WCHAR *szSpacing[] = {
        L"Successive Byte Boundaries", /* 0x00 */
        L"32-bit Boundaries",
        L"16-byte Boundaries", /* 0x02 */
        L"Out Of Spec" /* 0x03 */
    };

    return szSpacing[Code];
}

VOID CALLBACK
IPMIDeviceInfoEnumProc(BYTE *pBuf, BYTE Length)
{
    if (Length < 0x10) return;

    /* Interface Type */
    IoAddValueName(0, 0, IDS_DMI_IPMI_INTERFACE_TYPE);
    IoSetItemText(DmiIpmiInterfaceType(pBuf[0x04]));

    /* Specification Version */
    IoAddValueName(0, 0, IDS_DMI_IPMI_SPEC_VERSION);
    IoSetItemText(L"%u.%u", pBuf[0x05] >> 4, pBuf[0x05] & 0x0F);

    /* I2C Slave Address */
    IoAddValueName(0, 0, IDS_DMI_IPMI_I2C_SLAVE_ADDRESS);
    IoSetItemText(L"0x%02x", pBuf[0x06] >> 1);

    /* NV Storage Device Address */
    if (pBuf[0x07] != 0xFF)
    {
        IoAddValueName(0, 0, IDS_DMI_IPMI_NV_STORAGE_ADDRESS);
        IoSetItemText(L"%u", pBuf[0x07]);
    }

    /* Base Address */
    DmiIpmiBaseAddress(pBuf[0x04], pBuf + 0x08,
        Length < 0x11 ? 0 : (pBuf[0x10] >> 4) & 1);

    if (Length < 0x12) return;

    if (pBuf[0x04] != 0x04)
    {
        /* Register Spacing */
        IoAddValueName(0, 0, IDS_DMI_IPMI_REGISTER_SPACING);
        IoSetItemText(DmiIpmiRegisterSpacing(pBuf[0x10] >> 6));

        if (pBuf[0x10] & (1 << 3))
        {
            /* Interrupt Polarity */
            IoAddValueName(0, 0, IDS_DMI_IPMI_INTERRUPT_POLARITY);
            IoSetItemText(pBuf[0x10] & (1 << 1) ? L"Active High" : L"Active Low");

            /* Interrupt Trigger Mode */
            IoAddValueName(0, 0, IDS_DMI_IPMI_INTERRUPT_TRIGGER_MODE);
            IoSetItemText(pBuf[0x10] & (1 << 0) ? L"Level" : L"Edge");
        }
    }

    /* Interrupt Number */
    if (pBuf[0x11] != 0x00)
    {
        IoAddValueName(0, 0, IDS_DMI_IPMI_INTERRUPT_NUMBER);
        IoSetItemText(L"%x", pBuf[0x11]);
    }
}

VOID
DMI_IPMIDeviceInfo(VOID)
{
    DebugStartReceiving();

    if (!IsSmBiosWorks()) return;
    IoAddIcon(IDI_HW);

    if (EnumDMITablesByType(38, IPMIDeviceInfoEnumProc))
        AddDMIFooter();

    DebugEndReceiving();
}
