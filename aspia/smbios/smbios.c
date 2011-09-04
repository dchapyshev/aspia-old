/*
 * PROJECT:         Aspia
 * FILE:            aspia/smbios/smbios.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

/* System Management BIOS Specification:
   http://dmtf.org/standards/smbios
   Last version: 2.7.1
   Date: 1 Feb 2011
*/

#include "smbios_internal.h"

#include <tchar.h>
#include <strsafe.h>

#pragma warning(disable: 4996)

/* Function from misc.c */
BOOL GetBinaryFromRegistry(HKEY hRootKey, LPWSTR lpszPath, LPWSTR lpszKeyName, LPDWORD lpdwValue, DWORD dwSize);

PVOID
DRIVER_GetSMBIOSData(OUT DWORD* ReturnSize);

/* Definition for the GetSystemFirmwareTable function */
typedef BOOL (WINAPI *PGSFT)(DWORD, DWORD, PVOID, DWORD);

BOOL IsSmBIOSInitialized = FALSE;


BOOL
InitSmBIOSDataFromFirmwareTable(VOID)
{
    PGSFT GetSystemFirmwareTable;
    RAWSMBIOSDATA Data = {0};
    HINSTANCE hDLL;
    DWORD dwSize;
    UINT Ret;

    hDLL = LoadLibrary(_T("KERNEL32.DLL"));
    if (!hDLL) return FALSE;

    GetSystemFirmwareTable = (PGSFT)GetProcAddress(hDLL, "GetSystemFirmwareTable");
    if (!GetSystemFirmwareTable)
    {
        FreeLibrary(hDLL);
        return FALSE;
    }

    ZeroMemory(&SmbiosTableData, sizeof(SmbiosTableData));

    dwSize = sizeof(Data);

    Ret = GetSystemFirmwareTable('RSMB', 'PCAF', &Data, dwSize);
    if (!Ret)
    {
        FreeLibrary(hDLL);
        return FALSE;
    }

    CopyMemory(SmbiosTableData, Data.SMBIOSTableData, sizeof(SmbiosTableData));
    FreeLibrary(hDLL);

    return TRUE;
}

BOOL
InitSmBIOSData(VOID)
{
    PSMBIOS_ENTRY Entry;
    char *buf;
    DWORD dwSize;

    if (InitSmBIOSDataFromFirmwareTable())
    {
        IsSmBIOSInitialized = TRUE;
        return TRUE;
    }

    buf = DRIVER_GetSMBIOSData(&dwSize);
    if (!buf)
    {
        return FALSE;
    }
    Entry = (PSMBIOS_ENTRY)buf;

    CopyMemory(SmbiosTableData, buf + sizeof(SMBIOS_ENTRY), Entry->StructureTableLength);

    VirtualFree(buf, 0, MEM_RELEASE);

    IsSmBIOSInitialized = TRUE;
    return TRUE;
}

static SIZE_T
GetFormattedSectionLength(SIZE_T Index)
{
    if (SmbiosTableData[Index] > 127)
        return 0;

    return SmbiosTableData[Index + 1];
}

static SIZE_T
GetUnformattedSectionLength(SIZE_T Index)
{
    SIZE_T UnformattedSectionLength = SmbiosTableData[Index + 1];
    SIZE_T StartPoint = Index;

    Index += UnformattedSectionLength;

    while (Index < MAX_DATA - 1)
    {
        if ((SmbiosTableData[Index] == 0) && (SmbiosTableData[Index + 1] == 0))
        {
            return Index - UnformattedSectionLength + 2 - StartPoint;
        }
        ++Index;
    }

    return 0;
}

static BOOL
GetNextDataByType(SIZE_T Type, LPVOID lpBuffer, SIZE_T *Length, BOOL Reset)
{
    SIZE_T FormattedSectionLength = 0;
    SIZE_T UnformattedSectionLength = 0;
    SIZE_T CurrentTableLength = 0;
    static SIZE_T Index;

    if (Reset) Index = 0;

    for (;;)
    {
        FormattedSectionLength = GetFormattedSectionLength(Index);
        UnformattedSectionLength = GetUnformattedSectionLength(Index);
        CurrentTableLength = FormattedSectionLength + UnformattedSectionLength;

        if (SmbiosTableData[Index] == Type)
        {
            if (CurrentTableLength < *Length)
            {
                CopyMemory(lpBuffer, &(SmbiosTableData[Index]), CurrentTableLength);
                *Length = CurrentTableLength;
            }
            else break;

            Index += CurrentTableLength;
            return TRUE;
        }
        
        Index += CurrentTableLength;

        if (CurrentTableLength == 0) break;
    }
    *Length = 0;

    return FALSE;
}

