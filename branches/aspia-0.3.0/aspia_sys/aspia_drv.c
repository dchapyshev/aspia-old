/*
 * PROJECT:         Aspia
 * FILE:            driver/aspia_drv.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Shevchuk Maksim (maksim.shevchuk@gmail.com)
 *                  Dmitry Chapyshev (dmitry@aspia.ru)
                    ntldr (ntldr@diskcryptor.net, PGP key ID 0xC48251EB4F8E4E6)
 */

#include <ntifs.h>

#include "../include/version.h"
#include "ioctl_funct.h"


UNICODE_STRING SymbolicLinkName;
PDEVICE_OBJECT DeviceObject;


/* Driver Unload routines */
VOID
NTAPI
DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    IoDeleteSymbolicLink(&SymbolicLinkName);
    IoDeleteDevice(DeviceObject);

    return;
}

/* Driver IRP_MJ_DEVICE_CONTROL routines */
NTSTATUS
NTAPI
DriverIoControl(IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION Stack;
    PVOID InputBuffer;
    PVOID OutputBuffer;
    ULONG InputSize;
    ULONG OutputSize;

    PAGED_CODE();

    Stack = IoGetCurrentIrpStackLocation(Irp);

    InputBuffer = Irp->AssociatedIrp.SystemBuffer;
    OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
    InputSize = Stack->Parameters.DeviceIoControl.InputBufferLength;
    OutputSize = Stack->Parameters.DeviceIoControl.OutputBufferLength;

    Irp->IoStatus.Information = 0;

    switch (Stack->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_GET_SMBIOS:
            Status = IOCTL_GetSMBIOS(Irp, Stack);
            break;

        case IOCTL_GET_MSR:
            Status = IOCTL_GetMsr(Irp, Stack);
            break;

        case IOCTL_GET_PMC:
            Status = IOCTL_GetPmc(Irp, Stack);
            break;

        case IOCTL_READ_PCI_CONFIG:
            Status = IOCTL_ReadPciConfig(InputBuffer,
                                         InputSize,
                                         OutputBuffer,
                                         OutputSize,
                                         (PULONG)&Irp->IoStatus.Information);
            break;
        case IOCTL_WRITE_PCI_CONFIG:
            Status = IOCTL_WritePciConfig(InputBuffer,
                                          InputSize,
                                          OutputBuffer,
                                          OutputSize,
                                          (PULONG)&Irp->IoStatus.Information);
            break;

        case IOCTL_READ_PORT_DWORD:
        case IOCTL_READ_PORT_WORD:
        case IOCTL_READ_PORT_BYTE:
            Status = IOCTL_ReadPort(Stack->Parameters.DeviceIoControl.IoControlCode,
                                    InputBuffer,
                                    InputSize,
                                    OutputBuffer,
                                    OutputSize,
                                    (PULONG)&Irp->IoStatus.Information);
            break;

        case IOCTL_WRITE_PORT_DWORD:
        case IOCTL_WRITE_PORT_WORD:
        case IOCTL_WRITE_PORT_BYTE:
            Status = IOCTL_WritePort(Stack->Parameters.DeviceIoControl.IoControlCode,
                                     InputBuffer,
                                     InputSize,
                                     OutputBuffer,
                                     OutputSize,
                                     (PULONG)&Irp->IoStatus.Information);
            break;

        default:
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

/* Driver IRP_MJ_CREATE routines */
NTSTATUS
NTAPI
DriverDispatcher(IN PDEVICE_OBJECT DeviceObject,
                 IN PIRP Irp)
{
    NTSTATUS Status = STATUS_SUCCESS;
    SECURITY_SUBJECT_CONTEXT  subjContext;

    PAGED_CODE();

    SeCaptureSubjectContext(&subjContext);

    if (!SeTokenIsAdmin(SeQuerySubjectContextToken(&subjContext)))
        Status = STATUS_ACCESS_DENIED;

    SeReleaseSubjectContext(&subjContext);

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}


NTSTATUS
NTAPI
DriverEntry(IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath)
{
    UNICODE_STRING DeviceName;
    NTSTATUS Status;

    Status = IOCTL_Init();
    if (NT_SUCCESS(Status))
    {
        RtlInitUnicodeString(&DeviceName, L"\\Device\\Aspia");
        RtlInitUnicodeString(&SymbolicLinkName, L"\\DosDevices\\Aspia");

        /* Create Aspia device */
        Status = IoCreateDevice(DriverObject,
                                0,
                                &DeviceName,
                                FILE_DEVICE_UNKNOWN,
                                0,
                                FALSE,
                                &DeviceObject);
        if (NT_SUCCESS(Status))
        {
            Status = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);

            if (NT_SUCCESS(Status))
            {
                /* Register driver routines */
                DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverIoControl;
                DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverDispatcher;
                DriverObject->DriverUnload = DriverUnload;
            }
            else
            {
                IoDeleteDevice(DeviceObject);
            }
        }
    }

    return Status;
}
