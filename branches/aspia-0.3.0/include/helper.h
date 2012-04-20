/*
 * PROJECT:         Aspia
 * FILE:            include/helper.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */


#pragma once

#include <batclass.h>
#include <ntddscsi.h>

/* PCI Ports */
#define CONFIG_DATA    0xCFC
#define CONFIG_ADDRESS 0xCF8

#define IA32_THERM_STATUS        0x019c
#define IA32_TEMPERATURE_TARGET  0x01A2
#define IA32_PERF_STATUS         0x0198

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

/* SPD contents size */
#define SPD_MAX_SIZE 0x95

BOOL LoadDriver(VOID);
BOOL UnloadDriver(VOID);

PVOID GetSmbiosData(OUT DWORD* ReturnSize);

typedef VOID (CALLBACK *SMBUS_BASEADR_ENUMPROC)(WORD BaseAddress, DWORD ChipType);
VOID EnumSmBusBaseAddress(SMBUS_BASEADR_ENUMPROC lpEnumProc);
BOOL ReadSpdData(WORD BaseAddress, DWORD ChipType, BYTE Slot, BYTE *SpdData);

BOOL ReadMsr(IN UINT32 Register, IN UINT32 CpuIndex, OUT UINT64* Data);

WORD ReadIoPortWord(IN DWORD Port);
DWORD ReadIoPortDword(IN DWORD Port);
BYTE ReadIoPortByte(IN DWORD Port);

BOOL WriteIoPortWord(IN DWORD Port, IN WORD Value);
BOOL WriteIoPortDword(IN DWORD Port, IN DWORD Value);
BOOL WriteIoPortByte(IN DWORD Port, IN BYTE Value);

DWORD GetRegisterDataDword(IN DWORD Register, IN INT Offset);
WORD GetRegisterDataWord(IN DWORD Register, IN INT Offset);

BOOL ReadPciConfig(IN DWORD PciAddress, IN DWORD RegAddress, OUT PBYTE Value, IN DWORD Size);
BOOL WritePciConfig(IN DWORD PciAddress, IN DWORD RegAddress, IN PBYTE Value, IN DWORD Size);

BOOL ReadPmc(IN DWORD Index, OUT PDWORD eax, OUT PDWORD edx);

__inline BYTE
ReadPciConfigByte(IN DWORD PciAddress, IN BYTE RegAddress)
{
    BYTE Value;
    if (ReadPciConfig(PciAddress, RegAddress, (PBYTE)&Value, sizeof(BYTE)))
        return Value;
    else
        return (BYTE)-1;
}

__inline WORD
ReadPciConfigWord(IN DWORD PciAddress, IN BYTE RegAddress)
{
    WORD Value;
    if (ReadPciConfig(PciAddress, RegAddress, (PBYTE)&Value, sizeof(WORD)))
        return Value;
    else
        return (WORD)-1;
}

__inline DWORD
ReadPciConfigDword(IN DWORD PciAddress, IN BYTE RegAddress)
{
    DWORD Value;
    if (ReadPciConfig(PciAddress, RegAddress, (PBYTE)&Value, sizeof(DWORD)))
        return Value;
    else
        return (DWORD)-1;
}

__inline VOID
WritePciConfigByte(IN DWORD PciAddress, IN BYTE RegAddress, IN BYTE Value)
{
    WritePciConfig(PciAddress, RegAddress, (PBYTE)&Value , sizeof(BYTE));
}

__inline VOID
WritePciConfigWord(IN DWORD PciAddress, IN BYTE RegAddress, IN WORD Value)
{
    WritePciConfig(PciAddress, RegAddress, (PBYTE)&Value , sizeof(WORD));
}

__inline VOID
WritePciConfigDword(IN DWORD PciAddress, IN BYTE RegAddress, IN DWORD Value)
{
    WritePciConfig(PciAddress, RegAddress, (PBYTE)&Value , sizeof(DWORD));
}

/* SMART Defines */
#define INDEX_ATTRIB_INDEX       0
#define INDEX_ATTRIB_UNKNOWN1    1
#define INDEX_ATTRIB_UNKNOWN2    2
#define INDEX_ATTRIB_VALUE       3
#define INDEX_ATTRIB_WORST       4
#define INDEX_ATTRIB_RAW         5

