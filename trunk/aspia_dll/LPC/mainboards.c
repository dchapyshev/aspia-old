/*
 * PROJECT:         Aspia
 * FILE:            aspia/LPC/mainboards.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "../aspia_dll.h"
#include "lpc.h"


WORD
LPC_GetMBManufacturerId(LPWSTR lpManufacturer)
{
    if (SafeStrLen(lpManufacturer) < 1)
        return UNKNOWN_MB;

    if (wcscmp(lpManufacturer, L"Alienware") == 0)
    {
        return ALIENWARE_MB;
    }
    else if (wcscmp(lpManufacturer, L"Apple Inc.") == 0)
    {
        return APPLE_MB;
    }
    else if (wcscmp(lpManufacturer, L"ASRock") == 0)
    {
        return ASROCK_MB;
    }
    else if (wcscmp(lpManufacturer, L"ASUSTeK Computer INC.") == 0 ||
             wcscmp(lpManufacturer, L"ASUSTeK Computer Inc.") == 0)
    {
        return ASUS_MB;
    }
    else if (wcscmp(lpManufacturer, L"Dell Inc.") == 0)
    {
        return DELL_MB;
    }
    else if (wcscmp(lpManufacturer, L"DFI") == 0 ||
             wcscmp(lpManufacturer, L"DFI Inc.") == 0)
    {
        return DFI_MB;
    }
    else if (wcscmp(lpManufacturer, L"ECS") == 0)
    {
        return ECS_MB;
    }
    else if (wcscmp(lpManufacturer, L"EPoX COMPUTER CO., LTD") == 0)
    {
        return EPOX_MB;
    }
    else if (wcscmp(lpManufacturer, L"EVGA") == 0)
    {
        return EVGA_MB;
    }
    else if (wcscmp(lpManufacturer, L"First International Computer, Inc.") == 0)
    {
        return FIC_MB;
    }
    else if (wcscmp(lpManufacturer, L"FUJITSU SIEMENS") == 0 ||
             wcscmp(lpManufacturer, L"FUJITSU") == 0)
    {
        return FUJITSU_MB;
    }
    else if (wcscmp(lpManufacturer, L"Gigabyte Technology Co., Ltd.") == 0)
    {
        return GIGABYTE_MB;
    }
    else if (wcscmp(lpManufacturer, L"Hewlett-Packard") == 0)
    {
        return HP_MB;
    }
    else if (wcscmp(lpManufacturer, L"IBM") == 0)
    {
        return IBM_MB;
    }
    else if (wcscmp(lpManufacturer, L"Intel") == 0 ||
             wcscmp(lpManufacturer, L"Intel Corp.") == 0 ||
             wcscmp(lpManufacturer, L"Intel Corporation") == 0 ||
             wcscmp(lpManufacturer, L"INTEL Corporation") == 0)
    {
        return INTEL_MB;
    }
    else if (wcscmp(lpManufacturer, L"Lenovo") == 0 ||
             wcscmp(lpManufacturer, L"LENOVO") == 0)
    {
        return LENOVO_MB;
    }
    else if (wcscmp(lpManufacturer, L"Micro-Star International") == 0 ||
             wcscmp(lpManufacturer, L"MICRO-STAR INTERNATIONAL CO., LTD") == 0 ||
             wcscmp(lpManufacturer, L"MICRO-STAR INTERNATIONAL CO.,LTD") == 0 ||
             wcscmp(lpManufacturer, L"MSI") == 0)
    {
        return MSI_MB;
    }
    else if (wcscmp(lpManufacturer, L"Shuttle") == 0)
    {
        return SHUTTLE_MB;
    }
    else if (wcscmp(lpManufacturer, L"Supermicro") == 0)
    {
        return SUPERMICRO_MB;
    }
    else if (wcscmp(lpManufacturer, L"TOSHIBA") == 0)
    {
        return TOSHIBA_MB;
    }
    else if (wcscmp(lpManufacturer, L"XFX") == 0)
    {
        return XFX_MB;
    }
    else if (wcscmp(lpManufacturer, L"TYAN") == 0)
    {
        return TYAN_MB;
    }

    return UNKNOWN_MB;
}

typedef struct
{
    WORD wModel;
    LPWSTR lpModel;
} MB_MODELS;

MB_MODELS MbModelsList[] =
{
    /* ASRock */
    { _880GMH_USB3, L"880GMH/USB3" },
    { AOD790GX_128M, L"ASRock AOD790GX/128M" },
    { P55_Deluxe, L"P55 Deluxe" },
    /* ASUS */
    { Crosshair_III_Formula, L"Crosshair III Formula" },
    { M2N_SLI_DELUXE, L"M2N-SLI DELUXE" },
    { M4A79XTD_EVO, L"M4A79XTD EVO" },
    { P5W_DH_Deluxe, L"P5W DH Deluxe" },
    { P6X58D_E, L"P6X58D-E" },
    { P8P67, L"P8P67" },
    { P8P67_EVO, L"P8P67 EVO" },
    { P8P67_PRO, L"P8P67 PRO" },
    { P8P67_M_PRO, L"P8P67-M PRO" },
    { Rampage_Extreme, L"Rampage Extreme" },
    { Rampage_II_GENE, L"Rampage II GENE" },
    { AT3GC_I, L"AT3GC-I" },
    { P5K_VM, L"P5K-VM" },
    { P5QL_PRO, L"P5QL PRO" },
    { K8V_MX, L"K8V-MX" },
    { P4P800_VM, L"P4P800-VM" },
    { P9X79, L"P9X79" },
    { P6T, L"P6T" },
    { P8Z68_V_PRO, L"P8Z68-V PRO" },
    /* DFI */
    { LP_BI_P45_T2RS_Elite, L"LP BI P45-T2RS Elite" },
    { LP_DK_P55_T3eH9, L"LP DK P55-T3eH9" },
    /* ECS */
    { A890GXM_A, L"A890GXM-A" },
    /* EVGA */
    { X58_SLI_Classified, L"X58 SLI Classified" },
    /* Gigabyte */
    { _965P_S3, L"965P-S3" },
    { EP45_DS3R, L"EP45-DS3R" },
    { EP45_UD3R, L"EP45-UD3R" },
    { EX58_EXTREME, L"EX58-EXTREME" },
    { GA_MA770T_UD3, L"GA-MA770T-UD3" },
    { GA_MA785GMT_UD2H, L"GA-MA785GMT-UD2H" },
    { H67A_UD3H_B3, L"H67A-UD3H-B3" },
    { P35_DS3, L"P35-DS3" },
    { P35_DS3L, L"P35-DS3L" },
    { P55_UD4, L"P55-UD4" },
    { P55M_UD4, L"P55M-UD4" },
    { P67A_UD4_B3, L"P67A-UD4-B3" },
    { X38_DS5, L"X38-DS5" },
    { X58A_UD3R, L"X58A-UD3R" },
    { Z68X_UD7_B3, L"Z68X-UD7-B3" },
    { G31M_ES2C, L"G31M-ES2C" },
    { _965GM_S2, L"965GM-S2" },
    { G31M_ES2L, L"G31M-ES2L" },
    { _8I865GVMK, L"8I865GVMK" },
    /* Shuttle */
    { FH67, L"FH67" },
    /* EPOX */
    { _9NPA7I_9NPAI_9NPA7J_9NPAJ_3P, L"nForce4 DDR: 9NPA7I / 9NPAI / 9NPA7J / 9NPAJ-3P Series" },
    /* MSI */
    { G32M3_V2, L"G31M3 V2(MS-7529)" },
    { H55_G43, L"H55-G43(MS-7638)" },
    { G31TM_P35, L"G31TM-P35 (MS-7529)" },
    /* TYAN */
    { S2882, L"TYAN High-End Dual AMD Opteron, S2882" },
    {0}
};

