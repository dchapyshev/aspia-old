/*
 * PROJECT:         Aspia (Driver Helper DLL)
 * FILE:            driver/dll/driver_dll.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "driver_dll.h"

#include <stddef.h>

//#include "../driver/ioctl.h"

HANDLE hDriverFile = INVALID_HANDLE_VALUE;
const LPWSTR lpDriverName = L"Aspia";
HINSTANCE hInst;


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
drv_unload(VOID)
{
    SC_HANDLE scHandle;

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
GetCurrentPath(LPWSTR lpszPath, SIZE_T PathLen)
{
    SIZE_T Index;

    if (!GetModuleFileName(hInst, lpszPath, PathLen))
        return FALSE;

    for (Index = lstrlen(lpszPath); Index > 0; Index--)
    {
        if (lpszPath[Index] == L'\\')
        {
            lpszPath[Index + 1] = 0;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IsWin64System(VOID)
{
#ifdef _M_IX86
    typedef BOOL (WINAPI *IW64P)(HANDLE, PBOOL);

    IW64P IsWow64Process;
    HINSTANCE hDLL;
    BOOL Result;

    hDLL = LoadLibrary(L"KERNEL32.DLL");
    if (hDLL)
    {
        IsWow64Process = (IW64P)GetProcAddress(hDLL, "IsWow64Process");
        if (IsWow64Process)
        {
            if (IsWow64Process(GetCurrentProcess(), &Result))
            {
                FreeLibrary(hDLL);
                return Result;
            }
        }
    }

    FreeLibrary(hDLL);
    return FALSE;
#else
    return TRUE;
#endif
}

BOOL
drv_load(VOID)
{
    WCHAR szDriverExec[MAX_PATH];
    WCHAR szCurrent[MAX_PATH];
    WCHAR szCurrentPath[MAX_PATH];
    WCHAR szTemp[MAX_PATH];
    SC_HANDLE scHandle;
    BOOLEAN   canDelete = TRUE;
    BOOLEAN   bStarted;

    if (!GetTempPath(MAX_PATH, szTemp) ||
        !GetCurrentPath(szCurrentPath, MAX_PATH))
    {
        return FALSE;
    }

    StringCbPrintf(szCurrent, sizeof(szCurrent),
                   L"%s%s",
                   szCurrentPath,
                   IsWin64System() ? L"aspia_x64.sys" : L"aspia_x32.sys");

    StringCbPrintf(szDriverExec, sizeof(szDriverExec), L"%saspia.sys", szTemp);

    if (!CopyFile(szCurrent, szDriverExec, FALSE)) {
        StringCchCopy(szDriverExec, MAX_PATH, szCurrent);
        canDelete = FALSE;
    }

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
        drv_unload();
        return FALSE;
    }

    return TRUE;
}

PVOID
drv_get_smbios_data(OUT DWORD* ReturnSize)
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
drv_read_msr(IN UINT32 Register,
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
drv_read_io_port_word(IN DWORD Port)
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
drv_read_io_port_dword(IN DWORD Port)
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
drv_read_io_port_byte(IN DWORD Port)
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
drv_write_io_port_word(IN DWORD Port,
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
drv_write_io_port_dword(IN DWORD Port,
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
drv_write_io_port_byte(IN DWORD Port, IN BYTE Value)
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
drv_get_register_data_dword(IN DWORD Register,
                            IN INT Offset)
{
    drv_write_io_port_dword(CONFIG_ADDRESS, Register + Offset);
    return drv_read_io_port_dword(CONFIG_DATA);
}

WORD
drv_get_register_data_word(IN DWORD Register,
                           IN INT Offset)
{
    drv_write_io_port_dword(CONFIG_ADDRESS, Register + Offset);
    return drv_read_io_port_word(CONFIG_DATA);
}

BOOL
drv_read_pci_config(IN DWORD PciAddress,
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
drv_write_pci_config(IN DWORD PciAddress,
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

    InputBuffer = (PWRITE_PCI_CONFIG_INPUT)HeapAlloc(GetProcessHeap(), 0, InputSize);
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
    HeapFree(GetProcessHeap(), 0, InputBuffer);

    return Result;
}

BOOL
drv_read_pmc(IN DWORD Index, OUT PDWORD eax, OUT PDWORD edx)
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

BOOL APIENTRY
DllMain(HMODULE hinstDLL,
        DWORD  dwReason,
        LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
            hInst = hinstDLL;
            break;
    }

    return TRUE;
}
