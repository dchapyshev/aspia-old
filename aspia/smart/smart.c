/*
 * PROJECT:         Aspia
 * FILE:            aspia/smart/smart.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include "smart.h"

#pragma warning(disable: 4995)
#pragma warning(disable: 4996)

#define SMART_GET_VERSION        CTL_CODE(IOCTL_DISK_BASE, 0x0020, METHOD_BUFFERED, FILE_READ_ACCESS)
#define SMART_SEND_DRIVE_COMMAND CTL_CODE(IOCTL_DISK_BASE, 0x0021, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define SMART_RCV_DRIVE_DATA     CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define DRIVE_HEAD_REG    0xA0

#define READ_ATTRIBUTE_BUFFER_SIZE  512
#define IDENTIFY_BUFFER_SIZE        512
#define READ_THRESHOLD_BUFFER_SIZE  512
#define SMART_LOG_SECTOR_SIZE       512


HANDLE
SMART_Open(BYTE bDevNumber)
{
    WCHAR szPath[MAX_PATH];

    StringCbPrintf(szPath,
                   sizeof(szPath),
                   _T("\\\\.\\PhysicalDrive%d"),
                   bDevNumber);

    return CreateFile(szPath,
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE, 
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_SYSTEM,
                      NULL);
}

BOOL
SMART_Close(HANDLE hHandle)
{
    return CloseHandle(hHandle);
}

BOOL
SMART_GetVersion(HANDLE hHandle, GETVERSIONINPARAMS *pVersion)
{
    GETVERSIONINPARAMS Version = {0};
    DWORD cbBytesReturned;

    if (!DeviceIoControl(hHandle,
                         SMART_GET_VERSION,
                         NULL,
                         0, 
                         &Version,
                         sizeof(GETVERSIONINPARAMS),
                         &cbBytesReturned,
                         NULL))
    {
        return FALSE;
    }

    if (pVersion) *pVersion = Version;
    return TRUE;
}

BOOL
SMART_Enable(HANDLE hHandle, BYTE bDevNumber)
{
    SENDCMDOUTPARAMS CmdOut = {0};
    SENDCMDINPARAMS CmdIn = {0};
    DWORD lpcbBytesReturned;

    CmdIn.cBufferSize = 0;
    CmdIn.irDriveRegs.bFeaturesReg = ENABLE_SMART;
    CmdIn.irDriveRegs.bSectorCountReg = 1;
    CmdIn.irDriveRegs.bSectorNumberReg = 1;
    CmdIn.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
    CmdIn.irDriveRegs.bCylHighReg = SMART_CYL_HI;
    CmdIn.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
    CmdIn.irDriveRegs.bCommandReg = SMART_CMD; 
    CmdIn.bDriveNumber = bDevNumber;

    return DeviceIoControl(hHandle,
                           SMART_SEND_DRIVE_COMMAND,
                           &CmdIn, 
                           sizeof(SENDCMDINPARAMS),
                           &CmdOut,
                           sizeof(SENDCMDOUTPARAMS), 
                           &lpcbBytesReturned,
                           NULL);
}

BOOL
SMART_ReadDriveInformation(HANDLE hHandle, BYTE bDevNumber, IDSECTOR *Info)
{
    SENDCMDINPARAMS CmdIn = {0};
    DWORD cbBytesReturned;
    char szOut[IDENTIFY_BUFFER_SIZE + 16];
    BOOL bResult;

    CmdIn.cBufferSize = IDENTIFY_BUFFER_SIZE; 
    CmdIn.irDriveRegs.bFeaturesReg = 0; 
    CmdIn.irDriveRegs.bSectorCountReg = 1;
    CmdIn.irDriveRegs.bSectorNumberReg = 1;
    CmdIn.irDriveRegs.bCylLowReg = 0;
    CmdIn.irDriveRegs.bCylHighReg = 0;
    CmdIn.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
    CmdIn.irDriveRegs.bCommandReg = ID_CMD;
    CmdIn.bDriveNumber = bDevNumber;

    bResult = DeviceIoControl(hHandle,
                              SMART_RCV_DRIVE_DATA, 
                              &CmdIn,
                              sizeof(SENDCMDINPARAMS),
                              &szOut,
                              IDENTIFY_BUFFER_SIZE + 16,
                              &cbBytesReturned,
                              NULL);
    if (!bResult || !Info) return FALSE;

    CopyMemory(Info, szOut + 16, sizeof(IDSECTOR));

    return TRUE;
}

BOOL
SMART_ReadAttributesCmd(HANDLE hHandle, BYTE bDevNumber, SMART_DRIVE_INFO *Info)
{
    SENDCMDINPARAMS CmdIn = {0};
    DWORD cbBytesReturned;
    BYTE szOut[sizeof(SMART_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1];
    SMART_INFO *pSmartValues;
    PBYTE pByte1, pByte2;
    PDWORD pDword;
    UCHAR uIndex;
    BOOL bResult;

    CmdIn.cBufferSize = READ_ATTRIBUTE_BUFFER_SIZE; 
    CmdIn.irDriveRegs.bFeaturesReg = READ_ATTRIBUTES; 
    CmdIn.irDriveRegs.bSectorCountReg = 1;
    CmdIn.irDriveRegs.bSectorNumberReg = 1;
    CmdIn.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
    CmdIn.irDriveRegs.bCylHighReg = SMART_CYL_HI;
    CmdIn.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
    CmdIn.irDriveRegs.bCommandReg = SMART_CMD;
    CmdIn.bDriveNumber = bDevNumber;

    bResult = DeviceIoControl(hHandle,
                              SMART_RCV_DRIVE_DATA, 
                              &CmdIn,
                              sizeof(SENDCMDINPARAMS),
                              szOut,
                              sizeof(SMART_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,
                              &cbBytesReturned,
                              NULL);
    if (!bResult || !Info) return FALSE;

    Info->m_ucSmartValues = 0;
    Info->m_ucDriveIndex = bDevNumber;

    pByte1 = (PBYTE)(((SMART_ATAOUTPARAM*)szOut)->bBuffer);

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
SMART_ReadThresholdsCmd(HANDLE hHandle, BYTE bDriveNum, SMART_DRIVE_INFO *Info)
{
    BYTE szOut[sizeof(SMART_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1];
    SMART_INFO *pSmartValues;
    SENDCMDINPARAMS CmdIn;
    DWORD cbBytesReturned;
    PBYTE pByte1, pByte2;
    BYTE Count = 0;
    PDWORD pDword;
    UCHAR uIndex;
    BOOL bResult;

    CmdIn.cBufferSize = READ_THRESHOLD_BUFFER_SIZE;
    CmdIn.irDriveRegs.bFeaturesReg = READ_THRESHOLDS;
    CmdIn.irDriveRegs.bSectorCountReg = 1;
    CmdIn.irDriveRegs.bSectorNumberReg = 1;
    CmdIn.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
    CmdIn.irDriveRegs.bCylHighReg = SMART_CYL_HI;
    CmdIn.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
    CmdIn.irDriveRegs.bCommandReg= SMART_CMD;
    CmdIn.bDriveNumber = bDriveNum;

    bResult = DeviceIoControl(hHandle,
                              SMART_RCV_DRIVE_DATA,
                              &CmdIn,
                              sizeof(SENDCMDINPARAMS),
                              szOut,
                              sizeof(SMART_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,
                              &cbBytesReturned,
                              NULL);
    if (!bResult || !Info) return FALSE;

    pByte1 = (PBYTE)(((SMART_ATAOUTPARAM*)szOut)->bBuffer);

    for (uIndex = 0; uIndex < 30; ++uIndex)
    {
        pDword = (PDWORD)&pByte1[(uIndex * 12) + 7];
        pByte2 = &pByte1[(uIndex * 12) + 2];

        pByte2[INDEX_ATTRIB_RAW + 2] = pByte2[INDEX_ATTRIB_RAW+3] =
            pByte2[INDEX_ATTRIB_RAW+4] = pByte2[INDEX_ATTRIB_RAW+5] =
            pByte2[INDEX_ATTRIB_RAW+6] = 0;
        if (pByte2[0])
        {
            pSmartValues = &Info->m_stSmartInfo[Count];
            pSmartValues->m_dwThreshold = pDword[0];
            pSmartValues->bAttribId = pByte2[0];
            ++Count;
        }
    }

    return TRUE;
}

typedef struct
{
    DWORD dwID;
    BOOL bCritical;
    LPWSTR lpszName;
} INFO_STRUCT;

const INFO_STRUCT SmartAttribList[] =
{
    { 0x01, TRUE, _T("Raw Read Error Rate") },
    { 0x02, FALSE, _T("Throughput Performance") },
    { 0x03, FALSE, _T("Spin Up Time") },
    { 0x04, FALSE, _T("Start/Stop Count") },
    { 0x05, TRUE, _T("Reallocated Sectors Count") },
    { 0x06, FALSE, _T("Read Channel Margin") },
    { 0x07, FALSE, _T("Seek Error Rate") },
    { 0x08, FALSE, _T("Seek Time Performance") },
    { 0x09, FALSE, _T("Power-On Hours") },
    { 0x0A, FALSE, _T("Spin Retry Count") },
    { 0x0B, FALSE, _T("Recalibration Retries") },
    { 0x0C, FALSE, _T("Device Power Cycle Count") },
    { 0x0D, FALSE, _T("Soft Read Error Rate") },
    { 0xC1, FALSE, _T("Load/Unload Cycle Count") },
    { 0xC2, FALSE, _T("Temperature") },
    { 0xC4, TRUE, _T("Reallocation Event Count") },
    { 0xC5, TRUE, _T("Current Pending Sector Count") },
    { 0xC6, TRUE, _T("Uncorrectable Sector Count") },
    { 0xC7, FALSE, _T("UltraDMA CRC Error Count") },
    { 0xC8, FALSE, _T("Write Error Rate") },
    { 0xC9, FALSE, _T("Soft read error rate") },
    { 0xCA, FALSE, _T("Data Address Mark errors") },
    { 0xCB, FALSE, _T("Run out cancel") },
    { 0xCC, FALSE, _T("Soft ECC correction") },
    { 0xCD, FALSE, _T("Thermal asperity rate (TAR)") },
    { 0xCE, FALSE, _T("Flying height") },
    { 0xCF, FALSE, _T("Spin high current") },
    { 0xD0, FALSE, _T("Spin buzz") },
    { 0xD1, FALSE, _T("Offline seek performance") },
    { 0xDC, TRUE, _T("Disk Shift") },
    { 0xDD, FALSE, _T("G-Sense Error Rate") },
    { 0xDE, FALSE, _T("Loaded Hours") },
    { 0xDF, FALSE, _T("Load/Unload Retry Count") },
    { 0xE0, FALSE, _T("Load Friction") },
    { 0xE2, FALSE, _T("Load-in Time") },
    { 0xE3, FALSE, _T("Torque Amplification Count") },
    { 0xE4, FALSE, _T("Power-Off Retract Count") },
    { 0xE6, FALSE, _T("GMR Head Amplitude") },
    { 0xE7, FALSE, _T("Temperature") },
    { 0xF0, FALSE, _T("Head flying hours") },
    { 0xFA, FALSE, _T("Read error retry rate") },
    { 0xFE, FALSE, _T("Free Fall Event Count") },
    { 0x0, FALSE, NULL }
};

VOID
SMART_IDToText(DWORD dwIndex, LPWSTR lpszText, SIZE_T Size)
{
    ULONG Index = 0;

    do
    {
        if (SmartAttribList[Index].dwID == dwIndex)
        {
            StringCbCopy(lpszText, Size, SmartAttribList[Index].lpszName);
            return;
        }
        ++Index;
    }
    while (SmartAttribList[Index].dwID != 0x0);
}

BOOL
SMART_EnumData(HANDLE hSmartHandle, BYTE bDevNumber, SMART_ENUMDATAPROC lpEnumProc)
{
    GETVERSIONINPARAMS Version;
    SMART_DRIVE_INFO m_stDrivesInfo;
    SMART_RESULT Result;
    HANDLE hHandle;
    BYTE bIndex;

    if (!hSmartHandle)
    {
        hHandle = SMART_Open(bDevNumber);
        if (hHandle == INVALID_HANDLE_VALUE)
            return FALSE;
    }
    else
    {
        hHandle = hSmartHandle;
    }

    if (!SMART_GetVersion(hHandle, &Version))
        goto Failed;

    if (!SMART_Enable(hHandle, bDevNumber))
        goto Failed;

    if (!SMART_ReadAttributesCmd(hHandle, bDevNumber, &m_stDrivesInfo))
        goto Failed;

    if (!SMART_ReadThresholdsCmd(hHandle, bDevNumber, &m_stDrivesInfo))
        goto Failed;

    for (bIndex = 0; bIndex < m_stDrivesInfo.m_ucSmartValues; ++bIndex)
    {
        SMART_IDToText(m_stDrivesInfo.m_stSmartInfo[bIndex].bAttribId,
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

        if (!lpEnumProc(&Result)) break;
    }

    if (!hSmartHandle) SMART_Close(hHandle);
    return TRUE;

Failed:
    if (!hSmartHandle) SMART_Close(hHandle);
    return FALSE;
}

DWORD
SMART_GetHDDTemperature(HANDLE hSmartHandle,
                        BYTE bDevNumber)
{
    SMART_DRIVE_INFO m_stDrivesInfo;
    BYTE bIndex;
    HANDLE hHandle;

    if (!hSmartHandle)
    {
        hHandle = SMART_Open(bDevNumber);
        if (hHandle == INVALID_HANDLE_VALUE)
            return 0;
    }
    else
    {
        hHandle = hSmartHandle;
    }

    if (!SMART_Enable(hHandle, bDevNumber))
        goto Failed;

    if (!SMART_ReadAttributesCmd(hHandle, bDevNumber, &m_stDrivesInfo))
        goto Failed;

    if (!SMART_ReadThresholdsCmd(hHandle, bDevNumber, &m_stDrivesInfo))
        goto Failed;

    for (bIndex = 0; bIndex < m_stDrivesInfo.m_ucSmartValues; ++bIndex)
    {
        if (m_stDrivesInfo.m_stSmartInfo[bIndex].bAttribId == 0xC2 ||
            m_stDrivesInfo.m_stSmartInfo[bIndex].bAttribId == 0xe7)
        {
            if (!hSmartHandle) SMART_Close(hHandle);
            return m_stDrivesInfo.m_stSmartInfo[bIndex].m_dwAttribValue;
        }
    }

Failed:
    if (!hSmartHandle) SMART_Close(hHandle);
    return 0;
}
