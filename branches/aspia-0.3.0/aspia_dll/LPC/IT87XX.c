/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/IT87XX.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "../aspia_dll.h"
#include "lpc.h"


/* Consts */
#define ITE_VENDOR_ID                    0x90

/* Environment Controller */
#define ADDRESS_REGISTER_OFFSET          0x05
#define DATA_REGISTER_OFFSET             0x06

/* Environment Controller Registers */
#define CONFIGURATION_REGISTER           0x00
#define TEMPERATURE_BASE_REG             0x29
#define VENDOR_ID_REGISTER               0x58
#define FAN_TACHOMETER_DIVISOR_REGISTER  0x0B
#define VOLTAGE_BASE_REG                 0x20


static const BYTE FAN_TACHOMETER_REG[]     = { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
static const BYTE FAN_TACHOMETER_EXT_REG[] = { 0x18, 0x19, 0x1a, 0x81, 0x83 };


BYTE
IT87XX_ReadByte(WORD wAddressReg, WORD wDataReg, BYTE bRegister, BOOL *Valid)
{
    BYTE bValue;

    WriteIoPortByte(wAddressReg, bRegister);
    bValue = ReadIoPortByte(wDataReg);

    *Valid = (bRegister == ReadIoPortByte(wAddressReg));

    return bValue;
}

BOOL
IT87XX_WriteByte(WORD wAddressReg, WORD wDataReg, BYTE bRegister, BYTE bValue)
{
    WriteIoPortByte(wAddressReg, bRegister);
    WriteIoPortByte(wDataReg, bValue);

    return (bRegister == ReadIoPortByte(wAddressReg));
}

BYTE
IT87XX_ReadGPIO(INT iIndex, INT iGPIOCount, WORD wGPIOAddress)
{
    if (iIndex >= iGPIOCount)
        return 0;

    return ReadIoPortByte((WORD)(wGPIOAddress + iIndex));
}

VOID
IT87XX_WriteGPIO(INT iIndex, INT iGPIOCount, WORD wGPIOAddress, BYTE bValue)
{
    if (iIndex >= iGPIOCount)
        return;

    WriteIoPortByte((WORD)(wGPIOAddress + iIndex), bValue);
}

VOID
IT87XX_GetInfo(WORD wChipType,
               WORD wAddress,
               BYTE bVersion)
{
    WORD wAddressReg, wDataReg;
    BYTE bVendorId;
    WCHAR szText[MAX_STR_LEN];
    BOOL Valid, has16bitFanCounter;
    FLOAT fVoltageGain;
    INT i, iValue;

    wAddressReg = (WORD)(wAddress + ADDRESS_REGISTER_OFFSET);
    wDataReg = (WORD)(wAddress + DATA_REGISTER_OFFSET);

    /* Check vendor id */
    bVendorId = IT87XX_ReadByte(wAddressReg, wDataReg, VENDOR_ID_REGISTER, &Valid);

    if (!Valid || bVendorId != ITE_VENDOR_ID)
    {
        DebugTrace(L"wAddressReg = 0x%x, bVendorId = 0x%x, Valid = %d", wAddressReg, bVendorId, Valid);
        return;
    }

    /* Bit 0x10 of the configuration register should always be 1 */
    if ((IT87XX_ReadByte(wAddressReg, wDataReg, CONFIGURATION_REGISTER, &Valid) & 0x10) == 0)
        return;

    if (!Valid) return;

    LPC_ChipTypeToText(wChipType, szText, sizeof(szText));
    IoAddItem(0, 2, szText);

    /* IT8721F, IT8728F and IT8772E use a 12mV resultion ADC, all others 16mV */
    if (wChipType == IT8721F || wChipType == IT8728F ||
        wChipType == IT8771E || wChipType == IT8772E)
    {
        fVoltageGain = 0.012f;
    }
    else
    {
        fVoltageGain = 0.016f;
    }

    /* older IT8721F revision do not have 16-bit fan counters */
    if (wChipType == IT8712F && bVersion < 8)
    {
        has16bitFanCounter = FALSE;
    }
    else
    {
        has16bitFanCounter = TRUE;
    }

    for (i = 0; i < 9; i++)
    {
        BYTE v = IT87XX_ReadByte(wAddressReg,
                                 wDataReg,
                                 (BYTE)(VOLTAGE_BASE_REG + i),
                                 &Valid);
        FLOAT fValue;

        if (!Valid || v == 0xFF) continue;

        fValue = fVoltageGain * v;

        if (fValue > 0)
        {
            DebugTrace(L"Voltage[%d] = %f", i, fValue);

            if (SafeStrLen(LpcVoltageDesc[i].szDesc) > 0)
            {
                IoAddItem(1, 3, LpcVoltageDesc[i].szDesc);

                /* Voltage = value + (value - Vf) * Ri / Rf */
                IoSetItemText(L"%.3f V",
                    fValue + (fValue - LpcVoltageDesc[i].vf) * LpcVoltageDesc[i].ri / LpcVoltageDesc[i].rf);
            }
            else
            {
                StringCbPrintf(szText, sizeof(szText), L"Voltage #%d", i + 1);
                IoAddItem(1, 3, szText);

                IoSetItemText(L"%.3f V", fValue);
            }
        }
    }

    for (i = 0; i < 3; i++)
    {
        BYTE bValue = IT87XX_ReadByte(wAddressReg, wDataReg, (BYTE)(TEMPERATURE_BASE_REG + i), &Valid);

        if (!Valid) continue;

        if (bValue > 0 && bValue < 127)
        {
            DebugTrace(L"Temperature[%d] = %d", i, bValue);

            if (SafeStrLen(szLpcTempDesc[i]) > 0)
            {
                IoAddItem(1, 4, szLpcTempDesc[i]);

                IoSetItemText(L"%d °C", bValue);
            }
        }
    }

    if (has16bitFanCounter)
    {
        for (i = 0; i < 5; i++)
        {
            iValue = IT87XX_ReadByte(wAddressReg, wDataReg, FAN_TACHOMETER_REG[i], &Valid);

            if (!Valid) continue;

            iValue |= IT87XX_ReadByte(wAddressReg, wDataReg, FAN_TACHOMETER_EXT_REG[i], &Valid) << 8;

            if (!Valid) continue;

            if (iValue > 0x3f)
            {
                DOUBLE tmp = (iValue < 0xffff) ? 1.35e6f / (iValue * 2) : 0;

                DebugTrace(L"Fans[%d] = %f", i, tmp);

                if (tmp > 0)
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

                    IoSetItemText(L"%.0f RPM", tmp);
                }
            }
        }
    }
    else
    {
        for (i = 0; i < 5; i++)
        {
            INT iDivisor = 2;

            iValue = IT87XX_ReadByte(wAddressReg, wDataReg, FAN_TACHOMETER_REG[i], &Valid);

            if (!Valid) continue;

            if (i < 2)
            {
                INT iDivisors = IT87XX_ReadByte(wAddressReg, wDataReg, FAN_TACHOMETER_DIVISOR_REGISTER, &Valid);

                if (!Valid) continue;

                iDivisor = 1 << ((iDivisors >> (3 * i)) & 0x7);
            }

            if (iValue > 0)
            {
                DOUBLE tmp = (iValue < 0xff) ? 1.35e6f / (iValue * iDivisor) : 0;

                DebugTrace(L"Fans[%d] = %f", i, tmp);

                if (tmp > 0)
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

                    IoSetItemText(L"%.0f RPM", tmp);
                }
            }
        }
    }
}

