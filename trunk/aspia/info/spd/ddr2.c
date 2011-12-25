/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/spd/ddr2.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../../main.h"
#include "driver.h"
#include "../spd.h"


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
    WCHAR szText[MAX_STR_LEN], szFormat[MAX_STR_LEN];
    INT ItemIndex;

    LoadMUIString(IDS_SPD_TIMING_FORMAT, szFormat, MAX_STR_LEN);
    StringCbPrintf(szText, sizeof(szText), szFormat, 1000.00 / Latency);
    ItemIndex = IoAddItem(1, 1, szText);
    StringCbPrintf(szText, sizeof(szText),
                   L"%ld-%ld-%ld-%ld (CL-RCD-RP-RAS)/%ld-%ld-%ld-%ld-%ld-%ld (RC-RFC-RRD-WR-WTR-RTP)",
                   cl,
                   (DWORD)Round(GetSpdTime(Spd[0x1D])/Latency, 0),
                   (DWORD)Round(GetSpdTime(Spd[0x1B])/Latency, 0),
                   (DWORD)Round(Spd[0x1E]/Latency, 0),
                   (DWORD)Round((Spd[0x29] + GetSpdExtensionOfByte41(Spd[0x29]))/Latency, 0),
                   (DWORD)Round((Spd[0x2a] + GetSpdExtensionOfByte42(Spd[0x2a]))/Latency, 0),
                   (DWORD)Round(GetSpdTime(Spd[0x1C])/Latency, 0),
                   (DWORD)Round(GetSpdTime(Spd[0x24])/Latency, 0),
                   (DWORD)Round(GetSpdTime(Spd[0x25])/Latency, 0),
                   (DWORD)Round(GetSpdTime(Spd[0x26])/Latency, 0));
    IoSetItemText(ItemIndex, 1, szText);
}