static VOID
GetStringResourceByID(SIZE_T TargetId, UCHAR *InBuf, WCHAR *OutString)
{
    CHAR ArrayStringEntries[MAX_STRING_TABLE][MAX_STRING];
    SIZE_T Index = (SIZE_T)*(InBuf + 1);
    SIZE_T StillMoreData = *(InBuf + Index) + *(InBuf + Index + 1);
    SIZE_T StartThisString;
    SIZE_T Id = 0;
    SIZE_T Len;

    while (StillMoreData)
    {
        StartThisString = Index;

        while (*(InBuf + Index) != 0) ++Index;

        Len = Index - StartThisString;
        ++Index;

        if (Len > 0)
        {
            CopyMemory(ArrayStringEntries[Id], InBuf + StartThisString, Len + 1);
        }
        ++Id;
        StillMoreData = *(InBuf + Index) + *(InBuf + Index + 1);
    }

    TargetId -= 1;

    if (TargetId > Id) *(OutString) = 0;
    else
    {
        mbstowcs(OutString,
                 ArrayStringEntries[TargetId],
                 MAX_STRING * sizeof(WCHAR));    
    }
}

VOID
SMBIOS_FormFactorToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (FormFactorList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, FormFactorList[Index].lpszString);
            return;
        }
    }
    while (FormFactorList[++Index].dwValue != 0);
}

VOID
SMBIOS_MemoryDeviceToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (MemDevicesList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, MemDevicesList[Index].lpszString);
            return;
        }
    }
    while (MemDevicesList[++Index].dwValue != 0);
}

BOOL
SMBIOS_EnumMemoryDevices(SMBIOS_MEMENUMPROC lpMemEnumProc)
{
    SMBIOS_MEMINFO MemInfo = {0};
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR* pBuf = Buf;
    SIZE_T Count = 0;
    BOOL IsFound;
    SIZE_T Len;

    do 
    {
        ZeroMemory(&MemInfo, sizeof(SMBIOS_MEMINFO));

        Len = MAX_DATA;
        IsFound = FALSE;

        if (GetNextDataByType(MEMORY_DEVICES_INFO, &Buf, &Len, ((Count < 1) ? TRUE : FALSE)))
        {
            IsFound = TRUE;
            ++Count;
        }

        if (IsFound)
        {
            GetStringResourceByID(Buf[0x11], pBuf,
                                  MemInfo.Bank);
            CopyMemory(&MemInfo.DataWidth, pBuf + 0x0A, 2);
            GetStringResourceByID(Buf[0x10], pBuf,
                                  MemInfo.DeviceLocator);
            MemInfo.FormFactor = Buf[0x0E];
            GetStringResourceByID(Buf[0x17], pBuf,
                                  MemInfo.Manufactuer);
            MemInfo.MemoryType = Buf[0x12];
            GetStringResourceByID(Buf[0x1A], pBuf,
                                  MemInfo.PartNumber);
            GetStringResourceByID(Buf[0x18], pBuf,
                                  MemInfo.SerialNumber);
            CopyMemory(&MemInfo.Size, pBuf + 0x0C, 2);
            CopyMemory(&MemInfo.Speed, pBuf + 0x15, 2);
            CopyMemory(&MemInfo.TotalWidth, pBuf + 0x08, 2);

            if (!lpMemEnumProc(MemInfo)) break;
        }
    }
    while (IsFound);

    if (Count < 1) return FALSE;

    return TRUE;
}

VOID
SMBIOS_SramToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (SramTypeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, SramTypeList[Index].lpszString);
            return;
        }
    }
    while (SramTypeList[++Index].dwValue != 0);
}