/* SMART structures */
#pragma pack (push, id_data, 1)
typedef struct _IDSECTOR
{
    WORD wGenConfig;                   /* 0 */
    WORD wNumCyls;                     /* 1 */
    WORD wSpecConfig;                  /* 2 */
    WORD wNumHeads;                    /* 3 */
    WORD wUnused0[6];                  /* 4-9 */
    CHAR sSerialNumber[20];            /* 10-19 */
    WORD wBufferType;                  /* 20 */
    WORD wBufferSize;                  /* 21 */
    WORD wECCSize;                     /* 22 */
    CHAR sFirmwareRev[8];              /* 23-26 */
    CHAR sModelNumber[40];             /* 27-46 */
    WORD wUnused1;                     /* 47 */
    WORD wUnused2;                     /* 48 */
    WORD wCapabilities1;               /* 49 */
    WORD wCapabilities2;               /* 50 */
    DWORD wUnused3;                    /* 51, 52 */
    WORD wBS;                          /* 53 */
    WORD wUnused4[5];                  /* 54-58 */
    WORD wMultSectorStuff;             /* 59 */
    DWORD dwTotalAddressableSectors;   /* 60-61 */
    WORD wUnused5;                     /* 62 */
    WORD wMultiWordDMA;                /* 63 */
    WORD wPIOMode;                     /* 64 */
    WORD wUnused6;                     /* 65 */
    WORD wUnused7;                     /* 66 */
    WORD wUnused8;                     /* 67 */
    WORD wUnused9;                     /* 68 */
    WORD wUnused10[6];                 /* 69-74 */
    WORD wUnused11;                    /* 75 */
    WORD wSATACapabilities;            /* 76 */
    WORD wUnused12;                    /* 77 */
    WORD wSATAFeaturesSuported;        /* 78 */
    WORD wSATAFeaturesEnabled;         /* 79 */
    WORD wMajorVersion;                /* 80 */
    WORD wMinorVersion;                /* 81 */
    WORD wCommandSetSupport1;          /* 82 */
    WORD wCommandSetSupport2;          /* 83 */
    WORD wCommandSetSupport3;          /* 84 */
    WORD wCommandSetEnabled1;          /* 85 */
    WORD wCommandSetEnabled2;          /* 86 */
    WORD wCommandSetDefault;           /* 87 */
    WORD wUltraDMAMode;                /* 88 */
    WORD wUnused21;                    /* 89 */
    WORD wUnused22;                    /* 90 */
    WORD wUnused23;                    /* 91 */
    WORD wUnused24;                    /* 92 */
    WORD wUnused25;                    /* 93 */
    WORD wUnused26;                    /* 94 */
    WORD wUnused27;                    /* 95 */
    WORD wUnused28;                    /* 96 */
    WORD wUnused29;                    /* 97 */
    DWORD dwUnused30;                  /* 98, 99 */
    ULONGLONG ullUnused31;             /* 100-103 */
    WORD wUnused32;                    /* 104 */
    WORD wUnused33;                    /* 105 */
    WORD wUnused34;                    /* 106 */
    WORD wUnused35;                    /* 107 */
    WORD wUnused36;                    /* 108 */
    WORD wUnused37;                    /* 109 */
    WORD wUnused38;                    /* 110 */
    WORD wUnused39;                    /* 111 */
    WORD wUnused40[4];                 /* 112-115 */
    WORD wUnused41;                    /* 116 */
    DWORD dwUnused42;                  /* 117, 118 */
    WORD wUnused43[8];                 /* 119-126 */
    WORD wUnused44;                    /* 127 */
    WORD wUnused45;                    /* 128 */
    WORD wUnused46[31];                /* 129-159 */
    WORD wUnused47;                    /* 160 */
    WORD wUnused48[7];                 /* 161-167 */
    WORD wUnused49;                    /* 168 */
    WORD wDataSetManagement;           /* 169 */
    WORD wUnused51[4];                 /* 170-173 */
    WORD wUnused52[2];                 /* 174, 175 */
    CHAR bUnused53[60];                /* 176-205 */
    WORD wUnused54;                    /* 206 */
    WORD wUnused55[2];                 /* 207, 208 */
    WORD wUnused56;                    /* 209 */
    DWORD dwUnused57;                  /* 210, 211 */
    DWORD dwUnused58;                  /* 212, 213 */
    WORD wNvCacheCapabilities;         /* 214 */
    DWORD dwNvCacheLogicalBlocksSize;  /* 215, 216 */
    WORD wRotationRate;                /* 217 */
    WORD wUnused60;                    /* 218 */
    WORD wNvCacheOptions1;             /* 219 */
    WORD wNvCacheOptions2;             /* 220 */
    WORD wUnused61;                    /* 221 */
    WORD wUnused62;                    /* 222 */
    WORD wUnused63;                    /* 223 */
    WORD wUnused64[10];                /* 224-233 */
    WORD wUnused65;                    /* 234 */
    WORD wUnused68;                    /* 235 */
    WORD wUnused66[19];                /* 236-254 */
    WORD wUnused67;                    /* 255 */

} IDSECTOR, *PIDSECTOR;
#pragma pack (pop, id_data)

typedef struct
{
    BYTE m_ucAttribIndex;
    DWORD m_dwAttribValue;
    BYTE m_ucValue;
    BYTE m_ucWorst;
    DWORD m_dwThreshold;
    BYTE bAttribId;
} SMART_INFO;

typedef struct
{
    IDSECTOR m_stInfo;
    SMART_INFO m_stSmartInfo[256];
    BYTE m_ucSmartValues;
    BYTE m_ucDriveIndex;
} SMART_DRIVE_INFO;

typedef struct
{
    DWORD dwAttrID;
    DWORD dwAttrValue;
    DWORD dwWarrantyThreshold;
    DWORD dwWorstValue;
    WCHAR szName[256];
    BYTE bValue;
    BOOL IsCritical;
} SMART_RESULT;

