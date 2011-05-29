/*
 * PROJECT:         Aspia
 * FILE:            aspia/driver.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"

#include <stddef.h>


#define IOCTL_GET_SMBIOS \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 1,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_GET_MSR \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 2,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_GET_PMC \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 3,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_PORT_DWORD \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 4,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_PORT_WORD \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 5,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_PORT_BYTE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 6,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_WRITE_PORT_DWORD \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 7,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_WRITE_PORT_WORD \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 8,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_WRITE_PORT_BYTE \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 9,  METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_PCI_CONFIG \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 10, METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_WRITE_PCI_CONFIG \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 11, METHOD_BUFFERED, FILE_ALL_ACCESS)
#define IOCTL_READ_MEMORY \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 12, METHOD_BUFFERED, FILE_ALL_ACCESS)


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

typedef struct _READ_MSR_QUERY
{
    UINT32 Register;
    UINT32 CpuIndex;
} READ_MSR_QUERY, *PREAD_MSR_QUERY;

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
    LARGE_INTEGER Address;
    ULONG UnitSize;
    ULONG Count;
} READ_MEMORY_INPUT, *PREAD_MEMORY_INPUT;


HANDLE hDriverFile = INVALID_HANDLE_VALUE;
const LPWSTR lpDriverName = L"Aspia";

#ifdef _ASPIA_PORTABLE_
BOOL
DRIVER_ExtractFromExe(OUT LPWSTR lpszDriverPath,
                      IN SIZE_T PathSize)
{
    WCHAR szPath[MAX_PATH];
    BOOL IsWin64 = IsWin64System();
    HRSRC hRes = NULL;
    HGLOBAL hData = NULL;
    LPVOID pData = NULL;
    DWORD dwSize, dwWritten;
    HANDLE hFile;

    if (!GetTempPath(sizeof(szPath)/sizeof(WCHAR), szPath))
    {
        StringCbCopy(lpszDriverPath, PathSize, L"aspia.sys");
    }
    else
    {
        StringCbPrintf(lpszDriverPath,
                       PathSize,
                       L"%s\\aspia",
                       szPath);
        CreateDirectory(lpszDriverPath, NULL);
        StringCbCat(lpszDriverPath, PathSize, L"\\aspia.sys");
    }
    DeleteFile(lpszDriverPath);

    DebugTrace(L"Extract %s driver to: %s",
		       IsWin64 ? L"amd64" : L"x86", lpszDriverPath);
    
	hRes = FindResource(hInstance,
                        IsWin64 ?
                            MAKEINTRESOURCE(ID_DRIVER_X64_SYS) :
                            MAKEINTRESOURCE(ID_DRIVER_X32_SYS),
                        L"SYS");
    if (hRes == NULL) return FALSE;

    dwSize = SizeofResource(hInstance, hRes);
    if (dwSize == 0) return FALSE;

    hData = LoadResource(hInstance, hRes);
    if (hData == NULL) return FALSE;

    pData = LockResource(hData);
    if (pData == NULL) return FALSE;

    hFile = CreateFile(lpszDriverPath,
                       GENERIC_WRITE,
                       FILE_SHARE_WRITE,
                       0, OPEN_ALWAYS,
                       0, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    if (!WriteFile(hFile, pData, dwSize, &dwWritten, 0))
    {
        CloseHandle(hFile);
        return FALSE;
    }

    CloseHandle(hFile);
    return TRUE;
}
#endif /* _ASPIA_PORTABLE_ */

static BOOL
InstallDriver(IN SC_HANDLE scHandle,
              IN LPCTSTR lpDriverExec)
{
    SC_HANDLE hService;

    hService = CreateService(scHandle,
                             lpDriverName,
                             lpDriverName,
                             SERVICE_ALL_ACCESS,
                             SERVICE_KERNEL_DRIVER,
                             SERVICE_DEMAND_START,
                             SERVICE_ERROR_NORMAL,
                             lpDriverExec,
                             NULL, NULL, NULL,
                             NULL, NULL);
    if (!hService) return FALSE;

    CloseServiceHandle(hService);
    return TRUE;
}

static BOOL
RemoveDriver(IN SC_HANDLE scHandle)
{
    SC_HANDLE hService;

    hService = OpenService(scHandle,
                           lpDriverName,
                           SERVICE_ALL_ACCESS);
    if (!hService) return FALSE;

    if (!DeleteService(hService))
    {
        CloseServiceHandle(hService);
        return FALSE;
    }

    CloseServiceHandle(hService);
    return TRUE;
}