BOOL
SMBIOS_EnumCacheDevices(SMBIOS_CACHEENUMPROC lpCacheEnumProc)
{
    SMBIOS_CACHEINFO CacheInfo = {0};
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR* pBuf = Buf;
    SIZE_T Count = 0;
    BOOL IsFound;
    SIZE_T Len;

    do
    {
        Len = MAX_DATA;
        IsFound = FALSE;

        if (GetNextDataByType(CACHE_INFO, &Buf, &Len, (Count < 1) ? TRUE : FALSE))
        {
            IsFound = TRUE;
            ++Count;
        }

        if (IsFound)
        {
            CopyMemory(&CacheInfo.Configuration, pBuf + 0x05, 2);
            GetStringResourceByID(Buf[0x04], pBuf,
                                  CacheInfo.SocketDesignation);
            CopyMemory(&CacheInfo.MaxCacheSize, pBuf + 0x07, 2);
            CopyMemory(&CacheInfo.InstalledSize, pBuf + 0x09, 2);
            CopyMemory(&CacheInfo.SupportedSramType, pBuf + 0x0B, 2);
            CopyMemory(&CacheInfo.CurrentSramType, pBuf + 0x0D, 2);

            if (!lpCacheEnumProc(CacheInfo)) break;
        }
    }
    while (IsFound);

    if (Count < 1) return FALSE;

    return TRUE;
}

VOID
SMBIOS_ProcessorTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (ProcessorTypeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, ProcessorTypeList[Index].lpszString);
            return;
        }
    }
    while (ProcessorTypeList[++Index].dwValue != 0);
}

VOID
SMBIOS_ProcessorFamilyToText(USHORT Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (ProcessorFamilyList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, ProcessorFamilyList[Index].lpszString);
            return;
        }
    }
    while (ProcessorFamilyList[++Index].dwValue != 0);
}

VOID
SMBIOS_ProcessorStatusToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    UCHAR CpuStatus = Form & CPU_STATUS_MASK;
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (ProcessorStatusList[Index].dwValue == CpuStatus)
        {
            StringCbCopy(lpszText, Size, ProcessorStatusList[Index].lpszString);
            return;
        }
    }
    while (ProcessorStatusList[++Index].dwValue != 0);
}

VOID
SMBIOS_ProcessorUpgradeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (ProcessorUpgradeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, ProcessorUpgradeList[Index].lpszString);
            return;
        }
    }
    while (ProcessorUpgradeList[++Index].dwValue != 0);
}

BOOL
SMBIOS_EnumProcessorDevices(SMBIOS_PROCESSORENUMPROC lpProcessorEnumProc)
{
    SMBIOS_PROCESSORINFO ProcessorInfo;
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR* pBuf = Buf;
    SIZE_T Count = 0;
    BOOL IsFound;
    SIZE_T Len;

    do 
    {
        ZeroMemory(&ProcessorInfo, sizeof(SMBIOS_PROCESSORINFO));

        Len = MAX_DATA;
        IsFound = FALSE;

        if (GetNextDataByType(PROCESSOR_INFO, &Buf, &Len, (Count < 1) ? TRUE : FALSE))
        {
            IsFound = TRUE;
            ++Count;
        }

        if (IsFound)
        {
            GetStringResourceByID(Buf[0x04], pBuf,
                                  ProcessorInfo.SocketDesignation);
            ProcessorInfo.ProcessorType = Buf[0x05];
            ProcessorInfo.ProcessorFamily = Buf[0x06];
            GetStringResourceByID(Buf[0x07], pBuf,
                                  ProcessorInfo.Manufacturer);
            GetStringResourceByID(Buf[0x10], pBuf,
                                  ProcessorInfo.Version);
            ProcessorInfo.Voltage = Buf[0x11];
            CopyMemory(&ProcessorInfo.ExternalClock, pBuf + 0x12, 2);
            CopyMemory(&ProcessorInfo.MaxSpeed, pBuf + 0x14, 2);
            CopyMemory(&ProcessorInfo.CurentSpeed, pBuf + 0x16, 2);
            ProcessorInfo.Status = Buf[0x18];
            ProcessorInfo.Upgrade = Buf[0x19];

            if (!lpProcessorEnumProc(ProcessorInfo)) break;
        }
    }
    while (IsFound);

    if (Count < 1) return FALSE;
    
    return TRUE;
}