typedef struct
{
    BYTE bDriverError;
    BYTE bIDEStatus;
    BYTE bReserved[2];
    DWORD dwReserved[2];
} SMART_DRIVERSTAT;

typedef struct
{
    DWORD cBufferSize;
    SMART_DRIVERSTAT DriverStatus;
    BYTE bBuffer[1];
} SMART_ATAOUTPARAM;

typedef struct
{
    SENDCMDOUTPARAMS SendCmdOutParam;
    BYTE Data[IDENTIFY_BUFFER_SIZE - 1];
} READ_IDENTIFY_DATA_OUTDATA, *PREAD_IDENTIFY_DATA_OUTDATA;

typedef struct
{
    SENDCMDOUTPARAMS SendCmdOutParam;
    BYTE Data[READ_ATTRIBUTE_BUFFER_SIZE - 1];
} SMART_READ_DATA_OUTDATA, *PSMART_READ_DATA_OUTDATA;

typedef struct
{
    BYTE Id;
    BYTE Value;
    BYTE Reserved[10];
} SMART_THRESHOLD;

/* SMART Functions */
HANDLE OpenSmart(BYTE bDevNumber);
BOOL CloseSmart(HANDLE hHandle);
BOOL GetSmartVersion(HANDLE hHandle, GETVERSIONINPARAMS *pVersion);
BOOL EnableSmart(HANDLE hHandle, BYTE bDevNumber);
BOOL ReadSmartInfo(HANDLE hHandle, BYTE bDevNumber, IDSECTOR *Info);
BOOL ReadSmartAttributes(HANDLE hHandle, BYTE bDevNumber, SMART_DRIVE_INFO *Info);
BOOL ReadSmartThresholds(HANDLE hHandle, BYTE bDriveNum, SMART_DRIVE_INFO *Info);
typedef VOID (CALLBACK *SMART_ENUMDATAPROC)(SMART_RESULT *Result);
BOOL EnumSmartData(HANDLE hSmartHandle, BYTE bDevNumber, SMART_ENUMDATAPROC lpEnumProc);
INT GetSmartTemperature(HANDLE hSmartHandle, BYTE bDevNumber);
BOOL GetSmartDiskGeometry(BYTE bDevNumber, DISK_GEOMETRY *DiskGeometry);

/* scsioverata.c */
BOOL ScsiOverAtaEnumSmartData(HANDLE hSmartHandle, BYTE bDevNumber, SMART_ENUMDATAPROC lpEnumProc);
BOOL ScsiOverAtaReadSmartThresholds(HANDLE hHandle, BYTE bDriveNum, SMART_DRIVE_INFO *Info);
BOOL ScsiOverAtaReadSmartAttributes(HANDLE hHandle, BYTE bDevNumber, SMART_DRIVE_INFO *Info);
BOOL ScsiOverAtaEnableSmart(HANDLE hHandle);
BOOL ScsiOverAtaReadSmartInfo(HANDLE hHandle, BYTE bDevIndex, IDSECTOR *Info);

/* SCSI Defines */
#define SPT_SENSEBUFFER_LENGTH 32
#define SPT_DATABUFFER_LENGTH 512

#define FEATURE_PROFILE_LIST                     0x0   /* A list of all Profiles supported by the Drive */
#define FEATURE_CORE                             0x1   /* Mandatory behavior for all devices */
#define FEATURE_MORPHING                         0x2   /* The Drive is able to report operational changes to the Host and accept
                                                          Host requests to prevent operational changes. */
#define FEATURE_REMOVABLE                        0x3   /* The medium may be removed from the device */
#define FEATURE_WRITE_PROTECT                    0x4   /* The ability to control Write Protection status */
/* 0x5 - 0xF - Reserved */
#define FEATURE_RANDOM_READABLE                  0x10  /* The ability to read sectors with random addressing */
/* 0x11 - 0x1C - Reserved */
#define FEATURE_MULTI_READ                       0x1D  /* The Drive is able to read all CD media types; based on OSTA
                                                          MultiRead */
#define FEATURE_CD_READ                          0x1E  /* The ability to read CD specific structures */
#define FEATURE_DVD_READ                         0x1F  /* The ability to read DVD specific structures */
#define FEATURE_RANDOM_WRITABLE                  0x20  /* Write support for randomly addressed writes */
#define FEATURE_INCREMENTAL_STREAMING_WRITABLE   0x21  /* Write support for sequential recording */
#define FEATURE_FORMATTABLE                      0x23  /* Support for formatting of media. */
#define FEATURE_HARDWARE_DEFECT_MANAGEMENT       0x24  /* Ability of the Drive/media system to provide an apparently defect-free
                                                          space. */
#define FEATURE_WRITE_ONCE                       0x25  /* Write support for write-once media that is writable in random order. */
#define FEATURE_RESTRICTED_OVERWRITE             0x26  /* Write support for media that shall be written from Blocking boundaries
                                                          only. */
#define FEATURE_CDW_DASHR_CAV_WRITE              0x27  /* The ability to write high speed CD-RW media */
#define FEATURE_MRW                              0x28  /* The ability to recognize and read and optionally write MRW formatted 
                                                          media */
