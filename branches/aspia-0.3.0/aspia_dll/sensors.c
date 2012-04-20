/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/sensors.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


/* For AMD 0xF0 Family */
#define THERMTRIP_STATUS_REGISTER  0xE4
#define FIDVID_STATUS              0xC0010042


ULONG
GetTjmaxFromMSR(DWORD CpuIndex)
{
    UINT64 Value = 0;

    if (ReadMsr(IA32_TEMPERATURE_TARGET, CpuIndex, &Value))
    {
        if (Value != 0)
            return (Value >> 16) & 0xff;
    }

    return 100;
}

ULONG
GetTjmaxTemperature(DWORD CpuIndex)
{
    DWORD CoreCount = GetLogicalProcessorsCount();
    ULONG Tjmax = 100;
    CPU_IDS CpuIds = {0};

    GetProcessorIDs(&CpuIds);

    DebugTrace(L"Family = %d, Model = %d, Stepping = %d",
               CpuIds.Family, CpuIds.Model, CpuIds.Stepping);

    switch (CpuIds.Family)
    {
        case 0x06:
        {
            switch (CpuIds.Model)
            {
                case 0x0C: /* Atom N330 */
                    Tjmax = 95;
                    break;

                case 0x0f: /* Core 2 (65nm) */
                {
                    switch (CpuIds.Stepping)
                    {
                        case 0x06:
                        {
                            switch (CoreCount)
                            {
                                case 2:
                                    Tjmax = 90;
                                    break;
                                case 4:
                                    Tjmax = 100;
                                    break;
                                default:
                                    Tjmax = 95;
                                    break;
                            }
                        }
                        break;

                        case 0x0b:
                            Tjmax = 100;
                            break;
                        case 0x0d:
                            Tjmax = 95;
                            break;
                        default:
                            Tjmax = 95;
                            break;
                    }
                }
                break;

                case 0x17: /* Core 2 (45nm) */
                    Tjmax = 100;
                    break;
                case 0x1c: /* Atom (45nm) */
                {
                    switch (CpuIds.Stepping)
                    {
                        case 0x02:
                            Tjmax = 90;
                            break;
                        case 0x0A:
                            Tjmax = 100;
                            break;
                        default:
                            Tjmax = 90;
                            break;
                    }
                }
                break;

                case 0x1A: /* Intel Core i7 LGA1366 (45nm) */
                case 0x1E: /* Intel Core i5, i7 LGA1156 (45nm) */
                case 0x1F: /* Intel Core i5, i7 */
                case 0x25: /* Intel Core i3, i5, i7 LGA1156 (32nm) */
                case 0x2C: /* Intel Core i7 LGA1366 (32nm) 6 Core */
                case 0x2E: /* Intel Xeon Processor 7500 series */
                case 0x2A: /* Intel Core i5, i7 2xxx LGA1155 (32nm) */
                case 0x2D: /* Next Generation Intel Xeon Processor */
                    return GetTjmaxFromMSR(CpuIndex);
            }
        }
        break;

        case 0x0F:
        {
            switch (CpuIds.Model)
            {
                case 0x00: /* Pentium 4 (180nm) */
                case 0x01: /* Pentium 4 (130nm) */
                case 0x02: /* Pentium 4 (130nm) */
                case 0x03: /* Pentium 4, Celeron D (90nm) */
                case 0x04: /* Pentium 4, Pentium D (90nm) */
                case 0x06: /* Pentium 4, Pentium D, Celeron D (65nm) */
                    Tjmax = 100;
                    break;
            }
        }
        break;
    }

    return Tjmax;
}

VOID
GetIntelCpuInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szFormat[MAX_STR_LEN];
    BYTE bIndex;

    DebugTrace(L"Intel CPU");

    if (GetCPUName(szText, sizeof(szText)))
    {
        UINT64 Value;
        UINT Temp, Tjmax;
        bIndex = 0;

        while (ReadMsr(IA32_THERM_STATUS, bIndex, &Value))
        {
            Temp = ((Value >> 16) & 0xFF);

            if (Temp == 0)
            {
                ++bIndex;
                continue;
            }

            if (bIndex == 0)
            {
                IoAddHeaderString(0, 1, szText);
            }

            LoadMUIString(IDS_SENSOR_CORE_TEMPERATURE,
                          szFormat, MAX_STR_LEN);
            IoAddItem(1, 4, szFormat, bIndex + 1);

            Tjmax = GetTjmaxTemperature(bIndex);

            IoSetItemText(L"%d °C", Tjmax - Temp);

            DebugTrace(L"Core = %d, Tjmax = %d, Temp = %d, Result = %d",
                       bIndex + 1, Tjmax, Temp, Tjmax - Temp);
            ++bIndex;
        }

        IoAddFooter();
    }
}

