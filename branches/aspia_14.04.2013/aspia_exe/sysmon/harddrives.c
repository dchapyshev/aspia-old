/*
 * PROJECT:         Aspia System Monitor
 * FILE:            aspia_exe/sysmon/harddrives.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <aspia.h>
#include "sysmon.h"


typedef struct _HDD_DRIVES_
{
    INT ItemIndex;
    BYTE Index;
    HANDLE hDrive;
    WCHAR szName[MAX_STR_LEN];
    INT MaxTemp;
    INT MinTemp;
    struct _HDD_DRIVES_ *Next;
} HDD_DRIVES, *PHDD_DRIVES;

static CRITICAL_SECTION HardDrivesCriticalSection;
static PHDD_DRIVES HddDrivesInfo = NULL;
static BYTE HddSensorsCount = 0;


BOOL
InitHardDrivesSensors(VOID)
{
    PHDD_DRIVES HddDrives;
    IDSECTOR DriveInfo;
    INT Index, Temperature;

    HddSensorsCount = 0;

    HddDrivesInfo = (PHDD_DRIVES)Alloc(sizeof(HDD_DRIVES));
    if (!HddDrivesInfo) return FALSE;

    HddDrives = HddDrivesInfo;

    for (Index = 0; Index <= MAX_HDD_COUNT; Index++)
    {
        HddDrives->hDrive = OpenSmart(Index);
        if (HddDrives->hDrive == INVALID_HANDLE_VALUE)
            continue;

        if (!ReadSmartInfo(HddDrives->hDrive, Index, &DriveInfo))
            continue;

        ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                        sizeof(DriveInfo.sModelNumber));
        StringCbPrintf(HddDrives->szName, sizeof(HddDrives->szName),
                       L"%S", DriveInfo.sModelNumber);
        ChopSpaces(HddDrives->szName, sizeof(HddDrives->szName));

        HddDrives->Index = Index;
        HddDrives->ItemIndex = -1;

        Temperature = GetSmartTemperature(HddDrives->hDrive, Index);
        HddDrives->MaxTemp = HddDrives->MinTemp = Temperature;

        if (HddDrives->MaxTemp == 0) continue;

        ++HddSensorsCount;

        HddDrives->Next = (PHDD_DRIVES)Alloc(sizeof(HDD_DRIVES));
        HddDrives = HddDrives->Next;
        if (!HddDrives) break;
    }

    if (HddSensorsCount)
    {
        InitializeCriticalSection(&HardDrivesCriticalSection);
        return TRUE;
    }

    return FALSE;
}

VOID
EnumHardDrivesSensors(PHARDDRIVES_SENSORS_ENUMPROC lpEnumProc)
{
    BYTE Count = HddSensorsCount;
    PHDD_DRIVES HddDrives;
    INT Temperature;

    if (!TryEnterCriticalSection(&HardDrivesCriticalSection))
        return;

    HddDrives = HddDrivesInfo;

    while (Count)
    {
        Temperature = GetSmartTemperature(HddDrives->hDrive, HddDrives->Index);

        if (Temperature > HddDrives->MaxTemp)
            HddDrives->MaxTemp = Temperature;

        if (Temperature < HddDrives->MinTemp)
            HddDrives->MinTemp = Temperature;

        HddDrives->ItemIndex = lpEnumProc(HddDrives->ItemIndex, HddDrives->szName, Temperature,
                       HddDrives->MaxTemp, HddDrives->MinTemp);

        HddDrives = HddDrives->Next;

        --Count;
    }

    LeaveCriticalSection(&HardDrivesCriticalSection);
}

VOID
FreeHardDrivesSensors(VOID)
{
    PHDD_DRIVES HddDrives, Temp;

    HddDrives = HddDrivesInfo;

    while (HddSensorsCount)
    {
        CloseSmart(HddDrives->hDrive);

        Temp = HddDrives->Next;
        Free(HddDrives);
        HddDrives = Temp;

        --HddSensorsCount;
    }

    DeleteCriticalSection(&HardDrivesCriticalSection);
}
