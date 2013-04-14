/*
 * PROJECT:         Aspia System Monitor
 * FILE:            aspia_exe/sysmon/sysmon.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#define MAX_HDD_COUNT 32

typedef BOOL (*PSENSOR_INIT_FUNCTION)(VOID);
typedef VOID (*PSENSOR_UPDATE_FUNCTION)(PVOID);
typedef VOID (*PSENSOR_FREE_FUNCTION)(VOID);

typedef struct
{
    BOOL IsEnabled;
    BOOL IsInitialized;
    PSENSOR_INIT_FUNCTION InitFunction;
    PSENSOR_UPDATE_FUNCTION UpdateFunction;
    PVOID EnumProcFunction;
    PSENSOR_FREE_FUNCTION FreeFunction;
} SENSORS_LIST;

typedef INT (CALLBACK *PHARDDRIVES_SENSORS_ENUMPROC)
    (INT ItemIndex, LPWSTR SensorName, INT Current, INT Max, INT Min);

typedef INT (CALLBACK *PINTELCPU_SENSORS_ENUMPROC)
    (INT ItemIndex, LPWSTR CpuName, INT CoreIndex, INT Current, INT Max, INT Min);

typedef INT (CALLBACK *PBATTERY_SENSORS_ENUMPROC)
    (INT ItemIndex, INT BatteryIndex, DWORD Current, DWORD Max, DWORD Min);

typedef INT (CALLBACK *PLPC_SENSORS_ENUMPROC)
    (INT ItemIndex, LPWSTR ChipType, LPWSTR SensorName,
     INT SensorType, INT Current, INT Max, INT Min);

/* harddrives.c */
BOOL InitHardDrivesSensors(VOID);
VOID EnumHardDrivesSensors(PHARDDRIVES_SENSORS_ENUMPROC lpEnumProc);
VOID FreeHardDrivesSensors(VOID);

/* intelcpu.c */
BOOL InitIntelCpuSensors(VOID);
VOID EnumIntelCpuSensors(PINTELCPU_SENSORS_ENUMPROC lpEnumProc);
VOID FreeIntelCpuSensors(VOID);

/* battery.c */
BOOL InitBatterySensors(VOID);
VOID EnumBatterySensors(PBATTERY_SENSORS_ENUMPROC lpEnumProc);
VOID FreeBatterySensors(VOID);