BOOL
SMBIOS_GetBiosInformation(SMBIOS_BIOSINFO *BiosInfo)
{
    SIZE_T NumberOptionalBytes;
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR* pBuf = Buf;
    SIZE_T Len = MAX_DATA;
    SMBIOS_BIOSINFO Info = {0};

    if (GetNextDataByType(BIOS_INFO, &Buf, &Len, TRUE))
    {
        GetStringResourceByID(Buf[0x04], pBuf,
                              Info.Vendor);
        GetStringResourceByID(Buf[0x05], pBuf,
                              Info.BiosVersion);
        GetStringResourceByID(Buf[0x08], pBuf,
                              Info.BiosReleaseDate);
        Info.BiosRomSize = Buf[0x09];
        CopyMemory(&Info.BiosCharacteristics, &Buf[0x0A], 4);
        NumberOptionalBytes = Buf[0x01] - 0x12;
        CopyMemory(&Info.BiosExtension1, &Buf[0x12], NumberOptionalBytes);

        *BiosInfo = Info;
        return TRUE;
    }

    return FALSE;
}

VOID
SMBIOS_WakeupTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (WakeupTypeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, WakeupTypeList[Index].lpszString);
            return;
        }
    }
    while (WakeupTypeList[++Index].dwValue != 0);
}

BOOL
SMBIOS_GetSystemInformation(SMBIOS_SYSINFO *SysInfo)
{
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR* pBuf = Buf;
    SIZE_T Len = MAX_DATA;
    SMBIOS_SYSINFO Info = {0};

    if (GetNextDataByType(SYSTEM_INFO, &Buf, &Len, TRUE))
    {
        GetStringResourceByID(Buf[0x04], pBuf,
                              Info.Manufacturer);
        GetStringResourceByID(Buf[0x05], pBuf,
                              Info.ProductName);
        GetStringResourceByID(Buf[0x06], pBuf,
                              Info.Version);
        GetStringResourceByID(Buf[0x07], pBuf,
                              Info.SerialNumber);
        CopyMemory(&Info.Uuid, &Buf[0x08], 16);
        Info.Wakeup = Buf[0x18];

        *SysInfo = Info;

        return TRUE;
    }

    return FALSE;
}

VOID
SMBIOS_EnclStateToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (EnclStatusList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, EnclStatusList[Index].lpszString);
            return;
        }
    }
    while (EnclStatusList[++Index].dwValue != 0);
}

VOID
SMBIOS_EnclTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (EnclTypesList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, EnclTypesList[Index].lpszString);
            return;
        }
    }
    while (EnclTypesList[++Index].dwValue != 0);
}

VOID
SMBIOS_EnclSecStatusToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (EnclSecStatusList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, EnclSecStatusList[Index].lpszString);
            return;
        }
    }
    while (EnclSecStatusList[++Index].dwValue != 0);
}

BOOL
SMBIOS_EnumEnclosureInformation(SMBIOS_ENCLENUMPROC lpEnclEnumProc)
{
    SMBIOS_ENCLINFO Info;
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR* pBuf = Buf;
    SIZE_T Count = 0;
    BOOL IsFound;
    SIZE_T Len;

    do 
    {
        ZeroMemory(&Info, sizeof(SMBIOS_ENCLINFO));

        Len = MAX_DATA;
        IsFound = FALSE;

        if (GetNextDataByType(ENCLOSURE_INFO, &Buf, &Len, (Count < 1) ? TRUE : FALSE))
        {
            IsFound = TRUE;
            ++Count;
        }

        if (IsFound)
        {
            GetStringResourceByID(Buf[0x04], pBuf,
                                  Info.Manufacturer);
            Info.Type = Buf[0x05];
            GetStringResourceByID(Buf[0x06], pBuf,
                                  Info.Version);
            GetStringResourceByID(Buf[0x07], pBuf,
                                  Info.SerialNumber);
            Info.BootUpState = Buf[0x09];
            Info.PowerSupplyState = Buf[0x0a];
            Info.ThermalState = Buf[0x0b];
            Info.SecurityStatus = Buf[0x0c];

            if (!lpEnclEnumProc(Info)) break;
        }
    }
    while (IsFound);

    if (Count < 1) return FALSE;

    return TRUE;
}

