/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/spd/ddr.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../spd.h"
#include "aspia.h"
#include "../aspia_dll.h"


VOID
GetSpdManufacturer(BYTE *Spd, LPWSTR lpszString, SIZE_T Size)
{
    WCHAR szJEID[16] = {0};
    DWORD dwJEID;
    SIZE_T Index;

    StringCbPrintf(szJEID, sizeof(szJEID),
                   L"%02X%02X%02X%02X%02X%02X%02X%02X",
                   Spd[0x40], Spd[0x41], Spd[0x42], Spd[0x43],
                   Spd[0x44], Spd[0x45], Spd[0x46], Spd[0x47]);

    for (Index = 0x00; Index < 0x16; Index += 2)
    {
        if (szJEID[Index] == L'0' && szJEID[Index + 1] == L'0')
        {
            szJEID[Index + 0] = 0;
            szJEID[Index + 1] = 0;
        }
        if (szJEID[Index] == L'F' && szJEID[Index + 1] == L'F')
        {
            szJEID[Index + 0] = 0;
            szJEID[Index + 1] = 0;
        }
    }

    dwJEID = StrToHex(szJEID, 16);

    Index = 0;
    do
    {
        if (ManufacturerInfo[Index].dwID == dwJEID)
        {
            StringCbCopy(lpszString, Size,
                         ManufacturerInfo[Index].lpszValue);
            return;
        }
    }
    while (ManufacturerInfo[++Index].dwID != 0);

    StringCbCopy(lpszString, Size, L"Unknown");
}

INT
GetSpdDensity(BYTE v)
{
    switch (v)
    {
        case 0x01: return 1024;
        case 0x02: return 2048;
        case 0x04: return 4096;
        case 0x08: return 8192;
        case 0x10: return 16384;
        case 0x20: return 128;
        case 0x40: return 256;
        case 0x80: return 512;
        default: return 0;
    }
}

double
GetSpdCycleTime(BYTE v)
{
    double CycleTime = 0;

    switch (v & 0x0f)
    {
        case 0x00: CycleTime = 0.00; break;
        case 0x01: CycleTime = 0.10; break;
        case 0x02: CycleTime = 0.20; break;
        case 0x03: CycleTime = 0.30; break;
        case 0x04: CycleTime = 0.40; break;
        case 0x05: CycleTime = 0.50; break;
        case 0x06: CycleTime = 0.60; break;
        case 0x07: CycleTime = 0.70; break;
        case 0x08: CycleTime = 0.80; break;
        case 0x09: CycleTime = 0.90; break;
        case 0x0A: CycleTime = 0.25; break;
        case 0x0B: CycleTime = 0.33; break;
        case 0x0C: CycleTime = 0.66; break;
        case 0x0D: CycleTime = 0.75; break;
        default: break;
    }
    CycleTime += v / 0x10;
    return CycleTime;
}

VOID
GetSpdVoltage(BYTE *Spd, LPWSTR lpszString, SIZE_T Size)
{
    SIZE_T Index = 0;

    do
    {
        if (VoltageInfo[Index].dwID == Spd[0x08])
        {
            StringCbCopy(lpszString, Size,
                         VoltageInfo[Index].lpszValue);
            return;
        }
    }
    while (VoltageInfo[++Index].dwID != 0);

    StringCbCopy(lpszString, Size, L"Unknown");
}

VOID
GetSpdModuleWidth(BYTE *Spd, LPWSTR lpszString, SIZE_T Size)
{
    DWORD Width;

    if (Spd[0x07] == 0)
        Width = Spd[0x06];
    else
        Width = (Spd[0x07] * 100) + Spd[0x06];

    StringCbPrintf(lpszString, Size, L"%u bit", Width);
}

VOID
GetSpdRefreshRate(BYTE *Spd, LPWSTR lpszString, SIZE_T Size)
{
    SIZE_T Index = 0;
    DWORD dwRate = GetBitsBYTE(Spd[0x0C], 0, 6);

    do
    {
        if (RefreshRateInfo[Index].dwID == dwRate)
        {
            StringCbCopy(lpszString, Size,
                         RefreshRateInfo[Index].lpszValue);
            if (GetBitsBYTE(Spd[0x0C], 7, 7))
            {
                StringCbCat(lpszString, Size, L", Self-Refresh");
            }
            return;
        }
    }
    while (RefreshRateInfo[++Index].dwID != 0);

    StringCbCopy(lpszString, Size, L"Unknown");
}

