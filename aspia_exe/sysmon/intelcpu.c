/*
 * PROJECT:         Aspia System Monitor
 * FILE:            aspia_exe/sysmon/intelcpu.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <aspia.h>
#include "sysmon.h"


typedef struct _INTEL_CPUS_
{
    INT ItemIndex;
    BYTE Index;
    WCHAR szName[MAX_STR_LEN];
    INT Tjmax;
    INT MaxTemp;
    INT MinTemp;
    struct _INTEL_CPUS_ *Next;
} INTEL_CPUS, *PINTEL_CPUS;

static CRITICAL_SECTION IntelCpuCriticalSection;
static PINTEL_CPUS IntelCpuInfo = NULL;
static INT CpuSensorsCount = 0;


INT
GetIntelCpuTemperature(BYTE Index, INT Tjmax)
{
    UINT64 Value;
    UINT Temp;

    if (ReadMsr(IA32_THERM_STATUS, Index, &Value))
    {
        Temp = ((Value >> 16) & 0xFF);
        if (Temp == 0) return 0;

        return Tjmax - Temp;
    }

    return 0;
}

BOOL
InitIntelCpuSensors(VOID)
{
    WCHAR szText[MAX_STR_LEN] = {0};
    PINTEL_CPUS IntelCpu;

    CpuSensorsCount = 0;

    GetCPUVendor(szText, sizeof(szText));

    if (wcscmp(szText, L"GenuineIntel") == 0)
    {
        INT Count = GetPhysicalProcessorsCount();

        IntelCpuInfo = (PINTEL_CPUS)Alloc(sizeof(INTEL_CPUS));
        if (!IntelCpuInfo) return FALSE;

        IntelCpu = IntelCpuInfo;

        while (Count)
        {
            if (!GetCPUName(IntelCpu->szName, sizeof(IntelCpu->szName)))
                continue;

            IntelCpu->Index     = Count - 1;
            IntelCpu->Tjmax     = GetTjmaxTemperature(IntelCpu->Index);
            IntelCpu->ItemIndex = -1;
            IntelCpu->MaxTemp = IntelCpu->MinTemp =
                GetIntelCpuTemperature(IntelCpu->Index, IntelCpu->Tjmax);

            if (--Count) ++CpuSensorsCount;

            IntelCpu->Next = (PINTEL_CPUS)Alloc(sizeof(INTEL_CPUS));
            IntelCpu = IntelCpu->Next;
            if (!IntelCpu) break;
        }
    }

    if (CpuSensorsCount)
    {
        InitializeCriticalSection(&IntelCpuCriticalSection);
        
        return TRUE;
    }

    return FALSE;
}

VOID
EnumIntelCpuSensors(PINTELCPU_SENSORS_ENUMPROC lpEnumProc)
{
    PINTEL_CPUS IntelCpu;
    INT Count = CpuSensorsCount;
    INT Temperature;

    if (!TryEnterCriticalSection(&IntelCpuCriticalSection))
        return;

    IntelCpu = IntelCpuInfo;

    while (Count >= 0)
    {
        Temperature = GetIntelCpuTemperature(Count, IntelCpu->Tjmax);

        if (Temperature > IntelCpu->MaxTemp)
            IntelCpu->MaxTemp = Temperature;

        if (Temperature < IntelCpu->MinTemp)
            IntelCpu->MinTemp = Temperature;

        IntelCpu->ItemIndex = lpEnumProc(IntelCpu->ItemIndex,
                                         IntelCpu->szName,
                                         IntelCpu->Index,
                                         Temperature,
                                         IntelCpu->MaxTemp,
                                         IntelCpu->MinTemp);

        IntelCpu = IntelCpu->Next;
        --Count;
    }

    LeaveCriticalSection(&IntelCpuCriticalSection);
}

VOID
FreeIntelCpuSensors(VOID)
{
    PINTEL_CPUS IntelCpu, Temp;

    IntelCpu = IntelCpuInfo;

    while (CpuSensorsCount)
    {
        Temp = IntelCpu->Next;
        Free(IntelCpu);
        IntelCpu = Temp;

        --CpuSensorsCount;
    }

    DeleteCriticalSection(&IntelCpuCriticalSection);
}