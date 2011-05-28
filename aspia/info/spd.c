/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/spd.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include "spd.h"


#define UNKNOWN_SMBUS   0x00     // Unknown
#define ATISB_SMBUS     0x01     // ATI SB400/SB600
#define ICH789_SMBUS    0x02     // Intel ICH7/ICH8/ICH9
#define SIS968_SMBUS    0x03     // SIS 968
#define NVCK804_SMBUS   0x04     // Nvidia nForce CK804
#define SIS962_SMBUS    0x05     // SIS 962/963
#define ICHX_SMBUS      0x06     // Intel ICHx
#define VIA8235_SMBUS   0x07     // VIA 8235/8237/8237A/8251

#define SMBUS_CLASS_CODE 0x0C0500


/* Vendor Id Codes */
#define VENDOR_ID_INTEL  0x8086
#define VENDOR_ID_ATI    0x1002
#define VENDOR_ID_NVIDIA 0x10DE


DWORD
ReadPciDword(BYTE bus, BYTE dev, BYTE func, BYTE offset)
{
    DWORD value, ind;
        
    ind = 0x80000000 + bus * 0x10000 + dev * 0x800 + func * 0x100;
    value = DRIVER_GetRegisterDataDword(ind, offset);

    return value;
}

WORD
ReadPciWord(BYTE bus, BYTE dev, BYTE func, BYTE offset)
{
    DWORD value, ind;
    BYTE div;

    ind = 0x80000000 + bus * 0x10000 + dev * 0x800 + func * 0x100 + offset;
    div = (BYTE)(ind % (0x04));
    ind = ind - div;
    value = DRIVER_GetRegisterDataDword(ind, 0);

    return (value >> (8 * div)) & 0xffff;
}

VOID
WritePciByte(BYTE bus, BYTE dev, BYTE func, BYTE offset, BYTE byte_data)
{
    DWORD  ind, value;
    BYTE div;
     
    ind = 0x80000000 + bus * 0x10000 + dev * 0x800 + func * 0x100 + offset;
    div = (BYTE)(ind % (0x04));
    ind = ind - div;
    value = DRIVER_GetRegisterDataDword(ind, 0);

    switch (div)
    {
        case 0:
            value = (value & 0xffffff00) + byte_data * 0x00000001;
            break;
        case 1:
            value = (value & 0xffff00ff) + byte_data * 0x00000100;
            break;
        case 2:
            value = (value & 0xff00ffff) + byte_data * 0x00010000;
            break;
        case 3:
            value = (value & 0x00ffffff) + byte_data * 0x01000000;
            break;
        default:
        break;
    }
    DRIVER_WriteIoPortDword(CONFIG_DATA, value);
}

BYTE
ReadPciByte(BYTE bus, BYTE dev, BYTE func, BYTE offset)
{
    DWORD ind, value;
    BYTE div;
     
    ind = 0x80000000 + bus * 0x10000 + dev * 0x800 + func * 0x100 + offset;
    div = (BYTE)(ind % (0x04));
    ind = ind - div;
    value = DRIVER_GetRegisterDataDword(ind, 0);

    return (value >> (8 * div)) & 0xff;
}

VOID
WritePciWord(BYTE bus, BYTE dev, BYTE func, BYTE offset, WORD word_data)
{
    DWORD ind, Value;
    BYTE div;
     
    ind = 0x80000000 + bus * 0x10000 + dev * 0x800 + func * 0x100 + offset;
    div = (BYTE)(ind % (0x04));
    ind = ind - div;
    Value = DRIVER_GetRegisterDataDword(ind, 0);

    switch (div)
    {
        case 0:
            Value = (Value & 0xffff0000) + word_data * 0x00000001;
            break;
        case 2:
            Value = (Value & 0x0000ffff) + word_data * 0x00010000;
            break;
        default:
            break;
    }
    DRIVER_WriteIoPortDword(CONFIG_DATA, Value);
}

