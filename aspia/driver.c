/*
 * PROJECT:         Aspia
 * FILE:            aspia/driver.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"

#include <stddef.h>

#include "../driver/sys/ioctl.h"

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
    BOOL bRes = TRUE;

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
    if (!hService) 
    {
        DWORD dwErr = GetLastError();

        if (dwErr != ERROR_SERVICE_EXISTS)
            bRes = FALSE;
    }
    if (hService)
        CloseServiceHandle(hService);
    return bRes;
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
    BOOL bRes = TRUE;

    hService = OpenService(scHandle,
                           lpDriverName,
                           SERVICE_ALL_ACCESS);
    if (!hService) return FALSE;

    if (!StartService(hService, 0, NULL))
    {
        DWORD dwErr = GetLastError();

        if (dwErr != ERROR_SERVICE_ALREADY_RUNNING)
            bRes = FALSE;
    }
    if (hService)
        CloseServiceHandle(hService);

    return bRes;
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

    if (!SettingsInfo.AllowKmDriver)
        return FALSE;

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

    if (!SettingsInfo.AllowKmDriver)
    {
        DebugTrace(L"Kernel-mode driver disabled by user");
        return FALSE;
    }

    DebugTrace(L"Loading driver...");

#ifdef _ASPIA_PORTABLE_
    if (!DRIVER_ExtractFromExe(szDriverExec, sizeof(szDriverExec)))
        return FALSE;
#else
    if (!GetTempPath(MAX_PATH, szTemp)) return FALSE;

    StringCbPrintf(szCurrent, sizeof(szCurrent),
                   L"%s%s",
                   ParamsInfo.szCurrentPath,
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

    bStarted = InstallDriver(scHandle, szDriverExec) &&
               StartDriver(scHandle);

    if (canDelete) DeleteFile(szDriverExec);

    if (!bStarted)
    {
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

    // получим SMBIOS_ENTRY
    if (DeviceIoControl(hDriverFile,
                        (DWORD)IOCTL_GET_SMBIOS,
                        NULL, 0,
                        &SmbiosEntry,
                        sizeof(SMBIOS_ENTRY),
                        &ReadByte, 0) &&
        sizeof(SMBIOS_ENTRY) == ReadByte)
    {
        // Вычислим сколько надо памяти для SMBIOS_ENTRY и данных
        NeedSize = SmbiosEntry.StructureTableLength + sizeof(SMBIOS_ENTRY);

        // Выделим память под всё это
        Data = VirtualAlloc(NULL, NeedSize,
                            MEM_COMMIT | MEM_RESERVE,
                            PAGE_READWRITE);

        if (Data)
        {
            // Получаем SMBIOS_ENTRY и данные
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
            else // если неполучилось заспросить данные, то освобождаем память
            {
                VirtualFree(Data, 0, MEM_RELEASE);
                Data = NULL;
            }
        }
    }

    return Data;
}

// Получение данных MSR из регистра
// hFile - хендл открытого драйвера
// Register - номер регистра
// CpuIndex - номер процессора
// Data - указатель на переменную в которую запищутся данные MSR регистра
// Возвращенает true или false 
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
DRIVER_ReadIoPortWord(IN DWORD Port)
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
DRIVER_ReadIoPortDword(IN DWORD Port)
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
    DWORD Value;

    DeviceIoControl(hDriverFile,
                    (DWORD)IOCTL_READ_PORT_BYTE,
                    &Port, sizeof(Port),
                    &Value, sizeof(Value),
                    &ReadByte, 0);
    return (BYTE)Value;
}

BOOL
DRIVER_WriteIoPortWord(IN DWORD Port,
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
DRIVER_WriteIoPortDword(IN DWORD Port,
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
DRIVER_WriteIoPortByte(IN DWORD Port, IN BYTE Value)
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
