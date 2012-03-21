/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/spd/ddr3.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../spd.h"
#include "aspia.h"
#include "../aspia_dll.h"


VOID
GetDDR3Manufacturer(BYTE *Spd, LPWSTR lpName, SIZE_T Size)
{
    INT ContCount = (Spd[0x75] & 0x3F); /* bits 6 - 0 */
    DWORD id;
    SIZE_T Index;

    switch (ContCount)
    {
        case 0:
            id = Spd[0x76];
            break;
        case 1:
            id = Spd[0x76] | 0x7f00;
            break;
        case 2:
            id = Spd[0x76] | 0x7f0000;
            break;
        case 3:
            id = Spd[0x76] | 0x7f000000;
            break;
        default:
            id = Spd[0x76];
            break;
    }

    DebugTrace(L"ContCount = %x, id = %x", ContCount, id);

    Index = 0;
    do
    {
        if (ManufacturerInfo[Index].dwID == id)
        {
            StringCbCopy(lpName, Size,
                         ManufacturerInfo[Index].lpszValue);
            return;
        }
    }
    while (ManufacturerInfo[++Index].dwID != 0);

    StringCbCopy(lpName, Size, L"Unknown");
}

INT
GetDDR3BanksCount(BYTE *Spd)
{
    switch ((Spd[0x04] & 0x10) >> 4) /* bits 6-4 */
    {
        case 0: return 8;
        case 1: return 16;
        case 2: return 32;
        case 3: return 64;
        default: return 8; /* Unknown */
    }
}

INT
GetDDR3RanksCount(BYTE *Spd)
{
    switch ((Spd[0x07] & 0x08) >> 3) /* bits 5-3 */
    {
        case 0: return 1;
        case 1: return 2;
        case 2: return 3;
        case 3: return 4;
        default: return 1; /* Unknown */
    }
}

VOID
ShowDDR3MemoryTimings(BYTE *Spd, double Latency, int cl)
{
    WCHAR szText[MAX_STR_LEN], szFormat[MAX_STR_LEN];
    INT ItemIndex;

    LoadMUIString(IDS_SPD_TIMING_FORMAT, szFormat, MAX_STR_LEN);
    StringCbPrintf(szText, sizeof(szText), szFormat, 1000.0 / Latency);
    ItemIndex = IoAddItem(1, 1, szText);
    IoSetItemText(ItemIndex, 1, L"%i-%i-%i-%i (CL-RCD-RP-RAS)/%i-%i-%i-%i-%i-%i (RC-RFC-RRD-WR-WTR-RTP)",
                  cl,
                  (int)Round((Spd[0x12] * 0.125)/Latency, 0),
                  (int)Round((Spd[0x14] * 0.125)/Latency, 0),
                  (int)Round(((Spd[0x16] | (Spd[0x15] & 0x0F) << 8) * 0.125)/Latency, 0),
                  (int)Round(((Spd[0x17] | ((Spd[0x15] >> 4) << 8)) * 0.125)/Latency, 0),
                  (int)Round(((Spd[0x18] | (Spd[0x19] << 8)) * 0.125)/Latency, 0),
                  (int)Round((Spd[0x13] * 0.125)/Latency, 0),
                  (int)Round((Spd[0x11] * 0.125)/Latency, 0),
                  (int)Round((Spd[0x1A] * 0.125)/Latency, 0),
                  (int)Round((Spd[0x1B] * 0.125)/Latency, 0));
}