ULONG
GetPciAddress(BYTE bus, BYTE device, BYTE function)
{
    return (((bus & 0xFF) << 8) | ((device & 0x1F) << 3) | (function & 7));
}

VOID
GetAmd10CpuInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    ULONG Device;
    DWORD ThermValue;

    DebugTrace(L"AMD CPU");

    Device = GetPciAddress(0, 24, 3);

    if (GetCPUName(szText, sizeof(szText)))
    {
        IoAddHeaderString(0, 1, szText);

        LoadMUIString(IDS_SENSOR_TEMPERATURE, szText, MAX_STR_LEN);
        IoAddItem(1, 1, szText);

        ThermValue = ReadPciConfigDword(Device, 0xA4);

        IoSetItemText(L"%d °C",
                      ((ThermValue >> 21) & 0x7FF) / 8);

        IoAddFooter();
    }
}

VOID
GetAmd0FCpuInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szFormat[MAX_STR_LEN];
    INT Count;
    ULONG Device;
    DWORD ThermValue;
    INT Offset = -49;
    CPU_IDS CpuIds = {0};
    BYTE ThermSenseCoreSelCPU0;
    BYTE ThermSenseCoreSelCPU1;

    Device = GetPciAddress(0, 24, 3);

    if (GetCPUName(szText, sizeof(szText)))
    {
        for (Count = 0; Count < 2; Count++)
        {
            if (Count == 0)
            {
                IoAddHeaderString(0, 1, szText);
            }

            LoadMUIString(IDS_SENSOR_CORE_TEMPERATURE, szFormat, MAX_STR_LEN);
            IoAddItem(1, 1, szFormat, Count + 1);

            GetProcessorIDs(&CpuIds);

            if (CpuIds.Model < 40)
            {
                /* AMD Athlon 64 Processors */
                ThermSenseCoreSelCPU0 = 0x0;
                ThermSenseCoreSelCPU1 = 0x4;
            }
            else
            {
                /* AMD NPT Family 0Fh Revision F, G have the core selection swapped */
                ThermSenseCoreSelCPU0 = 0x4;
                ThermSenseCoreSelCPU1 = 0x0;
            }

            WritePciConfigDword(Device,
                                THERMTRIP_STATUS_REGISTER,
                                (Count == 0) ? ThermSenseCoreSelCPU0 : ThermSenseCoreSelCPU1);

            ThermValue = ReadPciConfigDword(Device, THERMTRIP_STATUS_REGISTER);

            /* AM2+ 65nm +21 offset */
            if (CpuIds.Model >= 0x69 && CpuIds.Model != 0xc1 &&
                CpuIds.Model != 0x6c && CpuIds.Model != 0x7c)
                Offset += 21;

            IoSetItemText(L"%u °C",
                          ((ThermValue >> 16) & 0xFF) + Offset);
        }

        IoAddFooter();
    }
}

