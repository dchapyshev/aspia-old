/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/W836XX.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include "lpc.h"


#define WINBOND_VENDOR_ID             0x5CA3
#define HIGH_BYTE                     0x80

#define ADDRESS_REGISTER_OFFSET       0x05
#define DATA_REGISTER_OFFSET          0x06

#define VOLTAGE_VBAT_REG              0x51
#define BANK_SELECT_REGISTER          0x4E
#define VENDOR_ID_REGISTER            0x4F
#define TEMPERATURE_SOURCE_SELECT_REG 0x49

static BYTE TEMPERATURE_REG[] = { 0x50, 0x50, 0x27 };
BYTE TEMPERATURE_BANK[] = { 1, 2, 0 };

BYTE FAN_TACHO_REG[] = { 0x28, 0x29, 0x2A, 0x3F, 0x53 };
BYTE FAN_TACHO_BANK[] = { 0, 0, 0, 0, 5 };
BYTE FAN_BIT_REG[] = { 0x47, 0x4B, 0x4C, 0x59, 0x5D };
BYTE FAN_DIV_BIT0[] = { 36, 38, 30, 8, 10 };
BYTE FAN_DIV_BIT1[] = { 37, 39, 31, 9, 11 };
BYTE FAN_DIV_BIT2[] = { 5, 6, 7, 23, 15 };


BYTE
W836XX_ReadByte(WORD address, BYTE bank, BYTE reg)
{
    DRIVER_WriteIoPortByte((WORD)(address + ADDRESS_REGISTER_OFFSET), BANK_SELECT_REGISTER);
    DRIVER_WriteIoPortByte((WORD)(address + DATA_REGISTER_OFFSET), bank);
    DRIVER_WriteIoPortByte((WORD)(address + ADDRESS_REGISTER_OFFSET), reg);

    return DRIVER_ReadIoPortByte((WORD)(address + DATA_REGISTER_OFFSET));
}

VOID
W836XX_WriteByte(WORD address, BYTE bank, BYTE reg, BYTE value)
{
    DRIVER_WriteIoPortByte((WORD)(address + ADDRESS_REGISTER_OFFSET), BANK_SELECT_REGISTER);
    DRIVER_WriteIoPortByte((WORD)(address + DATA_REGISTER_OFFSET), bank);
    DRIVER_WriteIoPortByte((WORD)(address + ADDRESS_REGISTER_OFFSET), reg);
    DRIVER_WriteIoPortByte((WORD)(address + DATA_REGISTER_OFFSET), value);
}

DWORD
W836XX_SetBit(DWORD target, INT bit, INT value)
{
    DWORD mask;

    if ((value & 1) != value)
        return 0;

    if (bit < 0 || bit > 63)
        return 0;

    mask = (((DWORD)1) << bit);
    return value > 0 ? target | mask : target & ~mask;
}

BOOL
IsWinbondVendor(WORD address)
{
    WORD vendorId = (WORD)((W836XX_ReadByte(address, HIGH_BYTE, VENDOR_ID_REGISTER) << 8) |
        W836XX_ReadByte(address, 0, VENDOR_ID_REGISTER));

    return (vendorId == WINBOND_VENDOR_ID);
}

