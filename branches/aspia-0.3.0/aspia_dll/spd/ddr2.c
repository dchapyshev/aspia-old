/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/spd/ddr2.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "../spd.h"
#include "../aspia_dll.h"


static double
GetSpdExtensionOfByte41(BYTE Byte)
{
    switch (Byte >> 3)
    {
        case 0x00: return 0.00;
        case 0x01: return 0.25;
        case 0x02: return 0.33;
        case 0x03: return 0.50;
        case 0x04: return 0.66;
        case 0x05: return 0.75;
    }
    return 0.00;
}

static double
GetSpdExtensionOfByte42(BYTE Byte)
{
    switch ((Byte & 0x0F) >> 2)
    {
        case 0x00: return 0.00;
        case 0x01: return 0.25;
        case 0x02: return 0.33;
        case 0x03: return 0.50;
        case 0x04: return 0.66;
        case 0x05: return 0.75;
    }
    return 0.00;
}

VOID
ShowMemoryTimings(BYTE *Spd, double Latency, int cl)
{
    WCHAR szFormat[MAX_STR_LEN];

    LoadMUIString(IDS_SPD_TIMING_FORMAT, szFormat, MAX_STR_LEN);
    IoAddItem(2, 1, szFormat, 1000.00 / Latency);
    IoSetItemText(L"%i-%i-%i-%i (CL-RCD-RP-RAS)/%i-%i-%i-%i-%i-%i (RC-RFC-RRD-WR-WTR-RTP)",
                  cl,
                  (int)Round(GetSpdTime(Spd[0x1D])/Latency, 0),
                  (int)Round(GetSpdTime(Spd[0x1B])/Latency, 0),
                  (int)Round(Spd[0x1E]/Latency, 0),
                  (int)Round((Spd[0x29] + GetSpdExtensionOfByte41(Spd[0x29]))/Latency, 0),
                  (int)Round((Spd[0x2a] + GetSpdExtensionOfByte42(Spd[0x2a]))/Latency, 0),
                  (int)Round(GetSpdTime(Spd[0x1C])/Latency, 0),
                  (int)Round(GetSpdTime(Spd[0x24])/Latency, 0),
                  (int)Round(GetSpdTime(Spd[0x25])/Latency, 0),
                  (int)Round(GetSpdTime(Spd[0x26])/Latency, 0));
}