VOID
ShowSpdDataForDDR3(BYTE *Spd)
{
    WCHAR szText[MAX_STR_LEN];
    INT ItemIndex, Size;
    WCHAR *lpText;

    DebugTrace(L"Show data for DDR3");

    /* Model */
    StringCbPrintf(szText, sizeof(szText),
                   L"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                   Spd[0x80], Spd[0x81], Spd[0x82], Spd[0x83], Spd[0x84],
                   Spd[0x85], Spd[0x86], Spd[0x87], Spd[0x88], Spd[0x89],
                   Spd[0x8A], Spd[0x8B], Spd[0x8C], Spd[0x8D], Spd[0x8E],
                   Spd[0x8F], Spd[0x90], Spd[0x91]);
    IoAddHeaderString(0, szText, 0);

    ItemIndex = IoAddValueName(1, IDS_MANUFACTURER, 0);
    GetDDR3Manufacturer(Spd, szText, sizeof(szText));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SERIAL_NUMBER, 0);
    IoSetItemText(ItemIndex, 1, L"%02X%02X%02X%02X",
                  Spd[0x7A], Spd[0x7B], Spd[0x7C], Spd[0x7D]);

    ItemIndex = IoAddValueName(1, IDS_SPD_PRODUCT_DATE, 0);
    IoSetItemText(ItemIndex, 1, L"Week %ld%ld, Year 20%ld%ld",
                  Spd[0x79] >> 4, Spd[0x79] & 0x0F,
                  Spd[0x78] >> 4, Spd[0x78] & 0x0F);

    ItemIndex = IoAddValueName(1, IDS_SPD_MEMORY_TYPE, 0);
    GetSpdModuleType(Spd, szText, sizeof(szText));
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddValueName(1, IDS_SPD_DIMM_TYPE, 0);

    switch (Spd[0x03] & 0x0F)
    {
        case 1:  lpText = L"RDIMM";        break;
        case 2:  lpText = L"UDIMM";        break;
        case 3:  lpText = L"SO-DIMM";      break;
        case 4:  lpText = L"Micro-DIMM";   break;
        case 5:  lpText = L"Mini-DIMM";    break;
        case 6:  lpText = L"Mini-RDIMM";   break;
        case 7:  lpText = L"Mini-UDIMM";   break;
        case 8:  lpText = L"Mini-CDIMM";   break;
        case 9:  lpText = L"72b-SO-UDIMM"; break;
        case 10: lpText = L"72b-SO-RDIMM"; break;
        case 11: lpText = L"72b-SO-CDIMM"; break;
        case 12: lpText = L"LRDIMM";       break;
        default: lpText = L"Unknown";      break;
    }
    IoSetItemText(ItemIndex, 1, lpText);

    ItemIndex = IoAddValueName(1, IDS_SPD_MODULE_SIZE, 0);
    switch (Spd[0x04] & 0x0F) /* bits 3-0 */
    {
        case 0:  Size = 256;  break;
        case 1:  Size = 512;  break;
        case 2:  Size = 1024; break;
        case 3:  Size = 2048; break;
        case 4:  Size = 4096; break;
        case 5:  Size = 8192; break;
        case 6:  Size = 16384;break;
        default: Size = 0;    break;
    }
    IoSetItemText(ItemIndex, 1, L"%d Mb (%ld ranks, %ld banks)",
                  Size * GetDDR3RanksCount(Spd),
                  GetDDR3RanksCount(Spd),
                  GetDDR3BanksCount(Spd));

    ItemIndex = IoAddValueName(1, IDS_SPD_FREQUENT, 0);
    switch (Spd[0x0C])
    {
        case 0x14:
            lpText = L"400 MHz (DDR3-800)";
            break;
        case 0x0F:
            lpText = L"533 MHz (DDR3-1066)";
            break;
        case 0x0C:
            lpText = L"667 MHz (DDR3-1333)";
            break;
        case 0x0A:
            lpText = L"800 MHz (DDR3-1600)";
            break;
        case 0x09:
            lpText = L"933 MHz (DDR3-1866)";
            break;
        case 0x08:
            lpText = L"1067 MHz (DDR3-2133)";
            break;
    }
    IoSetItemText(ItemIndex, 1, lpText);

    ItemIndex = IoAddValueName(1, IDS_SPD_ROW_ADDRESS_BITS, 0);
    switch ((Spd[0x05] & 0x3F) >> 3) /* bits 5 - 3 */
    {
        case 0: lpText = L"12"; break;
        case 1: lpText = L"13"; break;
        case 2: lpText = L"14"; break;
        case 3: lpText = L"15"; break;
        case 4: lpText = L"16"; break;
        default:lpText = L"Unknown"; break;
    }
    IoSetItemText(ItemIndex, 1, lpText);

    ItemIndex = IoAddValueName(1, IDS_SPD_COLUMN_ADDR_BITS, 0);
    switch (Spd[0x05] & 0x07) /* bits 2 - 0 */
    {
        case 0: lpText = L"9"; break;
        case 1: lpText = L"10"; break;
        case 2: lpText = L"11"; break;
        case 3: lpText = L"12"; break;
        default:lpText = L"Unknown"; break;
    }
    IoSetItemText(ItemIndex, 1, lpText);

    ItemIndex = IoAddValueName(1, IDS_SPD_DEVICE_WIDTH, 0);
    switch (Spd[0x07] & 0x07) /* bits 2 - 0 */
    {
        case 0: lpText = L"4 bits"; break;
        case 1: lpText = L"8 bits"; break;
        case 2: lpText = L"16 bits"; break;
        case 3: lpText = L"32 bits"; break;
        default:lpText = L"Unknown"; break;
    }
    IoSetItemText(ItemIndex, 1, lpText);

    ItemIndex = IoAddValueName(1, IDS_SPD_BUS_WIDTH, 0);
    switch (Spd[0x08] & 0x07) /* bits 2 - 0 */
    {
        case 0: lpText = L"8 bits"; break;
        case 1: lpText = L"16 bits"; break;
        case 2: lpText = L"32 bits"; break;
        case 3: lpText = L"64 bits"; break;
        default:lpText = L"Unknown"; break;
    }
    IoSetItemText(ItemIndex, 1, lpText);

    szText[0] = 0;
    ItemIndex = IoAddValueName(1, IDS_SPD_MODULE_NORM_VOLTAGE, 0);
    if (((Spd[0x06] & 0x07) >> 2) == 1) /* bit 2 */
        StringCbCat(szText, sizeof(szText), L"1.2V, ");
    if (((Spd[0x06] & 0x03) >> 1) == 1) /* bit 1 */
        StringCbCat(szText, sizeof(szText), L"1.35V, ");
    if ((Spd[0x06] & 0x01) == 0) /* bit 0 */
        StringCbCat(szText, sizeof(szText), L"1.5V, ");
    if (szText[0]) szText[wcslen(szText) - 2] = 0;
    IoSetItemText(ItemIndex, 1, szText);

    IoAddHeader(1, IDS_SPD_MEMORY_TIMINGS, 1);

    ItemIndex = IoAddHeaderString(1, L"Minimum SDRAM Cycle Time (tCKmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns", Spd[0x0c] * 0.125);

    ItemIndex = IoAddHeaderString(1, L"CAS# Latencies Supported", 1);
    szText[0] = 0;
    if (Spd[0x0e] & 0x01)
        StringCbCat(szText, sizeof(szText), L"4, ");
    if ((Spd[0x0e] & 0x02) >> 1)
        StringCbCat(szText, sizeof(szText), L"5, ");
    if ((Spd[0x0e] & 0x04) >> 2)
        StringCbCat(szText, sizeof(szText), L"6, ");
    if ((Spd[0x0e] & 0x08) >> 3)
        StringCbCat(szText, sizeof(szText), L"7, ");
    if ((Spd[0x0e] & 0x10) >> 4)
        StringCbCat(szText, sizeof(szText), L"8, ");
    if ((Spd[0x0e] & 0x20) >> 5)
        StringCbCat(szText, sizeof(szText), L"9, ");
    if ((Spd[0x0e] & 0x40) >> 6)
        StringCbCat(szText, sizeof(szText), L"10, ");
    if ((Spd[0x0e] & 0x80) >> 7)
        StringCbCat(szText, sizeof(szText), L"11, ");
    if (Spd[0x0f] & 0x01)
        StringCbCat(szText, sizeof(szText), L"12, ");
    if ((Spd[0x0f] & 0x02) >> 1)
        StringCbCat(szText, sizeof(szText), L"13, ");
    if ((Spd[0x0f] & 0x04) >> 2)
        StringCbCat(szText, sizeof(szText), L"14, ");
    if ((Spd[0x0f] & 0x08) >> 3)
        StringCbCat(szText, sizeof(szText), L"15, ");
    if ((Spd[0x0f] & 0x10) >> 4)
        StringCbCat(szText, sizeof(szText), L"16, ");
    if ((Spd[0x0f] & 0x20) >> 5)
        StringCbCat(szText, sizeof(szText), L"17, ");
    if ((Spd[0x0f] & 0x40) >> 6)
        StringCbCat(szText, sizeof(szText), L"18, ");
    szText[wcslen(szText) - 2] = 0;
    IoSetItemText(ItemIndex, 1, szText);

    ItemIndex = IoAddHeaderString(1, L"Minimum CAS# Latency Time (tAAmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns", Spd[0x10] * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum RAS# to CAS# Delay (tRCDmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns", Spd[0x12] * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum Row Procharge Time (tRPmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns", Spd[0x14] * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum Active to Precharge Time (tRASmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns",
                  (FLOAT)(Spd[0x16] | (Spd[0x15] & 0x0F) << 8) * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum Write Recovery Time (tWRmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns", Spd[0x11] * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum Row Active to Row Active Delay (tRRDmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns", Spd[0x13] * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum Active to Active/Refresh Time (tRCmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns",
                  (Spd[0x17] | ((Spd[0x15] >> 4) << 8)) * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum Refresh Recovery Time Delay (tRFCmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns",
                  (Spd[0x18] | (Spd[0x19] << 8)) * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum Internal Write to Read Command Delay (tWTRmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns", Spd[0x1A] * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum Internal Read to Precharge Command Delay (tRTPmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns", Spd[0x1B] * 0.125f);

    ItemIndex = IoAddHeaderString(1, L"Minimum Four Activate Window Delay Time (tFAWmin)", 1);
    IoSetItemText(ItemIndex, 1, L"%.3f ns",
                  (Spd[0x1D] | ((Spd[0x1C] & 0x0F) << 8)) * 0.125f);

    if ((Spd[0x0e] & 0x04) >> 2)
        ShowDDR3MemoryTimings(Spd, 2.18, 6); /* 400 */
    if ((Spd[0x0e] & 0x08) >> 3)
        ShowDDR3MemoryTimings(Spd, 1.84, 7); /* 533 */
    if ((Spd[0x0e] & 0x10) >> 4)
        ShowDDR3MemoryTimings(Spd, 1.642, 8); /* 609 */
    if ((Spd[0x0e] & 0x20) >> 5)
        ShowDDR3MemoryTimings(Spd, 1.459, 9); /* 667 */

    IoAddFooter();
}