BOOL
SMBIOS_GetBoardInformation(SMBIOS_BOARDINFO *BoardInfo)
{
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR* pBuf = Buf;
    SIZE_T Len = MAX_DATA;
    SMBIOS_BOARDINFO Info = {0};

    if (GetNextDataByType(BOARD_INFO, &Buf, &Len, TRUE))
    {
        GetStringResourceByID(Buf[0x04], pBuf,
                              Info.Manufacturer);
        GetStringResourceByID(Buf[0x05], pBuf,
                              Info.Product);
        GetStringResourceByID(Buf[0x06], pBuf,
                              Info.Version);
        GetStringResourceByID(Buf[0x07], pBuf,
                              Info.SerialNumber);

        *BoardInfo = Info;
        return TRUE;
    }

    return FALSE;
}

VOID
SMBIOS_SlotTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (SlotTypeList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, SlotTypeList[Index].lpszString);
            return;
        }
    }
    while (SlotTypeList[++Index].dwValue != 0);
}

VOID
SMBIOS_BusWidthToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (BusWidthList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, BusWidthList[Index].lpszString);
            return;
        }
    }
    while (BusWidthList[++Index].dwValue != 0);
}

VOID
SMBIOS_SlotLengthToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (SlotLengthList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, SlotLengthList[Index].lpszString);
            return;
        }
    }
    while (SlotLengthList[++Index].dwValue != 0);
}

BOOL
SMBIOS_EnumSlotsInformation(SMBIOS_SLOTENUMPROC lpSlotEnumProc)
{
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR *pBuf = Buf;
    SIZE_T Count = 0;
    SMBIOS_SLOTINFO Info;
    BOOL IsFound;
    SIZE_T Len;

    do 
    {
        ZeroMemory(&Info, sizeof(SMBIOS_SLOTINFO));

        Len = MAX_DATA;
        IsFound = FALSE;

        if (GetNextDataByType(SLOTS_INFO, &Buf, &Len, (Count < 1) ? TRUE : FALSE))
        {
            IsFound = TRUE;
            ++Count;
        }

        if (IsFound)
        {
            GetStringResourceByID(Buf[0x04], pBuf,
                                  Info.SlotDesignation);
            Info.SlotType = Buf[0x05];
            Info.SlotDataBusWidth = Buf[0x06];
            Info.SlotLength = Buf[0x08];

            if (!lpSlotEnumProc(Info)) break;
        }
    }
    while (IsFound);

    if (Count < 1) return FALSE;
    
    return TRUE;
}

VOID
SMBIOS_PortTypesToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (PortTypesList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, PortTypesList[Index].lpszString);
            return;
        }
    }
    while (PortTypesList[++Index].dwValue != 0);
}

VOID
SMBIOS_PortConnectorToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (PortConnectorList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, PortConnectorList[Index].lpszString);
            return;
        }
    }
    while (PortConnectorList[++Index].dwValue != 0);
}

BOOL
SMBIOS_EnumPortsInformation(SMBIOS_PORTENUMPROC lpPortEnumProc)
{
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR *pBuf = Buf;
    SIZE_T Count = 0;
    SMBIOS_PORTINFO Info;
    BOOL IsFound;
    SIZE_T Len;

    do 
    {
        ZeroMemory(&Info, sizeof(SMBIOS_PORTINFO));

        Len = MAX_DATA;
        IsFound = FALSE;

        if (GetNextDataByType(PORTS_INFO, &Buf, &Len, (Count < 1) ? TRUE : FALSE))
        {
            IsFound = TRUE;
            ++Count;
        }

        if (IsFound)
        {
            GetStringResourceByID(Buf[0x04], pBuf,
                                  Info.IntRefDesignation);
            Info.IntConnectorType = Buf[0x05];
            GetStringResourceByID(Buf[0x06], pBuf,
                                  Info.ExtRefDesignation);
            Info.ExtConnectorType = Buf[0x07];
            Info.PortType = Buf[0x08];

            if (!lpPortEnumProc(Info)) break;
        }
    }
    while (IsFound);

    if (Count < 1) return FALSE;
    
    return TRUE;
}