DWORD
GetSmBusBaseAddress(WORD *BaseAddress)
{
    if (!BaseAddress) return UNKNOWN_SMBUS;

    /* SIS 968 South Bridge */
    if (ReadPciDword(0, 2, 0, 0) == 0x09681039)
    {
        *BaseAddress = (WORD)(ReadPciWord(0x00, 0x02, 0x00, 0x9a) & 0xff00);
        return SIS968_SMBUS;
    }
    /* SIS 962/963 South Bridge */
    else if (ReadPciDword(0, 2, 0, 0) == 0x00081039)
    {
        WritePciByte(0x00, 0x02, 0x00, 0x77, (ReadPciByte(0x00, 0x20, 0x00, 0x77) & 0xef));
        WritePciByte(0x00, 0x02, 0x01, 0x40, 0x01);
        WritePciWord(0x00, 0x02, 0x01, 0x20, 0x0c00);
        WritePciByte(0x00, 0x02, 0x01, 0x04, 0x01);

        *BaseAddress = (WORD)(ReadPciWord(0x00, 0x02, 0x01, 0x20) & 0xfff0);

        return SIS962_SMBUS;
    }
    /* VIA 8235/8237/8237A/8251 */
    else if ((ReadPciDword(0x00, 0x11, 0x00, 0x00) & 0xf000ffff) == 0x30001106)
    {
        *BaseAddress = (WORD)(ReadPciWord(0x00, 0x11, 0x00, 0xd0) & 0xfff0);
        return VIA8235_SMBUS;
    }
    else
    {
        DWORD io_cf8;
        DWORD io_cfc;
        DWORD io_cfc_bak = 0x00;
        DWORD Class;

        for (io_cf8 = 0x80000000; io_cf8 < 0x80ffff00; io_cf8 += 0x100)
        {
            io_cfc = DRIVER_GetRegisterDataDword(io_cf8, 0);

            if ((io_cfc != 0x00000000) && (io_cfc != 0xffffffff) && (io_cfc != io_cfc_bak))
            {
                Class = DRIVER_GetRegisterDataDword(io_cf8, 8);

                if ((Class / 0x100) == SMBUS_CLASS_CODE)
                {
                    DWORD level;

                    switch (io_cfc & 0xffff)
                    {
                        /* ATI SB400/SB600 */
                        case VENDOR_ID_ATI:
                        {
                            *BaseAddress = DRIVER_GetRegisterDataWord(io_cf8, 0x10) & 0xFFF0;
                            return ATISB_SMBUS;
                        }
                        break;

                        case VENDOR_ID_INTEL:
                        {
                            level = ReadPciDword(0x00, 0x1f, 0x00, 0x00) & 0xff00ffff;

                            if ((level == 0x27008086)||(level == 0x28008086)||(level == 0x29008086))
                            {
                                *BaseAddress = DRIVER_GetRegisterDataWord(io_cf8, 0x20) & 0xFFF0;
                                return ICH789_SMBUS;
                            }
                            else
                            {
                                *BaseAddress = DRIVER_GetRegisterDataWord(io_cf8, 0x20) & 0xFFF0;
                                return ICHX_SMBUS;
                            }
                        }
                        break;

                        case VENDOR_ID_NVIDIA:
                        {
                            /* NVIDIA nForce CK804 */
                            if (ReadPciDword(0, 1, 1, 0) == 0x005210DE)
                            {
                                *BaseAddress = (ReadPciWord(0x00, 0x01, 0x01, 0x50)) & 0xfff0;
                                return NVCK804_SMBUS;
                            }
                            return UNKNOWN_SMBUS;
                        }
                        break;

                        default:
                            break;
                    }
                }
                io_cfc_bak = io_cfc;
            }
        }
    }

    return UNKNOWN_SMBUS;
}

VOID
quaere(BYTE *v)
{
    INT i, j;

    for (i = 0; i < 0x80; i++)
    {
        if (v[i] == 0x80)
        {
            for (j = 0; j < 0x80; j++)
            {
                v[j] = v[j + i];
            }
            break;
        }
    }
}

BOOL
ReadICH789SmBus(BYTE *SpdData,
                WORD BaseAddress,
                BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < 0x80; Index++)
    {
        error = 0;
        do
        {
            DRIVER_WriteIoPortByte(BaseAddress + 0x00, 0xFF);
            flag = DRIVER_ReadIoPortByte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x9F) != 0);

        DRIVER_WriteIoPortByte((BaseAddress + 0x04), Slot);
        DRIVER_WriteIoPortByte((BaseAddress + 0x03), Index);
        DRIVER_WriteIoPortByte((BaseAddress + 0x02), 0x48);

        error = 0;
        do
        {
            if ((DRIVER_ReadIoPortByte(BaseAddress + 0x00) & 0x04) == 0x04)
            {
                /* No module in channel */
                return FALSE;
            }

            if (++error > 0x100)
            {
                /* Read error */
                return FALSE;
            }

            Sleep(1);
        }
        while ((DRIVER_ReadIoPortByte(BaseAddress + 0x00) & 0x02) != 0x02);

        SpdData[Index] = DRIVER_ReadIoPortByte(BaseAddress + 0x05);
    }

    quaere(SpdData);

    return TRUE;
}