BOOL CALLBACK
NvidiaGPUsEnumProc(PNVIDIA_GPU_INFO GpuInfo)
{
    /* Add GPU Name */
    IoAddHeaderString(0, 6, GpuInfo->szName);

    /* GPU board ambient temperature */
    if (GpuInfo->BoardTemp.CurrentTemp > 0)
    {
        IoAddValueName(1, 4, IDS_NVIDIA_BOARD_TEMP);
        IoSetItemText(L"%ld °C",
                      GpuInfo->BoardTemp.CurrentTemp);
    }

    /* GPU core temperature */
    if (GpuInfo->GpuTemp.CurrentTemp > 0)
    {
        IoAddValueName(1, 4, IDS_NVIDIA_CORE_TEMP);
        IoSetItemText(L"%ld °C",
                      GpuInfo->GpuTemp.CurrentTemp);
    }

    /* GPU memory temperature */
    if (GpuInfo->MemoryTemp.CurrentTemp > 0)
    {
        IoAddValueName(1, 4, IDS_NVIDIA_MEMORY_TEMP);
        IoSetItemText(L"%ld °C",
                      GpuInfo->MemoryTemp.CurrentTemp);
    }

    /* GPU power supply temperature */
    if (GpuInfo->PowerSupplyTemp.CurrentTemp > 0)
    {
        IoAddValueName(1, 4, IDS_NVIDIA_POWER_SUPPLY_TEMP);
        IoSetItemText(L"%ld °C",
                      GpuInfo->PowerSupplyTemp.CurrentTemp);
    }

    /* Visual Computing Device Board temperature */
    if (GpuInfo->VcdBoardTemp.CurrentTemp > 0)
    {
        IoAddValueName(1, 4, IDS_NVIDIA_VCD_BOARD_TEMP);
        IoSetItemText(L"%ld °C",
                      GpuInfo->VcdBoardTemp.CurrentTemp);
    }

    /* Visual Computing Device Inlet temperature */
    if (GpuInfo->VcdInletTemp.CurrentTemp > 0)
    {
        IoAddValueName(1, 4, IDS_NVIDIA_VCD_INLET_TEMP);
        IoSetItemText(L"%ld °C",
                      GpuInfo->VcdInletTemp.CurrentTemp);
    }

    /* Visual Computing Device Outlet temperature */
    if (GpuInfo->VcdOutletTemp.CurrentTemp > 0)
    {
        IoAddValueName(1, 4, IDS_NVIDIA_VCD_OUTLET_TEMP);
        IoSetItemText(L"%ld °C",
                      GpuInfo->VcdOutletTemp.CurrentTemp);
    }

    /* Fan Speed */
    if (GpuInfo->FanSpeed > 0)
    {
        IoAddValueName(1, 5, IDS_NVIDIA_FAN_SPEED);
        IoSetItemText(L"%ld RPM",
                      GpuInfo->FanSpeed);
    }

    IoAddFooter();

    return TRUE;
}

VOID
HW_SensorInfo(VOID)
{
    IDSECTOR DriveInfo;
    WCHAR szText[MAX_STR_LEN];
    HANDLE hHandle;
    BYTE bIndex;

    DebugStartReceiving();

    IoAddIcon(IDI_HDD);
    IoAddIcon(IDI_CPU);
    IoAddIcon(IDI_HW);
    IoAddIcon(IDI_VOLTAGE);
    IoAddIcon(IDI_TEMPERATURE);
    IoAddIcon(IDI_FAN);
    IoAddIcon(IDI_NVIDIA);
    IoAddIcon(IDI_ATI);

    /* Hard Drives */
    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        hHandle = OpenSmart(bIndex);
        if (hHandle == INVALID_HANDLE_VALUE) continue;

        if (ReadSmartInfo(hHandle, bIndex, &DriveInfo))
        {
            INT Temp = GetSmartTemperature(hHandle, bIndex);

            if (Temp <= 0 || Temp > 100) continue;

            ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                            sizeof(DriveInfo.sModelNumber));
            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sModelNumber);
            ChopSpaces(szText, sizeof(szText));
            IoAddHeaderString(0, 0, szText);

            IoAddValueName(1, 4, IDS_SENSOR_TEMPERATURE);
            IoSetItemText(L"%d °C", Temp);

            IoAddFooter();
        }

        CloseSmart(hHandle);
    }

    /* CPUs */
    GetCPUVendor(szText, sizeof(szText));
    if (wcscmp(szText, L"GenuineIntel") == 0)
        GetIntelCpuInfo();
    else if (wcscmp(szText, L"AuthenticAMD") == 0)
    {
        /*CPU_IDS CpuIds = {0};

        GetProcessorIDs(&CpuIds);

        switch (CpuIds.Family)
        {
            case 0x0F:
                GetAmd0FCpuInfo();
                break;
            case 0x10:
            case 0x11:
                GetAmd10CpuInfo();
                break;
        }*/
    }

    GetLPCSensorsInfo();

    if (!IsNvidiaApiInitialized())
    {
        InitNvidiaApi();
    }
    if (IsNvidiaApiInitialized())
    {
        EnumNvidiaGPUs(NvidiaGPUsEnumProc);
    }

    DebugEndReceiving();
}
