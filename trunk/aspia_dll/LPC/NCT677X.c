/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/NCT677X.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "lpc.h"


#define ADDRESS_REGISTER_OFFSET  0x05
#define DATA_REGISTER_OFFSET     0x06
#define BANK_SELECT_REGISTER     0x4E

#define NUVOTON_VENDOR_ID        0x5CA3

#define VENDOR_ID_HIGH_REGISTER  0x804F
#define VENDOR_ID_LOW_REGISTER   0x004F
#define VOLTAGE_VBAT_REG         0x0551

static WORD TEMPERATURE_REG[] =
    { 0x027, 0x73,  0x75,  0x77,  0x150, 0x250, 0x62B, 0x62C, 0x62D };
const WORD TEMPERATURE_HALF_REG[] =
    { 0,     0x74,  0x76,  0x78,  0x151, 0x251, 0x62E, 0x62E, 0x62E };
const WORD TEMPERATURE_SRC_REG[] =
    { 0x621, 0x100, 0x200, 0x300, 0x622, 0x623, 0x624, 0x625, 0x626 };
const INT TEMPERATURE_HALF_BIT[] =
    { -1,    7,     7,     7,     7,     7,     0,     1,     2     };
const WORD VOLTAGE_REG[] =
    { 0x20,  0x21,  0x22,  0x23,  0x24,  0x25,  0x26,  0x550, 0x551 };
const WORD FAN_RPM_REG[] =
    { 0x656, 0x658, 0x65A, 0x65C, 0x65E};
const WORD FAN_PWM_OUT_REG[] =
    { 0x001, 0x003, 0x011 };
const WORD FAN_PWM_COMMAND_REG[] =
    { 0x109, 0x209, 0x309 };
const WORD FAN_CONTROL_MODE_REG[] =
    { 0x102, 0x202, 0x302 };


#define NCT6771F_SYSTIN                1
#define NCT6771F_CPUTIN                2
#define NCT6771F_AUXTIN                3
#define NCT6771F_SMBUSMASTER           4
#define NCT6771F_PECI_0                5
#define NCT6771F_PECI_1                6
#define NCT6771F_PECI_2                7
#define NCT6771F_PECI_3                8
#define NCT6771F_PECI_4                9
#define NCT6771F_PECI_5                10
#define NCT6771F_PECI_6                11
#define NCT6771F_PECI_7                12
#define NCT6771F_PCH_CHIP_CPU_MAX_TEMP 13
#define NCT6771F_PCH_CHIP_TEMP         14
#define NCT6771F_PCH_CPU_TEMP          15
#define NCT6771F_PCH_MCH_TEMP          16
#define NCT6771F_PCH_DIM0_TEMP         17
#define NCT6771F_PCH_DIM1_TEMP         18
#define NCT6771F_PCH_DIM2_TEMP         19
#define NCT6771F_PCH_DIM3_TEMP         20

#define NCT6776F_SYSTIN                1
#define NCT6776F_CPUTIN                2
#define NCT6776F_AUXTIN                3
#define NCT6776F_SMBUSMASTER_0         4
#define NCT6776F_SMBUSMASTER_1         5
#define NCT6776F_SMBUSMASTER_2         6
#define NCT6776F_SMBUSMASTER_3         7
#define NCT6776F_SMBUSMASTER_4         8
#define NCT6776F_SMBUSMASTER_5         9
#define NCT6776F_SMBUSMASTER_6         10
#define NCT6776F_SMBUSMASTER_7         11
#define NCT6776F_PECI_0                12
#define NCT6776F_PECI_1                13
#define NCT6776F_PCH_CHIP_CPU_MAX_TEMP 14
#define NCT6776F_PCH_CHIP_TEMP         15
#define NCT6776F_PCH_CPU_TEMP          16
#define NCT6776F_PCH_MCH_TEMP          17
#define NCT6776F_PCH_DIM0_TEMP         18
#define NCT6776F_PCH_DIM1_TEMP         19
#define NCT6776F_PCH_DIM2_TEMP         20
#define NCT6776F_PCH_DIM3_TEMP         21
#define NCT6776F_BYTE_TEMP             22


BYTE
NCT677X_ReadByte(WORD port, WORD address)
{
    BYTE bank = (BYTE)(address >> 8);
    BYTE reg = (BYTE)(address & 0xFF);

    WriteIoPortByte((WORD)(port + ADDRESS_REGISTER_OFFSET), BANK_SELECT_REGISTER);
    WriteIoPortByte((WORD)(port + DATA_REGISTER_OFFSET), bank);
    WriteIoPortByte((WORD)(port + ADDRESS_REGISTER_OFFSET), reg);

    return ReadIoPortByte((WORD)(port + DATA_REGISTER_OFFSET));
}

