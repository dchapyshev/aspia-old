/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/W836XX.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "../aspia_dll.h"
#include "lpc.h"


#define WINBOND_VENDOR_ID             0x5CA3
#define HIGH_BYTE                     0x80

#define ADDRESS_REGISTER_OFFSET       0x05
#define DATA_REGISTER_OFFSET          0x06

#define VOLTAGE_VBAT_REG              0x51
#define BANK_SELECT_REGISTER          0x4E
#define VENDOR_ID_REGISTER            0x4F
#define TEMPERATURE_SOURCE_SELECT_REG 0x49

static const BYTE TEMPERATURE_REG[] = { 0x50, 0x50, 0x27 };
const BYTE TEMPERATURE_BANK[]       = { 1,    2,    0 };

const BYTE FAN_TACHO_REG[]  = { 0x28, 0x29, 0x2A, 0x3F, 0x53 };
const BYTE FAN_TACHO_BANK[] = { 0,    0,    0,    0,    5    };
const BYTE FAN_BIT_REG[]    = { 0x47, 0x4B, 0x4C, 0x59, 0x5D };
const BYTE FAN_DIV_BIT0[]   = { 36,   38,   30,   8,    10   };
const BYTE FAN_DIV_BIT1[]   = { 37,   39,   31,   9,    11   };
const BYTE FAN_DIV_BIT2[]   = { 5,    6,    7,    23,   15   };


BYTE
W836XX_ReadByte(WORD address, BYTE bank, BYTE reg)
{
    WriteIoPortByte((WORD)(address + ADDRESS_REGISTER_OFFSET), BANK_SELECT_REGISTER);
    WriteIoPortByte((WORD)(address + DATA_REGISTER_OFFSET), bank);
    WriteIoPortByte((WORD)(address + ADDRESS_REGISTER_OFFSET), reg);

    return ReadIoPortByte((WORD)(address + DATA_REGISTER_OFFSET));
}

VOID
W836XX_WriteByte(WORD address, BYTE bank, BYTE reg, BYTE value)
{
    WriteIoPortByte((WORD)(address + ADDRESS_REGISTER_OFFSET), BANK_SELECT_REGISTER);
    WriteIoPortByte((WORD)(address + DATA_REGISTER_OFFSET), bank);
    WriteIoPortByte((WORD)(address + ADDRESS_REGISTER_OFFSET), reg);
    WriteIoPortByte((WORD)(address + DATA_REGISTER_OFFSET), value);
}

DWORD
W836XX_SetBit(DWORD target, WORD bit, WORD value)
{
    if (((value & 1) == value) && bit <= 63)
    {
        DWORD mask = (((DWORD)1) << bit);
        return value > 0 ? target | mask : target & ~mask;
    }

    return value;
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
    WCHAR szText[MAX_STR_LEN];
    BYTE voltageRegister[10] = {0};
    BYTE voltageBank[10] = {0};
    INT iMaxVoltages = 0, iMaxFans = 0, i, value;
    FLOAT fvalue, voltageGain = 0.008f;
    DWORD offset, divisor, count;
    DWORD newBits, bits = 0;

    if (!IsWinbondVendor(address))
        return;

    LPC_ChipTypeToText(wChipType, szText, sizeof(szText));
    IoAddItem(0, 2, szText);

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
            voltageRegister[4] = 0x24; voltageRegister[5] = 0x25;
            voltageRegister[6] = 0x26;

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

                if (SafeStrLen(LpcVoltageDesc[i].szDesc) > 0)
                {
                    IoAddItem(1, 3, LpcVoltageDesc[i].szDesc);

                    IoSetItemText(L"%.3f V",
                        fvalue + (fvalue - LpcVoltageDesc[i].vf) * LpcVoltageDesc[i].ri / LpcVoltageDesc[i].rf);
                }
            }
        }
        else
        {
            BOOL valid;

            /* Battery voltage */
            valid = (W836XX_ReadByte(address, 0, 0x5D) & 0x01) > 0;

            if (valid)
            {
                BYTE tmp = W836XX_ReadByte(address, 5, VOLTAGE_VBAT_REG);

                fvalue = voltageGain * tmp;
                DebugTrace(L"Voltages[%d] = %f, voltageGain = %f, tmp = %d", i, fvalue, voltageGain, tmp);

                IoAddItem(1, 3, L"VBat");

                IoSetItemText(L"%.3f V", fvalue);
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

        if (temp <= 125 && temp >= -55)
        {
            DebugTrace(L"Temperatures[%d] = %f", i, temp);

            if (SafeStrLen(szLpcTempDesc[i]) > 0)
            {
                IoAddItem(1, 4, szLpcTempDesc[i]);

                IoSetItemText(L"%.2f °C", temp);
            }
        }
    }

    for (i = 0; i < 5; i++)
    {
        bits = (bits << 8) | W836XX_ReadByte(address, 0, FAN_BIT_REG[i]);
    }

    newBits = bits;

    for (i = 0; i < iMaxFans; i++)
    {
        /* assemble fan divisor */
        offset = (((bits >> FAN_DIV_BIT2[i]) & 1) << 2) |
                 (((bits >> FAN_DIV_BIT1[i]) & 1) << 1) |
                  ((bits >> FAN_DIV_BIT0[i]) & 1);

        divisor = 1 << offset;

        count = W836XX_ReadByte(address, FAN_TACHO_BANK[i], FAN_TACHO_REG[i]);

        /* update fan divisor */
        if (count > 192 && offset < 7)
        {
            offset++;
        }
        else if (count < 96 && offset > 0)
        {
            offset--;
        }

        fvalue = (count < 0xff) ? 1.35e6f / ((FLOAT)count * (FLOAT)divisor) : 0;

        DebugTrace(L"count = %d, divisor = %d, Fans[%d] = %f", count, divisor, i, fvalue);

        if (fvalue > 0.0f)
        {
            if (SafeStrLen(szLpcFanDesc[i]) > 0)
            {
                IoAddItem(1, 5, szLpcFanDesc[i]);
            }
            else
            {
                StringCbPrintf(szText, sizeof(szText), L"Fans #%d", i + 1);
                IoAddItem(1, 5, szText);
            }

            IoSetItemText(L"%.0f RPM", fvalue);
        }

        newBits = W836XX_SetBit(newBits, FAN_DIV_BIT2[i], (offset >> 2) & 1);
        newBits = W836XX_SetBit(newBits, FAN_DIV_BIT1[i], (offset >> 1) & 1);
        newBits = W836XX_SetBit(newBits, FAN_DIV_BIT0[i],  offset       & 1);
    }

    /* write new fan divisors */
    for (i = 4; i >= 0; i--)
    {
        BYTE oldByte = bits & 0xFF;
        BYTE newByte = newBits & 0xFF;

        if (oldByte != newByte)
            W836XX_WriteByte(address, 0, FAN_BIT_REG[i], newByte);

        bits = bits >> 8;
        newBits = newBits >> 8;
    }
}