VOID
GetSpdErrorDetection(BYTE *Spd, LPWSTR lpszString, SIZE_T Size)
{
    SIZE_T Index = 0;

    do
    {
        if (ErrorDetectInfo[Index].dwID == Spd[0x0B]) /* Byte 11 */
        {
            StringCbCopy(lpszString, Size,
                         ErrorDetectInfo[Index].lpszValue);
            return;
        }
    }
    while (ErrorDetectInfo[++Index].dwID != 0);
    StringCbCopy(lpszString, Size, L"Unknown");
}

double
GetSpdTime(BYTE Byte)
{
    double Time = 0;

    switch (GetBitsBYTE(Byte, 0, 1))
    {
        case 0x00: Time = 0.00; break;
        case 0x01: Time = 0.25; break;
        case 0x02: Time = 0.50; break;
        case 0x03: Time = 0.75; break;
        default: break;
    }

    return Time + (Byte >> 2);
}

VOID
ShowSpdDataForDDR(BYTE *Spd)
{
    WCHAR szText[MAX_STR_LEN];
    INT Rank;
    double CycleTime;

    DebugTrace(L"Show data for DDR1");

    /* Model */
    StringCbPrintf(szText, sizeof(szText),
                   L"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                   Spd[0x49], Spd[0x4A], Spd[0x4B], Spd[0x4C], Spd[0x4D],
                   Spd[0x4E], Spd[0x4F], Spd[0x50], Spd[0x51], Spd[0x52],
                   Spd[0x53], Spd[0x54], Spd[0x55], Spd[0x56], Spd[0x57],
                   Spd[0x58], Spd[0x59], Spd[0x5A]);
    IoAddHeaderString(0, 0, szText);

    IoAddValueName(1, 0, IDS_MANUFACTURER);
    GetSpdManufacturer(Spd, szText, sizeof(szText));
    IoSetItemText(szText);

    IoAddValueName(1, 0, IDS_SERIAL_NUMBER);
    IoSetItemText(L"%02X%02X%02X%02X",
                  Spd[0x5F], Spd[0x60], Spd[0x61], Spd[0x62]);

    IoAddValueName(1, 0, IDS_SPD_PRODUCT_DATE);
    IoSetItemText(L"Week %ld, Year 20%ld",
                  Spd[0x5E], Spd[0x5D]);

    IoAddValueName(1, 0, IDS_SPD_MEMORY_TYPE);
    GetSpdModuleType(Spd, szText, sizeof(szText));
    IoSetItemText(szText);

    Rank = (Spd[0x05] & 0x07);

    IoAddValueName(1, 0, IDS_SPD_MODULE_SIZE);
    IoSetItemText(L"%ld MB (%ld ranks, %ld banks)",
                  GetSpdDensity(Spd[0x1F]) * Rank,
                  Rank, Spd[0x11]);

    CycleTime = GetSpdCycleTime(Spd[0x09]);

    IoAddValueName(1, 0, IDS_SPD_FREQUENT);
    IoSetItemText(L"%.1f MHz", (double)(1000 / CycleTime));

    IoAddValueName(1, 0, IDS_SPD_VOLTAGE);
    GetSpdVoltage(Spd, szText, sizeof(szText));
    IoSetItemText(szText);

    IoAddValueName(1, 0, IDS_SPD_MODULE_WIDTH);
    GetSpdModuleWidth(Spd, szText, sizeof(szText));
    IoSetItemText(szText);

    IoAddValueName(1, 0, IDS_SPD_ERROR_DETECTION);
    GetSpdErrorDetection(Spd, szText, sizeof(szText));
    IoSetItemText(szText);

    IoAddValueName(1, 0, IDS_SPD_FET_SWITCH_EXT);
    IoSetItemText(GetBitsBYTE(Spd[0x15], 4, 4) ? L"Enabled" : L"Disabled");

    IoAddValueName(1, 0, IDS_SPD_WEAK_DRIVER);
    IoSetItemText(GetBitsBYTE(Spd[0x16], 0, 0) ? L"Supported" : L"Not Supported");

    IoAddHeader(1, 1, IDS_SPD_MEMORY_TIMINGS);

    IoAddValueName(2, 1, IDS_SPD_BURST_LENGTHS);
    szText[0] = 0;
    if (GetBitsBYTE(Spd[0x10], 3, 3))
        StringCbCat(szText, sizeof(szText), L"8, ");
    if (GetBitsBYTE(Spd[0x10], 2, 2))
        StringCbCat(szText, sizeof(szText), L"4, ");
    if (GetBitsBYTE(Spd[0x10], 1, 1))
        StringCbCat(szText, sizeof(szText), L"2, ");
    if (GetBitsBYTE(Spd[0x10], 0, 0))
        StringCbCat(szText, sizeof(szText), L"1, ");
    szText[wcslen(szText) - 2] = 0;
    IoSetItemText(szText);

    IoAddValueName(2, 1, IDS_SPD_REFRESH_RATE);
    GetSpdRefreshRate(Spd, szText, sizeof(szText));
    IoSetItemText(szText);

    IoAddValueName(2, 1, IDS_SPD_SUPPORTED_CAS_LATENCIES);
    szText[0] = 0;
    if (GetBitsBYTE(Spd[0x12], 6, 6))
        StringCbCat(szText, sizeof(szText), L"4.0, ");
    if (GetBitsBYTE(Spd[0x12], 5, 5))
        StringCbCat(szText, sizeof(szText), L"3.5, ");
    if (GetBitsBYTE(Spd[0x12], 4, 4))
        StringCbCat(szText, sizeof(szText), L"3.0, ");
    if (GetBitsBYTE(Spd[0x12], 3, 3))
        StringCbCat(szText, sizeof(szText), L"2.5, ");
    if (GetBitsBYTE(Spd[0x12], 2, 2))
        StringCbCat(szText, sizeof(szText), L"2.0, ");
    if (GetBitsBYTE(Spd[0x12], 1, 1))
        StringCbCat(szText, sizeof(szText), L"1.5, ");
    if (GetBitsBYTE(Spd[0x12], 0, 0))
        StringCbCat(szText, sizeof(szText), L"1.0, ");
    szText[wcslen(szText) - 2] = 0;
    IoSetItemText(szText);

    IoAddItem(2, 1, L"Minimum RAS to CAS delay (tRCD)");
    IoSetItemText(L"%.2f ns", GetSpdTime(Spd[0x1D]));

    IoAddItem(2, 1, L"Minimum row precharge time (tRP)");
    IoSetItemText(L"%.2f ns", GetSpdTime(Spd[0x1B]));

    IoAddItem(2, 1, L"Minimum active to precharge time (tRAS)");
    IoSetItemText(L"%ld ns", Spd[0x1E]);

    IoAddItem(2, 1, L"Minimum row active–row active delay (tRRD)");
    IoSetItemText(L"%0.2f ns", GetSpdTime(Spd[0x1C]));

    IoAddItem(2, 1, L"Minimum refresh to active/refresh time (tRFC)");
    IoSetItemText(L"%0.2f ns", Spd[0x2a]);

    IoAddItem(2, 1, L"Minimum active to active/refresh time (tRC)");
    IoSetItemText(L"%0.2f ns", Spd[0x29]);

    IoAddItem(2, 1, L"Address/command setup time from clock (tIS)");
    IoSetItemText(L"%0.2f ns",
                  ((double)GetBitsBYTE(Spd[0x20], 4, 7) / 10.00) + ((double)GetBitsBYTE(Spd[0x20], 0, 3) / 100.00));

    IoAddItem(2, 1, L"Address/command hold time after clock (tIH)");
    IoSetItemText(L"%0.2f ns",
                  ((double)GetBitsBYTE(Spd[0x21], 4, 7) / 10.00) + ((double)GetBitsBYTE(Spd[0x21], 0, 3) / 100.00));

    IoAddItem(2, 1, L"Data input setup time from strobe (tDS)");
    IoSetItemText(L"%0.2f ns",
                  ((double)GetBitsBYTE(Spd[0x22], 4, 7) / 10.00) + ((double)GetBitsBYTE(Spd[0x22], 0, 3) / 100.00));

    IoAddItem(2, 1, L"Data input hold time after strobe (tDH)");
    IoSetItemText(L"%0.2f ns",
                  ((double)GetBitsBYTE(Spd[0x23], 4 ,7) / 10.00) + ((double)GetBitsBYTE(Spd[0x23], 0, 3) / 100.00));

    IoAddFooter();
}
