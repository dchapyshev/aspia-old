/*
 * PROJECT:         Aspia
 * FILE:            aspia/scsi/scsi.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

typedef struct _SRB_IO_CONTROL
{
    ULONG HeaderLength;
    UCHAR Signature[8];
    ULONG Timeout;
    ULONG ControlCode;
    ULONG ReturnCode;
    ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

HANDLE SCSI_Open(BYTE bDevNumber);
BOOL SCSI_Close(HANDLE hHandle);
BOOL SCSI_ReadDriveInformation(HANDLE hHandle, BYTE bDevNumber, void *Info);