BOOL
ReadICHXSmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < 0x80; Index++)
    {
        error = 0;
        do
        {
            DRIVER_WriteIoPortByte(BaseAddress + 0x00, 0xFF);
            flag = DRIVER_ReadIoPortByte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x9F) != 0);

        DRIVER_WriteIoPortByte((BaseAddress + 0x04), Slot);
        DRIVER_WriteIoPortByte((BaseAddress + 0x03), Index);
        DRIVER_WriteIoPortByte((BaseAddress + 0x02), 0x48);

        error = 0;
        do
        {
            if ((DRIVER_ReadIoPortByte(BaseAddress + 0x00) & 0x04) == 0x04)
            {
                /* No module in channel */
                return FALSE;
            }
            if (++error > 0x100)
            {
                /* Read error */
                return FALSE;
            }
            Sleep(1);
        }
        while ((DRIVER_ReadIoPortByte(BaseAddress + 0x00) & 0x02) != 0x02);

        Spd[Index] = DRIVER_ReadIoPortByte(BaseAddress + 0x05);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
ReadSIS962SmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < 0x80; Index++)
    {
        error = 0;
        do
        {
            DRIVER_WriteIoPortByte(BaseAddress + 0x00, 0xFF);
            flag = DRIVER_ReadIoPortByte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while (flag != 0);

        DRIVER_WriteIoPortByte((BaseAddress + 0x04), Slot);
        DRIVER_WriteIoPortByte((BaseAddress + 0x05), Index);
        DRIVER_WriteIoPortByte((BaseAddress + 0x03), 0x12);

        error = 0;
        do
        {
            if ((DRIVER_ReadIoPortByte(BaseAddress + 0x00) & 0x02) == 0x02)
            {
                /* No module in channel */
                return FALSE;
            }
            if (++error > 0x100)
            {
                /* Read error */
                return FALSE;
            }
            Sleep(1);
        }
        while (DRIVER_ReadIoPortByte(BaseAddress + 0x00) != 0x08);

        Spd[Index] = DRIVER_ReadIoPortByte(BaseAddress + 0x08);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
ReadSIS968SmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < 0x80; Index++)
    {
        error = 0;
        do
        {
            DRIVER_WriteIoPortByte(BaseAddress + 0x00, 0xFF);
            flag = DRIVER_ReadIoPortByte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while(flag != 0);

        DRIVER_WriteIoPortByte((BaseAddress + 0x04), Slot);
        DRIVER_WriteIoPortByte((BaseAddress + 0x05), Index);
        DRIVER_WriteIoPortByte((BaseAddress + 0x03), 0x12);

        error = 0;
        do
        {
            if ((DRIVER_ReadIoPortByte(BaseAddress + 0x00) & 0x02) == 0x02)
            {
                /* No module in channel */
                return FALSE;
            }
            if (++error > 0x100)
            {
                /* Read error */
                return FALSE;
            }
            Sleep(1);
        }
        while ((DRIVER_ReadIoPortByte(BaseAddress + 0x00) & 0x08) != 0x08);

        Spd[Index] = DRIVER_ReadIoPortByte(BaseAddress + 0x10);
        DRIVER_WriteIoPortByte(BaseAddress + 0x00, flag & 0x08);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
ReadNVCK804SmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < 0x80; Index++)
    {
        error = 0;
        do
        {
            DRIVER_WriteIoPortByte(BaseAddress + 0x00, 0xFF);
            flag = DRIVER_ReadIoPortByte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x9F) != 0);

        DRIVER_WriteIoPortByte((BaseAddress + 0x02), Slot - 1);
        DRIVER_WriteIoPortByte((BaseAddress + 0x03), Index);
        DRIVER_WriteIoPortByte((BaseAddress + 0x00), 0x07);

        error = 0;
        do
        {
            if (++error > 0x100)
            {
                /* Read error */
                return FALSE;
            }
            if ((DRIVER_ReadIoPortByte(BaseAddress + 0x01) & 0x10) == 0x10)
            {
                /* No module in channel */
                return FALSE;
            }
            Sleep(1);
        }
        while (DRIVER_ReadIoPortByte(BaseAddress + 0x01) != 0x80);
        Spd[Index] = DRIVER_ReadIoPortByte(BaseAddress + 0x04);
    }
    return TRUE;
  //quaere(Spd);
}

BOOL
ReadATISBSmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < 0x80; Index++)
    {
        error = 0;
        do
        {
            DRIVER_WriteIoPortByte(BaseAddress + 0x00, 0xFF);
            flag = DRIVER_ReadIoPortByte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x9F) != 0);

        DRIVER_WriteIoPortByte((BaseAddress + 0x04), Slot);
        DRIVER_WriteIoPortByte((BaseAddress + 0x03), Index);
        DRIVER_WriteIoPortByte((BaseAddress + 0x02), 0x48);

        error = 0;
        do
        {
            if (++error > 0x8000)
            {
                /* Read error */
                return FALSE;
            }
            if ((DRIVER_ReadIoPortByte(BaseAddress + 0x00) & 0x06) == 0x06)
            {
                /* No module in channel */
                return FALSE;
            }
            //Sleep(1);
        }
        while ((DRIVER_ReadIoPortByte(BaseAddress + 0x00) & 0x06) != 0x02);
        Spd[Index] = DRIVER_ReadIoPortByte(BaseAddress + 0x05);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
