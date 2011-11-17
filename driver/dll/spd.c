/*
 * PROJECT:         Aspia (Driver Helper DLL)
 * FILE:            driver/dll/spd.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <windows.h>
#include <stddef.h>
#include <strsafe.h>
#include "driver.h"


#define PCI_MAX_DEV 32
#define PCI_MAX_FUN 8

#define UNKNOWN_SMBUS   0x00     // Unknown
#define ATISB_SMBUS     0x01     // ATI SB400/SB600
#define ICH789_SMBUS    0x02     // Intel ICH7/ICH8/ICH9
#define SIS968_SMBUS    0x03     // SIS 968
#define NVCK804_SMBUS   0x04     // Nvidia nForce CK804
#define SIS962_SMBUS    0x05     // SIS 962/963
#define ICHX_SMBUS      0x06     // Intel ICHx
#define VIA8235_SMBUS   0x07     // VIA 8235/8237/8237A/8251
#define AMD8111_SMBUS   0x08     // AMD-8111

#define SMBUS_CLASS_CODE 0x0C0500


/* Vendor Id Codes */
#define VENDOR_ID_INTEL  0x8086
#define VENDOR_ID_ATI    0x1002
#define VENDOR_ID_NVIDIA 0x10DE
#define VENDOR_ID_AMD    0x1022

DWORD
ReadPciDword(BYTE bus, BYTE dev, BYTE func, BYTE offset)
{
    DWORD value, ind;
        
    ind = 0x80000000 + bus * 0x10000 + dev * 0x800 + func * 0x100;
    value = drv_get_register_data_dword(ind, offset);

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
    value = drv_get_register_data_dword(ind, 0);

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
    value = drv_get_register_data_dword(ind, 0);

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
    drv_write_io_port_dword(CONFIG_DATA, value);
}

BYTE
ReadPciByte(BYTE bus, BYTE dev, BYTE func, BYTE offset)
{
    DWORD ind, value;
    BYTE div;
     
    ind = 0x80000000 + bus * 0x10000 + dev * 0x800 + func * 0x100 + offset;
    div = (BYTE)(ind % (0x04));
    ind = ind - div;
    value = drv_get_register_data_dword(ind, 0);

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
    Value = drv_get_register_data_dword(ind, 0);

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
    drv_write_io_port_dword(CONFIG_DATA, Value);
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
        BYTE dev, fun;
        DWORD chip, addr;
        INT i;

        INT offsets[] =
        {
            0x90, /* PIIX4, VIA596, VIA686 */
            0xD0, /* VT8233/A/C */
            0x20, /* Intel801_ICH's */
            0x58, /* AMD756, 766, 768 */
            0x14, /* NVidia nForce */
            0xE2, /* ALI7101 */
            0x10, /* AMD8111 */
            0x50, /* NVidia nForce2, bus0 */
            0x54, /* NVidia nForce2, bus1 */
            0
        };

        for (dev = 0; dev < PCI_MAX_DEV; dev++)
        {
            for (fun = 0; fun < PCI_MAX_FUN; fun++)
            {
                chip = ReadPciDword(0, dev, fun, 0);
                if (chip == 0xFFFFFFFF)
                    continue;

                for (i = 0; offsets[i] != 0; i++)
                {
                    DWORD dev_class = ReadPciDword(0, dev, fun, 8);

                    if (dev_class / 0x100 != SMBUS_CLASS_CODE)
                        continue;

                    addr = ReadPciWord(0, dev, fun, offsets[i]);

                    if (addr == 0 || addr == 0xFFFFFFFF ||
                        (addr & 0x0F) != 1 || (addr & 0xFFF00000) != 0)
                        continue;

                    DebugTrace(L"SMBus found at bus = 0, dev = 0x%x, func = 0x%x, offset = 0x%x",
                               dev, fun, offsets[i]);

                    if (chip == 0x740B1022 || /* AMD-756 */
                        chip == 0x74131022) /* AMD-766 */
                    {
                        *BaseAddress = (addr & 0xFF00) + 0xE0;
                    }
                    else
                    {
                        *BaseAddress = (addr & 0xFFF0);
                    }

                    switch (chip)
                    {
                        case 0x30741106: /* VIA8233 */
                        case 0x31471106: /* VIA8233A */
                        case 0x31091106: /* VIA8233C */
                        case 0x31771106: /* VIA8235 */
                        case 0x82351106: /* VIA8235M */
                        case 0x32271106: /* VIA8237 */
                            return VIA8235_SMBUS;

                        case 0x740B1022: /* AMD756 */
                        case 0x74131022: /* AMD766 */
                        case 0x01B410DE: /* NFORCE */
                        case 0x005210DE: /* NFORCE CK804 */
                        case 0x74431022: /* AMD768 */
                            return NVCK804_SMBUS;

                        case 0x71138086: /* PIIX4 */
                        case 0x719B8086: /* PII440MX */
                        case 0x02001166: /* SRVWSB4 */
                        case 0x02011166: /* SRVWSB5 */
                        case 0x94631055: /* EFVIC66 */
                        case 0x24138086: /* I801AA */
                        case 0x24238086: /* I801AB */
                        case 0x24438086: /* I801BA */
                        case 0x24838086: /* I801CA */
                        case 0x24C38086: /* I801DB */
                        case 0x24D38086: /* I801EB */
                        case 0x266A8086: /* IICH6 */
                        case 0x283e8086: /* ICH8 */
                            return ICHX_SMBUS;

                        case 0x43851002: /* ATI SB850 */
                        case 0x30401106: /* VIA 586 */
                        case 0x30501106: /* VIA 596 */
                        case 0x30511106: /* VIA596B */
                        case 0x30571106: /* VIA 686 */
                        case 0x006410DE: /* NFORCE2 */
                        case 0x710110B9: /* ALI7101 */
                        case 0x153510B9: /* ALI1535 */
                        case 0x164710B9: /* ALI1647 */
                        case 0x25A48086: /* I6300ESB */
                        case 0x746A1022: /* AMD-8111 */
                            DebugTrace(L"Unsupported chip type (0x%x) yet!", chip);
                            return UNKNOWN_SMBUS;

                        default:
                            DebugTrace(L"chip = 0x%x", chip);
                            break;
                    }

                    return UNKNOWN_SMBUS;
                }
            }
        }
    }

    return UNKNOWN_SMBUS;
}