#define FEATURE_ENHANCED_DEFECT_REPORTING        0x29  /* The ability to control RECOVERED ERROR reporting */
#define FEATURE_DVD_PLUSRW                       0x2A  /* The ability to recognize, read and optionally write DVD+RW media */
#define FEATURE_DVD_PLUSR                        0x2B  /* The ability to read DVD+R recorded media formats */
#define FEATURE_RIGID_RESTRICTED_OVERWRITE       0x2C  /* Write support for media that is required to be written from Blocking
                                                          boundaries with length of integral multiple of Blocking size only. */
#define FEATURE_CD_TRACK_AT_ONCE                 0x2D  /* Ability to write CD with Track at Once recording */
#define FEATURE_CD_MASTERING                     0x2E  /* The ability to write CD with Session at Once or Raw write methods. */
#define FEATURE_DVD_DASHR_DASHRW_WRITE           0x2F  /* The ability to write DVD specific structures */
#define FEATURE_LJ_RECORDING                     0x33  /* The ability to record in layer jump mode */
#define FEATURE_LJ_RIGID_RESTRICTED_OVERWRITE    0x34  /* The ability to perform Layer Jump recording on Rigid Restricted
                                                          Overwritable media  */
#define FEATURE_STOP_LONG_OPERATION              0x35  /* The ability to stop the long immediate operation by a command. */
/* 0x36 - Reserved */
#define FEATURE_CD_DASHRW_MEDIA_WRITE_SUPPORT    0x37  /* The ability to report CD –RW media sub-types that are supported for
                                                          write  */
#define FEATURE_BD_DASHR_POW                     0x38  /* Logical Block overwrite service on BD-R discs formatted as
                                                          SRM+POW.  */
/* 0x39 - Reserved */
#define FEATURE_DVD_PLUSRW_DL                    0x3A  /* The ability to read DVD+RW Dual Layer recorded media formats */
#define FEATURE_DVD_PLUSR_DL                     0x3B  /* The ability to read DVD+R Dual Layer recorded media formats */
/* 0x3C - 0x3F - Reserved */
#define FEATURE_BD_READ                          0x40  /* The ability to read control structures and user data from a BD disc */
#define FEATURE_BD_WRITE                         0x41  /* The ability to write control structures and user data to certain BD discs */
#define FEATURE_TSR                              0x42  /* Timely, Safe Recording permits the Host to schedule defect 
                                                          management. */
/* 0x43 - 0x4F - Reserved */
#define FEATURE_HD_DVD_READ                      0x50  /* The ability to read control structures and user data from a HD DVD disc */
#define FEATURE_HD_DVD_WRITE                     0x51  /* The ability to write control structures and user data to certain HD DVD
                                                          discs */
#define FEATURE_HD_DVD_DASHRW_FRAGMENT_RECORDING 0x52  /* The ability to record HD DVD-RW in fragment recording mode */
/* 0x53 - 0x7F - Reserved */
#define FEATURE_HYBRID_DISC                      0x80  /* The ability to access some Hybrid Discs.  */
/* 0x81 - 0xFF - Reserved */
#define FEATURE_POWER_MANAGEMENT                 0x100 /* Host and device directed power management  */
#define FEATURE_SMART                            0x101 /* Ability to perform Self Monitoring Analysis and Reporting Technology */
#define FEATURE_EMBEDDED_CHANGER                 0x102 /* Single mechanism multiple disc changer */
#define FEATURE_MICROCODE_UPGRADE                0x104 /* Ability for the device to accept new microcode via the interface */
#define FEATURE_TIMEOUT                          0x105 /* Ability to respond to all commands within a specific time */
#define FEATURE_DVD_CSS                          0x106 /* Ability to perform DVD CSS/CPPM authentication and RPC */
#define FEATURE_REAL_TIME_STREAMING              0x107 /* Ability to read and write using Host requested performance parameters */
#define FEATURE_DRIVE_SERIAL_NUMBER              0x108 /* The Drive has a unique identifier */
#define FEATURE_DCBS                             0x10A /* The ability to read and/or write DCBs  */
#define FEATURE_DVD_CPRM                         0x10B /* The Drive supports DVD CPRM authentication  */
#define FEATURE_FIRMWARE_INFORMATION             0x10C /* Firmware creation date report */
#define FEATURE_AACS                             0x10D /* The ability to decode and optionally encode AACS protected
                                                          information */
#define FEATURE_DVD_CSS_MANAGED_RECORDING        0x10E /* The ability to perform DVD CSS managed recording */
/* 0x10F - Reserved */
#define FEATURE_VCPS                             0x110 /* The ability to decode and optionally encode VCPS protected
                                                          information */
/* 0x111 - 0x112 - Reserved */
#define FEATURE_SECURDISK                        0x113 /* The ability to encode and decode SecurDisc protected information */
/* 0x114 - 0x142 - Reserved */
#define FEATURE_OSSC                             0x142 /* TCG Optical Security Subsystem Class Feature */
/* 0x143 - 0xFEFF - Reserved */
/* 0xFF00 - 0xFFFF - Vendor Specific */

