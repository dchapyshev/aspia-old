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

/* Mainboard Model Ids */
#define UNKNOWN_MODEL         0x0000
/* ASRock */
#define _880GMH_USB3          0x0001
#define AOD790GX_128M         0x0002
#define P55_Deluxe            0x0003
/* ASUS */
#define Crosshair_III_Formula 0x0004
#define M2N_SLI_DELUXE        0x0005
#define M4A79XTD_EVO          0x0006
#define P5W_DH_Deluxe         0x0007
#define P6X58D_E              0x0008
#define P8P67                 0x0009
#define P8P67_EVO             0x0010
#define P8P67_PRO             0x0011
#define P8P67_M_PRO           0x0012
#define Rampage_Extreme       0x0013
#define Rampage_II_GENE       0x0014
#define AT3GC_I               0x0015
/* DFI */
#define LP_BI_P45_T2RS_Elite  0x0016
#define LP_DK_P55_T3eH9       0x0017
/* ECS */
#define A890GXM_A             0x0018
/* EVGA */
#define X58_SLI_Classified    0x0019
/* Gigabyte */
#define _965P_S3              0x0020
#define EP45_DS3R             0x0021
#define EP45_UD3R             0x0022
#define EX58_EXTREME          0x0023
#define GA_MA770T_UD3         0x0024
#define GA_MA785GMT_UD2H      0x0025
#define H67A_UD3H_B3          0x0026
#define P35_DS3               0x0027
#define P35_DS3L              0x0028
#define P55_UD4               0x0029
#define P55M_UD4              0x0030
#define P67A_UD4_B3           0x0031
#define X38_DS5               0x0032
#define X58A_UD3R             0x0033
#define Z68X_UD7_B3           0x0034
/* Shuttle */
#define FH67                  0x0035

typedef struct
{
    WCHAR szDesc[MAX_STR_LEN];
    FLOAT ri;
    FLOAT rf;
    //FLOAT vf;
}
LPC_VOLTAGE_DESC;


#define LPC_INDEX_MAX 10

extern LPC_VOLTAGE_DESC LpcVoltageDesc[LPC_INDEX_MAX];
extern WCHAR szLpcTempDesc[LPC_INDEX_MAX][MAX_STR_LEN];
extern WCHAR szLpcFanDesc[LPC_INDEX_MAX][MAX_STR_LEN];
