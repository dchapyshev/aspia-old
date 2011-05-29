/*
 * PROJECT:         Aspia
 * FILE:            ioctl_funct.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Shevchuk Maksim (maksim.shevchuk@gmail.com)
 *                  Dmitry Chapyshev (dmitry@aspia.ru)
 */
#pragma once

#include "ioctl.h"

/* 5.2.1  SMBIOS Structure Table Entry Point */
#define SMBIOS_REGION        0xF0000 /* 0xF0000 - 0xFFFFF */
#define SMBIOS_REGION_SIZE   0x10000
#define SMBIOS_SEARCH_STEP   16

#define SMBIOS_ANCHOR        '_MS_' /* Reverse anchor */

/* Bus Number, Device Number and Function Number to PCI Device Address */
#define PciBusDevFunc(Bus, Dev, Func) ((Bus & 0xFF) << 8) | ((Dev & 0x1F) << 3) | (Func & 7)
/* PCI Device Address to Bus Number */
#define PciGetBus(a) ((a >> 8) & 0xFF)
/* PCI Device Address to Device Number */
#define PciGetDev(a) ((a >> 3) & 0x1F)
/* PCI Device Address to Function Number */
#define PciGetFunc(a) (a & 7)

typedef struct _SMBIOS_ENTRY
{
    CHAR Anchor[4];
    UCHAR EntryPointCrc;
    UCHAR EntryPointLength;
    UCHAR MajorVer;
    UCHAR MinorVer;
    USHORT MaximumStructureSize;
    UCHAR EntryPointRevision;
    CHAR FormattedArea[5];
    CHAR IntermediateAnchor[5];
    UCHAR IntermediateCrc;
    USHORT StructureTableLength;
    UINT32 StructureTableAddress;
    USHORT NumberOfSMBIOSStructures;
    UCHAR SMBIOSBCDRevision;
} SMBIOS_ENTRY, *PSMBIOS_ENTRY;


NTSTATUS
NTAPI
IOCTL_GetSMBIOS(IN PIRP, IN PIO_STACK_LOCATION);

NTSTATUS
NTAPI
IOCTL_GetMsr(IN PIRP, IN PIO_STACK_LOCATION);

NTSTATUS
NTAPI
IOCTL_GetPmc(IN PIRP, IN PIO_STACK_LOCATION);

NTSTATUS
NTAPI
IOCTL_ReadPort(IN ULONG, IN PVOID, IN ULONG, OUT PVOID, IN ULONG, OUT PULONG);

NTSTATUS
NTAPI
IOCTL_WritePort(IN ULONG, IN PVOID, IN ULONG, OUT PVOID, IN ULONG, OUT PULONG);

NTSTATUS
NTAPI
IOCTL_ReadPciConfig(IN PVOID, IN ULONG, OUT PVOID, IN ULONG, OUT PULONG);

NTSTATUS
NTAPI
IOCTL_WritePciConfig(IN PVOID, IN ULONG, OUT PVOID, IN ULONG, OUT PULONG);

NTSTATUS
NTAPI
IOCTL_Init();