VOID
ShowSpdDataForDDR2(BYTE *Spd)
{
    WCHAR szText[MAX_STR_LEN], szType[MAX_STR_LEN],
          szManuf[MAX_STR_LEN], szPart[MAX_STR_LEN], *ptr;
    INT Rank;
    double CycleTime;

    DebugTrace(L"Show data for DDR2");

    GetSpdModuleType(Spd, szType, sizeof(szType));
    GetSpdManufacturer(Spd, szManuf, sizeof(szManuf));

    /* Model */
    StringCbPrintf(szPart, sizeof(szPart),
                   L"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                   Spd[0x49], Spd[0x4A], Spd[0x4B], Spd[0x4C], Spd[0x4D],
                   Spd[0x4E], Spd[0x4F], Spd[0x50], Spd[0x51], Spd[0x52],
                   Spd[0x53], Spd[0x54], Spd[0x55], Spd[0x56], Spd[0x57],
                   Spd[0x58], Spd[0x59], Spd[0x5A]);
    ChopSpaces(szPart, sizeof(szPart));

    StringCbPrintf(szText, sizeof(szText),
                   L"%s %s", szManuf,
                   (szPart[0] == 0) ? szType : szPart);

    IoAddHeaderString(0, 0, (szText[0] == 0) ? L"Unknown" : szText);

    IoAddValueName(1, 0, IDS_MANUFACTURER);
    IoSetItemText(szManuf);

    IoAddValueName(1, 0, IDS_SERIAL_NUMBER);
    if (Spd[0x5F] == 0xFF && Spd[0x60] == 0xFF &&
        Spd[0x61] == 0xFF && Spd[0x62] == 0xFF)
    {
        StringCbCopy(szText, sizeof(szText), L"N/A");
    }
    else
    {
        StringCbPrintf(szText, sizeof(szText), L"%02X%02X%02X%02X",
                       Spd[0x5F], Spd[0x60], Spd[0x61], Spd[0x62]);
    }
    IoSetItemText(szText);

    IoAddValueName(1, 0, IDS_SPD_PRODUCT_DATE);
    IoSetItemText(L"Week %ld, Year 20%.2ld",
                  Spd[0x5E], Spd[0x5D]);

    IoAddValueName(1, 0, IDS_SPD_MEMORY_TYPE);
    IoSetItemText(szType);

    IoAddValueName(1, 0, IDS_SPD_DIMM_TYPE);
    switch (GetBitsBYTE(Spd[0x14], 0, 5))
    {
        case 0x01: ptr = L"RDIMM"; break;
        case 0x02: ptr = L"UDIMM"; break;
        case 0x04: ptr = L"SO-DIMM"; break;
        case 0x06: ptr = L"SO-CDIMM"; break;
        case 0x07: ptr = L"SO-RDIMM"; break;
        case 0x08: ptr = L"Micro-DIMM"; break;
        case 0x10: ptr = L"Mini-DIMM"; break;
        case 0x20: ptr = L"Mini-UDIMM"; break;
        case 0x00:
        default:
            ptr = L"Unknown";
            break;
    }
    IoSetItemText(ptr);

    Rank = ((Spd[0x05] & 0x07) + 1);

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

    IoAddValueName(1, 0, IDS_SPD_ANALYSIS_PROBE);
    IoSetItemText(GetBitsBYTE(Spd[0x15], 6, 6) ? L"Present" : L"Not Present");

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
    szText[wcslen(szText) - 2] = 0;
    IoSetItemText(szText);

    IoAddValueName(2, 1, IDS_SPD_REFRESH_RATE);
    GetSpdRefreshRate(Spd, szText, sizeof(szText));
    IoSetItemText(szText);

    IoAddValueName(2, 1, IDS_SPD_SUPPORTED_CAS_LATENCIES);
    szText[0] = 0;
    if (GetBitsBYTE(Spd[0x12], 7, 7))
        StringCbCat(szText, sizeof(szText), L"7.0, ");
    if (GetBitsBYTE(Spd[0x12], 6, 6))
        StringCbCat(szText, sizeof(szText), L"6.0, ");
    if (GetBitsBYTE(Spd[0x12], 5, 5))
        StringCbCat(szText, sizeof(szText), L"5.0, ");
    if (GetBitsBYTE(Spd[0x12], 4, 4))
        StringCbCat(szText, sizeof(szText), L"4.0, ");
    if (GetBitsBYTE(Spd[0x12], 3, 3))
        StringCbCat(szText, sizeof(szText), L"3.0, ");
    if (GetBitsBYTE(Spd[0x12], 2, 2))
        StringCbCat(szText, sizeof(szText), L"2.0, ");
    szText[wcslen(szText) - 2] = 0;
    IoSetItemText(szText);

    IoAddItem(2, 1, L"Minimum RAS to CAS delay (tRCD)");
    IoSetItemText(L"%.2f ns", GetSpdTime(Spd[0x1D]));

    IoAddItem(2, 1, L"Minimum row precharge time (tRP)");
    IoSetItemText(L"%.2f ns", GetSpdTime(Spd[0x1B]));

    IoAddItem(2, 1, L"Minimum active to precharge time (tRAS)");
    IoSetItemText(L"%ld ns", Spd[0x1E]);

    IoAddItem(2, 1, L"Internal read to precharge command delay (tRTP)");
    IoSetItemText(L"%0.2f ns", GetSpdTime(Spd[0x26]));

    IoAddItem(2, 1, L"Internal write to read command delay (tWTR)");
    IoSetItemText(L"%0.2f ns", GetSpdTime(Spd[0x25]));

    IoAddItem(2, 1, L"Minimum write recovery time (tWR)");
    IoSetItemText(L"%0.2f ns", GetSpdTime(Spd[0x24]));

    IoAddItem(2, 1, L"Minimum row active–row active delay (tRRD)");
    IoSetItemText(L"%0.2f ns", GetSpdTime(Spd[0x1C]));

    IoAddItem(2, 1, L"Minimum refresh to active/refresh time (tRFC)");
    IoSetItemText(L"%0.2f ns", Spd[0x2a] + GetSpdExtensionOfByte42(Spd[0x2a]));

    IoAddItem(2, 1, L"Minimum active to active/refresh time (tRC)");
    IoSetItemText(L"%0.2f ns", Spd[0x29] + GetSpdExtensionOfByte41(Spd[0x29]));

    IoAddItem(2, 1, L"Address/command setup time from clock (tIS)");
    IoSetItemText(L"%0.2f ns",
                  ((double)(Spd[0x20] >> 4) / 10.00) + ((double)(Spd[0x20] & 0x0F) / 100.00));

    IoAddItem(2, 1, L"Address/command hold time after clock (tIH)");
    IoSetItemText(L"%0.2f ns",
                  ((double)(Spd[0x21] >> 4) / 10.00) + ((double)(Spd[0x21] & 0x0F) / 100.00));

    IoAddItem(2, 1, L"Data input setup time from strobe (tDS)");
    IoSetItemText(L"%0.2f ns",
                  ((double)(Spd[0x22] >> 4) / 10.00) + ((double)(Spd[0x22] & 0x0F) / 100.00));

    IoAddItem(2, 1, L"Data input hold time after strobe (tDH)");
    IoSetItemText(L"%0.2f ns",
                  ((double)(Spd[0x23] >> 4) / 10.00) + ((double)(Spd[0x23] & 0x0F) / 100.00));

    if ((Spd[0x12] & 0x80) >> 6)
        ShowMemoryTimings(Spd, 1.87, 7); // 533 MHz
    if ((Spd[0x12] & 0x40) >> 5)
        ShowMemoryTimings(Spd, 2.50, 6); // 400 MHz
    if ((Spd[0x12] & 0x20) >> 4)
        ShowMemoryTimings(Spd, 3.00, 5); // 333 MHz
    if ((Spd[0x12] & 0x10) >> 3)
        ShowMemoryTimings(Spd, 3.75, 4); // 266 MHz
    if ((Spd[0x12] & 0x08) >> 2)
        ShowMemoryTimings(Spd, 5.00, 3); // 200 MHz
    if ((Spd[0x12] & 0x04) >> 1)
        ShowMemoryTimings(Spd, 6.02, 2); // 166 MHz

    IoAddFooter();
}
