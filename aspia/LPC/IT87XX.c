/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/IT87XX.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"


/* Consts */
BYTE ITE_VENDOR_ID                   = 0x90;

/* Environment Controller */
BYTE ADDRESS_REGISTER_OFFSET         = 0x05;
BYTE DATA_REGISTER_OFFSET            = 0x06;

/* Environment Controller Registers */
BYTE CONFIGURATION_REGISTER          = 0x00;
BYTE TEMPERATURE_BASE_REG            = 0x29;
BYTE VENDOR_ID_REGISTER              = 0x58;
BYTE FAN_TACHOMETER_DIVISOR_REGISTER = 0x0B;
BYTE FAN_TACHOMETER_REG[]            = { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
BYTE FAN_TACHOMETER_EXT_REG[]        = { 0x18, 0x19, 0x1a, 0x81, 0x83 };
BYTE VOLTAGE_BASE_REG                = 0x20;

