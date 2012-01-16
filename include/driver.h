/*
 * PROJECT:         Aspia (Driver Helper DLL)
 * FILE:            include/driver.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include <windows.h>
#include <batclass.h>
#include <commctrl.h>

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
        return 0xFF;
}

__inline WORD
ReadPciConfigWord(IN DWORD PciAddress, IN BYTE RegAddress)
{
    WORD Value;
    if (ReadPciConfig(PciAddress, RegAddress, (PBYTE)&Value, sizeof(WORD)))
        return Value;
    else
        return 0xFFFF;
}

__inline DWORD
ReadPciConfigDword(IN DWORD PciAddress, IN BYTE RegAddress)
{
    DWORD Value;
    if (ReadPciConfig(PciAddress, RegAddress, (PBYTE)&Value, sizeof(DWORD)))
        return Value;
    else
        return 0xFFFFFFFF;
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
typedef struct 
{
    WORD wGenConfig;
    WORD wNumCyls;
    WORD wReserved;
    WORD wNumHeads;
    WORD wBytesPerTrack;
    WORD wBytesPerSector;
    WORD wSectorsPerTrack;
    WORD wVendorUnique[3];
    BYTE sSerialNumber[20];
    WORD wBufferType;
    WORD wBufferSize;
    WORD wECCSize;
    BYTE sFirmwareRev[8];
    BYTE sModelNumber[39];
    WORD wMoreVendorUnique;
    WORD wDoubleWordIO;
    WORD wCapabilities;
    WORD wReserved1;
    WORD wPIOTiming;
    WORD wDMATiming;
    WORD wBS;
    WORD wNumCurrentCyls;
    WORD wNumCurrentHeads;
    WORD wNumCurrentSectorsPerTrack;
    WORD ulCurrentSectorCapacity;
    WORD wMultSectorStuff;
    DWORD ulTotalAddressableSectors;
    WORD wSingleWordDMA;
    WORD wMultiWordDMA;
    BYTE bReserved[127];
} IDSECTOR;

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
DWORD GetSmartTemperature(HANDLE hSmartHandle, BYTE bDevNumber);
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
