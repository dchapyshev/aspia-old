/*
 * PROJECT:         Aspia
 * FILE:            aspia/scsi/scsi.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <windows.h>
#include <wchar.h>
#include <strsafe.h>
#include "driver.h"

#define SCSI_MINIPORT_BUFFER_SIZE 512
#define MAX_ATTRIBUTE             30

#define IOCTL_SCSI_BASE FILE_DEVICE_CONTROLLER
#define IOCTL_SCSI_MINIPORT CTL_CODE(IOCTL_SCSI_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define FILE_DEVICE_SCSI 0x0000001b

#define IOCTL_SCSI_MINIPORT_IDENTIFY                ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS      ((FILE_DEVICE_SCSI << 16) + 0x0502)
#define IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS   ((FILE_DEVICE_SCSI << 16) + 0x0503)
#define IOCTL_SCSI_MINIPORT_ENABLE_SMART            ((FILE_DEVICE_SCSI << 16) + 0x0504)
#define IOCTL_SCSI_MINIPORT_DISABLE_SMART           ((FILE_DEVICE_SCSI << 16) + 0x0505)

#define  IDE_ATA_IDENTIFY    0xEC


HANDLE
OpenScsi(BYTE bDevNumber)
{
    WCHAR szPath[MAX_PATH];

    StringCbPrintf(szPath, sizeof(szPath),
                   L"\\\\.\\Scsi%d:",
                   bDevNumber);

    return CreateFile(szPath,
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL,
                      OPEN_EXISTING,
                      0,
                      NULL);
}

BOOL
CloseScsi(HANDLE hHandle)
{
    return CloseHandle(hHandle);
}

BOOL
ReadScsiInfo(HANDLE hHandle, BYTE bDevNumber, IDSECTOR *Info)
{
    BYTE Buf[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) +
        IDENTIFY_BUFFER_SIZE] = {0};
    SRB_IO_CONTROL *IoControl = (SRB_IO_CONTROL *)Buf;
    SENDCMDINPARAMS *CmdIn =
        (SENDCMDINPARAMS *)(Buf + sizeof(SRB_IO_CONTROL));
    DWORD cbBytesReturned;

    IoControl->HeaderLength = sizeof(SRB_IO_CONTROL);
    IoControl->Timeout = 10000;
    IoControl->Length = sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE;
    IoControl->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
    strncpy((PCHAR)IoControl->Signature, "SCSIDISK", 8);

    CmdIn->irDriveRegs.bCommandReg = ID_CMD;
    CmdIn->bDriveNumber = bDevNumber;

    if (DeviceIoControl(hHandle, IOCTL_SCSI_MINIPORT,
                        Buf, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
                        Buf, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) +
                            IDENTIFY_BUFFER_SIZE,
                        &cbBytesReturned, NULL))
    {
        SENDCMDOUTPARAMS *pOut = (SENDCMDOUTPARAMS *)(Buf + sizeof(SRB_IO_CONTROL));

        if(*(pOut->bBuffer) && Info)
        {
            CopyMemory(Info, pOut->bBuffer, sizeof(IDSECTOR));
            return TRUE;
        }
    }

    DebugTrace(L"drv_read_scsi_info() failed. Error code: 0x%x",
               GetLastError());

    return FALSE;
}

BOOL
EnableScsiSmart(HANDLE hHandle, BYTE bDevNumber, BYTE bCmd)
{
    BYTE Buf[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) +
        SCSI_MINIPORT_BUFFER_SIZE] = {0};
    SRB_IO_CONTROL *IoControl = (SRB_IO_CONTROL *)Buf;
    SENDCMDINPARAMS *CmdIn =
        (SENDCMDINPARAMS *)(Buf + sizeof(SRB_IO_CONTROL));
    DWORD cbBytesReturned;

    IoControl->HeaderLength = sizeof(SRB_IO_CONTROL);
    IoControl->Timeout = 2;
    IoControl->Length = sizeof(SENDCMDOUTPARAMS) + SCSI_MINIPORT_BUFFER_SIZE;
    memcpy((PCHAR)IoControl->Signature, "SCSIDISK", 8);

    if(bCmd == DISABLE_SMART)
        IoControl->ControlCode = IOCTL_SCSI_MINIPORT_DISABLE_SMART;
    else
        IoControl->ControlCode = IOCTL_SCSI_MINIPORT_ENABLE_SMART;

    CmdIn->irDriveRegs.bFeaturesReg     = bCmd;
    CmdIn->irDriveRegs.bSectorCountReg  = 1;
    CmdIn->irDriveRegs.bSectorNumberReg = 1;
    CmdIn->irDriveRegs.bCylLowReg       = SMART_CYL_LOW;
    CmdIn->irDriveRegs.bCylHighReg      = SMART_CYL_HI;
    CmdIn->irDriveRegs.bCommandReg      = SMART_CMD;
    CmdIn->cBufferSize                  = SCSI_MINIPORT_BUFFER_SIZE;
    CmdIn->bDriveNumber                 = bDevNumber;

    return DeviceIoControl(hHandle, IOCTL_SCSI_MINIPORT,
                           Buf, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
                           Buf, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) +
                               SCSI_MINIPORT_BUFFER_SIZE,
                           &cbBytesReturned, NULL);
}

BOOL
ReadScsiSmartAttributes(HANDLE hHandle, BYTE bDevNumber)
{
    BYTE Buf[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) +
             READ_ATTRIBUTE_BUFFER_SIZE] = {0};
    SRB_IO_CONTROL *IoControl = (SRB_IO_CONTROL*)Buf;
    SENDCMDINPARAMS *CmdId =
        (SENDCMDINPARAMS*)(Buf + sizeof(SRB_IO_CONTROL));
    DWORD cbBytesReturned;

    IoControl->Length = sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE;
    IoControl->ControlCode = IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS;
    IoControl->HeaderLength = sizeof(SRB_IO_CONTROL);
    IoControl->Timeout = 2;
    memcpy((PCHAR)IoControl->Signature, "SCSIDISK", 8);

    CmdId->irDriveRegs.bFeaturesReg     = READ_ATTRIBUTES;
    CmdId->irDriveRegs.bSectorCountReg  = 1;
    CmdId->irDriveRegs.bSectorNumberReg = 1;
    CmdId->irDriveRegs.bCylLowReg       = SMART_CYL_LOW;
    CmdId->irDriveRegs.bCylHighReg      = SMART_CYL_HI;
    CmdId->irDriveRegs.bCommandReg      = SMART_CMD;
    CmdId->cBufferSize                  = READ_ATTRIBUTE_BUFFER_SIZE;
    CmdId->bDriveNumber                 = bDevNumber;

    if (DeviceIoControl(hHandle, IOCTL_SCSI_MINIPORT,
                        Buf, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
                        Buf, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) +
                             READ_ATTRIBUTE_BUFFER_SIZE,
                        &cbBytesReturned, NULL))
    {
        SENDCMDOUTPARAMS *pOut =
            (SENDCMDOUTPARAMS *)(Buf + sizeof(SRB_IO_CONTROL));

        // copy data
        return TRUE;
    }

    return FALSE;
}

BOOL
ReadScsiSmartThresholds(HANDLE hHandle, BYTE bDevNumber)
{
    BYTE Buf[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) +
        READ_THRESHOLD_BUFFER_SIZE] = {0};
    SRB_IO_CONTROL *IoControl = (SRB_IO_CONTROL *)Buf;
    SENDCMDINPARAMS *CmdIn = (SENDCMDINPARAMS *)(Buf + sizeof(SRB_IO_CONTROL));
    DWORD cbBytesReturned;

    IoControl->HeaderLength = sizeof(SRB_IO_CONTROL);
    IoControl->Timeout = 2;
    IoControl->Length = sizeof(SENDCMDOUTPARAMS) + READ_THRESHOLD_BUFFER_SIZE;
    IoControl->ControlCode = IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS;
    strncpy((PCHAR)IoControl->Signature, "SCSIDISK", 8);

    CmdIn->irDriveRegs.bFeaturesReg     = READ_THRESHOLDS;
    CmdIn->irDriveRegs.bSectorCountReg  = 1;
    CmdIn->irDriveRegs.bSectorNumberReg = 1;
    CmdIn->irDriveRegs.bCylLowReg       = SMART_CYL_LOW;
    CmdIn->irDriveRegs.bCylHighReg      = SMART_CYL_HI;
    CmdIn->irDriveRegs.bCommandReg      = SMART_CMD;
    CmdIn->cBufferSize                  = READ_THRESHOLD_BUFFER_SIZE;
    CmdIn->bDriveNumber                 = bDevNumber;

    if (DeviceIoControl(hHandle, IOCTL_SCSI_MINIPORT,
                        Buf, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
                        Buf, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) +
                            READ_THRESHOLD_BUFFER_SIZE,
                        &cbBytesReturned, NULL))
    {
        SENDCMDOUTPARAMS *pOut = (SENDCMDOUTPARAMS *)(Buf + sizeof(SRB_IO_CONTROL));
        if (*(pOut->bBuffer) > 0)
        {
            // copy data
        }
    }

    return FALSE;
}
