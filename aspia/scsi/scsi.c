/*
 * PROJECT:         Aspia
 * FILE:            aspia/scsi/scsi.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "scsi.h"

#pragma warning(disable: 4995)
#pragma warning(disable: 4996)

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

#define IOCTL_SCSI_BASE FILE_DEVICE_CONTROLLER
#define IOCTL_SCSI_MINIPORT CTL_CODE(IOCTL_SCSI_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define FILE_DEVICE_SCSI 0x0000001b
#define IOCTL_SCSI_MINIPORT_IDENTIFY  ((FILE_DEVICE_SCSI << 16) + 0x0501)
#define  IDE_ATA_IDENTIFY    0xEC


HANDLE
SCSI_Open(BYTE bDevNumber)
{
    TCHAR szPath[MAX_PATH];

    StringCchPrintf(szPath, sizeof(szPath)/sizeof(TCHAR), _T("\\\\.\\Scsi%d:"), bDevNumber);
    return CreateFile(szPath,
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE, 
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_SYSTEM,
                      NULL);
}

BOOL
SCSI_Close(HANDLE hHandle)
{
    return CloseHandle(hHandle);
}

BOOL
SCSI_ReadDriveInformation(HANDLE hHandle, BYTE bDevNumber, void *Info)
{
    char Buffer[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE];
    SRB_IO_CONTROL *SrbIoControl = (SRB_IO_CONTROL*)Buffer;
    SENDCMDINPARAMS *CmdIn = (SENDCMDINPARAMS*)(Buffer + sizeof(SRB_IO_CONTROL));
    SENDCMDOUTPARAMS *CmdOut;
    DWORD cbBytesReturned;
    BOOL bResult;

    //ZeroMemory(Buffer, sizeof(Buffer));
    memset ( Buffer, 0, sizeof ( Buffer ) ); 

    SrbIoControl->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
    SrbIoControl->HeaderLength = sizeof(SRB_IO_CONTROL);
    SrbIoControl->Length = sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE;
    SrbIoControl->Timeout = 10000;
    strcpy((char*)SrbIoControl->Signature, "SCSIDISK");

    CmdIn->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
    CmdIn->bDriveNumber = bDevNumber;

    bResult = DeviceIoControl(hHandle,
                              IOCTL_SCSI_MINIPORT, 
                              Buffer,
                              sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
                              Buffer,
                              sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE,
                              &cbBytesReturned,
                              NULL);
    if (!bResult || !Info) return FALSE;

    CmdOut = (SENDCMDOUTPARAMS*)(Buffer + sizeof(SRB_IO_CONTROL));
    Info = (LPVOID)(IDSECTOR*)CmdOut->bBuffer;

    return TRUE;
}