LPC_VOLTAGE_DESC LpcVoltageDesc[LPC_INDEX_MAX];
WCHAR szLpcTempDesc[LPC_INDEX_MAX][MAX_STR_LEN];
WCHAR szLpcFanDesc[LPC_INDEX_MAX][MAX_STR_LEN];

WORD
LPC_GetMBModelId(LPWSTR lpModel)
{
    SIZE_T Index = 0;

    if (!lpModel) return UNKNOWN_MODEL;

    do
    {
        if (wcscmp(MbModelsList[Index].lpModel, lpModel) == 0)
            return MbModelsList[Index].wModel;
    }
    while (MbModelsList[++Index].wModel != 0);

    return UNKNOWN_MODEL;
}

VOID
InitVItem(INT Index, LPWSTR lpDesc, FLOAT ri, FLOAT rf, FLOAT vf)
{
    LpcVoltageDesc[Index].ri = ri;
    LpcVoltageDesc[Index].rf = rf;
    LpcVoltageDesc[Index].vf = vf;
    StringCbCopy(LpcVoltageDesc[Index].szDesc,
                 sizeof(LpcVoltageDesc[Index].szDesc),
                 lpDesc);
}

VOID
InitTItem(INT Index, LPWSTR lpDesc)
{
    StringCbCopy(szLpcTempDesc[Index],
                 sizeof(szLpcTempDesc[Index]),
                 lpDesc);
}

VOID
InitFItem(INT Index, LPWSTR lpDesc)
{
    StringCbCopy(szLpcFanDesc[Index],
                 sizeof(szLpcFanDesc[Index]),
                 lpDesc);
}