VOID
quaere(BYTE *v)
{
    INT i, j;

    for (i = 0; i < SPD_MAX_SIZE; i++)
    {
        if (v[i] == SPD_MAX_SIZE)
        {
            for (j = 0; j < SPD_MAX_SIZE; j++)
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

    for (Index = 0; Index < SPD_MAX_SIZE; Index++)
    {
        error = 0;
        do
        {
            drv_write_io_port_byte(BaseAddress + 0x00, 0xFF);
            flag = drv_read_io_port_byte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x9F) != 0);

        drv_write_io_port_byte((BaseAddress + 0x04), Slot);
        drv_write_io_port_byte((BaseAddress + 0x03), Index);
        drv_write_io_port_byte((BaseAddress + 0x02), 0x48);

        error = 0;
        do
        {
            if ((drv_read_io_port_byte(BaseAddress + 0x00) & 0x04) == 0x04)
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
        while ((drv_read_io_port_byte(BaseAddress + 0x00) & 0x02) != 0x02);

        SpdData[Index] = drv_read_io_port_byte(BaseAddress + 0x05);
    }

    quaere(SpdData);

    return TRUE;
}

BOOL
ReadICHXSmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < SPD_MAX_SIZE; Index++)
    {
        error = 0;
        do
        {
            drv_write_io_port_byte(BaseAddress + 0x00, 0xFF);
            flag = drv_read_io_port_byte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x9F) != 0);

        drv_write_io_port_byte((BaseAddress + 0x04), Slot);
        drv_write_io_port_byte((BaseAddress + 0x03), Index);
        drv_write_io_port_byte((BaseAddress + 0x02), 0x48);

        error = 0;
        do
        {
            if ((drv_read_io_port_byte(BaseAddress + 0x00) & 0x04) == 0x04)
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
        while ((drv_read_io_port_byte(BaseAddress + 0x00) & 0x02) != 0x02);

        Spd[Index] = drv_read_io_port_byte(BaseAddress + 0x05);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
ReadSIS962SmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < SPD_MAX_SIZE; Index++)
    {
        error = 0;
        do
        {
            drv_write_io_port_byte(BaseAddress + 0x00, 0xFF);
            flag = drv_read_io_port_byte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while (flag != 0);

        drv_write_io_port_byte((BaseAddress + 0x04), Slot);
        drv_write_io_port_byte((BaseAddress + 0x05), Index);
        drv_write_io_port_byte((BaseAddress + 0x03), 0x12);

        error = 0;
        do
        {
            if ((drv_read_io_port_byte(BaseAddress + 0x00) & 0x02) == 0x02)
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
        while (drv_read_io_port_byte(BaseAddress + 0x00) != 0x08);

        Spd[Index] = drv_read_io_port_byte(BaseAddress + 0x08);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
ReadSIS968SmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < SPD_MAX_SIZE; Index++)
    {
        error = 0;
        do
        {
            drv_write_io_port_byte(BaseAddress + 0x00, 0xFF);
            flag = drv_read_io_port_byte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while(flag != 0);

        drv_write_io_port_byte((BaseAddress + 0x04), Slot);
        drv_write_io_port_byte((BaseAddress + 0x05), Index);
        drv_write_io_port_byte((BaseAddress + 0x03), 0x12);

        error = 0;
        do
        {
            if ((drv_read_io_port_byte(BaseAddress + 0x00) & 0x02) == 0x02)
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
        while ((drv_read_io_port_byte(BaseAddress + 0x00) & 0x08) != 0x08);

        Spd[Index] = drv_read_io_port_byte(BaseAddress + 0x10);
        drv_write_io_port_byte(BaseAddress + 0x00, flag & 0x08);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
ReadNVCK804SmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < SPD_MAX_SIZE; Index++)
    {
        error = 0;
        do
        {
            drv_write_io_port_byte(BaseAddress + 0x00, 0xFF);
            flag = drv_read_io_port_byte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x9F) != 0);

        drv_write_io_port_byte((BaseAddress + 0x02), Slot - 1);
        drv_write_io_port_byte((BaseAddress + 0x03), Index);
        drv_write_io_port_byte((BaseAddress + 0x00), 0x07);

        error = 0;
        do
        {
            if (++error > 0x100)
            {
                /* Read error */
                return FALSE;
            }
            if ((drv_read_io_port_byte(BaseAddress + 0x01) & 0x10) == 0x10)
            {
                /* No module in channel */
                return FALSE;
            }
            Sleep(1);
        }
        while (drv_read_io_port_byte(BaseAddress + 0x01) != SPD_MAX_SIZE);
        Spd[Index] = drv_read_io_port_byte(BaseAddress + 0x04);
    }
    return TRUE;
  //quaere(Spd);
}

BOOL
ReadATISBSmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < SPD_MAX_SIZE; Index++)
    {
        error = 0;
        do
        {
            drv_write_io_port_byte(BaseAddress + 0x00, 0xFF);
            flag = drv_read_io_port_byte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x9F) != 0);

        drv_write_io_port_byte((BaseAddress + 0x04), Slot);
        drv_write_io_port_byte((BaseAddress + 0x03), Index);
        drv_write_io_port_byte((BaseAddress + 0x02), 0x48);

        error = 0;
        do
        {
            if (++error > 0x8000)
            {
                /* Read error */
                return FALSE;
            }
            if ((drv_read_io_port_byte(BaseAddress + 0x00) & 0x06) == 0x06)
            {
                /* No module in channel */
                return FALSE;
            }
            //Sleep(1);
        }
        while ((drv_read_io_port_byte(BaseAddress + 0x00) & 0x06) != 0x02);
        Spd[Index] = drv_read_io_port_byte(BaseAddress + 0x05);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
ReadVIA8235SmBus(BYTE *Spd, WORD BaseAddress, BYTE Slot)
{
    INT error;
    BYTE flag, Index;

    for (Index = 0; Index < SPD_MAX_SIZE; Index++)
    {
        error = 0;

        drv_write_io_port_byte(BaseAddress + 0x00, 0xFF);
        do
        {
            flag = drv_read_io_port_byte(BaseAddress + 0x00);

            if (++error > 0x8000)
            {
                /* Reset error */
                return FALSE;
            }
        }
        while ((flag & 0x40) != 0x40);

        drv_write_io_port_byte((BaseAddress + 0x04), Slot);
        drv_write_io_port_byte((BaseAddress + 0x03), Index);
        drv_write_io_port_byte((BaseAddress + 0x02), 0x48);

        error = 0;

        flag = drv_read_io_port_byte(BaseAddress + 0x00);
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
            flag = drv_read_io_port_byte(BaseAddress + 0x00);
        }
        while ((flag & 0x02) != 0x02);

        Spd[Index] = drv_read_io_port_byte(BaseAddress + 0x05);
    }
    quaere(Spd);

    return TRUE;
}

BOOL
drv_read_spd_data(BYTE Slot, BYTE *SpdData)
{
    WORD BaseAddress = 0;
    DWORD dwType;

    dwType = GetSmBusBaseAddress(&BaseAddress);

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
            DebugTrace(L"Unknown SMBus");
            break;
    }

    return FALSE;
}
