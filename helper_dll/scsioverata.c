/*
 * PROJECT:         Aspia (Helper DLL)
 * FILE:            helper_dll/smart.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "helper_dll.h"


BOOL
ScsiOverAtaReadSmartInfo(HANDLE hHandle, BYTE bDevIndex, IDSECTOR *Info)
{
    SCSI_PASS_THROUGH_WBUF SCSICmd = {0};
    DWORD bytesReturned;

    SCSICmd.spt.Length             = sizeof(SCSI_PASS_THROUGH);
    SCSICmd.spt.PathId             = 0;
    SCSICmd.spt.TargetId           = 1;
    SCSICmd.spt.Lun                = 0;
    SCSICmd.spt.TimeOutValue       = 5;
    SCSICmd.spt.SenseInfoLength    = SPT_SENSEBUFFER_LENGTH;
    SCSICmd.spt.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_WBUF, SenseBuffer);
    SCSICmd.spt.DataIn             = SCSI_IOCTL_DATA_IN;
    SCSICmd.spt.DataTransferLength = IDENTIFY_BUFFER_SIZE;
    SCSICmd.spt.DataBufferOffset   = offsetof(SCSI_PASS_THROUGH_WBUF, DataBuffer);

    SCSICmd.spt.CdbLength = 12;
    SCSICmd.spt.Cdb[0]    = 0xA1;
    SCSICmd.spt.Cdb[1]    = (4 << 1) | 0;
    SCSICmd.spt.Cdb[2]    = (1 << 3) | (1 << 2) | 2;
    SCSICmd.spt.Cdb[3]    = 0;
    SCSICmd.spt.Cdb[4]    = 1;
    SCSICmd.spt.Cdb[5]    = 0;
    SCSICmd.spt.Cdb[6]    = 0;
    SCSICmd.spt.Cdb[7]    = 0;
    SCSICmd.spt.Cdb[8]    = 0xA0;
    SCSICmd.spt.Cdb[9]    = ID_CMD;

    if (!DeviceIoControl(hHandle,
                         IOCTL_SCSI_PASS_THROUGH,
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH),
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_WBUF),
                         &bytesReturned,
                         NULL))
    {
        return FALSE;
    }

    __try
    {
        CopyMemory(Info, SCSICmd.DataBuffer, sizeof(IDSECTOR));
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DebugTrace(L"Exception when copying memory!");
        return FALSE;
    }

    return TRUE;
}

BOOL
ScsiOverAtaEnableSmart(HANDLE hHandle)
{
    SCSI_PASS_THROUGH_WBUF SCSICmd = {0};
    DWORD bytesReturned;

    SCSICmd.spt.Length             = sizeof(SCSI_PASS_THROUGH);
    SCSICmd.spt.PathId             = 0;
    SCSICmd.spt.TargetId           = 0;
    SCSICmd.spt.Lun                = 0;
    SCSICmd.spt.TimeOutValue       = 5;
    SCSICmd.spt.SenseInfoLength    = READ_ATTRIBUTE_BUFFER_SIZE;
    SCSICmd.spt.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_WBUF, SenseBuffer);
    SCSICmd.spt.DataIn             = SCSI_IOCTL_DATA_IN;
    SCSICmd.spt.DataBufferOffset   = offsetof(SCSI_PASS_THROUGH_WBUF, DataBuffer);

    SCSICmd.spt.CdbLength = 12;
    SCSICmd.spt.Cdb[0]    = 0xA1;
    SCSICmd.spt.Cdb[1]    = (4 << 1) | 0;;
    SCSICmd.spt.Cdb[2]    = (1 << 3) | (1 << 2) | 2;
    SCSICmd.spt.Cdb[3]    = ENABLE_SMART;
    SCSICmd.spt.Cdb[4]    = 1;
    SCSICmd.spt.Cdb[5]    = 1;
    SCSICmd.spt.Cdb[6]    = SMART_CYL_LOW;
    SCSICmd.spt.Cdb[7]    = SMART_CYL_HI;
    SCSICmd.spt.Cdb[8]    = 0xA0;
    SCSICmd.spt.Cdb[9]    = SMART_CMD;

    return DeviceIoControl(hHandle,
                           IOCTL_SCSI_PASS_THROUGH,
                           &SCSICmd,
                           sizeof(SCSI_PASS_THROUGH),
                           &SCSICmd,
                           sizeof(SCSI_PASS_THROUGH_WBUF),
                           &bytesReturned,
                           NULL);
}

BOOL
ScsiOverAtaReadSmartAttributes(HANDLE hHandle,
                               BYTE bDevNumber,
                               SMART_DRIVE_INFO *Info)
{
    SCSI_PASS_THROUGH_WBUF SCSICmd = {0};
    SMART_INFO *pSmartValues;
    PBYTE pByte1, pByte2;
    DWORD bytesReturned;
    PDWORD pDword;
    UCHAR uIndex;

    SCSICmd.spt.Length             = sizeof(SCSI_PASS_THROUGH);
    SCSICmd.spt.PathId             = 0;
    SCSICmd.spt.TargetId           = 0;
    SCSICmd.spt.Lun                = 0;
    SCSICmd.spt.TimeOutValue       = 5;
    SCSICmd.spt.SenseInfoLength    = 24;
    SCSICmd.spt.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_WBUF, SenseBuffer);
    SCSICmd.spt.DataIn             = SCSI_IOCTL_DATA_IN;
    SCSICmd.spt.DataTransferLength = READ_ATTRIBUTE_BUFFER_SIZE;
    SCSICmd.spt.DataBufferOffset   = offsetof(SCSI_PASS_THROUGH_WBUF, DataBuffer);

    SCSICmd.spt.CdbLength = 12;
    SCSICmd.spt.Cdb[0]    = 0xA1;
    SCSICmd.spt.Cdb[1]    = (4 << 1) | 0;;
    SCSICmd.spt.Cdb[2]    = (1 << 3) | (1 << 2) | 2;
    SCSICmd.spt.Cdb[3]    = READ_ATTRIBUTES;
    SCSICmd.spt.Cdb[4]    = 1;
    SCSICmd.spt.Cdb[5]    = 1;
    SCSICmd.spt.Cdb[6]    = SMART_CYL_LOW;
    SCSICmd.spt.Cdb[7]    = SMART_CYL_HI;
    SCSICmd.spt.Cdb[8]    = 0xA0;
    SCSICmd.spt.Cdb[9]    = SMART_CMD;

    if (!DeviceIoControl(hHandle,
                         IOCTL_SCSI_PASS_THROUGH,
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH),
                         &SCSICmd,
                         offsetof(SCSI_PASS_THROUGH_WBUF, DataBuffer) + SCSICmd.spt.DataTransferLength,//sizeof(SCSI_PASS_THROUGH_WBUF),
                         &bytesReturned,
                         NULL))
    {
        return FALSE;
    }

    Info->m_ucSmartValues = 0;
    Info->m_ucDriveIndex = bDevNumber;

    pByte1 = (PBYTE)(SCSICmd.DataBuffer);

    for (uIndex = 0; uIndex < 30; ++uIndex)
    {
        pByte2 = &pByte1[(uIndex * 12) + 2];
        pDword = (PDWORD)&pByte2[INDEX_ATTRIB_RAW];

        pByte2[INDEX_ATTRIB_RAW + 2] = pByte2[INDEX_ATTRIB_RAW + 3] =
            pByte2[INDEX_ATTRIB_RAW + 4] = pByte2[INDEX_ATTRIB_RAW + 5] =
            pByte2[INDEX_ATTRIB_RAW + 6] = 0;
        if (pByte2[INDEX_ATTRIB_INDEX])
        {
            pSmartValues = &Info->m_stSmartInfo[Info->m_ucSmartValues];
            pSmartValues->m_ucAttribIndex = pByte2[INDEX_ATTRIB_INDEX];
            pSmartValues->m_ucValue = pByte2[INDEX_ATTRIB_VALUE];
            pSmartValues->m_ucWorst = pByte2[INDEX_ATTRIB_WORST];
            pSmartValues->m_dwAttribValue = pDword[0];
            pSmartValues->m_dwThreshold = MAXDWORD;
            ++Info->m_ucSmartValues;
        }
    }

    return TRUE;
}

BOOL
ScsiOverAtaReadSmartThresholds(HANDLE hHandle,
                               BYTE bDriveNum,
                               SMART_DRIVE_INFO *Info)
{
    SCSI_PASS_THROUGH_WBUF SCSICmd = {0};
    SMART_READ_DATA_OUTDATA ReadData = {0};
    SMART_THRESHOLD Threshold[30];
    SMART_INFO *pSmartValues;
    DWORD bytesReturned;
    BYTE Count = 0;
    UCHAR uIndex;

    SCSICmd.spt.Length             = sizeof(SCSI_PASS_THROUGH);
    SCSICmd.spt.PathId             = 0;
    SCSICmd.spt.TargetId           = 0;
    SCSICmd.spt.Lun                = 0;
    SCSICmd.spt.TimeOutValue       = 5;
    SCSICmd.spt.SenseInfoLength    = 24;
    SCSICmd.spt.SenseInfoOffset    = offsetof(SCSI_PASS_THROUGH_WBUF, SenseBuffer);
    SCSICmd.spt.DataIn             = SCSI_IOCTL_DATA_IN;
    SCSICmd.spt.DataTransferLength = READ_THRESHOLD_BUFFER_SIZE;
    SCSICmd.spt.DataBufferOffset   = offsetof(SCSI_PASS_THROUGH_WBUF, DataBuffer);

    SCSICmd.spt.CdbLength = 12;
    SCSICmd.spt.Cdb[0]    = 0xA1;
    SCSICmd.spt.Cdb[1]    = (4 << 1) | 0;;
    SCSICmd.spt.Cdb[2]    = (1 << 3) | (1 << 2) | 2;
    SCSICmd.spt.Cdb[3]    = READ_THRESHOLDS;
    SCSICmd.spt.Cdb[4]    = 1;
    SCSICmd.spt.Cdb[5]    = 1;
    SCSICmd.spt.Cdb[6]    = SMART_CYL_LOW;
    SCSICmd.spt.Cdb[7]    = SMART_CYL_HI;
    SCSICmd.spt.Cdb[8]    = 0xA0;
    SCSICmd.spt.Cdb[9]    = SMART_CMD;

    if (!DeviceIoControl(hHandle,
                         IOCTL_SCSI_PASS_THROUGH,
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH),
                         &SCSICmd,
                         sizeof(SCSI_PASS_THROUGH_WBUF),
                         &bytesReturned,
                         NULL))
    {
        return FALSE;
    }

    for (uIndex = 0; uIndex < 30; ++uIndex)
    {
        memcpy(&(Threshold[uIndex]),
               &(SCSICmd.DataBuffer[uIndex * sizeof(SMART_THRESHOLD) + 2]),
               sizeof(SMART_THRESHOLD));

        if (Threshold[uIndex].Id)
        {
            pSmartValues = &Info->m_stSmartInfo[Count];
            pSmartValues->m_dwThreshold = Threshold[uIndex].Value;
            pSmartValues->bAttribId = Threshold[uIndex].Id;
            ++Count;
        }
    }

    return TRUE;
}

BOOL
ScsiOverAtaEnumSmartData(HANDLE hSmartHandle,
                         BYTE bDevNumber,
                         SMART_ENUMDATAPROC lpEnumProc)
{
    SMART_DRIVE_INFO m_stDrivesInfo;
    BYTE bIndex, Count;
    SMART_RESULT Result;
    HANDLE hHandle;
    IDSECTOR IdInfo = {0};

    DebugTrace(L"ScsiOverAtaEnumSmartData(%x, %d, %p) called",
               hSmartHandle, bDevNumber, lpEnumProc);

    if (!hSmartHandle)
    {
        hHandle = OpenSmart(bDevNumber);
        if (hHandle == INVALID_HANDLE_VALUE)
            return FALSE;
    }
    else
    {
        hHandle = hSmartHandle;
    }

    if (!ScsiOverAtaEnableSmart(hHandle))
    {
        DebugTrace(L"ScsiOverAtaEnableSmart() failed!");
        goto Failed;
    }

    if (!ScsiOverAtaReadSmartAttributes(hHandle, bDevNumber, &m_stDrivesInfo))
    {
        DebugTrace(L"ScsiOverAtaReadSmartAttributes() failed!");
        goto Failed;
    }

    if (!ScsiOverAtaReadSmartThresholds(hHandle, bDevNumber, &m_stDrivesInfo))
    {
        DebugTrace(L"ScsiOverAtaReadSmartThresholds() failed!");
        goto Failed;
    }

    if (!ScsiOverAtaReadSmartInfo(hHandle, bDevNumber, &IdInfo))
    {
        DebugTrace(L"ScsiOverAtaReadSmartInfo() failed!");
        goto Failed;
    }

    ChangeByteOrder((PCHAR)IdInfo.sModelNumber,
                    sizeof(IdInfo.sModelNumber));
    strupr(IdInfo.sModelNumber);

    for (bIndex = 0, Count = 0; bIndex < m_stDrivesInfo.m_ucSmartValues; ++bIndex)
    {
        Result.IsCritical =
            SMART_IDToText(SmartAttribList,
                           m_stDrivesInfo.m_stSmartInfo[bIndex].bAttribId,
                           Result.szName,
                           sizeof(Result.szName));

        /* ID */
        Result.dwAttrID = m_stDrivesInfo.m_stSmartInfo[bIndex].bAttribId;
        /* Данные RAW */
        Result.dwAttrValue = m_stDrivesInfo.m_stSmartInfo[bIndex].m_dwAttribValue;
        /* Порог Threshold */
        Result.dwWarrantyThreshold = m_stDrivesInfo.m_stSmartInfo[bIndex].m_dwThreshold;
        /* Наихудшее Worst */
        Result.dwWorstValue = m_stDrivesInfo.m_stSmartInfo[bIndex].m_ucWorst;
        /* Значение Value */
        Result.bValue = m_stDrivesInfo.m_stSmartInfo[bIndex].m_ucValue;

        lpEnumProc(&Result);
    }

    if (!hSmartHandle) CloseSmart(hHandle);
    return TRUE;

Failed:
    if (!hSmartHandle) CloseSmart(hHandle);
    return FALSE;
}