VOID
SMBIOS_OnboardDeviceTypeToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (OnboardDeviceTypesList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, OnboardDeviceTypesList[Index].lpszString);
            return;
        }
    }
    while (OnboardDeviceTypesList[++Index].dwValue != 0);
}

BOOL
SMBIOS_EnumOnboardInformation(SMBIOS_ONBOARDENUMPROC lpOnboardEnumProc)
{
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR* pBuf = Buf;
    SIZE_T Len = MAX_DATA;
    SMBIOS_ONBOARDINFO Info = {0};
    CHAR Count;
    UCHAR Type;

    if (GetNextDataByType(ONBOARD_INFO, &Buf, &Len, TRUE))
    {
        Count = (Buf[0x01] - 4) / 2;

        do
        {
            GetStringResourceByID(Buf[5 + 2 * (Count - 1)], pBuf,
                                  Info.Description);

            Type = Buf[4 + 2 * (Count - 1)];

            Info.DeviceStatus = (Type & 0x80) >> 0x6; /* Bit 7 */
            Info.DeviceType = Type & 0x7F; /* Bits 6:0 */

            if (!lpOnboardEnumProc(Info)) break;
        }
        while (--Count < 0);

        return TRUE;
    }

    return FALSE;
}

VOID
SMBIOS_BatteryChemistryToText(UCHAR Form, LPWSTR lpszText, SIZE_T Size)
{
    SIZE_T Index = 0;

    if (lpszText) lpszText[0] = 0;
    do
    {
        if (BatteryDeviceChemistryList[Index].dwValue == Form)
        {
            StringCbCopy(lpszText, Size, BatteryDeviceChemistryList[Index].lpszString);
            return;
        }
    }
    while (BatteryDeviceChemistryList[++Index].dwValue != 0);
}

BOOL
SMBIOS_EnumBatteryInformation(SMBIOS_BATTERYENUMPROC lpBatteryEnumProc)
{
    UCHAR Buf[MAX_DATA] = {0};
    UCHAR *pBuf = Buf;
    SIZE_T Count = 0;
    SMBIOS_BATTERYINFO Info;
    BOOL IsFound;
    SIZE_T Len;

    do 
    {
        ZeroMemory(&Info, sizeof(SMBIOS_BATTERYINFO));

        Len = MAX_DATA;
        IsFound = FALSE;

        if (GetNextDataByType(BATTERY_INFO, &Buf, &Len, (Count < 1) ? TRUE : FALSE))
        {
            IsFound = TRUE;
            ++Count;
        }

        if (IsFound)
        {
            GetStringResourceByID(Buf[0x04], pBuf,
                                  Info.Location);
            GetStringResourceByID(Buf[0x05], pBuf,
                                  Info.Manufacturer);
            GetStringResourceByID(Buf[0x06], pBuf,
                                  Info.MenufDate);
            GetStringResourceByID(Buf[0x07], pBuf,
                                  Info.SerialNumber);
            GetStringResourceByID(Buf[0x08], pBuf,
                                  Info.DeviceName);
            Info.DeviceChemistry = Buf[0x09];
            Info.DesignCapacity = Buf[0x0A];
            Info.DesignVoltage = Buf[0x0C];
            GetStringResourceByID(Buf[0x0E], pBuf,
                                  Info.SBDSVersionNumber);
            Info.MaxErrorInData = Buf[0x0F];
            Info.SBDSSerialNumber = Buf[0x10];
            Info.SBDSManufDate = Buf[0x12];
            GetStringResourceByID(Buf[0x14], pBuf,
                                  Info.SBDSDeviceChemistry);
            Info.DesignCapacityMultiplier = Buf[0x15];

            if (!lpBatteryEnumProc(Info)) break;
        }
    }
    while (IsFound);

    if (Count < 1) return FALSE;
    
    return TRUE;
}

BOOL
IsSmBiosWorks(VOID)
{
    return IsSmBIOSInitialized;
}
