/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/cpuid.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


typedef struct
{
    DWORD dwFlag;
    LPWSTR lpszFeature;
    LPWSTR lpszDesc;
} FEATURE_INFO_STRUCT;

/* EAX = 0x01 ECX features */
const FEATURE_INFO_STRUCT ECX_01_FeaturesList[] =
{
    { 0x00000001, L"SSE3", L"SSE3 New Instructions" },
    { 0x00000002, L"PCLMULQDQ", L"PCLMULQDQ Instruction" },
    { 0x00000004, L"DTES64", L"64-Bit Debug Store" },
    { 0x00000008, L"MONITOR", L"MONITOR/MWAIT" },
    { 0x00000010, L"DS-CPL", L"CPL Qualified Debug Store" },
    { 0x00000020, L"VMX", L"Virtual Machine Extensions" },
    { 0x00000040, L"SMX", L"Safer Mode Extensions" },
    { 0x00000080, L"EIST", L"Enhanced Intel SpeedStep Technology" },
    { 0x00000100, L"TM2", L"Thermal Monitor" },
    { 0x00000200, L"SSSE3", L"Supplemental Streaming SIMD Extensions 3" },
    { 0x00000400, L"CNXT-ID", L"L1 Context ID" },
    { 0x00001000, L"FMA", L"Fused Multiply Add" },
    { 0x00002000, L"CMPXCHG16B", L"CMPXCHG16B Support" },
    { 0x00004000, L"XTPR", L"xTPR Update Control" },
    { 0x00008000, L"PDCM", L"Perf/Debug Capability MSR" },
    { 0x00020000, L"PCID", L"Process Context Identifiers" },
    { 0x00040000, L"DCA", L"Direct Cache Access" },
    { 0x00080000, L"SSE4.1", L"Streaming SIMD Extensions 4.1" },
    { 0x00100000, L"SSE4.2", L"Streaming SIMD Extensions 4.2" },
    { 0x00200000, L"x2APIC", L"Extended xAPIC Support" },
    { 0x00400000, L"MOVBE", L"MOVBE Instruction" },
    { 0x00800000, L"POPCNT", L"POPCNT Instruction Support" },
    { 0x01000000, L"TSC-DEADLINE", L"Time Stamp Counter Deadline" },
    { 0x02000000, L"AES", L"AES Instruction Extensions" },
    { 0x04000000, L"XSAVE", L"XSAVE/XSTOR States" },
    { 0x08000000, L"OSXSAVE", L"OS-Enabled Extended State Management" },
    { 0x10000000, L"AVX", L"Advanced Vector Extensions" },
    { 0 }
};

/* EAX = 0x01 EDX features */
const FEATURE_INFO_STRUCT EDX_01_FeaturesList[] =
{
    { 0x00000001, L"FPU", L"x87 FPU on Chip" },
    { 0x00000002, L"VME", L"Virtual-8086 Mode Enhancement" },
    { 0x00000004, L"DE", L"Debugging Extensions" },
    { 0x00000008, L"PSE", L"Page Size Extensions" },
    { 0x00000010, L"TSC", L"Time Stamp Counter" },
    { 0x00000020, L"MSR", L"RDMSR and WRMSR Support" },
    { 0x00000040, L"PAE", L"Physical Address Extensions" },
    { 0x00000080, L"MCE", L"Machine Check Exception" },
    { 0x00000100, L"CX8", L"CMPXCHG8B Inst." },
    { 0x00000200, L"APIC", L"APIC on Chip" },
    { 0x00000800, L"SEP", L"SYSENTER and SYSEXIT" },
    { 0x00001000, L"MTRR", L"Memory Type Range Registers" },
    { 0x00002000, L"PGE", L"PTE Global Bit" },
    { 0x00004000, L"MCA", L"Machine Check Architecture" },
    { 0x00008000, L"CMOV", L"Conditional Move/Compare Instruction" },
    { 0x00010000, L"PAT", L"Page Attribute Table" },
    { 0x00020000, L"PSE36", L"36-bit Page Size Extension" },
    { 0x00040000, L"PSN", L"Processor Serial Number" },
    { 0x00080000, L"CLFSH", L"CFLUSH Instruction" },
    { 0x00200000, L"DS", L"Debug Store" },
    { 0x00400000, L"ACPI", L"Thermal Monitor and Clock Ctrl" },
    { 0x00800000, L"MMX", L"MMX Technology" },
    { 0x01000000, L"FXSR", L"FXSAVE/FXRSTOR" },
    { 0x02000000, L"SSE", L"SSE Extensions" },
    { 0x04000000, L"SSE2", L"SSE2 Extensions" },
    { 0x08000000, L"SS", L"Self Snoop" },
    { 0x10000000, L"HTT", L"Multithreading" },
    { 0x20000000, L"TM", L"Thermal Monitor" },
    { 0x80000000, L"PBE", L"Pending Break Enable" },
    { 0 }
};