VOID
ShowSpdDataForDDR2(BYTE *Spd)
{
    WCHAR szText[MAX_STR_LEN], szType[MAX_STR_LEN],
          szManuf[MAX_STR_LEN], szPart[MAX_STR_LEN];
    INT ItemIndex, Rank;
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

    IoAddHeaderString(0, (szText[0] == 0) ? L"Unknown" : szText, 0);

    ItemIndex = IoAddValueName(1, IDS_MANUFACTURER, 0);
    IoSetItemText(ItemIndex, 1, szManuf);

    ItemIndex = IoAddValueName(1, IDS_SERIAL_NUMBER, 0);
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
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_PRODUCT_DATE, 0);
    StringCbPrintf(szText, sizeof(szText), L"Week %ld, Year 20%.2ld",
                   Spd[0x5E], Spd[0x5D]);
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_MEMORY_TYPE, 0);
    IoSetItemText(ItemIndex, 1, szType);

    ItemIndex = IoAddValueName(1, IDS_SPD_DIMM_TYPE, 0);
    if (Spd[0x14] & 0x01)
        StringCbPrintf(szText, sizeof(szText), L"RDIMM");
    else if ((Spd[0x14] & 0x02) >> 0)
        StringCbPrintf(szText, sizeof(szText), L"UDIMM");
    else if ((Spd[0x14] & 0x04) >> 1)
        StringCbPrintf(szText, sizeof(szText), L"SO-DIMM");
    else if ((Spd[0x14] & 0x08) >> 2)
        StringCbPrintf(szText, sizeof(szText), L"Micro-DIMM");
    else if ((Spd[0x14] & 0x10) >> 3)
        StringCbPrintf(szText, sizeof(szText), L"Mini-DIMM");
    else if ((Spd[0x14] & 0x20) >> 4)
        StringCbPrintf(szText, sizeof(szText), L"Mini-UDIMM");
    IoSetItemText(ItemIndex, 1, szText);

    Rank = ((Spd[0x05] & 0x07) + 1);

    ItemIndex = IoAddValueName(1, IDS_SPD_MODULE_SIZE, 0);
    StringCbPrintf(szText, sizeof(szText),
                   L"%ld MB (%ld ranks, %ld banks)",
                   GetSpdDensity(Spd[0x1F]) * Rank,
                   Rank, Spd[0x11]);
    IoSetItemText(ItemIndex, 1, szText);

    CycleTime = GetSpdCycleTime(Spd[0x09]);

    ItemIndex = IoAddValueName(1, IDS_SPD_FREQUENT, 0);
    StringCbPrintf(szText, sizeof(szText),
                   L"%.1f MHz", (double)(1000 / CycleTime));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_VOLTAGE, 0);
    GetSpdVoltage(Spd, szText, sizeof(szText));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_MODULE_WIDTH, 0);
    GetSpdModuleWidth(Spd, szText, sizeof(szText));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_ERROR_DETECTION, 0);
    GetSpdErrorDetection(Spd, szText, sizeof(szText));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_ANALYSIS_PROBE, 0);
    if ((Spd[0x15] & 0x40) >> 5)
        StringCbCopy(szText, sizeof(szText), L"Present");
    else
        StringCbCopy(szText, sizeof(szText), L"Not Present");
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_FET_SWITCH_EXT, 0);
    if ((Spd[0x15] & 0x10) >> 3)
        StringCbCopy(szText, sizeof(szText), L"Enabled");
    else
        StringCbCopy(szText, sizeof(szText), L"Disabled");
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_WEAK_DRIVER, 0);
    if (Spd[0x16] & 0x01)
        StringCbCopy(szText, sizeof(szText), L"Supported");
    else
        StringCbCopy(szText, sizeof(szText), L"Not Supported");
    IoSetItemText(ItemIndex, 1, szText);

    IoAddHeader(1, IDS_SPD_MEMORY_TIMINGS, 1);

    ItemIndex = IoAddValueName(1, IDS_SPD_BURST_LENGTHS, 1);
    szText[0] = 0;
    if ((Spd[0x10] & 0x08) >> 2)
        StringCbCat(szText, sizeof(szText), L"8, ");
    if ((Spd[0x10] & 0x04) >> 1)
        StringCbCat(szText, sizeof(szText), L"4, ");
    szText[wcslen(szText) - 2] = 0;
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_REFRESH_RATE, 1);
    GetSpdRefreshRate(Spd, szText, sizeof(szText));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_SUPPORTED_CAS_LATENCIES, 1);
    szText[0] = 0;
    if ((Spd[0x12] & 0x80) >> 6)
        StringCbCat(szText, sizeof(szText), L"7.0, ");
    if ((Spd[0x12] & 0x40) >> 5)
        StringCbCat(szText, sizeof(szText), L"6.0, ");
    if ((Spd[0x12] & 0x20) >> 4)
        StringCbCat(szText, sizeof(szText), L"5.0, ");
    if ((Spd[0x12] & 0x10) >> 3)
        StringCbCat(szText, sizeof(szText), L"4.0, ");
    if ((Spd[0x12] & 0x08) >> 2)
        StringCbCat(szText, sizeof(szText), L"3.0, ");
    if ((Spd[0x12] & 0x04) >> 1)
        StringCbCat(szText, sizeof(szText), L"2.0, ");
    szText[wcslen(szText) - 2] = 0;
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_RCD, 1);
    StringCbPrintf(szText, sizeof(szText), L"%.2f ns", GetSpdTime(Spd[0x1D]));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_RP, 1);
    StringCbPrintf(szText, sizeof(szText), L"%.2f ns", GetSpdTime(Spd[0x1B]));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_RAS, 1);
    StringCbPrintf(szText, sizeof(szText), L"%ld ns", Spd[0x1E]);
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_RTP, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns", GetSpdTime(Spd[0x26]));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_WTR, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns", GetSpdTime(Spd[0x25]));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_WR, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns", GetSpdTime(Spd[0x24]));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_RRD, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns", GetSpdTime(Spd[0x1C]));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_RFC, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns", Spd[0x2a] + GetSpdExtensionOfByte42(Spd[0x2a]));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_RC, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns", Spd[0x29] + GetSpdExtensionOfByte41(Spd[0x29]));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_IS, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns",
                   ((double)(Spd[0x20] >> 4) / 10.00) + ((double)(Spd[0x20] & 0x0F) / 100.00));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_IH, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns",
                   ((double)(Spd[0x21] >> 4) / 10.00) + ((double)(Spd[0x21] & 0x0F) / 100.00));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_DS, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns",
                   ((double)(Spd[0x22] >> 4) / 10.00) + ((double)(Spd[0x22] & 0x0F) / 100.00));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_TIMING_DH, 1);
    StringCbPrintf(szText, sizeof(szText), L"%0.2f ns",
                   ((double)(Spd[0x23] >> 4) / 10.00) + ((double)(Spd[0x23] & 0x0F) / 100.00));
    IoSetItemText(ItemIndex, 1, szText);

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
