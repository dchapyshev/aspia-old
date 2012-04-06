/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/F718XX.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "../aspia_dll.h"
#include "lpc.h"


/* Hardware Monitor */
#define ADDRESS_REGISTER_OFFSET 0x05
#define DATA_REGISTER_OFFSET    0x06

/* Hardware Monitor Registers */
#define FINTECK_VOLTAGE_BASE_REG 0x20
#define F71872F_VOLTAGE_BASE_REG 0x10

#define TEMPERATURE_CONFIG_REG       0x69
#define FINTECK_TEMPERATURE_BASE_REG 0x70
#define F71872F_TEMPERATURE_BASE_REG 0x1B

static const BYTE FINTECK_FAN_TACHOMETER_REG[] = { 0xA0, 0xB0, 0xC0, 0xD0 };
static const BYTE F71872F_FAN_TACHOMETER_REG[] = { 0x20, 0x22, 0x24 };


BYTE
F718XX_ReadByte(WORD wAddress, BYTE bRegister)
{
    WriteIoPortByte((WORD)(wAddress + ADDRESS_REGISTER_OFFSET), bRegister);
    return ReadIoPortByte((WORD)(wAddress + DATA_REGISTER_OFFSET));
}

VOID
F718XX_GetInfo(WORD wChipType, WORD wAddress)
{
    WCHAR szText[MAX_STR_LEN];
    INT iMaxVoltages;
    INT iMaxTemp = 3;
    INT iMaxFans;
    FLOAT voltageGain = 0.008f;
    BYTE voltage_base_reg;
    INT iValue, i;
    short sValue;
    BYTE bValue;

    switch (wChipType)
    {
        case F71858:
            voltage_base_reg = FINTECK_VOLTAGE_BASE_REG;
            iMaxVoltages = 3;
            iMaxFans = 4;
            break;

        case F71805F:
        case F71872F:
            voltage_base_reg = F71872F_VOLTAGE_BASE_REG;
            iMaxVoltages = 11;
            iMaxFans = 3;
            break;

        case F71882:
            voltage_base_reg = FINTECK_VOLTAGE_BASE_REG;
            iMaxVoltages = 9;
            iMaxFans = 4;
            break;

        default:
            voltage_base_reg = FINTECK_VOLTAGE_BASE_REG;
            iMaxVoltages = 9;
            iMaxFans = 3;
            break;
    }

    LPC_ChipTypeToText(wChipType, szText, sizeof(szText));
    IoAddItem(0, 2, szText);

    for (i = 0; i < iMaxVoltages; i++)
    {
        BYTE v = F718XX_ReadByte(wAddress, (BYTE)(voltage_base_reg + i));
        FLOAT fValue;

        fValue = voltageGain * v;
        DebugTrace(L"Voltages[%d] = %f", i, fValue);

        if (v == 0xFF) continue;

        if (SafeStrLen(LpcVoltageDesc[i].szDesc) > 0)
        {
            IoAddItem(1, 3, LpcVoltageDesc[i].szDesc);

            /* Voltage = value + (value - Vf) * Ri / Rf */
            IoSetItemText(L"%.3f V",
                          fValue + (fValue - 0) * LpcVoltageDesc[i].ri / LpcVoltageDesc[i].rf);
        }
        else
        {
            StringCbPrintf(szText, sizeof(szText), L"Voltage #%d", i + 1);
            IoAddItem(1, 3, szText);

            IoSetItemText(L"%.3f V", fValue);
        }
    }

    for (i = 0; i < iMaxTemp; i++)
    {
        switch (wChipType)
        {
            case F71858:
            {
                INT iTableMode = 0x3 & F718XX_ReadByte(wAddress, TEMPERATURE_CONFIG_REG);
                INT iHigh = F718XX_ReadByte(wAddress, (BYTE)(FINTECK_TEMPERATURE_BASE_REG + 2 * i));
                INT iLow = F718XX_ReadByte(wAddress, (BYTE)(FINTECK_TEMPERATURE_BASE_REG + 2 * i + 1));

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

                    if (SafeStrLen(szLpcTempDesc[i]) > 0)
                    {
                        IoAddItem(1, 4, szLpcTempDesc[i]);

                        IoSetItemText(L"%.1f °C", (FLOAT)sValue / 128.0f);
                    }
                }
            }
            break;

            case F71805F:
            case F71872F:
            {
                bValue = F718XX_ReadByte(wAddress, (BYTE)(F71872F_TEMPERATURE_BASE_REG + i));

                if (bValue > 0 && bValue < 127)
                {
                    DebugTrace(L"Temperatures[%d] = %d", i, bValue);

                    if (SafeStrLen(szLpcTempDesc[i]) > 0)
                    {
                        IoAddItem(1, 4, szLpcTempDesc[i]);

                        IoSetItemText(L"%d °C", bValue);
                    }
                }
            }
            break;

            default:
            {
                bValue = F718XX_ReadByte(wAddress, (BYTE)(FINTECK_TEMPERATURE_BASE_REG + 2 * (i + 1)));

                if (bValue > 0 && bValue < 127)
                {
                    DebugTrace(L"Temperatures[%d] = %d", i, bValue);

                    if (SafeStrLen(szLpcTempDesc[i]) > 0)
                    {
                        IoAddItem(1, 4, szLpcTempDesc[i]);

                        IoSetItemText(L"%d °C", bValue);
                    }
                }
            }
            break;
        }
    }

    for (i = 0; i < iMaxFans; i++)
    {
        if (wChipType == F71872F || wChipType == F71805F)
        {
            iValue = F718XX_ReadByte(wAddress, F71872F_FAN_TACHOMETER_REG[i]) << 8;
            iValue |= F718XX_ReadByte(wAddress, (BYTE)(F71872F_FAN_TACHOMETER_REG[i] + 1));
        }
        else
        {
            iValue = F718XX_ReadByte(wAddress, FINTECK_FAN_TACHOMETER_REG[i]) << 8;
            iValue |= F718XX_ReadByte(wAddress, (BYTE)(FINTECK_FAN_TACHOMETER_REG[i] + 1));
        }

        if (iValue > 0)
        {
            FLOAT fValue = (iValue < 0x0fff) ? 1.5e6f / (FLOAT)iValue : 0.0;
            DebugTrace(L"Fans[%d] = %f", i, fValue);

            if (fValue > 0.0)
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

                IoSetItemText(L"%.0f RPM", fValue);
            }
        }
    }
}