/* EAX = 0x80000001 ECX features */
const FEATURE_INFO_STRUCT ECX_81_FeaturesList[] =
{
    { 0x00000001, L"LAHF-SAHF", L"LAHF/SAHF available in 64-bit mode" },
    { 0x00000002, L"CmpLegacy", L"Core multi-processing legacy mode" },
    { 0x00000004, L"SVM", L"Secure Virtual Machine" },
    { 0x00000008, L"ExtApicSpace", L"Extended APIC Register Space" },
    { 0x00000010, L"AltMovCr8", L"LOCK MOV CR0 means MOV CR8" },
    { 0x00000020, L"LZCNT", L"LZCNT Support" },
    { 0x00000040, L"SSE4A", L"SSE4A Instruction Support" },
    { 0x00000080, L"MisAlignSse", L"Misaligned SSE Support Mode Available" },
    { 0x00000100, L"3DNowPrefetch", L"PREFETCH and PREFETCHW Support" },
    { 0x00001000, L"SKINIT", L"SKINIT and DEV support" },
    { 0 }
};

/* EAX = 0x80000001 EDX features */
const FEATURE_INFO_STRUCT EDX_81_FeaturesList[] =
{
    { 0x00000800, L"SYSCALL", L"SYSCALL/SYSRET available in 64-bit mode" },
    { 0x00100000, L"XD", L"Execute Disable Bit available" },
    { 0x00400000, L"MMXExt", L"Extensions to MMX instructions" },
    { 0x02000000, L"FFXSR", L"FXSAVE and FXRSTOR instruction optimizations" },
    { 0x04000000, L"1GB-PG", L"1GB page support" },
    { 0x08000000, L"RDTSCP", L"RDTSCP Support" },
    { 0x20000000, L"EM64T", L"64-bit Technology available" },
    { 0x40000000, L"3DNowExt", L"AMD extensions to 3DNow!" },
    { 0x80000000, L"3DNow!", L"3Dnow! instructions" },
    { 0 }
};

/* EAX = 0x80000007 EDX features */
const FEATURE_INFO_STRUCT EDX_87_FeaturesList[] =
{
    { 0x00000100, L"INVTSC", L"" },
    { 0 }
};

static VOID
EnumSupportedFeatures(const FEATURE_INFO_STRUCT *List, DWORD dwFlag)
{
    WCHAR szSupported[MAX_STR_LEN], szUnsupported[MAX_STR_LEN];
    SIZE_T Index = 0;

    LoadMUIString(IDS_CPUID_SUPPORTED,
                  szSupported,
                  sizeof(szSupported)/sizeof(WCHAR));
    LoadMUIString(IDS_CPUID_UNSUPPORTED,
                  szUnsupported,
                  sizeof(szUnsupported)/sizeof(WCHAR));

    do
    {
        IoAddItem(1, (List[Index].dwFlag & dwFlag) ? 1 : 2,
                  L"%s (%s)",
                  List[Index].lpszDesc,
                  List[Index].lpszFeature);
        IoSetItemText((List[Index].dwFlag & dwFlag) ? szSupported : szUnsupported);
    }
    while (List[++Index].dwFlag != 0);
}