static BOOL
StartDriver(IN SC_HANDLE scHandle)
{
    SC_HANDLE hService;

    hService = OpenService(scHandle,
                           lpDriverName,
                           SERVICE_ALL_ACCESS);
    if (!hService) return FALSE;

    if (!StartService(hService, 0, NULL))
    {
        CloseServiceHandle(hService);
        return FALSE;
    }

    CloseServiceHandle(hService);
    return TRUE;
}

static BOOL
StopDriver(IN SC_HANDLE scHandle)
{
    SERVICE_STATUS Status;
    SC_HANDLE hService;

    hService = OpenService(scHandle,
                           lpDriverName,
                           SERVICE_ALL_ACCESS);
    if (!hService) return FALSE;

    if (!ControlService(hService,
                        SERVICE_CONTROL_STOP,
                        &Status))
    {
        CloseServiceHandle(hService);
        return FALSE;
    }

    CloseServiceHandle(hService);
    return TRUE;
}

BOOL
CloseDevice(IN HANDLE hHandle)
{
    return CloseHandle(hHandle);
}

BOOL
DRIVER_Unload(VOID)
{
    SC_HANDLE scHandle;

    DebugTrace(L"Unloading driver");

    if (hDriverFile != INVALID_HANDLE_VALUE)
    {
        CloseDevice(hDriverFile);
    }

    scHandle = OpenSCManager(NULL, NULL,
                             SC_MANAGER_ALL_ACCESS);
    if (!scHandle) return FALSE;

    StopDriver(scHandle);

    if (!RemoveDriver(scHandle))
    {
        CloseServiceHandle(scHandle);
        return FALSE;
    }

    CloseServiceHandle(scHandle);
    return TRUE;
}

HANDLE
OpenDevice(VOID)
{
    return CreateFile(L"\\\\.\\Aspia",
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_SYSTEM,
                      NULL);
}

BOOL
DRIVER_Load(VOID)
{
    WCHAR szDriverExec[MAX_PATH];
#ifndef _ASPIA_PORTABLE_
	WCHAR szCurrent[MAX_PATH];
	WCHAR szTemp[MAX_PATH];
#endif
    SC_HANDLE scHandle;
	BOOLEAN   canDelete = TRUE;
	BOOLEAN   bStarted;

    DebugTrace(L"Loading driver...");

#ifdef _ASPIA_PORTABLE_
    if (!DRIVER_ExtractFromExe(szDriverExec, sizeof(szDriverExec)))
        return FALSE;
#else
	if (!GetTempPath(MAX_PATH, szTemp)) return FALSE;

    StringCbPrintf(szCurrent, sizeof(szCurrent),
                   L"%s%s",
                   SettingsInfo.szCurrentPath,
                   IsWin64System() ? L"aspia_x64.sys" : L"aspia_x32.sys");

	StringCbPrintf(szDriverExec, sizeof(szDriverExec), L"%saspia.sys", szTemp);

	if (!CopyFile(szCurrent, szDriverExec, FALSE)) {
		StringCchCopy(szDriverExec, MAX_PATH, szCurrent);
		canDelete = FALSE;
	}
	DebugTrace(L"Driver path: %s", szCurrent);
	DebugTrace(L"Driver temp path: %s", szDriverExec);
#endif

    scHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scHandle) return FALSE;

    DRIVER_Unload();

    bStarted = InstallDriver(scHandle, szDriverExec) && 
		       StartDriver(scHandle);

	if (canDelete) DeleteFile(szDriverExec);
	
    if (!bStarted) {
        CloseServiceHandle(scHandle);
        return FALSE;
    }

    CloseServiceHandle(scHandle);

    hDriverFile = OpenDevice();
    if (hDriverFile == INVALID_HANDLE_VALUE)
    {
        DRIVER_Unload();
        return FALSE;
    }

    DebugTrace(L"Driver was loaded successfully");

    return TRUE;
}

PVOID
DRIVER_GetSMBIOSData(OUT DWORD* ReturnSize)
{
    PVOID Data = NULL;
    DWORD ReadByte;
    DWORD NeedSize;
    SMBIOS_ENTRY SmbiosEntry;

    // ������� SMBIOS_ENTRY
    if (DeviceIoControl(hDriverFile,
                        (DWORD)IOCTL_GET_SMBIOS,
                        NULL, 0,
                        &SmbiosEntry,
                        sizeof(SMBIOS_ENTRY),
                        &ReadByte, 0) &&
        sizeof(SMBIOS_ENTRY) == ReadByte)
    {
        // �������� ������� ���� ������ ��� SMBIOS_ENTRY � ������
        NeedSize = SmbiosEntry.StructureTableLength + sizeof(SMBIOS_ENTRY);

        // ������� ������ ��� �� ���
        Data = VirtualAlloc(NULL, NeedSize,
                            MEM_COMMIT | MEM_RESERVE,
                            PAGE_READWRITE);

        if (Data)
        {
            // �������� SMBIOS_ENTRY � ������
            if (DeviceIoControl(hDriverFile,
                                (DWORD)IOCTL_GET_SMBIOS,
                                NULL, 0,
                                Data,
                                NeedSize,
                                &ReadByte, 0) &&
                NeedSize == ReadByte)
            {
                if (ReturnSize)
                {
                    *ReturnSize = NeedSize;
                }
            }
            else // ���� ������������ ���������� ������, �� ����������� ������
            {
                VirtualFree(Data, 0, MEM_RELEASE);
                Data = NULL;
            }
        }
    }

    return Data;
}

