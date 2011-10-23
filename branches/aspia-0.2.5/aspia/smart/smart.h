/*
 * PROJECT:         Aspia
 * FILE:            aspia/smart/smart.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#define INDEX_ATTRIB_INDEX       0
#define INDEX_ATTRIB_UNKNOWN1    1
#define INDEX_ATTRIB_UNKNOWN2    2
#define INDEX_ATTRIB_VALUE       3
#define INDEX_ATTRIB_WORST       4
#define INDEX_ATTRIB_RAW         5

typedef struct
{
    BYTE bDriverError;
    BYTE bIDEStatus;
    BYTE bReserved[2];
    DWORD dwReserved[2];
} SMART_DRIVERSTAT;

typedef struct
{
    DWORD cBufferSize;
    SMART_DRIVERSTAT DriverStatus;
    BYTE bBuffer[1];
} SMART_ATAOUTPARAM;

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

typedef struct
{
    BYTE m_ucAttribIndex;
    DWORD m_dwAttribValue;
    BYTE m_ucValue;
    BYTE m_ucWorst;
    DWORD m_dwThreshold;
    BYTE bAttribId;
} SMART_INFO;

typedef struct
{
    BOOL m_bCritical;
    BYTE m_ucAttribId;
    LPWSTR m_csAttribName;
    LPWSTR m_csAttribDetails;
} SMART_DETAILS;

typedef struct
{
    IDSECTOR m_stInfo;
    SMART_INFO m_stSmartInfo[256];
    BYTE m_ucSmartValues;
    BYTE m_ucDriveIndex;
} SMART_DRIVE_INFO;

typedef struct
{
    DWORD dwAttrID;
    DWORD dwAttrValue;
    DWORD dwWarrantyThreshold;
    DWORD dwWorstValue;
    WCHAR szName[256];
    BYTE bValue;
} SMART_RESULT;

typedef BOOL (CALLBACK *SMART_ENUMDATAPROC)(SMART_RESULT *Result);

BOOL SMART_EnumData(HANDLE hSmartHandle, BYTE bDevNumber, SMART_ENUMDATAPROC lpEnumProc);
BOOL SMART_ReadDriveInformation(HANDLE hHandle, BYTE bDevNumber, IDSECTOR *Info);
DWORD SMART_GetHDDTemperature(HANDLE hSmartHandle, BYTE bDevNumber);
HANDLE SMART_Open(BYTE bDevNumber);
BOOL SMART_Close(HANDLE hHandle);
