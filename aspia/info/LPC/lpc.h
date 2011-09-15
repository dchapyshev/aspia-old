/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/lpc.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <windows.h>
#include <wchar.h>


#define ATK0110    0x0110

#define F71858     0x0507
#define F71862     0x0601
#define F71869     0x0814
#define F71882     0x0541
#define F71889AD   0x1005
#define F71889ED   0x0909
#define F71889F    0x0723

#define IT8712F    0x8712
#define IT8716F    0x8716
#define IT8718F    0x8718
#define IT8720F    0x8720
#define IT8721F    0x8721
#define IT8726F    0x8726
#define IT8728F    0x8728
#define IT8772E    0x8772

#define NCT6771F   0xB470
#define NCT6776F   0xC330

#define W83627DHG  0xA020
#define W83627DHGP 0xB070
#define W83627EHF  0x8800
#define W83627HF   0x5200
#define W83627THF  0x8280
#define W83667HG   0xA510
#define W83667HGB  0xB350
#define W83687THF  0x8541


typedef struct
{
    DWORD dwChip;
    LPWSTR lpDesc;
} CHIP_INFO, *PCHIP_INFO;

/* lpc.c */
extern CHIP_INFO ChipInfo[];
VOID LPC_ChipTypeToText(DWORD dwChip, LPWSTR lpText, SIZE_T Size);


/* IT87XX.c */
VOID IT87XX_GetInfo(WORD wChipType, WORD wAddress, WORD wGPIOAddress, BYTE bVersion);