// ��������� ������ MSR �� ��������
// hFile - ����� ��������� ��������
// Register - ����� ��������
// CpuIndex - ����� ����������
// Data - ��������� �� ���������� � ������� ��������� ������ MSR ��������
// ������������ true ��� false 
BOOL
DRIVER_GetMSRData(IN UINT32 Register,
                  IN UINT32 CpuIndex,
                  OUT UINT64* Data)
{
    DWORD ReadByte;
    READ_MSR_QUERY Query;

    Query.CpuIndex = CpuIndex;
    Query.Register = Register;

    return DeviceIoControl(hDriverFile,
                           (DWORD)IOCTL_GET_MSR,
                           &Query,
                           sizeof(READ_MSR_QUERY),
                           Data, sizeof(UINT64),
                           &ReadByte, 0);
}

WORD
DRIVER_ReadIoPortWord(IN WORD Port)
{
    WORD Value = 0;
    DWORD ReadByte;

    DeviceIoControl(hDriverFile,
                    (DWORD)IOCTL_READ_PORT_WORD,
                    &Port, sizeof(WORD),
                    &Value, sizeof(Value),
                    &ReadByte, 0);
    return Value;
}

DWORD
DRIVER_ReadIoPortDword(IN WORD Port)
{
    DWORD ReadByte = 0, Value;

    if (!DeviceIoControl(hDriverFile,
                         (DWORD)IOCTL_READ_PORT_DWORD,
                         &Port, sizeof(DWORD),
                         &Value, sizeof(Value),
                         &ReadByte, 0))
    {
        return 0;
    }

    return Value;
}

BYTE
DRIVER_ReadIoPortByte(IN DWORD Port)
{
    DWORD ReadByte;
    WORD Value;

    DeviceIoControl(hDriverFile,
                    (DWORD)IOCTL_READ_PORT_BYTE,
                    &Port, sizeof(Port),
                    &Value, sizeof(Value),
                    &ReadByte, 0);
    return (BYTE)Value;
}

BOOL
DRIVER_WriteIoPortWord(IN WORD Port,
                       IN WORD Value)
{
    PORT_WRITE_INPUT InBuffer;
    DWORD ReadByte, Length;

    InBuffer.u.ShortData = Value;
    InBuffer.PortNumber = (ULONG)Port;

    Length = offsetof(PORT_WRITE_INPUT, u.CharData) +
        sizeof(InBuffer.u.ShortData);

    return DeviceIoControl(hDriverFile,
                           (DWORD)IOCTL_WRITE_PORT_WORD,
                           &InBuffer, Length,
                           NULL, 0,
                           &ReadByte, NULL);
}

BOOL
DRIVER_WriteIoPortDword(IN WORD Port,
                        IN DWORD Value)
{
    PORT_WRITE_INPUT InBuffer;
    DWORD ReadByte, Length;

    InBuffer.u.LongData = Value;
    InBuffer.PortNumber = (ULONG)Port;

    Length = offsetof(PORT_WRITE_INPUT, u.CharData) +
        sizeof(InBuffer.u.LongData);

    return DeviceIoControl(hDriverFile,
                           (DWORD)IOCTL_WRITE_PORT_DWORD,
                           &InBuffer, Length,
                           NULL, 0,
                           &ReadByte, NULL);
}

BOOL
DRIVER_WriteIoPortByte(IN WORD Port, IN BYTE Value)
{
    PORT_WRITE_INPUT InBuffer;
    DWORD ReadByte, Length;

    InBuffer.u.CharData = Value;
    InBuffer.PortNumber = (ULONG)Port;

    Length = offsetof(PORT_WRITE_INPUT, u.CharData) +
        sizeof(InBuffer.u.CharData);

    return DeviceIoControl(hDriverFile,
                           (DWORD)IOCTL_WRITE_PORT_BYTE,
                           &InBuffer, Length,
                           NULL, 0,
                           &ReadByte, NULL);
}

