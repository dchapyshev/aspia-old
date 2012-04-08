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

#define MAX_DATA               0xFA00 /* 64K */
#define MAX_STRING             0x3E8  /* 1K */
#define MAX_CONTAINED_ELEMENTS 255
#define MAX_CONTAINED_RECORD   255

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
#define BUILDIN_POINT_DEV_INF0  0x15 /* Type 21 */
#define BATTERY_INFO            0x16 /* Type 22 */
#define VOLTAGE_INFO            0x1A /* Type 26 */
#define COOLING_INFO            0x1B /* Type 27 */
#define TEMPERATURE_INFO        0x1C /* Type 28 */
#define SYSTEM_BOOT_INFO        0x20 /* Type 32 */
#define ONBOARD_EXT_INFO        0x29 /* Type 41 */

BOOL InitSmBIOSData(VOID);

extern BOOL IsSmBIOSInitialized;

/* Memory Information */
#define MEM_SIZE_MASK    0x07FFF

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

/* Processor Information */
#define USE_LEGACY_VOLTAGE_MASK 0x80
#define LEGACY_VOLTAGE_MASK     0x7F
#define VOLTAGE_MASK            0x0F
#define _5_VOLTS                0x01
#define _3_3_VOLTS              0x02
#define _2_9_VOLTS              0x04

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

typedef VOID (CALLBACK *SMBIOS_TABLE_ENUMPROC)(BYTE *pBuf, BYTE Length);

/* System Information */
typedef struct
{
    DWORD TimeLow;
    WORD TimeMid;
    WORD TimeHiAndVersion;
    BYTE ClockSeqHiAndReserved;
    BYTE ClockLowSeq;
    BYTE Node[0x06];
} SystemUuid;

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

typedef struct
{
    BYTE Type;
    BYTE Length;
    WORD Handle;
    BYTE *Data;
} DMI_HEADER;

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

#define CPU_STATUS_MASK 0x07

BOOL
SMBIOS_GetMainboardName(LPWSTR lpName, SIZE_T NameSize,
                        LPWSTR lpManuf, SIZE_T ManufSize);
