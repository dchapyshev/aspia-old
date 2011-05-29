/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/sonsors.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include "../smart/smart.h"


/* For AMD 0xF0 Family */
#define THERMTRIP_STATUS_REGISTER  0xE4
#define FIDVID_STATUS              0xC0010042


ULONG
GetTjmaxFromMSR(DWORD CpuIndex)
{
    UINT64 Value = 0;

    if (DRIVER_GetMSRData(IA32_TEMPERATURE_TARGET, CpuIndex, &Value))
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
    INT Index;

    DebugTrace(L"Intel CPU");

    if (GetCPUName(szText, sizeof(szText)))
    {
        UINT64 Value;
        UINT Temp, Tjmax;
        bIndex = 0;

        while (DRIVER_GetMSRData(IA32_THERM_STATUS, bIndex, &Value))
        {
            Temp = ((Value >> 16) & 0xFF);

            if (Temp == 0)
            {
                ++bIndex;
                continue;
            }

            if (bIndex == 0)
            {
                IoAddHeaderString(szText, 1);
            }

            LoadMUIString(IDS_SENSOR_CORE_TEMPERATURE,
                          szFormat, MAX_STR_LEN);
            StringCbPrintf(szText, sizeof(szText), szFormat, bIndex + 1);
            Index = IoAddItem(-1, szText);

            Tjmax = GetTjmaxTemperature(bIndex);

            StringCbPrintf(szText, sizeof(szText),
                           L"%d °C", Tjmax - Temp);
            IoSetItemText(Index, 1, szText);

            DebugTrace(L"Core = %d, Tjmax = %d, Temp = %d, Result = %d",
                       bIndex + 1, Tjmax, Temp, Tjmax - Temp);
            ++bIndex;
        }
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
    INT Index;
    ULONG Device;
    DWORD ThermValue;

    DebugTrace(L"AMD CPU");

    Device = GetPciAddress(0, 24, 3);

    if (GetCPUName(szText, sizeof(szText)))
    {
        IoAddHeaderString(szText, 1);

        LoadMUIString(IDS_SENSOR_TEMPERATURE, szText, MAX_STR_LEN);
        Index = IoAddItem(-1, szText);

        ThermValue = ReadPciConfigDword(Device, 0xA4);

        StringCbPrintf(szText, sizeof(szText),
                       L"%d °C",
                       ((ThermValue >> 21) & 0x7FF) / 8);
        IoSetItemText(Index, 1, szText);
    }
}

VOID
GetAmd0FCpuInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szFormat[MAX_STR_LEN];
    INT Index, Count;
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
                IoAddHeaderString(szText, 1);
            }

            LoadMUIString(IDS_SENSOR_CORE_TEMPERATURE, szFormat, MAX_STR_LEN);
            StringCbPrintf(szText, sizeof(szText), szFormat, Count + 1);
            Index = IoAddItem(-1, szText);

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

            StringCbPrintf(szText, sizeof(szText),
                           L"%u °C",
                           ((ThermValue >> 16) & 0xFF) + Offset);
            IoSetItemText(Index, 1, szText);
        }
    }
}

VOID
HW_SensorInfo(VOID)
{
    IDSECTOR DriveInfo;
    WCHAR szText[MAX_STR_LEN];
    HANDLE hHandle;
    BYTE bIndex;
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_HDD);
    IoAddIcon(IDI_CPU);

    /* Hard Drives */
    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        hHandle = SMART_Open(bIndex);
        if (hHandle == INVALID_HANDLE_VALUE) continue;

        if (SMART_ReadDriveInformation(hHandle, bIndex, &DriveInfo))
        {
            ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                            sizeof(DriveInfo.sModelNumber));
            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sModelNumber);
            ChopSpaces(szText, sizeof(szText));
            IoAddHeaderString(szText, 0);

            StringCbPrintf(szText, sizeof(szText),
                           L"%ld °C",
                           SMART_GetHDDTemperature(hHandle, bIndex));

            Index = IoAddValueName(IDS_SENSOR_TEMPERATURE, -1);
            IoSetItemText(Index, 1, szText);

            IoAddFooter();
        }

        SMART_Close(hHandle);
    }

    /* CPUs */
    GetCPUVendor(szText, sizeof(szText));
    if (wcscmp(szText, L"GenuineIntel") == 0)
        GetIntelCpuInfo();
    else if (wcscmp(szText, L"AuthenticAMD") == 0)
    {
        CPU_IDS CpuIds = {0};

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
        }
    }

    DebugEndReceiving();
}