VOID
NCT677X_WriteByte(WORD port, WORD address, BYTE value)
{
    BYTE bank = (BYTE)(address >> 8);
    BYTE reg = (BYTE)(address & 0xFF);

    WriteIoPortByte((WORD)(port + ADDRESS_REGISTER_OFFSET), BANK_SELECT_REGISTER);
    WriteIoPortByte((WORD)(port + DATA_REGISTER_OFFSET), bank);
    WriteIoPortByte((WORD)(port + ADDRESS_REGISTER_OFFSET), reg);
    WriteIoPortByte((WORD)(port + DATA_REGISTER_OFFSET), value);
}

BOOL
IsNuvotonVendor(WORD port)
{
    return ((NCT677X_ReadByte(port, VENDOR_ID_HIGH_REGISTER) << 8) |
            NCT677X_ReadByte(port, VENDOR_ID_LOW_REGISTER)) == NUVOTON_VENDOR_ID;
}

VOID
NCT677X_GetInfo(WORD wChipType, BYTE revision, WORD port)
{
    INT minFanRPM;
    INT iMaxFans;
    FLOAT fValue;
    BOOL Valid;
    INT i, iValue;

    if (!IsNuvotonVendor(port))
        return;

    switch (wChipType)
    {
        case NCT6771F:
            iMaxFans = 4;
            /* min value RPM value with 16-bit fan counter */
            minFanRPM = (INT)(1.35e6 / 0xFFFF);
            break;

        case NCT6776F:
            iMaxFans = 5;
            /* min value RPM value with 13-bit fan counter */
            minFanRPM = (INT)(1.35e6 / 0x1FFF);
            break;
    }

    for (i = 0; i < 9; i++)
    {
        fValue = 0.008f * NCT677X_ReadByte(port, VOLTAGE_REG[i]);
        Valid = (fValue > 0.0) ? TRUE : FALSE;

        /* check if battery voltage monitor is enabled */
        if (Valid && VOLTAGE_REG[i] == VOLTAGE_VBAT_REG)
            Valid = (NCT677X_ReadByte(port, 0x005D) & 0x01) > 0;

        DebugTrace(L"Voltages[%d] = %f", i, Valid ? iValue : 0.0);
    }

    for (i = 8; i >= 0; i--)
    {
        BYTE bSource;

        iValue = ((BYTE)NCT677X_ReadByte(port, TEMPERATURE_REG[i])) << 1;

        if (TEMPERATURE_HALF_BIT[i] > 0)
        {
            iValue |= ((NCT677X_ReadByte(port, TEMPERATURE_HALF_REG[i]) >> TEMPERATURE_HALF_BIT[i]) & 0x1);
        }

        bSource = NCT677X_ReadByte(port, TEMPERATURE_SRC_REG[i]);

        fValue = 0.5f * iValue;

        if (fValue < 125 && fValue > -55)
        {
            switch (wChipType)
            {
                case NCT6771F:
                {
                    switch (bSource)
                    {
                        case NCT6771F_PECI_0:
                            DebugTrace(L"Temperatures[0] = %f", fValue);
                            break;
                        case NCT6771F_CPUTIN:
                            DebugTrace(L"Temperatures[1] = %f", fValue);
                            break;
                        case NCT6771F_AUXTIN:
                            DebugTrace(L"Temperatures[2] = %f", fValue);
                            break;
                        case NCT6771F_SYSTIN:
                            DebugTrace(L"Temperatures[3] = %f", fValue);
                            break;
                    }
                }
                break;

                case NCT6776F:
                {
                    switch (bSource)
                    {
                        case NCT6776F_PECI_0:
                            DebugTrace(L"Temperatures[0] = %f", fValue);
                            break;
                        case NCT6776F_CPUTIN:
                            DebugTrace(L"Temperatures[1] = %f", fValue);
                            break;
                        case NCT6776F_AUXTIN:
                            DebugTrace(L"Temperatures[2] = %f", fValue);
                            break;
                        case NCT6776F_SYSTIN:
                            DebugTrace(L"Temperatures[3] = %f", fValue);
                            break;
                    }
                }
                break;
            }
        }
    }

    for (i = 0; i < iMaxFans; i++)
    {
        BYTE high = NCT677X_ReadByte(port, FAN_RPM_REG[i]);
        BYTE low = NCT677X_ReadByte(port, (WORD)(FAN_RPM_REG[i] + 1));

        iValue = (high << 8) | low;

        DebugTrace(L"Fans[%d] = %f", i, iValue > minFanRPM ? iValue : 0);
    }
}
