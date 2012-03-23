/*
 * PROJECT:         Aspia
 * FILE:            include/helper.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */


#pragma once

#include <batclass.h>

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
typedef BOOL (CALLBACK *SMART_ENUMDATAPROC)(SMART_RESULT *Result);
BOOL EnumSmartData(HANDLE hSmartHandle, BYTE bDevNumber, SMART_ENUMDATAPROC lpEnumProc);
INT GetSmartTemperature(HANDLE hSmartHandle, BYTE bDevNumber);
BOOL GetSmartDiskGeometry(BYTE bDevNumber, DISK_GEOMETRY *DiskGeometry);

/* SCSI Structures */
typedef struct _SRB_IO_CONTROL
{
    ULONG HeaderLength;
    UCHAR Signature[8];
    ULONG Timeout;
    ULONG ControlCode;
    ULONG ReturnCode;
    ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

/* SCSI Functions */
HANDLE OpenScsi(BYTE bDevNumber);
BOOL CloseScsi(HANDLE hHandle);
BOOL ReadScsiInfo(HANDLE hHandle, BYTE bDevNumber, IDSECTOR *Info);

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
#define DebugTrace(_msg, ...) WriteDebugLog(__FILE__, __LINE__, __FUNCTION__, _msg, ##__VA_ARGS__)
#define DebugStartReceiving() DebugTrace(L"Start data receiving")
#define DebugEndReceiving() DebugTrace(L"End data receiving")
#define DebugAllocFailed() DebugTrace(L"Alloc() failed")

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
