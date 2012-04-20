/*
 * PROJECT:         Aspia (Helper DLL)
 * FILE:            helper_dll/scsi.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"

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

HANDLE
OpenScsiByDriveLetter(WCHAR letter)
{
    WCHAR szPath[MAX_PATH];

    StringCbPrintf(szPath, sizeof(szPath),
                   L"\\\\.\\%c:",
                   letter);

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
GetConfinurationScsi(HANDLE hHandle,
                     WORD wProfile,
                     PSCSI_GET_CONFIG pConfiguration)
{
    SCSI_PASS_THROUGH_DIRECT_WBUF SCSICmd = {0};
    PSCSI_GET_CONFIG pConfig;
    LPVOID dataBuffer = NULL;
    ULONG dataBufferSize;
    DWORD bytesReturned;

    dataBufferSize = sizeof(SCSI_GET_CONFIG);
    dataBuffer = HeapAlloc(GetProcessHeap(),
                           HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                           dataBufferSize);
    if (!dataBuffer)
    {
        return FALSE;
    }

    pConfig = (PSCSI_GET_CONFIG)dataBuffer;

    SCSICmd.spt.Length             = sizeof(SCSI_PASS_THROUGH);
    SCSICmd.spt.PathId             = 0;
    SCSICmd.spt.TargetId           = 1;
    SCSICmd.spt.Lun                = 0;
    SCSICmd.spt.TimeOutValue       = 5;
    SCSICmd.spt.SenseInfoLength    = SPT_SENSEBUFFER_LENGTH;
    SCSICmd.spt.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_WBUF, SenseBuffer);
    SCSICmd.spt.DataIn             = SCSI_IOCTL_DATA_IN;
    SCSICmd.spt.DataTransferLength = dataBufferSize;
    SCSICmd.spt.DataBuffer         = pConfig;

    SCSICmd.spt.CdbLength = 10;
    SCSICmd.spt.Cdb[0]    = SCSI_GET_CONFIGURATION;
    SCSICmd.spt.Cdb[1]    = 0;
    SCSICmd.spt.Cdb[2]    = (wProfile >> 8) & 0xFF;
    SCSICmd.spt.Cdb[3]    = wProfile & 0xFF;
    SCSICmd.spt.Cdb[7]    = (dataBufferSize >> 8) & 0xFF;
    SCSICmd.spt.Cdb[8]    = dataBufferSize & 0xFF;

    if (!DeviceIoControl(hHandle,
                         IOCTL_SCSI_PASS_THROUGH_DIRECT,
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_DIRECT_WBUF),
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_DIRECT_WBUF),
                         &bytesReturned,
                         FALSE))
    {
        HeapFree(GetProcessHeap(), 0, dataBuffer);
        return FALSE;
    }

    CopyMemory(pConfiguration, pConfig, sizeof(SCSI_GET_CONFIG));
    HeapFree(GetProcessHeap(), 0, dataBuffer);

    return TRUE;
}

BOOL
GetCDCapabilitiesScsi(HANDLE hHandle,
                      PSCSI_CD_CAPABILITIES pCapabilities)
{
    SCSI_PASS_THROUGH_DIRECT_WBUF SCSICmd = {0};
    BYTE dataBuffer[36];
    DWORD bytesReturned;

    SCSICmd.spt.Length             = sizeof(SCSI_PASS_THROUGH_DIRECT);
    SCSICmd.spt.PathId             = 0;
    SCSICmd.spt.TargetId           = 1;
    SCSICmd.spt.Lun                = 0;
    SCSICmd.spt.TimeOutValue       = 10;
    SCSICmd.spt.SenseInfoLength    = 24;
    SCSICmd.spt.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_WBUF, SenseBuffer);
    SCSICmd.spt.DataIn             = SCSI_IOCTL_DATA_IN;
    SCSICmd.spt.DataTransferLength = 36;
    SCSICmd.spt.DataBuffer         = dataBuffer;

    SCSICmd.spt.CdbLength = 10;
    SCSICmd.spt.Cdb[0]    = 0x1A;
    SCSICmd.spt.Cdb[2]    = 0x2A;
    SCSICmd.spt.Cdb[4]    = 36;

    if (!DeviceIoControl(hHandle,
                         IOCTL_SCSI_PASS_THROUGH_DIRECT,
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_DIRECT_WBUF),
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_DIRECT_WBUF),
                         &bytesReturned,
                         FALSE))
    {
        return FALSE;
    }

    *pCapabilities = *(PSCSI_CD_CAPABILITIES)(dataBuffer + 4);

    return TRUE;
}

BOOL
GetInquiryScsi(HANDLE hHandle,
               PINQUIRYDATA pInquiry)
{
    SCSI_PASS_THROUGH_DIRECT_WBUF SCSICmd = {0};
    PINQUIRYDATA pInqStd;
    LPVOID dataBuffer = NULL;
    ULONG dataBufferSize;
    DWORD bytesReturned;

    dataBufferSize = sizeof(INQUIRYDATA);
    dataBuffer = HeapAlloc(GetProcessHeap(),
                           HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                           dataBufferSize);
    if (!dataBuffer)
    {
        return FALSE;
    }

    pInqStd = (PINQUIRYDATA)dataBuffer;

    SCSICmd.spt.Length             = sizeof(SCSI_PASS_THROUGH);
    SCSICmd.spt.PathId             = 0;
    SCSICmd.spt.TargetId           = 1;
    SCSICmd.spt.Lun                = 0;
    SCSICmd.spt.TimeOutValue       = 5;
    SCSICmd.spt.SenseInfoLength    = SPT_SENSEBUFFER_LENGTH;
    SCSICmd.spt.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_WBUF, SenseBuffer);
    SCSICmd.spt.DataIn             = SCSI_IOCTL_DATA_IN;
    SCSICmd.spt.DataTransferLength = dataBufferSize;
    SCSICmd.spt.DataBuffer         = pInqStd;

    SCSICmd.spt.CdbLength = 6;
    SCSICmd.spt.Cdb[0]    = SCSI_INQUIRY;
    SCSICmd.spt.Cdb[1]    = 0;
    SCSICmd.spt.Cdb[3]    = (dataBufferSize >> 8) & 0xFF;
    SCSICmd.spt.Cdb[4]    = dataBufferSize & 0xFF;

    if (!DeviceIoControl(hHandle,
                         IOCTL_SCSI_PASS_THROUGH_DIRECT,
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_DIRECT_WBUF),
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_DIRECT_WBUF),
                         &bytesReturned,
                         FALSE))
    {
        HeapFree(GetProcessHeap(), 0, dataBuffer);
        return FALSE;
    }

    CopyMemory(pInquiry, pInqStd, sizeof(INQUIRYDATA));
    HeapFree(GetProcessHeap(), 0, dataBuffer);

    return TRUE;
}

BOOL
GetCDReportKeyScsi(HANDLE hHandle,
                   PREPORT_KEY_DATA pKeyData)
{
    SCSI_PASS_THROUGH_DIRECT_WBUF SCSICmd = {0};
    PREPORT_KEY_DATA pKey;
    REPORT_KEY Key = {0};
    LPVOID dataBuffer = NULL;
    ULONG dataBufferSize;
    DWORD bytesReturned;

    dataBufferSize = sizeof(REPORT_KEY_DATA);
    dataBuffer = HeapAlloc(GetProcessHeap(),
                           HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                           dataBufferSize);
    if (!dataBuffer)
    {
        return FALSE;
    }

    pKey = (PREPORT_KEY_DATA)dataBuffer;

    SCSICmd.spt.Length             = sizeof(SCSI_PASS_THROUGH);
    SCSICmd.spt.PathId             = 0;
    SCSICmd.spt.TargetId           = 1;
    SCSICmd.spt.Lun                = 0;
    SCSICmd.spt.TimeOutValue       = 5;
    SCSICmd.spt.SenseInfoLength    = SPT_SENSEBUFFER_LENGTH;
    SCSICmd.spt.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_WBUF, SenseBuffer);
    SCSICmd.spt.DataIn             = SCSI_IOCTL_DATA_IN;
    SCSICmd.spt.DataTransferLength = dataBufferSize;
    SCSICmd.spt.DataBuffer         = pKey;
    SCSICmd.spt.CdbLength          = sizeof(REPORT_KEY);

    Key.OperationCode    = SCSI_REPORT_KEY;
    Key.AllocationLength = sizeof(REPORT_KEY_DATA);
    Key.AGID             = 0;
    Key.KeyFormat        = KEY_FORMAT_RPC_STATE;

    CopyMemory(SCSICmd.spt.Cdb, &Key, sizeof(REPORT_KEY));

    if (!DeviceIoControl(hHandle,
                         IOCTL_SCSI_PASS_THROUGH_DIRECT,
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_DIRECT_WBUF),
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_DIRECT_WBUF),
                         &bytesReturned,
                         FALSE))
    {
        HeapFree(GetProcessHeap(), 0, dataBuffer);
        return FALSE;
    }

    CopyMemory(pKeyData, pKey, sizeof(REPORT_KEY_DATA));
    HeapFree(GetProcessHeap(), 0, dataBuffer);

    return TRUE;
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

    DebugTrace(L"ReadScsiInfo() failed. Error code: 0x%x",
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
