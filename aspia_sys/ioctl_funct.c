/*
 * PROJECT:         Aspia
 * FILE:            ioctl_funct.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Shevchuk Maksim (maksim.shevchuk@gmail.com)
 *                  Dmitry Chapyshev (dmitry@aspia.ru)
                    ntldr (ntldr@diskcryptor.net, PGP key ID 0xC48251EB4F8E4E6)
 */

#include <ntifs.h>
#include <stddef.h>

#include "ioctl_funct.h"


KAFFINITY (NTAPI *_KeSetSystemAffinityThread)(IN KAFFINITY Affinity);
KAFFINITY (NTAPI *_KeSetSystemAffinityThreadEx)(IN KAFFINITY Affinity);
VOID (NTAPI *_KeRevertToUserAffinityThreadEx)(IN KAFFINITY  Affinity);


/* Read MSR register value for specified CPU */ 
NTSTATUS
NTAPI
ReadMsrByRegisterAndCpuIndex(IN UINT32 CpuIndex,
                             IN UINT32 Register,
                             OUT UINT64* Output)
{
    NTSTATUS Status;
    KAFFINITY ActiveAffinity;
    KAFFINITY ReqAffinity;

    /* Get active CPU mask */
    ActiveAffinity = KeQueryActiveProcessors();
    /* calc requested mask */
    ReqAffinity = ActiveAffinity & ((KAFFINITY)1 << CpuIndex);

    /* If not available the specified CPU, return STATUS_UNSUCCESSFUL */
    if (ReqAffinity == 0) return STATUS_UNSUCCESSFUL;

    /* Use KeSetSystemAffinityThreadEx instead of the KeSetSystemAffinityThread routine whenever possible */
    if (_KeSetSystemAffinityThreadEx && _KeRevertToUserAffinityThreadEx) {
        ActiveAffinity = _KeSetSystemAffinityThreadEx(ReqAffinity);
    } else {
        _KeSetSystemAffinityThread(ReqAffinity);
    }
    __try {
        /* Get MSR Reg value */
        *Output = __readmsr(Register);
        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
    /* Set default CPU mask to the current thread */
    if (_KeSetSystemAffinityThreadEx && _KeRevertToUserAffinityThreadEx) {
        _KeRevertToUserAffinityThreadEx(ActiveAffinity);
    } else {
        _KeSetSystemAffinityThread(ActiveAffinity);
    }
    return Status;
}

/* Find and read SMBIOS Data */
NTSTATUS
NTAPI
IOCTL_GetSMBIOS(IN PIRP Irp,
                IN PIO_STACK_LOCATION Stack)
{
    NTSTATUS Status = STATUS_NOT_FOUND;
    PSMBIOS_ENTRY SmbiosEntry;
    LARGE_INTEGER PhysAddr;
    PVOID MapAddr;
    int x;

    /* Check minimal size of output buffer */
    if (Stack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(SMBIOS_ENTRY))
    {
        /* Map SMBIOS region to virtual memory */
        PhysAddr.QuadPart = (LONGLONG)SMBIOS_REGION;
        MapAddr = (UINT32*)MmMapIoSpace(PhysAddr,
                                        SMBIOS_REGION_SIZE,
                                        MmNonCached);

        if (MapAddr)
        {
            /* Search SMBIOS anchor "_SM_" */
            for (x = 0; x < SMBIOS_REGION_SIZE; x += SMBIOS_SEARCH_STEP)
            {
                if (*(UINT32*)((ULONG_PTR)MapAddr + x) == SMBIOS_ANCHOR)
                {
                    /* Copy SMBIOS Entry Point Struct */
                    memcpy(Irp->AssociatedIrp.SystemBuffer,
                            (PVOID)((ULONG_PTR)MapAddr + x),
                            sizeof(SMBIOS_ENTRY));

                    SmbiosEntry = (PSMBIOS_ENTRY)Irp->AssociatedIrp.SystemBuffer;
                    Irp->IoStatus.Information = sizeof(SMBIOS_ENTRY);
                    Status = STATUS_SUCCESS;
                    break;
                }
            }
            /* Unmap SMBIOS region */
            MmUnmapIoSpace(MapAddr, SMBIOS_REGION_SIZE);

            if (NT_SUCCESS(Status) &&
                Stack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(SMBIOS_ENTRY) + SmbiosEntry->StructureTableLength)
            {
                /* Map SMBIOS data to virtual memory */
                PhysAddr.QuadPart = (LONGLONG)SmbiosEntry->StructureTableAddress;
                MapAddr = (UINT32*)MmMapIoSpace(PhysAddr,
                                                SmbiosEntry->StructureTableLength,
                                                MmNonCached);

                if (MapAddr)
                {
                    /* Copy SMBIOS data */
                    memcpy((PVOID)((ULONG_PTR)Irp->AssociatedIrp.SystemBuffer + sizeof(SMBIOS_ENTRY)),
                            MapAddr,
                            SmbiosEntry->StructureTableLength);
                    /* Unmap SMBIOS data */
                    MmUnmapIoSpace(MapAddr, SmbiosEntry->StructureTableLength);
                    Irp->IoStatus.Information += SmbiosEntry->StructureTableLength;
                }
            }
        }
        else
        {
            Status = STATUS_INTERNAL_ERROR;
        }
    }
    else
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    return Status;
}

/* Read MSR data */
NTSTATUS
NTAPI
IOCTL_GetMsr(IN PIRP Irp,
             IN PIO_STACK_LOCATION Stack)
{
    PREAD_MSR_QUERY Query;
    NTSTATUS Status;

    if (Stack->Parameters.DeviceIoControl.InputBufferLength == sizeof(READ_MSR_QUERY) &&
        Stack->Parameters.DeviceIoControl.OutputBufferLength == sizeof(UINT64))
    {
        Query = (PREAD_MSR_QUERY)Irp->AssociatedIrp.SystemBuffer;

        Status = ReadMsrByRegisterAndCpuIndex(Query->CpuIndex, 
                                              Query->Register, 
                                              (UINT64*)Irp->AssociatedIrp.SystemBuffer);
        if (NT_SUCCESS(Status))
        {
            Irp->IoStatus.Information = sizeof(UINT64);
        }
    }
    else
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    return Status;
}


/* Read PMC value for specified CPU */ 
NTSTATUS
NTAPI
ReadPmcByRegisterAndCpuIndex(IN UINT32 CpuIndex,
                             IN ULONG Counter,
                             OUT UINT64* Output)
{
    NTSTATUS Status;
    KAFFINITY ActiveAffinity;
    KAFFINITY ReqAffinity;

    /* Get active CPU mask */
    ActiveAffinity = KeQueryActiveProcessors();
    /* calc requested mask */
    ReqAffinity = ActiveAffinity & ((KAFFINITY)1 << CpuIndex);

    /* If not available the specified CPU, return STATUS_UNSUCCESSFUL */
    if (ReqAffinity == 0) return STATUS_UNSUCCESSFUL;

    /* Use KeSetSystemAffinityThreadEx instead of the KeSetSystemAffinityThread routine whenever possible */
    if (_KeSetSystemAffinityThreadEx && _KeRevertToUserAffinityThreadEx) {
        ActiveAffinity = _KeSetSystemAffinityThreadEx(ReqAffinity);
    } else {
        _KeSetSystemAffinityThread(ReqAffinity);
    }
    __try {
        /* Get PMC Reg value */
        *Output = __readpmc(Counter);
        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
    /* Set default CPU mask to the current thread */
    if (_KeSetSystemAffinityThreadEx && _KeRevertToUserAffinityThreadEx) {
        _KeRevertToUserAffinityThreadEx(ActiveAffinity);
    } else {
        _KeSetSystemAffinityThread(ActiveAffinity);
    }
    return Status;
}

/* Read PMC data */
NTSTATUS
NTAPI
IOCTL_GetPmc(IN PIRP Irp,
             IN PIO_STACK_LOCATION Stack)
{
    PREAD_PMC_QUERY Query;
    NTSTATUS Status;

    if (Stack->Parameters.DeviceIoControl.InputBufferLength == sizeof(READ_PMC_QUERY) &&
        Stack->Parameters.DeviceIoControl.OutputBufferLength == sizeof(UINT64))
    {
        Query = (PREAD_PMC_QUERY)Irp->AssociatedIrp.SystemBuffer;

        Status = ReadPmcByRegisterAndCpuIndex(Query->CpuIndex, 
                                              Query->Counter, 
                                              (UINT64*)Irp->AssociatedIrp.SystemBuffer);
        if (NT_SUCCESS(Status))
        {
            Irp->IoStatus.Information = sizeof(UINT64);
        }
    }
    else
    {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    return Status;
}

NTSTATUS
NTAPI
IOCTL_ReadPort(IN ULONG ControlCode,
               IN PVOID InputBuffer, 
               IN ULONG InputSize, 
               OUT PVOID OutputBuffer, 
               IN ULONG OutputSize, 
               OUT PULONG BytesReturned)
{
    ULONG Port = *(PULONG)InputBuffer;

    switch (ControlCode)
    {
        case IOCTL_READ_PORT_BYTE:
            *(PUCHAR)OutputBuffer = READ_PORT_UCHAR((PUCHAR)(ULONG_PTR)Port);
            break;
        case IOCTL_READ_PORT_WORD:
            *(PUSHORT)OutputBuffer = READ_PORT_USHORT((PUSHORT)(ULONG_PTR)Port);
            break;
        case IOCTL_READ_PORT_DWORD:
            *(PULONG)OutputBuffer = READ_PORT_ULONG((PULONG)(ULONG_PTR)Port);
            break;
        default:
            *BytesReturned = 0;
            return STATUS_INVALID_PARAMETER;
    }

    *BytesReturned = InputSize;
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
IOCTL_WritePort(IN ULONG ControlCode,
                IN PVOID InputBuffer, 
                IN ULONG InputSize, 
                OUT PVOID OutputBuffer, 
                IN ULONG OutputSize, 
                OUT PULONG BytesReturned)
{
    PPORT_WRITE_INPUT Data = (PPORT_WRITE_INPUT)InputBuffer;
    ULONG Port = Data->PortNumber;

    switch (ControlCode)
    {

        case IOCTL_WRITE_PORT_BYTE:
            WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)Port, Data->u.CharData);
            break;
        case IOCTL_WRITE_PORT_WORD:
            WRITE_PORT_USHORT((PUSHORT)(ULONG_PTR)Port, Data->u.ShortData);
            break;
        case IOCTL_WRITE_PORT_DWORD:
            WRITE_PORT_ULONG((PULONG)(ULONG_PTR)Port, Data->u.LongData);
            break;
        default:
            return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
ReadPciConfig(ULONG PciAddress,
              ULONG Offset,
              PVOID Data,
              int Length)
{
    PCI_SLOT_NUMBER Slot;
    ULONG BusNumber;
    int Error;

    BusNumber = PciGetBus(PciAddress);
    Slot.u.AsULONG = 0;
    Slot.u.bits.DeviceNumber = PciGetDev(PciAddress);
    Slot.u.bits.FunctionNumber = PciGetFunc(PciAddress);

    Error = HalGetBusDataByOffset(PCIConfiguration,
                                  BusNumber,
                                  Slot.u.AsULONG,
                                  Data,
                                  Offset,
                                  Length);

    if (Error == 0)
    {
        return STATUS_UNSUCCESSFUL;
    }
    else if (Length != 2 && Error == 2)
    {
        return STATUS_UNSUCCESSFUL;
    }
    else if(Length != Error)
    {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
IOCTL_ReadPciConfig(IN PVOID InputBuffer, 
                    IN ULONG InputSize, 
                    OUT PVOID OutputBuffer, 
                    IN ULONG OutputSize, 
                    OUT PULONG BytesReturned)
{
    PREAD_PCI_CONFIG_INPUT Data =
        (PREAD_PCI_CONFIG_INPUT)InputBuffer;
    NTSTATUS Status;

    if (InputSize != sizeof(READ_PCI_CONFIG_INPUT))
    {
        return STATUS_INVALID_PARAMETER;
    }

    Status = ReadPciConfig(Data->PciAddress,
                           Data->PciOffset,
                           OutputBuffer,
                           OutputSize);

    if (NT_SUCCESS(Status))
    {
        *BytesReturned = OutputSize;
    }
    else
    {
        *BytesReturned = 0;
    }

    return Status;
}

NTSTATUS
NTAPI
WritePciConfig(ULONG PciAddress,
               ULONG Offset,
               PVOID Data,
               int Length)
{
    PCI_SLOT_NUMBER Slot;
    ULONG BusNumber;
    int Error;

    BusNumber = PciGetBus(PciAddress);

    Slot.u.AsULONG = 0;
    Slot.u.bits.DeviceNumber = PciGetDev(PciAddress);
    Slot.u.bits.FunctionNumber = PciGetFunc(PciAddress);

    Error = HalSetBusDataByOffset(PCIConfiguration,
                                  BusNumber,
                                  Slot.u.AsULONG,
                                  Data,
                                  Offset,
                                  Length);

    if (Error != Length)
    {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
IOCTL_WritePciConfig(IN PVOID InputBuffer, 
                     IN ULONG InputSize, 
                     OUT PVOID OutputBuffer, 
                     IN ULONG OutputSize, 
                     OUT PULONG BytesReturned)

{
    PWRITE_PCI_CONFIG_INPUT Data;
    ULONG WriteSize;

    if (InputSize < offsetof(WRITE_PCI_CONFIG_INPUT, Data))
    {
        return STATUS_INVALID_PARAMETER;
    }

    Data = (PWRITE_PCI_CONFIG_INPUT)InputBuffer;
    WriteSize = InputSize - offsetof(WRITE_PCI_CONFIG_INPUT, Data);
    
    *BytesReturned = 0;

    return WritePciConfig(Data->PciAddress,
                          Data->PciOffset,
                          &Data->Data,
                          WriteSize);

}

NTSTATUS
NTAPI
IOCTL_Init()
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    UNICODE_STRING FunctionName;

    RtlInitUnicodeString(&FunctionName, L"KeSetSystemAffinityThread");
    _KeSetSystemAffinityThread = MmGetSystemRoutineAddress(&FunctionName);

    RtlInitUnicodeString(&FunctionName, L"KeSetSystemAffinityThreadEx");
    _KeSetSystemAffinityThreadEx = MmGetSystemRoutineAddress(&FunctionName);

    RtlInitUnicodeString(&FunctionName, L"KeRevertToUserAffinityThreadEx");
    _KeRevertToUserAffinityThreadEx = MmGetSystemRoutineAddress(&FunctionName);

    if (_KeSetSystemAffinityThread || (_KeSetSystemAffinityThreadEx && _KeRevertToUserAffinityThreadEx))
        Status = STATUS_SUCCESS;
   
    return Status;
}