BOOL
GetCPUName(LPWSTR lpszCpuName, SIZE_T Size)
{
    INT CPUInfoBase[4] = {-1}, CPUInfo[4] = {-1};
    char szBrandString[0x40] = {0};

    __cpuid(CPUInfoBase, 0x80000000);

    if (CPUInfoBase[0] >= 0x80000002)
    {
        __cpuid(CPUInfo, 0x80000002);

        CopyMemory(szBrandString, CPUInfo, sizeof(CPUInfo));

        if (CPUInfoBase[0] >= 0x80000003)
        {
            __cpuid(CPUInfo, 0x80000003);
            CopyMemory(szBrandString + 16, CPUInfo, sizeof(CPUInfo));
        }

        if (CPUInfoBase[0] >= 0x80000004)
        {
            __cpuid(CPUInfo, 0x80000004);
            CopyMemory(szBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }

        StringCbPrintf(lpszCpuName, Size, L"%S", szBrandString);
        ChopSpaces(lpszCpuName, Size);

        return TRUE;
    }

    return FALSE;
}

VOID
GetCPUVendor(LPWSTR lpszCpuVendor, SIZE_T Size)
{
    INT CPUInfo[4] = {-1};
    char szCpuString[0x20] = {0};

    __cpuid(CPUInfo, 0);

    *((INT*)szCpuString) = CPUInfo[1];
    *((INT*)(szCpuString + 4)) = CPUInfo[3];
    *((INT*)(szCpuString + 8)) = CPUInfo[2];

    StringCbPrintf(lpszCpuVendor, Size, L"%S", szCpuString);
}

INT
GetLogicalProcessorsCount(VOID)
{
    INT max, physical_count, logical_count = 1;
    BOOL IsOldCpu = TRUE;
    INT CPUInfo_A[4] = {-1};
    INT CPUInfo_B[4] = {-1};
    INT CPUInfo[4] = {-1};

    __cpuid(CPUInfo, 0);
    max = CPUInfo[0];

    __cpuid(CPUInfo_A, 1);

    if (CPUInfo_A[3] & (1 << 28))
    {
        if (max >= 0xB)
        {
            __cpuidex(CPUInfo_B, 0xB, 0);
            if (CPUInfo_B[1])
                IsOldCpu = FALSE;
        }

        if (IsOldCpu)
        {
            logical_count = (CPUInfo_A[1] >> 16) & 0x7F;
        }
        else
        {
            logical_count = CPUInfo_B[1];

            __cpuidex(CPUInfo_B, 0xB, 1);
            physical_count = CPUInfo_B[1];

            logical_count =
                (logical_count * physical_count) / 2;
        }
    }

    return logical_count;
}

INT
GetPhysicalProcessorsCount(VOID)
{
    BOOL IsOldCpu = TRUE;
    INT CPUInfo_A[4] = {-1};
    INT CPUInfo_B[4] = {-1};
    INT CPUInfo[4] = {-1};
    INT max, physical_count;

    __cpuid(CPUInfo, 0);
    max = CPUInfo[0];

    __cpuid(CPUInfo_A, 1);

    if (CPUInfo_A[3] & (1 << 28))
    {
        if (max >= 0xB)
        {
            __cpuidex(CPUInfo_B, 0xB, 0);
            if (CPUInfo_B[1])
                IsOldCpu = FALSE;
        }

        if (IsOldCpu)
        {
            __cpuidex(CPUInfo, 4, 0);
            physical_count = (CPUInfo[0] >> 26) + 1;
        }
        else
        {
            __cpuidex(CPUInfo_B, 0xB, 1);
            physical_count = CPUInfo_B[1] / 2;
        }
    }
    else
    {
        physical_count = 1;
    }

    return physical_count;
}

VOID
GetProcessorIDs(PCPU_IDS Id)
{
    DWORD ExtModel = 0, ExtFamily = 0;
    INT CPUInfo[4] = {-1};

    if (!Id) return;

    __cpuid(CPUInfo, 1);

    Id->Family = (CPUInfo[0] >> 8) & 0xf;
    Id->Model = (CPUInfo[0] >> 4) & 0xf;
    Id->Stepping = CPUInfo[0] & 0xf;

    ExtModel = (CPUInfo[0] >> 16) & 0xf;
    ExtFamily = (CPUInfo[0] >> 20) & 0xff;

    if (Id->Family == 0x06 || Id->Family == 0x0F)
    {
        Id->Model += (ExtModel << 4);
    }

    if (Id->Family == 0xF)
    {
        Id->Family += ExtFamily;
    }
}

BOOL
Is64BitCpu(VOID)
{
    INT CPUInfo[4] = {-1};

    __cpuid(CPUInfo, 0x80000001);

    return CPUInfo[3] & 0x20000000;
}

VOID
CPUIDInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    DWORD dwECX, dwEDX;
    INT CPUInfo[4] = {-1};
    CPU_IDS CpuIds = {0};
    INT Count;

    IoAddIcon(IDI_CPU);
    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);

    IoAddHeader(0, 0, IDS_PROP);

    /* Get CPU Name */
    if (GetCPUName(szText, sizeof(szText)))
    {
        IoAddValueName(1, 0, IDS_CPUID_NAME);
        IoSetItemText(szText);
    }

    /* Get CPU Vendor */
    GetCPUVendor(szText, sizeof(szText));
    IoAddValueName(1, 0, IDS_MANUFACTURER);
    IoSetItemText(szText);

    GetProcessorIDs(&CpuIds);

    /* Stepping ID */
    IoAddValueName(1, 0, IDS_CPUID_STEPPINGID);
    IoSetItemText(L"%d (%xh)",
                  CpuIds.Stepping, CpuIds.Stepping);

    /* Model */
    IoAddValueName(1, 0, IDS_CPUID_MODEL);
    IoSetItemText(L"%d (%xh)",
                  CpuIds.Model, CpuIds.Model);

    /* Family */
    IoAddValueName(1, 0, IDS_CPUID_FAMILY);
    IoSetItemText(L"%d (%xh)",
                  CpuIds.Family, CpuIds.Family);

    /* Physical processors count */
    Count = GetPhysicalProcessorsCount();
    if (Count > 0)
    {
        IoAddValueName(1, 0, IDS_CPUID_PHYSICAL_COUNT);
        IoSetItemText(L"%d", Count);
    }

    /* Logical processors count */
    Count = GetLogicalProcessorsCount();
    if (Count > 0)
    {
        IoAddValueName(1, 0, IDS_CPUID_LOGICAL_COUNT);
        IoSetItemText(L"%d", Count);
    }

    IoAddFooter();
    IoAddHeader(0, 0, IDS_CPUID_FEATURES);

    /* Get ECX and EDX features (EAX = 0x1) */
    __cpuid(CPUInfo, 0);
    if (CPUInfo[0] >= 1)
    {
        __cpuid(CPUInfo, 1);
        dwECX = CPUInfo[2];
        dwEDX = CPUInfo[3];

        EnumSupportedFeatures(EDX_01_FeaturesList, dwEDX);
        EnumSupportedFeatures(ECX_01_FeaturesList, dwECX);
    }

    /* Get ECX and EDX features (EAX = 0x80000001) */
    __cpuid(CPUInfo, 0x80000000);
    if (CPUInfo[0] >= 0x80000001)
    {
        __cpuid(CPUInfo, 0x80000001);
        dwECX = CPUInfo[2];
        dwEDX = CPUInfo[3];

        EnumSupportedFeatures(EDX_81_FeaturesList, dwEDX);
        EnumSupportedFeatures(ECX_81_FeaturesList, dwECX);
    }

    /* Get EDX features (EAX = 0x80000007) */
    __cpuid(CPUInfo, 0x80000000);
    if (CPUInfo[0] >= 0x80000007)
    {
        __cpuid(CPUInfo, 0x80000007);
        dwEDX = CPUInfo[3];

        EnumSupportedFeatures(EDX_87_FeaturesList, dwEDX);
    }
}
