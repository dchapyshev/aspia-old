/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/edid.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#define DETAILED_TIMING_DESCRIPTIONS_START      0x36
#define DETAILED_TIMING_DESCRIPTION_SIZE        18
#define NO_DETAILED_TIMING_DESCRIPTIONS         4
#define ID_MANUFACTURER_NAME                    0x08
#define DESCRIPTOR_DATA         5
#define MONITOR_NAME            0xfc
#define MONITOR_LIMITS          0xfd
#define ID_MODEL                0x0a
#define UNKNOWN_DESCRIPTOR      -1
#define DETAILED_TIMING_BLOCK   -2

#define UPPER_NIBBLE(x) (((128|64|32|16) & (x)) >> 4)
#define COMBINE_HI_8LO(hi, lo) ((((unsigned)hi) << 8) | (unsigned)lo)

#define H_ACTIVE_LO  (unsigned)Block[2]
#define H_ACTIVE_HI  UPPER_NIBBLE((unsigned)Block[ 4 ])
#define H_ACTIVE     COMBINE_HI_8LO(H_ACTIVE_HI, H_ACTIVE_LO)

#define V_ACTIVE_LO  (unsigned)Block[ 5 ]
#define V_ACTIVE_HI  UPPER_NIBBLE((unsigned)Block[7])
#define V_ACTIVE     COMBINE_HI_8LO(V_ACTIVE_HI, V_ACTIVE_LO)

#define DPMS_FLAGS          0x18
#define DPMS_ACTIVE_OFF     (1 << 5)
#define DPMS_SUSPEND        (1 << 6)
#define DPMS_STANDBY        (1 << 7)

#define MANUFACTURE_WEEK       0x10
#define MANUFACTURE_YEAR       0x11
#define EDID_STRUCT_VERSION    0x12
#define EDID_STRUCT_REVISION   0x13


__inline double
GetDiagonalSize(INT Horiz, INT Vert)
{
    /* Находим диагональ монитора по теореме Пифагора
       и переводим из сантиметров в дюймы */
    return (double)(sqrt((Horiz * Horiz) + (Vert * Vert))) / 2.54;
}


WCHAR* GetEdidVendorSign(BYTE const *Block);
INT EdidBlockType(BYTE* Block);