VOID
W836XX_GetInfo(WORD wChipType, BYTE revision, WORD address)
{
    BOOL peciTemperature[3];
    BYTE voltageRegister[10] = {0};
    BYTE voltageBank[10] = {0};
    INT iMaxVoltages, iMaxFans, i, value;
    FLOAT fvalue, voltageGain = 0.008f;
    DWORD newBits, bits;

    if (!IsWinbondVendor(address))
        return;

    switch (wChipType)
    {
        case W83667HG:
        case W83667HGB:
        {
            /* note temperature sensor registers that read PECI */
            BYTE flag = W836XX_ReadByte(address, 0, TEMPERATURE_SOURCE_SELECT_REG);
            peciTemperature[0] = (flag & 0x04) != 0;
            peciTemperature[1] = (flag & 0x40) != 0;
            peciTemperature[2] = FALSE;
        }
        break;

        case W83627DHG:
        case W83627DHGP:
        {
            /* note temperature sensor registers that read PECI */
            BYTE sel = W836XX_ReadByte(address, 0, TEMPERATURE_SOURCE_SELECT_REG);
            peciTemperature[0] = (sel & 0x07) != 0;
            peciTemperature[1] = (sel & 0x70) != 0;
            peciTemperature[2] = FALSE;
        }
        break;

        default:
            /* no PECI support */
            peciTemperature[0] = FALSE;
            peciTemperature[1] = FALSE;
            peciTemperature[2] = FALSE;
            break;
    }

    switch (wChipType)
    {
        case W83627EHF:
        {
            iMaxVoltages = 10;

            voltageRegister[0] = 0x20; voltageRegister[1] = 0x21;
            voltageRegister[2] = 0x22; voltageRegister[3] = 0x23;
            voltageRegister[4] = 0x24; voltageRegister[5] = 0x25;
            voltageRegister[6] = 0x26; voltageRegister[7] = 0x50;
            voltageRegister[8] = 0x51; voltageRegister[9] = 0x52;

            voltageBank[7] = 5; voltageBank[8] = 5;
            voltageBank[9] = 5;

            voltageGain = 0.008f;
            iMaxFans = 5;
        }
        break;

        case W83627DHG:
        case W83627DHGP:
        case W83667HG:
        case W83667HGB:
        {
            iMaxVoltages = 9;

            voltageRegister[0] = 0x20; voltageRegister[1] = 0x21;
            voltageRegister[2] = 0x22; voltageRegister[3] = 0x23;
            voltageRegister[4] = 0x24; voltageRegister[5] = 0x25;
            voltageRegister[6] = 0x26; voltageRegister[7] = 0x50;
            voltageRegister[8] = 0x51;

            voltageBank[7] = 5; voltageBank[8] = 5;

            voltageGain = 0.008f;
            iMaxFans = 5;
        }
        break;

        case W83627HF:
        case W83627THF:
        case W83687THF:
        {
            iMaxVoltages = 7;

            voltageRegister[0] = 0x20; voltageRegister[1] = 0x21;
            voltageRegister[2] = 0x22; voltageRegister[3] = 0x23;
            voltageRegister[4] = 0x24; voltageRegister[5] = 0x50;
            voltageRegister[6] = 0x51;

            voltageBank[5] = 5; voltageBank[6] = 5;

            voltageGain = 0.016f;
            iMaxFans = 3;
        }
        break;
    }

    for (i = 0; i < iMaxVoltages; i++)
    {
        if (voltageRegister[i] != VOLTAGE_VBAT_REG)
        {
            /* two special VCore measurement modes for W83627THF */
            if ((wChipType == W83627HF || wChipType == W83627THF ||
                wChipType == W83687THF) && i == 0)
            {
                BYTE vrmConfiguration = W836XX_ReadByte(address, 0, 0x18);

                value = W836XX_ReadByte(address, voltageBank[i], voltageRegister[i]);

                if ((vrmConfiguration & 0x01) == 0)
                    fvalue = 0.016f * value; /* VRM8 formula */
                else
                    fvalue = 0.00488f * value + 0.69f; /* VRM9 formula */
            }
            else
            {
                value = W836XX_ReadByte(address, voltageBank[i], voltageRegister[i]);
                fvalue = voltageGain * value;
            }

            if (fvalue > 0)
            {
                DebugTrace(L"Voltages[%d] = %f", i, fvalue);
            }
        }
        else
        {
            BOOL valid;

            /* Battery voltage */
            valid = (W836XX_ReadByte(address, 0, 0x5D) & 0x01) > 0;

            if (valid)
            {
                fvalue = voltageGain * W836XX_ReadByte(address, 5, VOLTAGE_VBAT_REG);
                DebugTrace(L"Voltages[%d] = %f", i, fvalue);
            }
        }
    }

    for (i = 0; i < 3; i++)
    {
        FLOAT temp;

        value = (W836XX_ReadByte(address, TEMPERATURE_BANK[i], TEMPERATURE_REG[i])) << 1;

        if (TEMPERATURE_BANK[i] > 0)
            value |= W836XX_ReadByte(address, TEMPERATURE_BANK[i], (BYTE)(TEMPERATURE_REG[i] + 1)) >> 7;

        temp = value / 2.0f;

        if (temp <= 125 && temp >= -55 && !peciTemperature[i])
        {
            DebugTrace(L"Temperatures[%d] = %f", i, temp);
        }
    }

    bits = 0;
    for (i = 0; i < 4; i++)
        bits = (bits << 8) | W836XX_ReadByte(address, 0, FAN_BIT_REG[i]);

    newBits = bits;

    for (i = 0; i < iMaxFans; i++)
    {
        INT divisorBits, divisor;
        INT count = W836XX_ReadByte(address, FAN_TACHO_BANK[i], FAN_TACHO_REG[i]);

        /* assemble fan divisor */
        divisorBits = (INT)(
            (((bits >> FAN_DIV_BIT2[i]) & 1) << 2) |
            (((bits >> FAN_DIV_BIT1[i]) & 1) << 1) |
            ((bits >> FAN_DIV_BIT0[i]) & 1));

        divisor = 1 << divisorBits;

        fvalue = (count < 0xff) ? 1.35e6f / (count * divisor) : 0;

        DebugTrace(L"Fans[%d] = %f", i, fvalue);

        /* update fan divisor */
        if (count > 192 && divisorBits < 7)
            divisorBits++;
        if (count < 96 && divisorBits > 0)
            divisorBits--;

        newBits = W836XX_SetBit(newBits, FAN_DIV_BIT2[i], (divisorBits >> 2) & 1);
        newBits = W836XX_SetBit(newBits, FAN_DIV_BIT1[i], (divisorBits >> 1) & 1);
        newBits = W836XX_SetBit(newBits, FAN_DIV_BIT0[i], divisorBits & 1);
    }

    /* write new fan divisors */
    for (i = 4; i >= 0; i--)
    {
        BYTE oldByte = (BYTE)(bits & 0xFF);
        BYTE newByte = (BYTE)(newBits & 0xFF);

        bits = bits >> 8;
        newBits = newBits >> 8;

        if (oldByte != newByte)
            W836XX_WriteByte(address, 0, FAN_BIT_REG[i], newByte);
    }
}