ReadVIA8235SmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < 0x80; Index++)
    {
        error = 0;

        DRIVER_WriteIoPortByte(BaseAddress + 0x00, 0xFF);
        do
        {
            flag = DRIVER_ReadIoPortByte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x40) != 0x40);

        DRIVER_WriteIoPortByte((BaseAddress + 0x04), Slot);
        DRIVER_WriteIoPortByte((BaseAddress + 0x03), Index);
        DRIVER_WriteIoPortByte((BaseAddress + 0x02), 0x48);

        error = 0;

        flag = DRIVER_ReadIoPortByte(BaseAddress + 0x00);
        do
        {
            if (++error > 0x100)
            {
                /* Read error */
                return FALSE;
            }
            if ((flag & 0x04) == 0x04)
            {
                /* No module in channel */
                return FALSE;
            }
            Sleep(1);
            flag = DRIVER_ReadIoPortByte(BaseAddress + 0x00);
        }
        while ((flag & 0x02) != 0x02);

        Spd[Index] = DRIVER_ReadIoPortByte(BaseAddress + 0x05);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
ReadSPDDataFromSmBus(DWORD dwType,
                     WORD BaseAddress,
                     BYTE Slot,
                     BYTE *SpdData)
{
    if (BaseAddress == 0)
        return FALSE;

    switch (dwType)
    {
        case ATISB_SMBUS:
            return ReadATISBSmBus(SpdData, BaseAddress, Slot);

        case SIS968_SMBUS:
            return ReadSIS968SmBus(SpdData, BaseAddress, Slot);

        case ICH789_SMBUS:
            return ReadICH789SmBus(SpdData, BaseAddress, Slot);

        case ICHX_SMBUS:
            return ReadICHXSmBus(SpdData, BaseAddress, Slot);

        case NVCK804_SMBUS:
            return ReadNVCK804SmBus(SpdData, BaseAddress, Slot);

        case SIS962_SMBUS:
            return ReadSIS962SmBus(SpdData, BaseAddress, Slot);

        case VIA8235_SMBUS:
            return ReadVIA8235SmBus(SpdData, BaseAddress, Slot);

        default:
            break;
    }

    return FALSE;
}

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

BOOL
IsBadSpdData(BYTE *Spd)
{
    SIZE_T Index;

    for (Index = 0x40; Index < 0x80; Index++)
    {
        if (Spd[Index] == 0xff)
            return TRUE;
    }

    if (Spd[0x00] != 0x80)
        return TRUE;

    return FALSE;
}

VOID
ShowSpdData(BYTE *Spd)
{
    if (IsBadSpdData(Spd))
    {
        DebugTrace(L"Bad SPD data");
        return;
    }

    switch (Spd[0x02])
    {
        case 0x06: /* DDR SGRAM */
        case 0x07: /* DDR SDRAM */
            ShowSpdDataForDDR(Spd);
            break;
        case 0x08: /* DDR2 SDRAM */
        case 0x09: /* DDR2 SDRAM FB-DIMM */
        case 0x10: /* DDR2 SDRAM FB-DIMM PROBE */
            ShowSpdDataForDDR2(Spd);
            break;
        case 0x11: /* DDR3 SDRAM */
            ShowSpdDataForDDR3(Spd);
            break;
        default: /* Unsupported type */
            break;
    }
}

VOID
HW_SPDInfo(VOID)
{
    BYTE SpdData[0x80] = {0};
    WORD BaseAddress = 0;
    DWORD dwType;

    DebugTrace(L"Start receiving SPD data");

    IoAddIcon(IDI_HW);
    IoAddIcon(IDI_TIME);

    dwType = GetSmBusBaseAddress(&BaseAddress);

    if (dwType == UNKNOWN_SMBUS)
    {
        DebugTrace(L"Unknown SMBus");
        return;
    }

    if (ReadSPDDataFromSmBus(dwType, BaseAddress, 0xA1, SpdData))
        ShowSpdData(SpdData);

    if (ReadSPDDataFromSmBus(dwType, BaseAddress, 0xA3, SpdData))
        ShowSpdData(SpdData);

    if (ReadSPDDataFromSmBus(dwType, BaseAddress, 0xA5, SpdData))
        ShowSpdData(SpdData);

    if (ReadSPDDataFromSmBus(dwType, BaseAddress, 0xA7, SpdData))
        ShowSpdData(SpdData);

    DebugTrace(L"End receiving SPD data");
}
