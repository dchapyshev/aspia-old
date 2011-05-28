/*
 * PROJECT:         Aspia
 * FILE:            ioctl_funct.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Shevchuk Maksim (maksim.shevchuk@gmail.com)
 *                  Dmitry Chapyshev (dmitry@aspia.ru)
 */
#pragma once


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

typedef struct _READ_MSR_QUERY
{
    UINT32 Register;
    UINT32 CpuIndex;
} READ_MSR_QUERY, *PREAD_MSR_QUERY;

typedef struct _READ_PMC_QUERY
{
    ULONG Counter;
    UINT32 CpuIndex;
} READ_PMC_QUERY, *PREAD_PMC_QUERY;

typedef struct _PORT_WRITE_INPUT
{
    ULONG PortNumber; 
    union
    {
        ULONG LongData;
        USHORT ShortData;
        UCHAR CharData;
    }u;
} PORT_WRITE_INPUT, *PPORT_WRITE_INPUT;

typedef struct  _READ_PCI_CONFIG_INPUT
{
    ULONG PciAddress;
    ULONG PciOffset;
} READ_PCI_CONFIG_INPUT, *PREAD_PCI_CONFIG_INPUT;

typedef struct _WRITE_PCI_CONFIG_INPUT
{
    ULONG PciAddress;
    ULONG PciOffset;
    UCHAR Data[1];
} WRITE_PCI_CONFIG_INPUT, *PWRITE_PCI_CONFIG_INPUT;

typedef struct _READ_MEMORY_INPUT
{
    PHYSICAL_ADDRESS Address;
    ULONG UnitSize;
    ULONG Count;
} READ_MEMORY_INPUT, *PREAD_MEMORY_INPUT;


#define IOCTL_GET_SMBIOS        CTL_CODE(FILE_DEVICE_UNKNOWN, 1,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_GET_MSR           CTL_CODE(FILE_DEVICE_UNKNOWN, 2,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_GET_PMC           CTL_CODE(FILE_DEVICE_UNKNOWN, 3,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_PORT_DWORD   CTL_CODE(FILE_DEVICE_UNKNOWN, 4,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_PORT_WORD    CTL_CODE(FILE_DEVICE_UNKNOWN, 5,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_PORT_BYTE    CTL_CODE(FILE_DEVICE_UNKNOWN, 6,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_WRITE_PORT_DWORD  CTL_CODE(FILE_DEVICE_UNKNOWN, 7,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_WRITE_PORT_WORD   CTL_CODE(FILE_DEVICE_UNKNOWN, 8,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_WRITE_PORT_BYTE   CTL_CODE(FILE_DEVICE_UNKNOWN, 9,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_PCI_CONFIG   CTL_CODE(FILE_DEVICE_UNKNOWN, 10, METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_WRITE_PCI_CONFIG  CTL_CODE(FILE_DEVICE_UNKNOWN, 11, METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_MEMORY       CTL_CODE(FILE_DEVICE_UNKNOWN, 12, METHOD_BUFFERED, FILE_ALL_ACCESS)


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
IOCTL_ReadMemory(IN PVOID, IN ULONG, OUT PVOID, IN ULONG, OUT PULONG);

NTSTATUS
NTAPI
IOCTL_Init();
