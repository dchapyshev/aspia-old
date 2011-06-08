/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/spd/ddr3.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../../main.h"
#include "../spd.h"


VOID
ShowSpdDataForDDR3(BYTE *Spd)
{
    WCHAR szText[MAX_STR_LEN];

    DebugTrace(L"Show data for DDR3");

    /* Model */
    StringCbPrintf(szText, sizeof(szText),
                   L"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                   Spd[0x80], Spd[0x81], Spd[0x82], Spd[0x83], Spd[0x84],
                   Spd[0x85], Spd[0x86], Spd[0x87], Spd[0x88], Spd[0x89],
                   Spd[0x8A], Spd[0x8B], Spd[0x8C], Spd[0x8D], Spd[0x8E],
                   Spd[0x8F], Spd[0x90], Spd[0x91]);
    IoAddHeaderString(0, szText, 0);

    IoAddFooter();
}
