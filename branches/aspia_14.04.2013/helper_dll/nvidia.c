/*
 * PROJECT:         Aspia (Helper DLL)
 * FILE:            helper_dll/nvidia.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <windows.h>
#include <wchar.h>

#include "helper_dll.h"
#include "nvidia.h"
#include "aspia.h"

typedef INT *(*nvapi_QueryInterface)(UINT Offset);

static nvapi_QueryInterface             pnvapi_QueryInterface             = NULL;
static NvAPI_Initialize                 pNvAPI_Initialize                 = NULL;
static NvAPI_GPU_GetFullName            pNvAPI_GPU_GetFullName            = NULL;
static NvAPI_GetInterfaceVersionString  pNvAPI_GetInterfaceVersionString  = NULL;
static NvAPI_GPU_GetThermalSettings     pNvAPI_GPU_GetThermalSettings     = NULL;
static NvAPI_EnumNvidiaDisplayHandle    pNvAPI_EnumNvidiaDisplayHandle    = NULL;
static NvAPI_GetPhysicalGPUsFromDisplay pNvAPI_GetPhysicalGPUsFromDisplay = NULL;
static NvAPI_EnumPhysicalGPUs           pNvAPI_EnumPhysicalGPUs           = NULL;
static NvAPI_GPU_GetTachReading         pNvAPI_GPU_GetTachReading         = NULL;
static NvAPI_GetDisplayDriverVersion    pNvAPI_GetDisplayDriverVersion    = NULL;

static BOOL IsApiInitialized = FALSE;

static HMODULE hNvApiDll = NULL;


BOOL
IsNvidiaApiInitialized(VOID)
{
    return IsApiInitialized;
}

VOID
FreeNvidiaApi(VOID)
{
    if (hNvApiDll) FreeLibrary(hNvApiDll);
    hNvApiDll = NULL;

    pNvAPI_Initialize                 = NULL;
    pNvAPI_GPU_GetFullName            = NULL;
    pNvAPI_GetInterfaceVersionString  = NULL;
    pNvAPI_GPU_GetThermalSettings     = NULL;
    pNvAPI_EnumNvidiaDisplayHandle    = NULL;
    pNvAPI_GetPhysicalGPUsFromDisplay = NULL;
    pNvAPI_EnumPhysicalGPUs           = NULL;
    pNvAPI_GPU_GetTachReading         = NULL;
    pNvAPI_GetDisplayDriverVersion    = NULL;

    IsApiInitialized = FALSE;
}

BOOL
InitNvidiaApi(VOID)
{
    if (hNvApiDll)
    {
        DebugTrace(L"Nvidia API is already initialized!");
        return TRUE;
    }

    hNvApiDll = LoadLibrary(L"nvapi.dll");
    if (!hNvApiDll)
    {
        DebugTrace(L"nvapi.dll not found!");
        goto Failed;
    }

    pnvapi_QueryInterface =
        (nvapi_QueryInterface)GetProcAddress(hNvApiDll, "nvapi_QueryInterface");
    if (!pnvapi_QueryInterface)
    {
        DebugTrace(L"nvapi_QueryInterface() not found in nvapi.dll!");
        goto Failed;
    }

    pNvAPI_Initialize =
        (NvAPI_Initialize)pnvapi_QueryInterface(0x0150E828);
    if (!pNvAPI_Initialize)
    {
        DebugTrace(L"NvAPI_Initialize() not found in nvapi.dll!");
        goto Failed;
    }

    pNvAPI_GPU_GetFullName =
        (NvAPI_GPU_GetFullName)pnvapi_QueryInterface(0xCEEE8E9F);
    if (!pNvAPI_GPU_GetFullName)
    {
        DebugTrace(L"NvAPI_GPU_GetFullName() not found in nvapi.dll!");
        goto Failed;
    }

    pNvAPI_GetInterfaceVersionString =
        (NvAPI_GetInterfaceVersionString)pnvapi_QueryInterface(0x01053FA5);
    if (!pNvAPI_GetInterfaceVersionString)
    {
        DebugTrace(L"NvAPI_GetInterfaceVersionString() not found in nvapi.dll!");
        goto Failed;
    }

    pNvAPI_GPU_GetThermalSettings =
        (NvAPI_GPU_GetThermalSettings)pnvapi_QueryInterface(0xE3640A56);
    if (!pNvAPI_GPU_GetThermalSettings)
    {
        DebugTrace(L"NvAPI_GPU_GetThermalSettings() not found in nvapi.dll!");
        goto Failed;
    }

    pNvAPI_EnumNvidiaDisplayHandle =
        (NvAPI_EnumNvidiaDisplayHandle)pnvapi_QueryInterface(0x9ABDD40D);
    if (!pNvAPI_EnumNvidiaDisplayHandle)
    {
        DebugTrace(L"NvAPI_EnumNvidiaDisplayHandle() not found in nvapi.dll!");
        goto Failed;
    }

    pNvAPI_GetPhysicalGPUsFromDisplay =
        (NvAPI_GetPhysicalGPUsFromDisplay)pnvapi_QueryInterface(0x34EF9506);
    if (!pNvAPI_GetPhysicalGPUsFromDisplay)
    {
        DebugTrace(L"NvAPI_GetPhysicalGPUsFromDisplay() not found in nvapi.dll!");
        goto Failed;
    }

    pNvAPI_EnumPhysicalGPUs =
        (NvAPI_EnumPhysicalGPUs)pnvapi_QueryInterface(0xE5AC921F);
    if (!pNvAPI_EnumPhysicalGPUs)
    {
        DebugTrace(L"NvAPI_EnumPhysicalGPUs() not found in nvapi.dll!");
        goto Failed;
    }

    pNvAPI_GPU_GetTachReading =
        (NvAPI_GPU_GetTachReading)pnvapi_QueryInterface(0x5F608315);
    if (!pNvAPI_GPU_GetTachReading)
    {
        DebugTrace(L"NvAPI_GPU_GetTachReading() not found in nvapi.dll!");
        goto Failed;
    }

    pNvAPI_GetDisplayDriverVersion =
        (NvAPI_GetDisplayDriverVersion)pnvapi_QueryInterface(0xF951A4D1);
    if (!pNvAPI_GetDisplayDriverVersion)
    {
        DebugTrace(L"NvAPI_GetDisplayDriverVersion() not found in nvapi.dll!");
        goto Failed;
    }

    if (pNvAPI_Initialize() != NVAPI_OK)
    {
        goto Failed;
    }

    IsApiInitialized = TRUE;

    return TRUE;

Failed:
    FreeNvidiaApi();
    return FALSE;
}

BOOL
EnumNvidiaGPUs(NVIDIA_GPU_ENUMPROC lpEnumProc)
{
    NvPhysicalGpuHandle NvHandles[NVAPI_MAX_PHYSICAL_GPUS];
    NV_GPU_THERMAL_SETTINGS_V1 TempSettings1;
    NV_GPU_THERMAL_SETTINGS_V2 TempSettings2;
    NvAPI_ShortString GpuName;
    NVIDIA_GPU_INFO GpuInfo = {0};
    ULONG FanSpeed, GpuCount, i, j;
    INT ErrorCode;

    if (!IsApiInitialized || !lpEnumProc) return FALSE;

    if (pNvAPI_EnumPhysicalGPUs(NvHandles, &GpuCount) != NVAPI_OK)
        return FALSE;

    if (GpuCount == 0) return FALSE;

    for (j = 0; j < GpuCount; j++)
    {
        /* Get GPU name */
        if (pNvAPI_GPU_GetFullName(NvHandles[j], GpuName) != NVAPI_OK)
        {
            StringCbCopy(GpuInfo.szName, sizeof(GpuInfo.szName),
                         L"Unknown NVIDIA GPU");
        }
        else
        {
            StringCbPrintf(GpuInfo.szName, sizeof(GpuInfo.szName),
                           L"NVIDIA %S", GpuName);
        }

        /* Get GPU temperature */
        TempSettings2.version = NV_GPU_THERMAL_SETTINGS_VER_2;
        TempSettings2.count = NVAPI_MAX_THERMAL_SENSORS_PER_GPU;

        ErrorCode = pNvAPI_GPU_GetThermalSettings(NvHandles[j],
                                                  NVAPI_THERMAL_TARGET_ALL,
                                                  &TempSettings2);

        if (ErrorCode == NVAPI_INCOMPATIBLE_STRUCT_VERSION)
        {
            TempSettings1.version = NV_GPU_THERMAL_SETTINGS_VER_1;
            TempSettings1.count = NVAPI_MAX_THERMAL_SENSORS_PER_GPU;

            ErrorCode = pNvAPI_GPU_GetThermalSettings(NvHandles[j],
                                                      NVAPI_THERMAL_TARGET_ALL,
                                                      &TempSettings1);

            if (ErrorCode == NVAPI_OK)
            {
                DebugTrace(L"Thermal settings structure version 1");

                for (i = 0; i < NVAPI_MAX_THERMAL_SENSORS_PER_GPU; i++)
                {
                    switch (TempSettings1.sensor[i].target)
                    {
                        case NVAPI_THERMAL_TARGET_GPU:
                            GpuInfo.GpuTemp.CurrentTemp =
                                TempSettings1.sensor[i].currentTemp;
                            GpuInfo.GpuTemp.DefaultMaxTemp =
                                TempSettings1.sensor[i].defaultMaxTemp;
                            GpuInfo.GpuTemp.DefaultMinTemp =
                                TempSettings1.sensor[i].defaultMinTemp;
                        break;

                        case NVAPI_THERMAL_TARGET_MEMORY:
                            GpuInfo.MemoryTemp.CurrentTemp =
                                TempSettings1.sensor[i].currentTemp;
                            GpuInfo.MemoryTemp.DefaultMaxTemp =
                                TempSettings1.sensor[i].defaultMaxTemp;
                            GpuInfo.MemoryTemp.DefaultMinTemp =
                                TempSettings1.sensor[i].defaultMinTemp;
                            break;

                        case NVAPI_THERMAL_TARGET_BOARD:
                            GpuInfo.BoardTemp.CurrentTemp =
                                TempSettings1.sensor[i].currentTemp;
                            GpuInfo.BoardTemp.DefaultMaxTemp =
                                TempSettings1.sensor[i].defaultMaxTemp;
                            GpuInfo.BoardTemp.DefaultMinTemp =
                                TempSettings1.sensor[i].defaultMinTemp;
                            break;
                    }
                }
            }
            else
            {
                DebugTrace(L"NvAPI_GPU_GetThermalSettings() failed! Error code = %d!", ErrorCode);
            }
        }
        else if (ErrorCode == NVAPI_OK)
        {
            DebugTrace(L"Thermal settings structure version 2");

            for (i = 0; i < NVAPI_MAX_THERMAL_SENSORS_PER_GPU; i++)
            {
                switch (TempSettings2.sensor[i].target)
                {
                    case NVAPI_THERMAL_TARGET_GPU:
                        GpuInfo.GpuTemp.CurrentTemp =
                            TempSettings2.sensor[i].currentTemp;
                        GpuInfo.GpuTemp.DefaultMaxTemp =
                            TempSettings2.sensor[i].defaultMaxTemp;
                        GpuInfo.GpuTemp.DefaultMinTemp =
                            TempSettings2.sensor[i].defaultMinTemp;
                        break;

                    case NVAPI_THERMAL_TARGET_MEMORY:
                        GpuInfo.MemoryTemp.CurrentTemp =
                            TempSettings2.sensor[i].currentTemp;
                        GpuInfo.MemoryTemp.DefaultMaxTemp =
                            TempSettings2.sensor[i].defaultMaxTemp;
                        GpuInfo.MemoryTemp.DefaultMinTemp =
                            TempSettings2.sensor[i].defaultMinTemp;
                        break;

                    case NVAPI_THERMAL_TARGET_POWER_SUPPLY:
                        GpuInfo.PowerSupplyTemp.CurrentTemp =
                            TempSettings2.sensor[i].currentTemp;
                        GpuInfo.PowerSupplyTemp.DefaultMaxTemp =
                            TempSettings2.sensor[i].defaultMaxTemp;
                        GpuInfo.PowerSupplyTemp.DefaultMinTemp =
                            TempSettings2.sensor[i].defaultMinTemp;
                        break;

                    case NVAPI_THERMAL_TARGET_BOARD:
                        GpuInfo.BoardTemp.CurrentTemp =
                            TempSettings2.sensor[i].currentTemp;
                        GpuInfo.BoardTemp.DefaultMaxTemp =
                            TempSettings2.sensor[i].defaultMaxTemp;
                        GpuInfo.BoardTemp.DefaultMinTemp =
                            TempSettings2.sensor[i].defaultMinTemp;
                        break;

                    case NVAPI_THERMAL_TARGET_VCD_BOARD:
                        GpuInfo.VcdBoardTemp.CurrentTemp =
                            TempSettings2.sensor[i].currentTemp;
                        GpuInfo.VcdBoardTemp.DefaultMaxTemp =
                            TempSettings2.sensor[i].defaultMaxTemp;
                        GpuInfo.VcdBoardTemp.DefaultMinTemp =
                            TempSettings2.sensor[i].defaultMinTemp;
                        break;

                    case NVAPI_THERMAL_TARGET_VCD_INLET:
                        GpuInfo.VcdInletTemp.CurrentTemp =
                            TempSettings2.sensor[i].currentTemp;
                        GpuInfo.VcdInletTemp.DefaultMaxTemp =
                            TempSettings2.sensor[i].defaultMaxTemp;
                        GpuInfo.VcdInletTemp.DefaultMinTemp =
                            TempSettings2.sensor[i].defaultMinTemp;
                        break;

                    case NVAPI_THERMAL_TARGET_VCD_OUTLET:
                        GpuInfo.VcdOutletTemp.CurrentTemp =
                            TempSettings2.sensor[i].currentTemp;
                        GpuInfo.VcdOutletTemp.DefaultMaxTemp =
                            TempSettings2.sensor[i].defaultMaxTemp;
                        GpuInfo.VcdOutletTemp.DefaultMinTemp =
                            TempSettings2.sensor[i].defaultMinTemp;
                        break;
                }
            }
        }
        else
        {
            DebugTrace(L"NvAPI_GPU_GetThermalSettings() failed! Error code = %d!", ErrorCode);
        }

        ErrorCode = pNvAPI_GPU_GetTachReading(NvHandles[j], &FanSpeed);
        if (ErrorCode == NVAPI_OK)
        {
            GpuInfo.FanSpeed = FanSpeed;
        }
        else
        {
            DebugTrace(L"NvAPI_GPU_GetTachReading() failed! Error code = %d!", ErrorCode);
        }

        lpEnumProc(&GpuInfo);
    }

    return TRUE;
}