#define PROFILE_NO_ACTIVE_PROFILE 0x0    // no active profile
#define PROFILE_REMOVABLE_DISK    0x2    // re-writable; with removable media
#define PROFILE_CD_ROM            0x8    // Read-Only Compact Disc
#define PROFILE_CD_DASHR          0x9    // Write Once Compact Disc
#define PROFILE_CD_DASHRW         0xA    // Re-writable compact Disc
#define PROFILE_DVD_ROM           0x10   // DVD-ROM
#define PROFILE_DVD_DASHR_SEQ     0x11   // Write once DVD using Sequential Recording
#define PROFILE_DVD_RAM           0x12   // Rewritable DVD
#define PROFILE_DVD_DASHRW_RES    0x13   // Re-writable DVD using Restricted Overwrite
#define PROFILE_DVD_DASHRW_SEQ    0x14   // Re-writable DVD using Sequential Recording
#define PROFILE_DVD_DASHR_DL_SEQ  0x15   // Dual Layer DVD-R using Sequential Recording
#define PROFILE_DVD_DASHR_DL_JUMP 0x16   // Dual Layer DVD-R using Layer Jump Recording
#define PROFILE_DVD_DASHRW_DL     0x17   // Dual Layer DVD-RW
#define PROFILE_DVD_DDR           0x18   // Write Once DVD Download Disc Recording for CSS managed recording
#define PROFILE_DVD_PLUSRW        0x1A   // DVD+ReWritable
#define PROFILE_DVD_PLUSR         0x1B   // DVD+Recordable
#define PROFILE_DVD_PLUSRW_DL     0x2A   // DVD+ReWritable Dual Layer
#define PROFILE_DVD_PLUSR_DL      0x2B   // DVD+Recordable Dual Layer
#define PROFILE_BD_ROM            0x40   // Blu-ray Disc ROM
#define PROFILE_BD_R_SRM          0x41   // Blu-ray Disc Recordable - Sequential Recording Mode
#define PROFILE_BD_R_RRM          0x42   // Blu-ray Disc Recordable - Random Recording Mode
#define PROFILE_BD_RE             0x43   // Blu-ray Disc Rewritable
#define PROFILE_HD_DVD_ROM        0x50   // Read-only HD DVD
#define PROFILE_HD_DVD_R          0x51   // Write-once HD DVD
#define PROFILE_HD_DVD_RAM        0x52   // Rewritable HD DVD
#define PROFILE_HD_DVD_RW         0x53   // Rewritable HD DVD
#define PROFILE_HD_DVD_R_DL       0x58   // Dual Layer Write-once HD-DVD
#define PROFILE_HD_DVD_RW_DL      0x5A   // Dual Layer Rewritable HD-DVD
#define PROFILE_NOT_CONFORMING    0xFFFF // Does not conform to any profile

/* SCSI Commands */
#define SCSI_FORMAT_UNIT                   0x04
#define SCSI_INQUIRY                       0x12
#define SCSI_START_STOP_UNIT               0x1B
#define SCSI_READ_FORMAT_CAPACITIES        0x23
#define SCSI_READ_CAPACITY                 0x25
#define SCSI_READ                          0x28
#define SCSI_WRITE                         0x2A
#define SCSI_SEEK                          0x2B
#define SCSI_VERIFY                        0x2F
#define SCSI_WRITE_AND_VERIFY              0x2E
#define SCSI_SYNCHRONIZE_CACHE             0x35
#define SCSI_GET_CONFIGURATION             0x46
#define SCSI_GET_EVENT_STATUS_NOTIFICATION 0x4A
#define SCSI_READ_DISK_INFORMATION         0x51
#define SCSI_READ_TRACK_INFORMATION        0x52
#define SCSI_RESERVE_TRACK                 0x53
#define SCSI_SEND_OPC_INFORMATION          0x54
#define SCSI_REPAIR_TRACK                  0x58
#define SCSI_CLOSE_TRACK_SESSION           0x5B
#define SCSI_READ_BUFFER_CAPACITY          0x5C
#define SCSI_SEND_CUE_SHEET                0x5D
#define SCSI_BLANK                         0xA1
#define SCSI_SEND_KEY                      0xA3
#define SCSI_REPORT_KEY                    0xA4
#define SCSI_LOAD_UNLOAD_MEDIUM            0xA6
#define SCSI_SET_READ_AHEAD                0xA7
#define SCSI_GET_PERFORMANCE               0xAC
#define SCSI_READ_DISK_STRUCTURE           0xAD
#define SCSI_SET_STREAMING                 0xB6
#define SCSI_READ_CD_MSF                   0xB9
#define SCSI_SET_CD_SPEED                  0xBB
#define SCSI_MECHANISM_STATUS              0xBD
#define SCSI_READ_CD                       0xBE
#define SCSI_SEND_DISK_STRUCTURE           0xBF

#pragma warning(disable:4201)
#pragma warning(disable:4214)

