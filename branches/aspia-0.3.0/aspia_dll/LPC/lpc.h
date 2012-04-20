/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/lpc.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include "aspia.h"


#define ATK0110    0x0110

#define F71805F    0x0406
#define F71858     0x0507
#define F71862     0x0601
#define F71869     0x0814
#define F71872F    0x0341
#define F71882     0x0541
#define F71889AD   0x1005
#define F71889ED   0x0909
#define F71889F    0x0723
#define F71808     0x0901

#define IT8512F    0x8512
#define IT8712F    0x8712
#define IT8716F    0x8716
#define IT8718F    0x8718
#define IT8720F    0x8720
#define IT8721F    0x8721
#define IT8726F    0x8726
#define IT8728F    0x8728
#define IT8752F    0x8752
#define IT8771E    0x8771
#define IT8772E    0x8772

#define NCT6771F   0xB470
#define NCT6776F   0xC330

#define W83627DHG  0xA020
#define W83627UHG  0xA230
#define W83627DHGP 0xB070
#define W83627EHF  0x8800
#define W83627HF   0x5200
#define W83627SF   0x5950
#define W83637HF   0x7080
#define W83627THF  0x8280
#define W83667HG   0xA510
#define W83667HGB  0xB350
#define W83687THF  0x8541
#define W83697HF   0x6010
#define W83697SF   0x6810


typedef struct
{
    DWORD dwChip;
    LPWSTR lpDesc;
} CHIP_INFO, *PCHIP_INFO;

/* lpc.c */
extern CHIP_INFO ChipInfo[];
VOID LPC_ChipTypeToText(DWORD dwChip, LPWSTR lpText, SIZE_T Size);
VOID LPC_MainboardInfoInit(WORD wChipType);


/* IT87XX.c */
VOID IT87XX_GetInfo(WORD wChipType, WORD wAddress, BYTE bVersion);

/* W836XX.c */
VOID W836XX_GetInfo(WORD wChipType, BYTE revision, WORD address);

/* F718XX.c */
VOID F718XX_GetInfo(WORD wChipType, WORD wAddress);

/* NCT677X.c */
VOID NCT677X_GetInfo(WORD wChipType, BYTE revision, WORD port);

/* mainboards.c */

/* Mainboard Manufacturer Ids */
#define UNKNOWN_MB     0x0000
#define ALIENWARE_MB   0x0001
#define APPLE_MB       0x0002
#define ASROCK_MB      0x0003
#define ASUS_MB        0x0004
#define DELL_MB        0x0005
#define DFI_MB         0x0006
#define ECS_MB         0x0007
#define EPOX_MB        0x0008
#define EVGA_MB        0x0009
#define FIC_MB         0x0010
#define FUJITSU_MB     0x0011
#define GIGABYTE_MB    0x0012
#define HP_MB          0x0013
#define IBM_MB         0x0014
#define INTEL_MB       0x0015
#define LENOVO_MB      0x0016
#define MSI_MB         0x0017
#define SHUTTLE_MB     0x0018
#define SUPERMICRO_MB  0x0019
#define TOSHIBA_MB     0x0020
#define XFX_MB         0x0021
#define TYAN_MB        0x0022

/* Mainboard Model Ids */
#define UNKNOWN_MODEL         0x0000
/* ASRock (0x0001 - 0x1000 */
#define _880GMH_USB3          0x0001
#define AOD790GX_128M         0x0002
#define P55_Deluxe            0x0003
/* ASUS  (0x1001 - 0x2000) */
#define Crosshair_III_Formula 0x1001
#define M2N_SLI_DELUXE        0x1002
#define M4A79XTD_EVO          0x1003
#define P5W_DH_Deluxe         0x1004
#define P6X58D_E              0x1005
#define P8P67                 0x1006
#define P8P67_EVO             0x1007
#define P8P67_PRO             0x1008
#define P8P67_M_PRO           0x1009
#define Rampage_Extreme       0x100A
#define Rampage_II_GENE       0x100B
#define AT3GC_I               0x100C
#define P5K_VM                0x100D
#define P5QL_PRO              0x100E
#define K8V_MX                0x100F
#define P4P800_VM             0x1010
#define P9X79                 0x1011
#define P6T                   0x1012
#define P8Z68_V_PRO           0x1013
/* DFI (0x2001 - 0x2300) */
#define LP_BI_P45_T2RS_Elite  0x2001
#define LP_DK_P55_T3eH9       0x2002
/* ECS (0x2301 - 0x2600) */
#define A890GXM_A             0x2301
/* EVGA (0x2601 - 0x2900) */
#define X58_SLI_Classified    0x2601
/* Gigabyte (0x2901 - 0x3900) */
#define _965P_S3              0x2901
#define EP45_DS3R             0x2902
#define EP45_UD3R             0x2903
#define EX58_EXTREME          0x2904
#define GA_MA770T_UD3         0x2905
#define GA_MA785GMT_UD2H      0x2906
#define H67A_UD3H_B3          0x2907
#define P35_DS3               0x2908
#define P35_DS3L              0x2909
#define P55_UD4               0x290A
#define P55M_UD4              0x290B
#define P67A_UD4_B3           0x290C
#define X38_DS5               0x290D
#define X58A_UD3R             0x290E
#define Z68X_UD7_B3           0x290F
#define G31M_ES2C             0x2910
#define _965GM_S2             0x2911
#define G31M_ES2L             0x2912
#define _8I865GVMK            0x2913
/* Shuttle (0x3901 - 0x4200) */
#define FH67                  0x3901
/* EPOX (0x4201 - 0x4700) */
#define _9NPA7I_9NPAI_9NPA7J_9NPAJ_3P 0x4201
/* MSI (0x4701 - 0x5700) */
#define G32M3_V2              0x4701
#define H55_G43               0x4702
#define G31TM_P35             0x4703
/* TYAN (0x5701 - 0x6000 */
#define S2882                 0x5701

typedef struct
{
    WCHAR szDesc[MAX_STR_LEN];
    FLOAT ri;
    FLOAT rf;
    FLOAT vf;
}
LPC_VOLTAGE_DESC;


#define LPC_INDEX_MAX 11

extern LPC_VOLTAGE_DESC LpcVoltageDesc[LPC_INDEX_MAX];
extern WCHAR szLpcTempDesc[LPC_INDEX_MAX][MAX_STR_LEN];
extern WCHAR szLpcFanDesc[LPC_INDEX_MAX][MAX_STR_LEN];