DWORD
DRIVER_GetRegisterDataDword(IN DWORD Register,
                            IN INT Offset)
{
    DRIVER_WriteIoPortDword(CONFIG_ADDRESS, Register + Offset);
    return DRIVER_ReadIoPortDword(CONFIG_DATA);
}

WORD
DRIVER_GetRegisterDataWord(IN DWORD Register,
                           IN INT Offset)
{
    DRIVER_WriteIoPortDword(CONFIG_ADDRESS, Register + Offset);
    return DRIVER_ReadIoPortWord(CONFIG_DATA);
}

BOOL
DRIVER_ReadPciConfig(IN DWORD PciAddress,
                     IN DWORD RegAddress,
                     OUT PBYTE Value,
                     IN DWORD Size)
{
    READ_PCI_CONFIG_INPUT InputBuffer;
    DWORD ReturnedLength = 0;

    if (!Value) return FALSE;

    if (Size == 2 && (RegAddress & 1) != 0)
    {
        return FALSE;
    }
    if (Size == 4 && (RegAddress & 3) != 0)
    {
        return FALSE;
    }

    InputBuffer.PciAddress = PciAddress;
    InputBuffer.PciOffset = RegAddress;

    return DeviceIoControl(hDriverFile,
                           (DWORD)IOCTL_READ_PCI_CONFIG,
                           &InputBuffer,
                           sizeof(InputBuffer),
                           Value,
                           Size,
                           &ReturnedLength,
                           NULL);
}

BOOL
DRIVER_WritePciConfig(IN DWORD PciAddress,
                      IN DWORD RegAddress,
                      IN PBYTE Value,
                      IN DWORD Size)
{
    DWORD ReturnedLength = 0;
    BOOL Result = FALSE;
    int InputSize = 0;
    PWRITE_PCI_CONFIG_INPUT InputBuffer;

    if (!Value) return FALSE;

    if (Size == 2 && (RegAddress & 1) != 0)
    {
        return FALSE;
    }
    if (Size == 4 && (RegAddress & 3) != 0)
    {
        return FALSE;
    }

    InputSize = offsetof(WRITE_PCI_CONFIG_INPUT, Data) + Size;

    InputBuffer = (PWRITE_PCI_CONFIG_INPUT)Alloc(InputSize);
    if (!InputBuffer) return FALSE;

    CopyMemory(InputBuffer->Data, Value, Size);
    InputBuffer->PciAddress = PciAddress;
    InputBuffer->PciOffset = RegAddress;

    Result = DeviceIoControl(hDriverFile,
                             (DWORD)IOCTL_WRITE_PCI_CONFIG,
                             InputBuffer,
                             InputSize,
                             NULL,
                             0,
                             &ReturnedLength,
                             NULL);
    Free(InputBuffer);

    return Result;
}

DWORD
DRIVER_ReadMemory(IN DWORD_PTR Address,
                  OUT PBYTE Buffer,
                  IN DWORD Count,
                  IN DWORD UnitSize)
{
    READ_MEMORY_INPUT InputBuffer;
    DWORD ReturnedLength = 0;
    BOOL Result = FALSE;
    DWORD Size = 0;

    if (!Buffer) return 0;

#ifndef _M_AMD64
        InputBuffer.Address.HighPart = 0;
        InputBuffer.Address.LowPart = (DWORD)Address;
#else
        InputBuffer.Address.QuadPart = Address;
#endif

    InputBuffer.UnitSize = UnitSize;
    InputBuffer.Count = Count;
    Size = InputBuffer.UnitSize * InputBuffer.Count;

    Result = DeviceIoControl(hDriverFile,
                             (DWORD)IOCTL_READ_MEMORY,
                             &InputBuffer,
                             sizeof(READ_MEMORY_INPUT),
                             Buffer,
                             Size,
                             &ReturnedLength,
                             NULL);

    if (Result && ReturnedLength == Size)
    {
        return Count * UnitSize;
    }
    else
    {
        return 0;
    }
}

BOOL
DRIVER_ReadPMC(IN DWORD Index, OUT PDWORD eax, OUT PDWORD edx)
{
    DWORD ReturnedLength = 0;
    BOOL Result = FALSE;
    BYTE OutputBuffer[8] = {0};

    if (!eax || !edx) return FALSE;

    Result = DeviceIoControl(hDriverFile,
                             (DWORD)IOCTL_GET_PMC,
                             &Index,
                             sizeof(Index),
                             &OutputBuffer,
                             sizeof(OutputBuffer),
                             &ReturnedLength,
                             NULL);

    if (Result)
    {
        CopyMemory(eax, OutputBuffer, 4);
        CopyMemory(edx, OutputBuffer + 4, 4);
    }

    return Result;
}