/* SCSI Structures */
#pragma pack(push, inquiry, 1)
typedef struct
{
    UCHAR DeviceType : 5;
    UCHAR DeviceTypeQualifier : 3;
    UCHAR DeviceTypeModifier : 7;
    UCHAR RemovableMedia : 1;
    union {
        UCHAR Versions;
        struct {
            UCHAR ANSIVersion : 3;
            UCHAR ECMAVersion : 3;
            UCHAR ISOVersion : 2;
        };
    };
    UCHAR ResponseDataFormat : 4;
    UCHAR HiSupport : 1;
    UCHAR NormACA : 1;
    UCHAR TerminateTask : 1;
    UCHAR AERC : 1;
    UCHAR AdditionalLength;
    UCHAR Reserved;
    UCHAR Addr16 : 1;
    UCHAR Addr32 : 1;
    UCHAR AckReqQ: 1;
    UCHAR MediumChanger : 1;
    UCHAR MultiPort : 1;
    UCHAR ReservedBit2 : 1;
    UCHAR EnclosureServices : 1;
    UCHAR ReservedBit3 : 1;
    UCHAR SoftReset : 1;
    UCHAR CommandQueue : 1;
    UCHAR TransferDisable : 1;
    UCHAR LinkedCommands : 1;
    UCHAR Synchronous : 1;
    UCHAR Wide16Bit : 1;
    UCHAR Wide32Bit : 1;
    UCHAR RelativeAddressing : 1;
    UCHAR VendorId[8];
    UCHAR ProductId[16];
    UCHAR ProductRevisionLevel[4];
    UCHAR VendorSpecific[20];
    UCHAR Reserved3[40];
} INQUIRYDATA, *PINQUIRYDATA;
#pragma pack(pop, inquiry)

#pragma pack(push, cd_cap, 1)
typedef struct
{
    BYTE PageCode:6;
    BYTE reserved1:1;
    BYTE PS:1;
    BYTE PageLength;
    BYTE CDR_Read:1;
    BYTE CDRW_Read:1;
    BYTE Method2:1;
    BYTE DVDROM_Read:1;
    BYTE DVDR_Read:1;
    BYTE DVDRAM_Read:1;
    BYTE reserved2:2;
    BYTE CDR_Write:1;
    BYTE CDRW_Write:1;
    BYTE Test_Write:1;
    BYTE reserved3:1;
    BYTE DVDR_Write:1;
    BYTE DVDRAM_Write:1;
    BYTE reserved4:2;
    BYTE AudioPlay:1;
    BYTE Composite:1;
    BYTE DigitalPort1:1;
    BYTE DigitalPort2:1;
    BYTE Mode2Form1:1;
    BYTE Mode2Form2:1;
    BYTE MultiSession:1;
    BYTE reserved5:1;
    BYTE CDDA_CommandsSupported:1;
    BYTE CDDAStream_is_Accurate:1;
    BYTE R_W_Supported:1;
    BYTE R_W_DeinterleavedAndCorrected:1;
    BYTE C2Pointers_are_Supported:1;
    BYTE ISRC:1;
    BYTE UPC:1;
    BYTE ReadBarCode:1;
    BYTE Lock:1;
    BYTE LockState:1;
    BYTE PreventJumper:1;
    BYTE Eject:1;
    BYTE reserved6:1;
    BYTE LoadingMechanismType:3;
    BYTE SeparateVolumeLevelsPerChannel:1;
    BYTE SeparateChannelMuteSupported:1;
    BYTE ChangerSupportsDiscPresentReporting:1;
    BYTE S_W_Slot_Selection_SSS:1;
    BYTE SideChangeCapable:1;
    BYTE PthroughWinLeadin:1;
    BYTE reserved7:2;
    WORD MaximumReadSpeedSupported;
    WORD NumberOfVolumeLevelsSupported;
    WORD BufferSize;
    WORD CurrentreadSpeedSelected;
    BYTE reserved8;
    BYTE reserved9:1;
    BYTE BCK:1;
    BYTE RCK:1;
    BYTE LSBF:1;
    BYTE Length:2;
    BYTE reserved10:2;
    WORD MaximumWriteSpeedSupported;
    WORD CurrentWriteSpeedSelected;
} SCSI_CD_CAPABILITIES, *PSCSI_CD_CAPABILITIES;
#pragma pack(pop, cd_cap)

#pragma pack(push, get_conf_feature, 1)
typedef struct
{
    USHORT featureCode;
    UCHAR current : 1;
    UCHAR persistent : 1;
    UCHAR version : 4;
    UCHAR rsvd1 : 2;
    UCHAR additionalLength;
    UCHAR additionalData[102];
} GET_CONFIGURATION_FEATURE, *PGET_CONFIGURATION_FEATURE;
#pragma pack(pop, get_conf_feature)

#pragma pack(push, report_key, 1)
typedef struct
{
    UCHAR OperationCode;
    UCHAR Reserved1 : 5;
    UCHAR Lun : 3;
    UCHAR LogicalBlockAddress[4];
    UCHAR Reserved2[2];
    USHORT AllocationLength;
    UCHAR KeyFormat : 6;
    UCHAR AGID : 2;
    UCHAR Control;
} REPORT_KEY, *PREPORT_KEY;
#pragma pack(pop, report_key)

