/*
 * PROJECT:         Aspia
 * FILE:            aspia/smart/smart.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <windows.h>
#include <wchar.h>
#include <strsafe.h>
#include "driver.h"


#define SMART_GET_VERSION        CTL_CODE(IOCTL_DISK_BASE, 0x0020, METHOD_BUFFERED, FILE_READ_ACCESS)
#define SMART_SEND_DRIVE_COMMAND CTL_CODE(IOCTL_DISK_BASE, 0x0021, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define SMART_RCV_DRIVE_DATA     CTL_CODE(IOCTL_DISK_BASE, 0x0022, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define DRIVE_HEAD_REG    0xA0

#define READ_ATTRIBUTE_BUFFER_SIZE  512
#define IDENTIFY_BUFFER_SIZE        512
#define READ_THRESHOLD_BUFFER_SIZE  512
#define SMART_LOG_SECTOR_SIZE       512


HANDLE
OpenSmart(BYTE bDevNumber)
{
    WCHAR szPath[MAX_PATH];

    StringCbPrintf(szPath,
                   sizeof(szPath),
                   L"\\\\.\\PhysicalDrive%d",
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
CloseSmart(HANDLE hHandle)
{
    return CloseHandle(hHandle);
}

BOOL
GetSmartVersion(HANDLE hHandle, GETVERSIONINPARAMS *pVersion)
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
EnableSmart(HANDLE hHandle, BYTE bDevNumber)
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
ReadSmartInfo(HANDLE hHandle, BYTE bDevNumber, IDSECTOR *Info)
{
    SENDCMDINPARAMS CmdIn = {0};
    DWORD cbBytesReturned;
    char szOut[IDENTIFY_BUFFER_SIZE + 16];
    BOOL bResult;

    if (!Info) return FALSE;

    CmdIn.cBufferSize = IDENTIFY_BUFFER_SIZE;
    CmdIn.irDriveRegs.bSectorCountReg = 1;
    CmdIn.irDriveRegs.bSectorNumberReg = 1;
    CmdIn.irDriveRegs.bDriveHeadReg = 0xA0 | ((bDevNumber & 1) << 4);
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

    if (!bResult)
    {
        DebugTrace(L"DeviceIoControl() failed. Error code = %d", GetLastError());
        return FALSE;
    }

    CopyMemory(Info, szOut + 16, sizeof(IDSECTOR));

    return TRUE;
}

BOOL
ReadSmartAttributes(HANDLE hHandle, BYTE bDevNumber, SMART_DRIVE_INFO *Info)
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
ReadSmartThresholds(HANDLE hHandle, BYTE bDriveNum, SMART_DRIVE_INFO *Info)
{
    SMART_READ_DATA_OUTDATA CmdOut = {0};
    SMART_THRESHOLD Threshold[30];
    SMART_INFO *pSmartValues;
    SENDCMDINPARAMS CmdIn = {0};
    DWORD cbBytesReturned;
    BYTE Count = 0;
    UCHAR uIndex;
    BOOL bResult;

    CmdIn.cBufferSize                  = READ_THRESHOLD_BUFFER_SIZE;
    CmdIn.irDriveRegs.bFeaturesReg     = READ_THRESHOLDS;
    CmdIn.irDriveRegs.bSectorCountReg  = 1;
    CmdIn.irDriveRegs.bSectorNumberReg = 1;
    CmdIn.irDriveRegs.bCylLowReg       = SMART_CYL_LOW;
    CmdIn.irDriveRegs.bCylHighReg      = SMART_CYL_HI;
    CmdIn.irDriveRegs.bDriveHeadReg    = DRIVE_HEAD_REG;
    CmdIn.irDriveRegs.bCommandReg      = SMART_CMD;
    CmdIn.bDriveNumber                 = bDriveNum;

    bResult = DeviceIoControl(hHandle,
                              SMART_RCV_DRIVE_DATA,
                              &CmdIn,
                              sizeof(SENDCMDINPARAMS),
                              &CmdOut, sizeof(SMART_READ_DATA_OUTDATA),
                              &cbBytesReturned,
                              NULL);
    if (!bResult || !Info) return FALSE;

    for (uIndex = 0; uIndex < 30; ++uIndex)
    {
        memcpy(&(Threshold[uIndex]),
               &(CmdOut.Data[uIndex * sizeof(SMART_THRESHOLD) + 1]),
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

typedef struct
{
    DWORD dwID;
    BOOL bCritical;
    LPWSTR lpszName;
} INFO_STRUCT;

const INFO_STRUCT SmartAttribList[] =
{
    { 0x01, TRUE, L"Raw Read Error Rate" },
    { 0x02, FALSE, L"Throughput Performance" },
    { 0x03, FALSE, L"Spin Up Time" },
    { 0x04, FALSE, L"Start/Stop Count" },
    { 0x05, TRUE, L"Reallocated Sectors Count" },
    { 0x06, FALSE, L"Read Channel Margin" },
    { 0x07, FALSE, L"Seek Error Rate" },
    { 0x08, FALSE, L"Seek Time Performance" },
    { 0x09, FALSE, L"Power-On Hours" },
    { 0x0A, FALSE, L"Spin Retry Count" },
    { 0x0B, FALSE, L"Recalibration Retries" },
    { 0x0C, FALSE, L"Device Power Cycle Count" },
    { 0x0D, FALSE, L"Soft Read Error Rate" },
    { 0xC1, FALSE, L"Load/Unload Cycle Count" },
    { 0xC2, FALSE, L"Temperature" },
    { 0xC4, TRUE, L"Reallocation Event Count" },
    { 0xC5, TRUE, L"Current Pending Sector Count" },
    { 0xC6, TRUE, L"Uncorrectable Sector Count" },
    { 0xC7, FALSE, L"UltraDMA CRC Error Count" },
    { 0xC8, FALSE, L"Write Error Rate" },
    { 0xC9, FALSE, L"Soft read error rate" },
    { 0xCA, FALSE, L"Data Address Mark errors" },
    { 0xCB, FALSE, L"Run out cancel" },
    { 0xCC, FALSE, L"Soft ECC correction" },
    { 0xCD, FALSE, L"Thermal asperity rate (TAR)" },
    { 0xCE, FALSE, L"Flying height" },
    { 0xCF, FALSE, L"Spin high current" },
    { 0xD0, FALSE, L"Spin buzz" },
    { 0xD1, FALSE, L"Offline seek performance" },
    { 0xDC, TRUE, L"Disk Shift" },
    { 0xDD, FALSE, L"G-Sense Error Rate" },
    { 0xDE, FALSE, L"Loaded Hours" },
    { 0xDF, FALSE, L"Load/Unload Retry Count" },
    { 0xE0, FALSE, L"Load Friction" },
    { 0xE2, FALSE, L"Load-in Time" },
    { 0xE3, FALSE, L"Torque Amplification Count" },
    { 0xE4, FALSE, L"Power-Off Retract Count" },
    { 0xE6, FALSE, L"GMR Head Amplitude" },
    { 0xE7, FALSE, L"Temperature" },
    { 0xF0, FALSE, L"Head flying hours" },
    { 0xFA, FALSE, L"Read error retry rate" },
    { 0xFE, FALSE, L"Free Fall Event Count" },
    { 0 }
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
EnumSmartData(HANDLE hSmartHandle, BYTE bDevNumber, SMART_ENUMDATAPROC lpEnumProc)
{
    GETVERSIONINPARAMS Version;
    SMART_DRIVE_INFO m_stDrivesInfo;
    SMART_RESULT Result;
    HANDLE hHandle;
    BYTE bIndex;

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

    if (!GetSmartVersion(hHandle, &Version))
        goto Failed;

    if (!EnableSmart(hHandle, bDevNumber))
        goto Failed;

    if (!ReadSmartAttributes(hHandle, bDevNumber, &m_stDrivesInfo))
        goto Failed;

    if (!ReadSmartThresholds(hHandle, bDevNumber, &m_stDrivesInfo))
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

    if (!hSmartHandle) CloseSmart(hHandle);
    return TRUE;

Failed:
    if (!hSmartHandle) CloseSmart(hHandle);
    return FALSE;
}

DWORD
GetSmartTemperature(HANDLE hSmartHandle,
                          BYTE bDevNumber)
{
    SMART_DRIVE_INFO m_stDrivesInfo;
    BYTE bIndex;
    HANDLE hHandle;

    if (!hSmartHandle)
    {
        hHandle = OpenSmart(bDevNumber);
        if (hHandle == INVALID_HANDLE_VALUE)
            return 0;
    }
    else
    {
        hHandle = hSmartHandle;
    }

    if (!EnableSmart(hHandle, bDevNumber))
        goto Failed;

    if (!ReadSmartAttributes(hHandle, bDevNumber, &m_stDrivesInfo))
        goto Failed;

    if (!ReadSmartThresholds(hHandle, bDevNumber, &m_stDrivesInfo))
        goto Failed;

    for (bIndex = 0; bIndex < m_stDrivesInfo.m_ucSmartValues; ++bIndex)
    {
        if (m_stDrivesInfo.m_stSmartInfo[bIndex].bAttribId == 0xC2 ||
            m_stDrivesInfo.m_stSmartInfo[bIndex].bAttribId == 0xe7)
        {
            if (!hSmartHandle) CloseSmart(hHandle);
            return m_stDrivesInfo.m_stSmartInfo[bIndex].m_dwAttribValue;
        }
    }

Failed:
    if (!hSmartHandle) CloseSmart(hHandle);
    return 0;
}

BOOL
GetSmartDiskGeometry(BYTE bDevNumber, DISK_GEOMETRY *DiskGeometry)
{
    WCHAR szPath[MAX_PATH];
    HANDLE hDevice;
    DWORD temp;
    BOOL Result;

    StringCbPrintf(szPath,
                   sizeof(szPath),
                   L"\\\\.\\PhysicalDrive%d",
                   bDevNumber);

    hDevice = CreateFile(szPath, 0,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    Result = DeviceIoControl(hDevice,
                             IOCTL_DISK_GET_DRIVE_GEOMETRY,
                             NULL, 0,
                             DiskGeometry, sizeof(*DiskGeometry),
                             &temp,
                             NULL);

    CloseHandle(hDevice);

    return Result;
}
