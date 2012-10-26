/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/spd.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"
#include "spd.h"


VOID
GetSpdModuleType(BYTE *Spd, LPWSTR lpszString, SIZE_T Size)
{
    SIZE_T Index = 0;

    do
    {
        if (RamTypeInfo[Index].dwID == Spd[0x02])
        {
            StringCbCopy(lpszString, Size,
                         RamTypeInfo[Index].lpszValue);
            return;
        }
    }
    while (RamTypeInfo[++Index].dwID != 0);

    StringCbCopy(lpszString, Size, L"Unknown");
}

/*
BOOL
IsValidChecksum(BYTE *Spd)
{
    DWORD checksum = 0;
    BYTE crc_coverage;
    INT i;

    if (Spd[0x02] >= 1 && Spd[0x02] <= 10)
    {
        for (i = 0; i <= 62; i++)
        {
            checksum += Spd[i];
        }
        checksum &= 255;

        return (checksum == Spd[0x63]) ? TRUE : FALSE;
    }
    else if (Spd[0x02] == 11)
    {
        BYTE bit;

        crc_coverage = (Spd[0x00] & 0x80) ? 117 : 126;

        for (i = 0; i < crc_coverage; i++)
        {
            checksum ^= Spd[i] << 8;

            for (bit = 0; bit < 8; bit++)
            {
                if (checksum & 0x8000)
                    checksum = (checksum << 1) ^ 0x1021;
                else
                    checksum <<= 1;
            }
        }
        checksum &= 0xFFFF;

        return ((checksum & 0xFF) == Spd[126] && (checksum >> 8) == Spd[127]) ? TRUE : FALSE;
    }

    return FALSE;
}
*/

VOID
ShowSpdData(BYTE *Spd)
{
    switch (Spd[0x02])
    {
        case 0x06: /* DDR SGRAM */
        case 0x07: /* DDR SDRAM */
            DebugTrace(L"DDR1 Memory (0x%x)", Spd[0x02]);
            ShowSpdDataForDDR(Spd);
            break;
        case 0x08: /* DDR2 SDRAM */
        case 0x09: /* DDR2 SDRAM FB-DIMM */
        case 0x0a: /* DDR2 SDRAM FB-DIMM PROBE */
            DebugTrace(L"DDR2 Memory (0x%x)", Spd[0x02]);
            ShowSpdDataForDDR2(Spd);
            break;
        case 0x0b: /* DDR3 SDRAM */
            DebugTrace(L"DDR3 Memory (0x%x)", Spd[0x02]);
            ShowSpdDataForDDR3(Spd);
            break;
        default: /* Unsupported type */
            DebugTrace(L"Unknown Memory Type (0x%x)", Spd[0x02]);
            break;
    }
}

VOID CALLBACK
EnumSmbusBaseAdrProc(WORD BaseAddress, DWORD ChipType)
{
    BYTE SpdData[SPD_MAX_SIZE] = {0};
    int i;

    for (i = 0xA0; i < 0xA8; i += 2)
    {
        if (IsCanceled) break;

        if (ReadSpdData(BaseAddress, ChipType, i + 1, SpdData))
        {
            DebugTrace(L"Show info in 0x%x slot", i + 1);
            ShowSpdData(SpdData);
        }
    }
}

VOID
HW_SPDInfo(VOID)
{
    DebugTrace(L"Start receiving SPD data");

    IoAddIcon(IDI_HW);
    IoAddIcon(IDI_TIME);

    EnumSmBusBaseAddress(EnumSmbusBaseAdrProc);

    DebugTrace(L"End receiving SPD data");
}