#pragma pack(push, report_key_data, 1)
typedef struct
{
    UCHAR Reserved1[4];
    UCHAR UserChanges: 3;
    UCHAR VendorResets: 3;
    UCHAR TypeCode: 2;
    UCHAR RegionMask;
    UCHAR RpcScheme;
    UCHAR Reserved2;
} REPORT_KEY_DATA, *PREPORT_KEY_DATA;
#pragma pack(pop, report_key_data)

enum MMC_KEY_FORMAT_CODE_CLASS0
{
    KEY_FORMAT_AGID_CSS  = 0,
    KEY_FORMAT_CHAL_KEY  = 1,
    KEY_FORMAT_KEY1  = 2,
    KEY_FORMAT_TITLE_KEY  = 4,
    KEY_FORMAT_ASF   = 5,
    KEY_FORMAT_RPC_STATE  = 8,
    KEY_FORMAT_AGID_CPRM  = 0x11,
    KEY_FORMAT_NONE  = 0x3F
};

typedef struct
{
    UCHAR dataLength[4];
    UCHAR rsvd1;
    UCHAR rsvd2;
    UCHAR currentProfile[2];
} GET_CONFIGURATION_HEADER, *PGET_CONFIGURATION_HEADER;

typedef struct
{
    GET_CONFIGURATION_HEADER Header;
    GET_CONFIGURATION_FEATURE Feature;
} SCSI_GET_CONFIG, *PSCSI_GET_CONFIG;

typedef struct
{
    SCSI_PASS_THROUGH_DIRECT spt;
    ULONG Filler;
    UCHAR SenseBuffer[SPT_SENSEBUFFER_LENGTH];
} SCSI_PASS_THROUGH_DIRECT_WBUF;

typedef struct
{
    SCSI_PASS_THROUGH spt;
    ULONG Filler;
    UCHAR SenseBuffer[SPT_SENSEBUFFER_LENGTH];
    UCHAR DataBuffer[SPT_DATABUFFER_LENGTH];
} SCSI_PASS_THROUGH_WBUF;

/* SCSI Functions */
HANDLE OpenScsi(BYTE bDevNumber);
BOOL CloseScsi(HANDLE hHandle);
BOOL ReadScsiInfo(HANDLE hHandle, BYTE bDevNumber, IDSECTOR *Info);
BOOL GetInquiryScsi(HANDLE hHandle, PINQUIRYDATA pInquiry);
HANDLE OpenScsiByDriveLetter(WCHAR letter);
BOOL GetConfinurationScsi(HANDLE hHandle, WORD wProfile, PSCSI_GET_CONFIG pConfiguration);
BOOL GetCDCapabilitiesScsi(HANDLE hHandle, PSCSI_CD_CAPABILITIES pCapabilities);
BOOL GetCDReportKeyScsi(HANDLE hHandle, PREPORT_KEY_DATA pKeyData);

/* Battery Functions */
HANDLE OpenBattery(LPWSTR lpszDevice);
BOOL CloseBattery(HANDLE hHandle);
ULONG GetBatteryTag(HANDLE hHandle);
BOOL QueryBatteryInfo(HANDLE hHandle, BATTERY_QUERY_INFORMATION_LEVEL InfoLevel, LPVOID lpBuffer, DWORD dwBufferSize);
BOOL QueryBatteryStatus(HANDLE hHandle, BATTERY_STATUS *lpBatteryStatus, DWORD dwBufferSize);

/* DEBUG Functions */
BOOL InitDebugLog(LPWSTR lpLogName, LPWSTR lpVersion);
VOID CloseDebugLog(VOID);
VOID WriteDebugLog(LPSTR lpFile, UINT iLine, LPSTR lpFunc, LPWSTR lpMsg, ...);

/* DEBUG Defines */
#ifdef _DEBUG_FUNCTIONS_
#define DebugTrace(_msg, ...) WriteDebugLog(__FILE__, __LINE__, __FUNCTION__, _msg, ##__VA_ARGS__)
#define DebugStartReceiving() DebugTrace(L"Start data receiving")
#define DebugEndReceiving() DebugTrace(L"End data receiving")
#define DebugAllocFailed() DebugTrace(L"Alloc() failed")
#else
#define DebugTrace(_msg, ...)
#define DebugStartReceiving()
#define DebugEndReceiving()
#define DebugAllocFailed()
#endif

