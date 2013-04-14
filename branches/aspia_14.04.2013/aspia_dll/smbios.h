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

/* BIOS Information */
typedef VOID (CALLBACK *SMBIOS_TABLE_ENUMPROC)(BYTE *pBuf, BYTE Length);

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