VOID
LPC_MainboardInfoInit(WORD wChipType)
{
    WCHAR szManufacturer[MAX_STR_LEN];
    WCHAR szModel[MAX_STR_LEN];
    WORD wManufId, wModelId = 0;

    SMBIOS_GetMainboardName(szModel, sizeof(szModel),
                            szManufacturer,
                            sizeof(szManufacturer));

    wManufId = LPC_GetMBManufacturerId(szManufacturer);
    wModelId = LPC_GetMBModelId(szModel);

    DebugTrace(L"Mainboard model: %s (id = 0x%x)\r\nMainboard manufacturer: %s (id = 0x%x)",
               szModel, wModelId, szManufacturer, wManufId);

    ZeroMemory(LpcVoltageDesc, sizeof(LpcVoltageDesc));
    ZeroMemory(szLpcTempDesc, sizeof(szLpcTempDesc));
    ZeroMemory(szLpcFanDesc, sizeof(szLpcFanDesc));

    switch (wChipType)
    {
        case IT8712F:
        case IT8716F:
        case IT8718F:
        case IT8720F:
        case IT8726F:
        {
            switch (wManufId)
            {
                case ASUS_MB:
                {
                    switch (wModelId)
                    {
                        case Crosshair_III_Formula: /* IT8720F */
                            InitVItem(8, L"VBat", 0.0f, 1.0f, 0.0f);
                            break;

                        case M2N_SLI_DELUXE:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(4, L"+12V",      30.0f, 10.0f, 0.0f);
                            InitVItem(7, L"+5VSB",     6.8f,  10.0f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Motherboard");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"Chassis Fan #1");
                            InitFItem(2, L"Power Fan");
                            break;

                        case M4A79XTD_EVO: /* IT8720F */
                            InitVItem(3, L"+5V",  6.8f,  10.0f, 0.0f);
                            InitVItem(8, L"VBat", 0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Motherboard");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"Chassis Fan #1");
                            InitFItem(2, L"Chassis Fan #2");
                            break;

                        case AT3GC_I: /* IT8718F */
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(4, L"+12V",      28.0f, 5.0f, 0.0f);
                            InitVItem(7, L"+5VSB",     6.8f,  10.0f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(1, L"Motherboard");

                            InitFItem(1, L"CPU Fan");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f, 1.0f, 0.0f);
                            InitVItem(8, L"VBat", 0.0f, 1.0f, 0.0f);
                            break;
                    }
                }
                break;

                case ASROCK_MB:
                {
                    switch (wModelId)
                    {
                        case P55_Deluxe: /* IT8720F */
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(4, L"+12V",      30.0f, 10.0f, 0.0f);
                            InitVItem(5, L"+5V",       0.0f,  1.0f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Motherboard");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"Chassis Fan #1");
                            InitFItem(2, L"Chassis Fan #2");
                            InitFItem(3, L"Chassis Fan #3");
                            InitFItem(4, L"Power Fan");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f, 1.0f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f, 1.0f, 0.0f);
                            break;
                    }
                }
                break;

                case DFI_MB:
                {
                    switch (wModelId)
                    {
                        case LP_BI_P45_T2RS_Elite: /* IT8718F */
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"FSB VTT",   0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(4, L"+12V",      30.0f, 10.0f, 0.0f);
                            InitVItem(5, L"NB Core",   0.0f,  1.0f, 0.0f);
                            InitVItem(6, L"VDIMM",     0.0f,  1.0f, 0.0f);
                            InitVItem(7, L"+5VSB",     6.8f,  10.0f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"System");
                            InitTItem(2, L"Chipset");
                            break;

                        case LP_DK_P55_T3eH9: /* IT8720F */
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"VTT",       0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(4, L"+12V",      30.0f, 10.0f, 0.0f);
                            InitVItem(5, L"CPU PLL",   0.0f,  1.0f, 0.0f);
                            InitVItem(6, L"DRAM",      0.0f,  1.0f, 0.0f);
                            InitVItem(7, L"+5VSB",     6.8f,  10.0f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"Chipset");
                            InitTItem(1, L"CPU PWM");
                            InitTItem(2, L"CPU");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"VTT",       0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(4, L"+12V",      30.0f, 10.0f, 0.0f);
                            InitVItem(6, L"DRAM",      0.0f,  1.0f, 0.0f);
                            InitVItem(7, L"+5VSB",     6.8f,  10.0f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);
                            break;
                    }
                }
                break;

                case GIGABYTE_MB:
                {
                    switch (wModelId)
                    {
                        case _965GM_S2:
                        case _965P_S3: /* IT8718F */
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(7, L"+12V",      27.0f, 9.1f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan");
                            break;

                        case EP45_DS3R: /* IT8718F */
                        case EP45_UD3R:
                        case X38_DS5:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(7, L"+12V",      27.0f, 9.1f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan #2");
                            InitFItem(2, L"Power Fan");
                            InitFItem(3, L"System Fan #1");
                            break;

                        case EX58_EXTREME: /* IT8720F */
                            InitVItem(0, L"CPU VCore", 0.0f, 1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f, 1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f, 10.0f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f, 1.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");
                            InitTItem(2, L"Northbridge");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan #2");
                            InitFItem(2, L"Power Fan");
                            InitFItem(3, L"System Fan #1");
                            break;

                        case P35_DS3: /* IT8718F */
                        case P35_DS3L:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(7, L"+12V",      27.0f, 9.1f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan #1");
                            InitFItem(2, L"System Fan #2");
                            InitFItem(3, L"Power Fan");
                            break;

                        case P55_UD4: /* IT8720F */
                        case P55M_UD4:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(5, L"+12V",      27.0f, 9.1f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(2, L"CPU");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan #2");
                            InitFItem(2, L"Power Fan");
                            InitFItem(3, L"System Fan #1");
                            break;

                        case GA_MA770T_UD3: /* IT8720F */
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(4, L"+12V",      27.0f, 9.1f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan #1");
                            InitFItem(2, L"System Fan #2");
                            InitFItem(3, L"Power Fan");
                            break;

                        case GA_MA785GMT_UD2H: /* IT8718F */
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(4, L"+12V",      27.0f, 9.1f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan");
                            InitFItem(2, L"NB Fan");
                            break;

                        case X58A_UD3R: /* IT8720F */
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f,  10.0f, 0.0f);
                            InitVItem(5, L"+12V",      27.0f, 9.1f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");
                            InitTItem(2, L"Northbridge");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan #2");
                            InitFItem(2, L"Power Fan");
                            InitFItem(3, L"System Fan #1");
                            break;

                        case G31M_ES2C: /* IT8718F */
                        case G31M_ES2L: /* IT8718F */
                            InitVItem(0, L"CPU VCore", 0.0f, 1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f, 1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f, 1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f, 10.0f, 0.0f);
                            InitVItem(7, L"+12V",      27.0f, 9.1f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f, 1.0f, 0.0f);

                            InitTItem(2, L"CPU");
                            break;

                        case _8I865GVMK: /* IT8712F */
                            InitVItem(0, L"CPU VCore", 0.0f, 1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f, 1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f, 1.0f, 0.0f);
                            InitVItem(3, L"+5V",       34.0f, 34.0f, 0.0f);
                            InitVItem(4, L"+12V",      27.0f, 9.1f, 0.0f);
                            InitVItem(5, L"VIN5",      0.0f, 1.0f, 0.0f);
                            InitVItem(6, L"VIN6",      0.0f, 1.0f, 0.0f);
                            InitVItem(7, L"5VSB",      34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"Temperature #1");
                            InitTItem(1, L"Temperature #2");
                            InitTItem(2, L"Temperature #3");

                            InitFItem(0, L"Fan #1");
                            InitFItem(1, L"Fan #2");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f, 1.0f, 0.0f);
                            InitVItem(1, L"DRAM",      0.0f, 1.0f, 0.0f);
                            InitVItem(2, L"+3.3V",     0.0f, 1.0f, 0.0f);
                            InitVItem(3, L"+5V",       6.8f, 10.0f, 0.0f);
                            InitVItem(8, L"VBat",      0.0f, 1.0f, 0.0f);
                            break;
                    }
                }
                break;

                default:
                    InitVItem(0, L"CPU VCore", 0.0f, 1.0f, 0.0f);
                    InitVItem(8, L"VBat",      0.0f, 1.0f, 0.0f);
                    break;
            }
        }
        break;

        case IT8721F:
        case IT8728F:
        case IT8771E:
        case IT8772E:
        {
            switch (wManufId)
            {
                case ECS_MB:
                {
                    switch (wModelId)
                    {
                        case A890GXM_A: /* IT8721F */
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"VDIMM",         0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"NB Voltage",    0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"Analog +3.3V",  10.0f, 10.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 10.0f, 10.0f, 0.0f);
                            InitVItem(8, L"VBat",          10.0f, 10.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");
                            InitTItem(2, L"Northbridge");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan");
                            InitFItem(2, L"Power Fan");
                            break;

                        default:
                            InitVItem(3, L"Analog +3.3V",  10.0f, 10.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 10.0f, 10.0f, 0.0f);
                            InitVItem(8, L"VBat",          10.0f, 10.0f, 0.0f);
                            break;
                    }
                }
                break;

                case GIGABYTE_MB:
                {
                    switch (wModelId)
                    {
                        case P67A_UD4_B3: /* IT8728F */
                            InitVItem(0, L"+12V",          100.0f, 10.0f, 0.0f);
                            InitVItem(1, L"+5V",           15.0f,  10.0f, 0.0f);
                            InitVItem(5, L"CPU VCore",     0.0f,   1.0f, 0.0f);
                            InitVItem(6, L"DRAM",          0.0f,   1.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 10.0f,  10.0f, 0.0f);
                            InitVItem(8, L"VBat",          10.0f,  10.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan #2");
                            InitFItem(2, L"Power Fan");
                            InitFItem(3, L"System Fan #1");
                            break;

                        case H67A_UD3H_B3: /* IT8728F */
                            InitVItem(0, L"VTT",           0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"+5V",           15.0f, 10.0f, 0.0f);
                            InitVItem(2, L"+12V",          68.0f, 22.0f, 0.0f);
                            InitVItem(5, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(6, L"DRAM",          0.0f,  1.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 10.0f, 10.0f, 0.0f);
                            InitVItem(8, L"VBat",          10.0f, 10.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(2, L"CPU");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"System Fan #1");
                            InitFItem(2, L"Power Fan");
                            InitFItem(3, L"System Fan #2");
                            break;

                        case Z68X_UD7_B3: /* IT8728F */
                            InitVItem(0, L"VTT",           0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"+3.3V",         13.3f, 20.5f, 0.0f);
                            InitVItem(2, L"+12V",          68.0f, 22.0f, 0.0f);
                            InitVItem(3, L"+5V",           14.3f, 20.0f, 0.0f);
                            InitVItem(5, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(6, L"DRAM",          0.0f,  1.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 10.0f, 10.0f, 0.0f);
                            InitVItem(8, L"VBat",          10.0f, 10.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");
                            InitTItem(2, L"System 3");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"Power Fan");
                            InitFItem(2, L"System Fan #1");
                            InitFItem(3, L"System Fan #2");
                            InitFItem(4, L"System Fan #3");
                            break;

                        default:
                            InitVItem(7, L"Standby +3.3V", 10.0f, 10.0f, 0.0f);
                            InitVItem(8, L"VBat",          10.0f, 10.0f, 0.0f);
                            break;
                    }
                }
                break;

                case SHUTTLE_MB:
                {
                    switch (wModelId)
                    {
                        case FH67: /* IT8772E */
                            InitVItem(0, L"CPU VCore",       0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"DRAM",            0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"PCH VCCIO",       0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"CPU VCCIO",       0.0f,  1.0f, 0.0f);
                            InitVItem(4, L"Graphic Voltage", 0.0f,  1.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V",   10.0f, 10.0f, 0.0f);
                            InitVItem(8, L"VBat",            10.0f, 10.0f, 0.0f);

                            InitTItem(0, L"System");
                            InitTItem(1, L"CPU");

                            InitFItem(0, L"Fan #1");
                            InitFItem(1, L"CPU Fan");
                            break;

                        default:
                            InitVItem(7, L"Standby +3.3V", 10.0f, 10.0f, 0.0f);
                            InitVItem(8, L"VBat",          10.0f, 10.0f, 0.0f);
                            break;
                    }
                }
                break;

                default:
                    InitVItem(7, L"Standby +3.3V", 10.0f, 10.0f, 0.0f);
                    InitVItem(8, L"VBat",          10.0f, 10.0f, 0.0f);
                    break;
            }
        }
        break;

        case F71858:
        {
            InitVItem(0, L"VCC3V",   150.0f, 150.0f, 0.0f);
            InitVItem(1, L"VSB3V",   150.0f, 150.0f, 0.0f);
            InitVItem(2, L"Battery", 150.0f, 150.0f, 0.0f);
        }
        break;

        case F71805F:
        case F71872F:
        {
            switch (wManufId)
            {
                case EPOX_MB:
                {
                    switch (wModelId)
                    {
                        case _9NPA7I_9NPAI_9NPA7J_9NPAJ_3P:
                            InitVItem(0, L"VCC",      100.0f, 100.0f, 0.0f);
                            InitVItem(1, L"+5V",      200.0f, 47.0f,  0.0f);
                            InitVItem(2, L"DIMM",     100.0f, 100.0f, 0.0f);
                            InitVItem(3, L"5VSB",     200.0f, 47.0f,  0.0f);
                            InitVItem(5, L"+12V",     200.0f, 20.0f,  0.0f);
                            InitVItem(6, L"Chipset",  0.0f,   1.0f,   0.0f);
                            InitVItem(7, L"CPU Core", 0.0f,   1.0f,   0.0f);
                            InitVItem(9, L"VSB",      100.0f, 100.0f, 0.0f);
                            InitVItem(10,L"VBat",     100.0f, 100.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Motherboard");
                            InitTItem(2, L"Aux");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"Power Fan");
                            InitFItem(2, L"Chassis Fan");
                            break;

                        default:
                            InitVItem(0,  L"VCC3.3V",  100.0f, 100.0f, 0.0f);
                            InitVItem(1,  L"VTT1.2V",  0.0f, 1.0f, 0.0f);
                            InitVItem(2,  L"VRAM",     100.0f, 100.0f, 0.0f);
                            InitVItem(3,  L"VCHIPSET", 47.0f, 100.0f, 0.0f);
                            InitVItem(4,  L"VCC5V",    200.0f, 47.0f, 0.0f);
                            InitVItem(5,  L"+12V",     200.0f, 20.0f, 0.0f);
                            InitVItem(6,  L"VCC1.5V",  0.0f, 1.0f, 0.0f);
                            InitVItem(7,  L"VCORE",    0.0f, 1.0f, 0.0f);
                            InitVItem(8,  L"VSB5V",    200.0f, 47.0f, 0.0f);
                            InitVItem(9,  L"VBATTERY", 100.0f, 100.0f, 0.0f);
                            InitVItem(10, L"VSB3.3V",  100.0f, 100.0f, 0.0f);
                            break;
                    }
                }
                break;

                default:
                    InitVItem(0,  L"VCC3.3V",  100.0f, 100.0f, 0.0f);
                    InitVItem(1,  L"VTT1.2V",  0.0f, 1.0f, 0.0f);
                    InitVItem(2,  L"VRAM",     100.0f, 100.0f, 0.0f);
                    InitVItem(3,  L"VCHIPSET", 47.0f, 100.0f, 0.0f);
                    InitVItem(4,  L"VCC5V",    200.0f, 47.0f, 0.0f);
                    InitVItem(5,  L"+12V",     200.0f, 20.0f, 0.0f);
                    InitVItem(6,  L"VCC1.5V",  0.0f, 1.0f, 0.0f);
                    InitVItem(7,  L"VCORE",    0.0f, 1.0f, 0.0f);
                    InitVItem(8,  L"VSB5V",    200.0f, 47.0f, 0.0f);
                    InitVItem(9,  L"VBATTERY", 100.0f, 100.0f, 0.0f);
                    InitVItem(10, L"VSB3.3V",  100.0f, 100.0f, 0.0f);
                    break;
            }
        }
        break;

        case F71862:
        case F71869:
        case F71882:
        case F71889AD:
        case F71889ED:
        case F71889F:
        {
            switch (wManufId)
            {
                case EVGA_MB:
                {
                    switch (wModelId)
                    {
                        case X58_SLI_Classified: /* F71882 */
                            InitVItem(0, L"VCC3V",     150.0f, 150.0f, 0.0f);
                            InitVItem(1, L"CPU VCore", 47.0f,  100.0f, 0.0f);
                            InitVItem(2, L"DIMM",      47.0f,  100.0f, 0.0f);
                            InitVItem(3, L"CPU VTT",   24.0f,  100.0f, 0.0f);
                            InitVItem(4, L"IOH Vcore", 24.0f,  100.0f, 0.0f);
                            InitVItem(5, L"+5V",       51.0f,  12.0f, 0.0f);
                            InitVItem(6, L"+12V",      56.0f,  6.8f, 0.0f);
                            InitVItem(7, L"3VSB",      150.0f, 150.0f, 0.0f);
                            InitVItem(8, L"VBat",      150.0f, 150.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"VREG");
                            InitTItem(2, L"System");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"Power Fan");
                            InitFItem(2, L"Chassis Fan");
                            break;

                        default:
                            InitVItem(0, L"VCC3V",     150.0f, 150.0f, 0.0f);
                            InitVItem(1, L"CPU VCore", 0.0f,   1.0f, 0.0f);
                            InitVItem(7, L"VSB3V",     150.0f, 150.0f, 0.0f);
                            InitVItem(8, L"VBat",      150.0f, 150.0f, 0.0f);
                            break;
                    }
                }
                break;

                case MSI_MB:
                {
                    switch (wModelId)
                    {
                        case G32M3_V2:
                            InitVItem(0, L"CPU I/O",   150.0f, 150.0f, 0.0f);
                            InitVItem(1, L"CPU Core0", 0.0f,   1.0f,   0.0f);
                            InitVItem(2, L"VIN2",      0.0f,   1.0f,   0.0f);
                            InitVItem(3, L"VIN3",      0.0f,   1.0f,   0.0f);
                            InitVItem(4, L"+5V",       200.0f, 47.0f,  0.0f);
                            InitVItem(5, L"+12V",      200.0f, 20.0f,  0.0f);
                            InitVItem(6, L"VIN6",      0.0f,   1.0f,   0.0f);
                            InitVItem(7, L"3VSB",      150.0f, 150.0f, 0.0f);
                            InitVItem(8, L"VBat",      150.0f, 150.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"Motherboard");

                            InitFItem(0, L"CPU Fan");
                            break;

                        case H55_G43:
                            InitVItem(0, L"CPU I/O",   150.0f, 150.0f, 0.0f);
                            InitVItem(1, L"CPU Core0", 0.0f,   1.0f,   0.0f);
                            InitVItem(2, L"VIN2",      0.0f,   1.0f,   0.0f);
                            InitVItem(3, L"+5V",       200.0f, 47.0f,  0.0f);
                            InitVItem(4, L"+12V",      200.0f, 20.0f,  0.0f);
                            InitVItem(5, L"VIN5",      0.0f,   1.0f,   0.0f);
                            InitVItem(6, L"VIN6",      0.0f,   1.0f,   0.0f);
                            InitVItem(7, L"3VSB",      150.0f, 150.0f, 0.0f);
                            InitVItem(8, L"VBat",      150.0f, 150.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"Motherboard");

                            InitFItem(0, L"CPU Fan");
                            break;

                        case G31TM_P35:
                            InitVItem(0, L"VCC3V",     150.0f, 150.0f, 0.0f);
                            InitVItem(1, L"Vcore",     0.0f,   1.0f,   0.0f);
                            InitVItem(2, L"VIN2",      0.0f,   1.0f,   0.0f);
                            InitVItem(3, L"VIN3",      0.0f,   1.0f,   0.0f);
                            InitVItem(4, L"+5V",       200.0f, 47.0f,  0.0f);
                            InitVItem(5, L"+12V",      200.0f, 20.0f,  0.0f);
                            InitVItem(6, L"VIN6",      0.0f,   1.0f,   0.0f);
                            InitVItem(7, L"3VSB",      150.0f, 150.0f, 0.0f);
                            InitVItem(8, L"VBat",      150.0f, 150.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Motherboard");

                            InitFItem(0, L"CPU Fan");
                            break;

                        default:
                            InitVItem(0, L"CPU I/O",   150.0f, 150.0f, 0.0f);
                            InitVItem(1, L"CPU Core0", 0.0f,   1.0f,   0.0f);
                            InitVItem(2, L"VIN2",      0.0f,   1.0f,   0.0f);
                            InitVItem(3, L"VIN3",      0.0f,   1.0f,   0.0f);
                            InitVItem(4, L"+5V",       200.0f, 47.0f,  0.0f);
                            InitVItem(5, L"+12V",      200.0f, 20.0f,  0.0f);
                            InitVItem(6, L"VIN6",      0.0f,   1.0f,   0.0f);
                            InitVItem(7, L"3VSB",      150.0f, 150.0f, 0.0f);
                            InitVItem(8, L"VBat",      150.0f, 150.0f, 0.0f);

                            InitTItem(0, L"CPU");

                            InitFItem(0, L"CPU Fan");
                            break;
                    }
                }
                break;

                default:
                    InitVItem(0, L"VCC3V",     150.0f, 150.0f, 0.0f);
                    InitVItem(1, L"CPU VCore", 0.0f,   1.0f, 0.0f);
                    InitVItem(7, L"VSB3V",     150.0f, 150.0f, 0.0f);
                    InitVItem(8, L"VBat",      150.0f, 150.0f, 0.0f);
                    break;
            }
        }
        break;

        case W83627EHF:
        {
            switch (wManufId)
            {
                case ASUS_MB:
                {
                    switch (wModelId)
                    {
                        case K8V_MX: /* W83627EHF */
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"Analog +3.3V",  34.0f, 34.0f, 0.0f);
                            InitVItem(4, L"+3.3V",         10.0f, 10.0f, 0.0f);
                            InitVItem(5, L"+5V",           20.0f, 10.0f, 0.0f);
                            InitVItem(6, L"+12V",          28.0f, 5.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(2, L"Motherboard");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"Chassis Fan");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"AVCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"3VCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(7, L"3VSB",      34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",      34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"System");

                            InitFItem(0, L"System Fan");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Auxiliary Fan");
                            InitFItem(3, L"CPU Fan #2");
                            InitFItem(4, L"Auxiliary Fan #2");
                            break;
                    }
                }
                break;

                case ASROCK_MB:
                {
                    switch (wModelId)
                    {
                        case AOD790GX_128M: /* W83627EHF */
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"Analog +3.3V",  34.0f, 34.0f, 0.0f);
                            InitVItem(4, L"+3.3V",         10.0f, 10.0f, 0.0f);
                            InitVItem(5, L"+5V",           20.0f, 10.0f, 0.0f);
                            InitVItem(6, L"+12V",          28.0f, 5.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(2, L"Motherboard");

                            InitFItem(0, L"CPU Fan");
                            InitFItem(1, L"Chassis Fan");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"AVCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"3VCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(7, L"3VSB",      34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",      34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"System");

                            InitFItem(0, L"System Fan");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Auxiliary Fan");
                            InitFItem(3, L"CPU Fan #2");
                            InitFItem(4, L"Auxiliary Fan #2");
                            break;
                    }
                }
                break;

                default:
                    InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                    InitVItem(2, L"AVCC",      34.0f, 34.0f, 0.0f);
                    InitVItem(3, L"3VCC",      34.0f, 34.0f, 0.0f);
                    InitVItem(7, L"3VSB",      34.0f, 34.0f, 0.0f);
                    InitVItem(8, L"VBAT",      34.0f, 34.0f, 0.0f);

                    InitTItem(0, L"CPU");
                    InitTItem(1, L"Auxiliary");
                    InitTItem(2, L"System");

                    InitFItem(0, L"System Fan");
                    InitFItem(1, L"CPU Fan");
                    InitFItem(2, L"Auxiliary Fan");
                    InitFItem(3, L"CPU Fan #2");
                    InitFItem(4, L"Auxiliary Fan #2");
                    break;
            }
        }
        break;

        case W83627DHG:
        case W83627DHGP:
        case W83667HG:
        case W83667HGB:
        {
            switch (wManufId)
            {
                case ASROCK_MB:
                {
                    switch (wModelId)
                    {
                        case _880GMH_USB3: /* W83627DHG-P */
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"+3.3V",         34.0f, 34.0f, 0.0f);
                            InitVItem(5, L"+5V",           15.0f, 7.5f, 0.0f);
                            InitVItem(6, L"+12V",          56.0f, 10.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(2, L"Motherboard");

                            InitFItem(0, L"Chassis Fan");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Power Fan");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"AVCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"3VCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(7, L"3VSB",      34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",      34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"System");

                            InitFItem(0, L"System Fan");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Auxiliary Fan");
                            InitFItem(3, L"CPU Fan #2");
                            InitFItem(4, L"Auxiliary Fan #2");
                            break;
                    }
                }
                break;

                case ASUS_MB:
                {
                    switch (wModelId)
                    {
                        case P6T: /* W83667HG */
                        case P6X58D_E: /* W83667HG */
                        case Rampage_II_GENE: /* W83667HG */
                            InitVItem(0, L"CPU VCore",     0.0f,   1.0f, 0.0f);
                            InitVItem(1, L"+12V",          11.5f,  1.91f, 0.0f);
                            InitVItem(2, L"Analog +3.3V",  34.0f,  34.0f, 0.0f);
                            InitVItem(3, L"+3.3V",         34.0f,  34.0f, 0.0f);
                            InitVItem(4, L"+5V",           15.0f,  7.5f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f,  34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f,  34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(2, L"Motherboard");

                            InitFItem(0, L"Chassis Fan #1");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Power Fan");
                            InitFItem(3, L"Chassis Fan #2");
                            InitFItem(4, L"Chassis Fan #3");
                            break;

                        case Rampage_Extreme: /* W83667HG */
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"+12V",          12.0f, 2.0f, 0.0f);
                            InitVItem(2, L"Analog +3.3V",  34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"+3.3V",         34.0f, 34.0f, 0.0f);
                            InitVItem(4, L"+5V",           15.0f, 7.5f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(2, L"Motherboard");

                            InitFItem(0, L"Chassis Fan #1");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Power Fan");
                            InitFItem(3, L"Chassis Fan #2");
                            InitFItem(4, L"Chassis Fan #3");
                            break;

                        case P5K_VM: /* W83627DHG */
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"+12V",          12.0f, 2.0f, 0.0f);
                            InitVItem(2, L"Analog +3.3V",  34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"+3.3V",         34.0f, 34.0f, 0.0f);
                            InitVItem(5, L"+5V",           15.0f, 7.5f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"Motherboard");

                            InitFItem(1, L"CPU Fan");
                            break;

                        case P5QL_PRO: /* W83667HG */
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"+12V",          12.0f, 2.0f, 0.0f);
                            InitVItem(2, L"Analog +3.3V",  34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"+3.3V",         34.0f, 34.0f, 0.0f);
                            InitVItem(5, L"+5V",           15.0f, 7.5f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"Motherboard");

                            InitFItem(1, L"CPU Fan");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"AVCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"3VCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(7, L"3VSB",      34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",      34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"System");

                            InitFItem(0, L"System Fan");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Auxiliary Fan");
                            InitFItem(3, L"CPU Fan #2");
                            InitFItem(4, L"Auxiliary Fan #2");
                            break;
                    }
                }
                break;

                default:
                    InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                    InitVItem(2, L"AVCC",      34.0f, 34.0f, 0.0f);
                    InitVItem(3, L"3VCC",      34.0f, 34.0f, 0.0f);
                    InitVItem(7, L"3VSB",      34.0f, 34.0f, 0.0f);
                    InitVItem(8, L"VBAT",      34.0f, 34.0f, 0.0f);

                    InitTItem(0, L"CPU");
                    InitTItem(1, L"Auxiliary");
                    InitTItem(2, L"System");

                    InitFItem(0, L"System Fan");
                    InitFItem(1, L"CPU Fan");
                    InitFItem(2, L"Auxiliary Fan");
                    InitFItem(3, L"CPU Fan #2");
                    InitFItem(4, L"Auxiliary Fan #2");
                    break;
            }
        }
        break;

        case W83627HF:
        case W83627THF:
        case W83687THF:
        {
            switch (wManufId)
            {
                case ASUS_MB:
                {
                    switch (wModelId)
                    {
                        case P4P800_VM:
                            InitVItem(0, L"Vcore1", 0.0f,  1.0f,  0.0f);
                            InitVItem(1, L"Vccp2", 0.0f,  1.0f,  0.0f);
                            InitVItem(2, L"+3.3V",  0.0f,  1.0f,  0.0f);
                            InitVItem(3, L"+5V",    34.0f, 51.0f, 0.0f);
                            InitVItem(4, L"+12V",   27.0f, 9.1f, 0.0f);
                            InitVItem(5, L"+5VSB",  34.0f, 34.0f,  0.0f);

                            InitTItem(0, L"Temperature #1");
                            InitTItem(1, L"Temperature #2");

                            InitFItem(0, L"Fan #1");
                            InitFItem(1, L"Fan #2");
                            break;
                    }
                }
                break;

                case TYAN_MB:
                {
                    switch (wModelId)
                    {
                        case S2882:
                            InitVItem(0, L"Vcore1", 0.0f,  1.0f,  0.0f);
                            InitVItem(1, L"Vcore2", 0.0f,  1.0f,  0.0f);
                            InitVItem(2, L"+3.3V",  0.0f,  1.0f,  0.0f);
                            InitVItem(3, L"+5V",    34.0f, 51.0f, 0.0f);
                            InitVItem(4, L"+12V",   34.0f, 4.2f,  0.0f);
                            InitVItem(5, L"+5VSB",  34.0f, 34.0f,  0.0f);

                            InitTItem(0, L"Motherboard");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"CPU");

                            InitFItem(0, L"Chassis Fan");
                            InitFItem(1, L"CPU1 Fan");
                            InitFItem(2, L"Power Fan");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(3, L"AVCC",      34.0f, 51.0f, 0.0f);
                            InitVItem(5, L"5VSB",      34.0f, 51.0f, 0.0f);
                            InitVItem(6, L"VBAT",      0.0f,  1.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"Auxiliary");
                            InitTItem(2, L"System");

                            InitFItem(0, L"System Fan");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Auxiliary Fan");
                            break;
                    }
                }
                break;

                default:
                    InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                    InitVItem(3, L"AVCC",      34.0f, 51.0f, 0.0f);
                    InitVItem(5, L"5VSB",      34.0f, 51.0f, 0.0f);
                    InitVItem(6, L"VBAT",      0.0f,  1.0f, 0.0f);

                    InitTItem(0, L"CPU");
                    InitTItem(1, L"Auxiliary");
                    InitTItem(2, L"System");

                    InitFItem(0, L"System Fan");
                    InitFItem(1, L"CPU Fan");
                    InitFItem(2, L"Auxiliary Fan");
                    break;
            }
        }
        break;

        case NCT6771F:
        case NCT6776F:
        {
            switch (wManufId)
            {
                case ASUS_MB:
                {
                    switch (wModelId)
                    {
                        case P8P67: /* NCT6776F */
                        case P8P67_EVO:
                        case P8P67_PRO:
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"+12V",          12.0f, 1.0f, 0.0f);
                            InitVItem(2, L"Analog +3.3V",  34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"+3.3V",         34.0f, 34.0f, 0.0f);
                            InitVItem(4, L"+5V",           12.0f, 3.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(2, L"Auxiliary");
                            InitTItem(3, L"Motherboard");

                            InitFItem(0, L"Chassis Fan #1");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Power Fan");
                            InitFItem(3, L"Chassis Fan #2");
                            break;

                        case P8P67_M_PRO: /* NCT6776F */
                        case P9X79: /* NCT6776F */
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"+12V",          11.0f, 1.0f, 0.0f);
                            InitVItem(2, L"Analog +3.3V",  34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"+3.3V",         34.0f, 34.0f, 0.0f);
                            InitVItem(4, L"+5V",           12.0f, 3.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(3, L"Motherboard");

                            InitFItem(0, L"Chassis Fan #1");
                            InitFItem(1, L"CPU Fan");
                            InitFItem(2, L"Chassis Fan #2");
                            InitFItem(3, L"Power Fan");
                            InitFItem(4, L"Auxiliary Fan");
                            break;

                        case P8Z68_V_PRO: /* NCT6776F */
                            InitVItem(0, L"CPU VCore",     0.0f,  1.0f, 0.0f);
                            InitVItem(1, L"+12V",          11.0f, 1.0f, 0.0f);
                            InitVItem(2, L"Analog +3.3V",  34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"+3.3V",         34.0f, 34.0f, 0.0f);
                            InitVItem(4, L"+5V",           12.0f, 3.0f, 0.0f);
                            InitVItem(7, L"Standby +3.3V", 34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",          34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(2, L"Auxiliary");
                            InitTItem(3, L"Motherboard");
                            break;

                        default:
                            InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                            InitVItem(2, L"AVCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(3, L"3VCC",      34.0f, 34.0f, 0.0f);
                            InitVItem(7, L"3VSB",      34.0f, 34.0f, 0.0f);
                            InitVItem(8, L"VBAT",      34.0f, 34.0f, 0.0f);

                            InitTItem(0, L"CPU");
                            InitTItem(1, L"CPU");
                            InitTItem(2, L"Auxiliary");
                            InitTItem(3, L"System");
                            break;
                    }
                }
                break;

                default:
                    InitVItem(0, L"CPU VCore", 0.0f,  1.0f, 0.0f);
                    InitVItem(2, L"AVCC",      34.0f, 34.0f, 0.0f);
                    InitVItem(3, L"3VCC",      34.0f, 34.0f, 0.0f);
                    InitVItem(7, L"3VSB",      34.0f, 34.0f, 0.0f);
                    InitVItem(8, L"VBAT",      34.0f, 34.0f, 0.0f);

                    InitTItem(0, L"CPU");
                    InitTItem(1, L"CPU");
                    InitTItem(2, L"Auxiliary");
                    InitTItem(3, L"System");
                    break;
            }
        }
        break;
    }
}