/* Misc Functions */
INT GetSystemColorDepth(VOID);
INT GetCpuUsage(VOID);
BOOL CreateScreenshot(HWND hwnd);
BOOL CenterWindow(HWND hWnd, HWND hWndCenter);
VOID ChangeByteOrder(PCHAR pString, USHORT StrSize);
BOOL GetCurrentPath(LPWSTR lpszPath, SIZE_T PathLen);
BOOL IsWin64System(VOID);
INT SafeStrLen(LPCWSTR lpString);
LPWSTR SafeStrCpyN(LPWSTR lpString1, LPCTSTR lpString2, INT iMaxLength);
INT SafeStrCmp(LPCTSTR lpString1, LPCTSTR lpString2);
BOOL IsUserAdmin(VOID);
SIZE_T StrToHex(LPWSTR lpszStr, SIZE_T StrLen);
BOOL IsWindows2000(VOID);
BOOL GetBinaryFromRegistry(HKEY hRootKey, LPWSTR lpszPath, LPWSTR lpszKeyName, LPBYTE lpdwValue, DWORD dwSize);
BOOL GetStringFromRegistry(BOOL Is64KeyRequired, HKEY hRootKey, LPWSTR lpszPath, LPWSTR lpszKeyName, LPWSTR lpszValue, DWORD dwSize);
INT AddIconToImageList(HINSTANCE hInst, HIMAGELIST hImageList, UINT IconIndex);
BOOL KillProcess(DWORD pid, BOOL KillTree);
BOOL GetFileExt(LPWSTR lpFileName, LPWSTR lpExt, SIZE_T ExtSize);

/* NVIDIA GPU Information */
typedef struct
{
    WCHAR szName[64];

    struct
    {
        LONG CurrentTemp;
        LONG DefaultMinTemp;
        LONG DefaultMaxTemp;
    }
    GpuTemp;

    struct
    {
        LONG CurrentTemp;
        LONG DefaultMinTemp;
        LONG DefaultMaxTemp;
    }
    MemoryTemp;

    struct
    {
        LONG CurrentTemp;
        LONG DefaultMinTemp;
        LONG DefaultMaxTemp;
    }
    PowerSupplyTemp;

    struct
    {
        LONG CurrentTemp;
        LONG DefaultMinTemp;
        LONG DefaultMaxTemp;
    }
    BoardTemp;

    struct
    {
        LONG CurrentTemp;
        LONG DefaultMinTemp;
        LONG DefaultMaxTemp;
    }
    VcdBoardTemp;

    struct
    {
        LONG CurrentTemp;
        LONG DefaultMinTemp;
        LONG DefaultMaxTemp;
    }
    VcdInletTemp;

    struct
    {
        LONG CurrentTemp;
        LONG DefaultMinTemp;
        LONG DefaultMaxTemp;
    }
    VcdOutletTemp;

    LONG FanSpeed;
}
NVIDIA_GPU_INFO, *PNVIDIA_GPU_INFO;

typedef BOOL (CALLBACK *NVIDIA_GPU_ENUMPROC)(PNVIDIA_GPU_INFO GpuInfo);

BOOL InitNvidiaApi(VOID);
VOID FreeNvidiaApi(VOID);
BOOL IsNvidiaApiInitialized(VOID);
BOOL EnumNvidiaGPUs(NVIDIA_GPU_ENUMPROC lpEnumProc);

__inline INT
GetClientWindowWidth(IN HWND hwnd)
{
    RECT Rect;

    GetClientRect(hwnd, &Rect);
    return (Rect.right - Rect.left);
}

__inline INT
GetClientWindowHeight(IN HWND hwnd)
{
    RECT Rect;

    GetClientRect(hwnd, &Rect);
    return (Rect.bottom - Rect.top);
}

__inline INT
GetWindowWidth(IN HWND hwnd)
{
    RECT Rect;

    GetWindowRect(hwnd, &Rect);
    return (Rect.right - Rect.left);
}

__inline INT
GetWindowHeight(IN HWND hwnd)
{
    RECT Rect;

    GetWindowRect(hwnd, &Rect);
    return (Rect.bottom - Rect.top);
}

BOOL GetIniFilePath(OUT LPWSTR lpszPath, IN SIZE_T PathLen);
double Round(double Argument, int Precision);
VOID ChopSpaces(LPWSTR s, SIZE_T size);
VOID ConvertSecondsToString(HINSTANCE hLangInst, LONGLONG Seconds, LPWSTR lpszString, SIZE_T Size);
BOOL TimeToString(time_t Time, LPWSTR lpTimeStr, SIZE_T Size);
BOOL GetFileDescription(LPWSTR lpszPath, LPWSTR lpszDesc, SIZE_T Size);
HICON GetFolderAssocIcon(LPWSTR lpszFolder);
INT LoadMUIStringF(HINSTANCE hLangInst, UINT ResID, LPWSTR Buffer, INT BufLen);
WCHAR* EscapePercentSymbols(WCHAR *pIn);

#define LoadMUIString(a, b, c) LoadMUIStringF(DllParams.hLangInst, a, b, c)

__inline DWORD
GetBitsDWORD(DWORD val, CHAR from, CHAR to)
{
    DWORD mask = (1 << (to + 1)) - 1;

    return ((to > 30) ? (val >> from) : ((val & mask) >> from));
}

__inline WORD
GetBitsWORD(WORD val, CHAR from, CHAR to)
{
    WORD mask = (1 << (to + 1)) - 1;

    return ((to > 14) ? (val >> from) : ((val & mask) >> from));
}

__inline BYTE
GetBitsBYTE(BYTE val, CHAR from, CHAR to)
{
    BYTE mask = (1 << (to + 1)) - 1;

    return ((to > 7) ? (val >> from) : ((val & mask) >> from));
}
