/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/F718XX.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include "lpc.h"


/* Hardware Monitor */
#define ADDRESS_REGISTER_OFFSET 0x05
#define DATA_REGISTER_OFFSET    0x06

/* Hardware Monitor Registers */
#define VOLTAGE_BASE_REG        0x20
#define TEMPERATURE_CONFIG_REG  0x69
#define TEMPERATURE_BASE_REG    0x70

static BYTE FAN_TACHOMETER_REG[] = { 0xA0, 0xB0, 0xC0, 0xD0 };


BYTE
F718XX_ReadByte(WORD wAddress, BYTE bRegister)
{
    DRIVER_WriteIoPortByte((WORD)(wAddress + ADDRESS_REGISTER_OFFSET), bRegister);
    return DRIVER_ReadIoPortByte((WORD)(wAddress + DATA_REGISTER_OFFSET));
}

VOID
F718XX_GetInfo(WORD wChipType, WORD wAddress)
{
    INT iMaxVoltages = ((wChipType == F71858) ? 3 : 9);
    INT iMaxTemp = 3;
    INT iMaxFans = ((wChipType == F71882 || wChipType == F71858) ? 4 : 3);
    INT iValue, i;
    short sValue;
    BYTE bValue;

    for (i = 0; i < iMaxVoltages; i++)
    {
        iValue = F718XX_ReadByte(wAddress, (BYTE)(VOLTAGE_BASE_REG + i));

        DebugTrace(L"Voltages[%d] = %f", i, 0.008f * iValue);
    }

    for (i = 0; i < iMaxTemp; i++)
    {
        switch (wChipType)
        {
            case F71858:
            {
                INT iTableMode = 0x3 & F718XX_ReadByte(wAddress, TEMPERATURE_CONFIG_REG);
                INT iHigh = F718XX_ReadByte(wAddress, (BYTE)(TEMPERATURE_BASE_REG + 2 * i));
                INT iLow = F718XX_ReadByte(wAddress, (BYTE)(TEMPERATURE_BASE_REG + 2 * i + 1));

                if (iHigh != 0xbb && iHigh != 0xcc)
                {
                    INT iBits = 0;

                    switch (iTableMode)
                    {
                        case 0: iBits = 0; break;
                        case 1: iBits = 0; break;
                        case 2: iBits = (iHigh & 0x80) << 8; break;
                        case 3: iBits = (iLow & 0x01) << 15; break;
                    }

                    iBits |= iHigh << 7;
                    iBits |= (iLow & 0xE0) >> 1;

                    sValue = (short)(iBits & 0xFFF0);

                    DebugTrace(L"Temperatures[%d] = %f", i, sValue / 128.0f);
                }
            }
            break;

            default:
            {
                bValue = F718XX_ReadByte(wAddress, (BYTE)(TEMPERATURE_BASE_REG + 2 * (i + 1)));

                if (bValue > 0 && bValue < 127)
                {
                    DebugTrace(L"Temperatures[%d] = %d", i, bValue);
                }
            }
            break;
        }
    }

    for (i = 0; i < iMaxFans; i++)
    {
        iValue = F718XX_ReadByte(wAddress, FAN_TACHOMETER_REG[i]) << 8;
        iValue |= F718XX_ReadByte(wAddress, (BYTE)(FAN_TACHOMETER_REG[i] + 1));

        if (iValue > 0)
        {
            DebugTrace(L"Fans[%d] = %f", i, (iValue < 0x0fff) ? 1.5e6f / iValue : 0);
        }
    }
}
