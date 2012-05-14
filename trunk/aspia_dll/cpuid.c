/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/cpuid.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


#define IA32_PERF_STATUS 0x0198
#define MSR_PLATFORM_INFO 0xCE

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

LONG
GetMicrocodeRevision(VOID)
{
    INT CPUInfo[4] = {-1};
    LARGE_INTEGER Value;

    if (!WriteMsr(0x8B, 0, 0, 0))
        return 0;

    __cpuid(CPUInfo, 1);

    if (ReadMsr(0x8B, 0, (UINT64*)&Value))
    {
        return Value.HighPart;
    }

    return 0;
}

BYTE
GetPlatformId(VOID)
{
    LARGE_INTEGER Value;

    if (ReadMsr(0x17, 0, (UINT64*)&Value))
    {
        return 1 << ((Value.HighPart >> 18) & 7);
    }

    return 0;
}

DOUBLE
GetCpuMultiplierCore(VOID)
{
    LARGE_INTEGER Value;

    if (ReadMsr(IA32_PERF_STATUS, 0, (UINT64*)&Value))
    {
        return ((Value.HighPart >> 8) & 0x1F) + 0.5 * ((Value.HighPart >> 14) & 1);
    }

    return 0;
}

DOUBLE
GetCpuMultiplierSandyBridge(VOID)
{
    LARGE_INTEGER Value;

    if (ReadMsr(MSR_PLATFORM_INFO, 0, (UINT64*)&Value))
    {
        return (Value.LowPart >> 8) & 0xFF;
    }

    return 0;
}

DOUBLE
GetCpuMultiplier(VOID)
{
    CPU_IDS CpuIds = {0};

    GetProcessorIDs(&CpuIds);

    switch (CpuIds.Family)
    {
        case 0x06:
        {
            switch (CpuIds.Model)
            {
                case 0x1A:
                case 0x1E:
                case 0x1F:
                case 0x25:
                case 0x2C:
                case 0x2E:
                case 0x2A:
                case 0x2D:
                    return GetCpuMultiplierSandyBridge();
            }
        }
        break;
    }

    return GetCpuMultiplierCore();
}

DOUBLE
GetCpuSpeed(INT CpuIndex)
{
    UINT64 Start, End, Freq;
    UINT64 TimeStampCount;
    HANDLE hThread;
    DWORD_PTR OldMask;

    if (CpuIndex != -1)
    {
        hThread = GetCurrentThread();

        OldMask = SetThreadAffinityMask(hThread, 1 << CpuIndex);
    }

    QueryPerformanceFrequency((LARGE_INTEGER*)&Freq);

    QueryPerformanceCounter((LARGE_INTEGER*)&Start);

    TimeStampCount = __rdtsc();

    Sleep(100);

    TimeStampCount = __rdtsc() - TimeStampCount;

    QueryPerformanceCounter((LARGE_INTEGER*)&End);

    if (CpuIndex != -1)
    {
        SetThreadAffinityMask(hThread, OldMask);
    }

    return (DOUBLE)(TimeStampCount * Freq / (End - Start)) / 1000000.0;
}

DOUBLE
GetCpuBusSpeed(INT CpuIndex)
{
    DOUBLE Multiplier = GetCpuMultiplier();

    if (Multiplier == 0) return 0;

    return GetCpuSpeed(CpuIndex) / Multiplier;
}

typedef struct
{
    UINT L1DataSize;
    UINT L1InstSize;
    UINT L1DataWays;
    UINT L1InstWays;
    UINT L1DataLines;
    UINT L1InstLines;

    UINT L2Size;
    UINT L2Ways;
    UINT L2Lines;

    UINT L3Size;
    UINT L3Ways;
    UINT L3Lines;
} INTEL_CACHE_INFO;

VOID
GetIntelCpuCacheInfo(INTEL_CACHE_INFO *Info)
{
    INT i, Values[16] = {0};
    INT CPUInfo[4] = {-1};
    BYTE Family;

    __cpuid(CPUInfo, 1);

    Family = (CPUInfo[0] >> 8) & 0xf;

    __cpuid(CPUInfo, 2);

    Values[0]  = (CPUInfo[0] & 0xFF000000) >> 24;
    Values[1]  = (CPUInfo[0] & 0x00FF0000) >> 16;
    Values[2]  = (CPUInfo[0] & 0x0000FF00) >> 8;
    Values[3]  =  CPUInfo[0] & 0x000000FF;

    Values[4]  = (CPUInfo[1] & 0xFF000000) >> 24;
    Values[5]  = (CPUInfo[1] & 0x00FF0000) >> 16;
    Values[6]  = (CPUInfo[1] & 0x0000FF00) >> 8;
    Values[7]  =  CPUInfo[1] & 0x000000FF;

    Values[8]  = (CPUInfo[2] & 0xFF000000) >> 24;
    Values[9]  = (CPUInfo[2] & 0x00FF0000) >> 16;
    Values[10] = (CPUInfo[2] & 0x0000FF00) >> 8;
    Values[11] =  CPUInfo[2] & 0x000000FF;

    Values[12] = (CPUInfo[3] & 0xFF000000) >> 24;
    Values[13] = (CPUInfo[3] & 0x00FF0000) >> 16;
    Values[14] = (CPUInfo[3] & 0x0000FF00) >> 8;
    Values[15] =  CPUInfo[3] & 0x000000FF;

    for (i = 0; i < 16; i++)
    {
        /* See Intel Processor Identification and the CPUID Instruction, Table 5-7 */
        switch (Values[i])
        {
            case 0x06: Info->L1InstSize = 8;    Info->L1InstWays = 4;  Info->L1InstLines = 32; break;
            case 0x08: Info->L1InstSize = 16;   Info->L1InstWays = 4;  Info->L1InstLines = 32; break;
            case 0x09: Info->L1InstSize = 32;   Info->L1InstWays = 4;  Info->L1InstLines = 64; break;
            case 0x0A: Info->L1DataSize = 8;    Info->L1DataWays = 2;  Info->L1DataLines = 32; break;
            case 0x0C: Info->L1DataSize = 16;   Info->L1DataWays = 4;  Info->L1DataLines = 32; break;
            case 0x0D: Info->L1DataSize = 16;   Info->L1DataWays = 4;  Info->L1DataLines = 64; break;
            case 0x0E: Info->L1DataSize = 24;   Info->L1DataWays = 6;  Info->L1DataLines = 64; break;
            case 0x10: Info->L1DataSize = 16;   Info->L1DataWays = 4;  Info->L1DataLines = 32; break;
            case 0x15: Info->L1InstSize = 16;   Info->L1InstWays = 4;  Info->L1InstLines = 32; break;
            case 0x1A: Info->L2Size     = 96;   Info->L2Ways     = 6;  Info->L2Lines     = 64; break;
            case 0x21: Info->L2Size     = 256;  Info->L2Ways     = 8;  Info->L2Lines     = 64; break;
            case 0x22: Info->L3Size     = 512;  Info->L3Ways     = 4;  Info->L3Lines     = 64; break;
            case 0x23: Info->L3Size     = 1024; Info->L3Ways     = 8;  Info->L3Lines     = 64; break;
            case 0x25: Info->L3Size     = 2048; Info->L3Ways     = 8;  Info->L3Lines     = 64; break;
            case 0x29: Info->L3Size     = 4096; Info->L3Ways     = 8;  Info->L3Lines     = 64; break;
            case 0x2C: Info->L1DataSize = 32;   Info->L1DataWays = 8;  Info->L1DataLines = 64; break;
            case 0x30: Info->L1InstSize = 32;   Info->L1InstWays = 8;  Info->L1InstLines = 64; break;
            case 0x39: Info->L2Size     = 128;  Info->L2Ways     = 4;  Info->L2Lines     = 64; break;
            case 0x3A: Info->L2Size     = 192;  Info->L2Ways     = 6;  Info->L2Lines     = 64; break;
            case 0x3B: Info->L2Size     = 128;  Info->L2Ways     = 2;  Info->L2Lines     = 64; break;
            case 0x3C: Info->L2Size     = 256;  Info->L2Ways     = 4;  Info->L2Lines     = 64; break;
            case 0x3D: Info->L2Size     = 384;  Info->L2Ways     = 6;  Info->L2Lines     = 64; break;
            case 0x3E: Info->L2Size     = 512;  Info->L2Ways     = 4;  Info->L2Lines     = 64; break;
            case 0x41: Info->L2Size     = 128;  Info->L2Ways     = 4;  Info->L2Lines     = 32; break;
            case 0x42: Info->L2Size     = 256;  Info->L2Ways     = 4;  Info->L2Lines     = 32; break;
            case 0x43: Info->L2Size     = 512;  Info->L2Ways     = 4;  Info->L2Lines     = 32; break;
            case 0x44: Info->L2Size     = 1024; Info->L2Ways     = 4;  Info->L2Lines     = 32; break;
            case 0x45: Info->L2Size     = 2048; Info->L2Ways     = 4;  Info->L2Lines     = 32; break;
            case 0x46: Info->L3Size     = 4096; Info->L3Ways     = 4;  Info->L3Lines     = 64; break;
            case 0x47: Info->L3Size     = 8192; Info->L3Ways     = 8;  Info->L3Lines     = 64; break;
            case 0x48: Info->L2Size     = 3072; Info->L2Ways     = 12; Info->L2Lines     = 64; break;

            case 0x49:
                if (Family != 0x0F)
                {
                    Info->L2Size = 4096; Info->L2Ways = 16; Info->L2Lines = 64;
                }
                else
                {
                    Info->L3Size = 4096; Info->L3Ways = 16; Info->L3Lines = 64;
                } break;
            case 0x4A: Info->L3Size     = 6144;  Info->L3Ways     = 12; Info->L3Lines     = 64;  break;
            case 0x4B: Info->L3Size     = 8192;  Info->L3Ways     = 16; Info->L3Lines     = 64;  break;
            case 0x4C: Info->L3Size     = 12288; Info->L3Ways     = 12; Info->L3Lines     = 64;  break;
            case 0x4D: Info->L3Size     = 16384; Info->L3Ways     = 16; Info->L3Lines     = 64;  break;
            case 0x4E: Info->L2Size     = 6144;  Info->L2Ways     = 24; Info->L2Lines     = 64;  break;
            case 0x60: Info->L1DataSize = 16;    Info->L1DataWays = 8;  Info->L1DataLines = 64;  break;
            case 0x66: Info->L1DataSize = 8;     Info->L1DataWays = 4;  Info->L1DataLines = 64;  break;
            case 0x67: Info->L1DataSize = 16;    Info->L1DataWays = 4;  Info->L1DataLines = 64;  break;
            case 0x68: Info->L1DataSize = 32;    Info->L1DataWays = 4;  Info->L1DataLines = 64;  break;
            case 0x77: Info->L1InstSize = 16;    Info->L1InstWays = 8;  Info->L1InstLines = 64;  break;
            case 0x78: Info->L2Size     = 1024;  Info->L2Ways     = 4;  Info->L2Lines     = 64;  break;
            case 0x79: Info->L2Size     = 128;   Info->L2Ways     = 8;  Info->L2Lines     = 64;  break;
            case 0x7A: Info->L2Size     = 256;   Info->L2Ways     = 8;  Info->L2Lines     = 64;  break;
            case 0x7B: Info->L2Size     = 512;   Info->L2Ways     = 8;  Info->L2Lines     = 64;  break;
            case 0x7C: Info->L2Size     = 1024;  Info->L2Ways     = 8;  Info->L2Lines     = 64;  break;
            case 0x7D: Info->L2Size     = 2048;  Info->L2Ways     = 8;  Info->L2Lines     = 64;  break;
            case 0x7E: Info->L2Size     = 256;   Info->L2Ways     = 8;  Info->L2Lines     = 128; break;
            case 0x7F: Info->L2Size     = 512;   Info->L2Ways     = 2;  Info->L2Lines     = 64;  break;
            case 0x80: Info->L2Size     = 512;   Info->L2Ways     = 8;  Info->L2Lines     = 64;  break;
            case 0x81: Info->L2Size     = 128;   Info->L2Ways     = 8;  Info->L2Lines     = 32;  break;
            case 0x82: Info->L2Size     = 256;   Info->L2Ways     = 8;  Info->L2Lines     = 32;  break;
            case 0x83: Info->L2Size     = 512;   Info->L2Ways     = 8;  Info->L2Lines     = 32;  break;
            case 0x84: Info->L2Size     = 1024;  Info->L2Ways     = 8;  Info->L2Lines     = 32;  break;
            case 0x85: Info->L2Size     = 2048;  Info->L2Ways     = 8;  Info->L2Lines     = 32;  break;
            case 0x86: Info->L2Size     = 512;   Info->L2Ways     = 8;  Info->L2Lines     = 64;  break;
            case 0x87: Info->L2Size     = 1024;  Info->L2Ways     = 8;  Info->L2Lines     = 64;  break;
            case 0x88: Info->L3Size     = 2048;  Info->L3Ways     = 4;  Info->L3Lines     = 64;  break;
            case 0x89: Info->L3Size     = 4096;  Info->L3Ways     = 4;  Info->L3Lines     = 64;  break;
            case 0x8A: Info->L3Size     = 8192;  Info->L3Ways     = 4;  Info->L3Lines     = 64;  break;
            case 0x8D: Info->L3Size     = 3096;  Info->L3Ways     = 12; Info->L3Lines     = 128; break;
            case 0xD0: Info->L3Size     = 512;   Info->L3Ways     = 4;  Info->L3Lines     = 64;  break;
            case 0xD1: Info->L3Size     = 1024;  Info->L3Ways     = 4;  Info->L3Lines     = 64;  break;
            case 0xD2: Info->L3Size     = 2048;  Info->L3Ways     = 4;  Info->L3Lines     = 64;  break;
            case 0xD6: Info->L3Size     = 1024;  Info->L3Ways     = 8;  Info->L3Lines     = 64;  break;
            case 0xD7: Info->L3Size     = 2048;  Info->L3Ways     = 8;  Info->L3Lines     = 64;  break;
            case 0xD8: Info->L3Size     = 4096;  Info->L3Ways     = 8;  Info->L3Lines     = 64;  break;
            case 0xDC: Info->L3Size     = 1536;  Info->L3Ways     = 12; Info->L3Lines     = 64;  break;
            case 0xDD: Info->L3Size     = 3072;  Info->L3Ways     = 12; Info->L3Lines     = 64;  break;
            case 0xDE: Info->L3Size     = 6144;  Info->L3Ways     = 12; Info->L3Lines     = 64;  break;
            case 0xE2: Info->L3Size     = 2048;  Info->L3Ways     = 16; Info->L3Lines     = 64;  break;
            case 0xE3: Info->L3Size     = 4096;  Info->L3Ways     = 16; Info->L3Lines     = 64;  break;
            case 0xE4: Info->L3Size     = 8192;  Info->L3Ways     = 16; Info->L3Lines     = 64;  break;
            case 0xEA: Info->L3Size     = 12288; Info->L3Ways     = 24; Info->L3Lines     = 64;  break;
            case 0xEB: Info->L3Size     = 18432; Info->L3Ways     = 24; Info->L3Lines     = 64;  break;
            case 0xEC: Info->L3Size     = 24576; Info->L3Ways     = 24; Info->L3Lines     = 64;  break;

            default:
                if (Values[i] != 0)
                    DebugTrace(L"Unknown value! Values[%d] = 0x%X", i, Values[i]);
                break;
        }
    }

    if (!Info->L1DataSize && !Info->L1InstSize && !Info->L2Size && !Info->L3Size)
    {
        DWORD Ways, Partitions, LineSize, Sets;
        UINT Index, Type, Level, CacheSize;

        DebugTrace(L"Use CPUID Function 04h to query cache params");

        ZeroMemory(CPUInfo, sizeof(CPUInfo));

        for (Index = 0; Index < 4; Index++)
        {
            __cpuidex(CPUInfo, 4, Index);

            Ways       = GetBitsDWORD(CPUInfo[1], 22, 31) + 1;
            Partitions = GetBitsDWORD(CPUInfo[1], 12, 21) + 1;
            LineSize   = GetBitsDWORD(CPUInfo[1], 0, 11) + 1;
            Sets       = CPUInfo[2] + 1;
            CacheSize  = (Ways * Partitions * LineSize * Sets) / 1024;
            Level      = GetBitsDWORD(CPUInfo[0], 5, 7);

            DebugTrace(L"Ways = %u, Partitions = %u, LineSize = %u, Sets = %u, CacheSize = %u, Level = %u",
                       Ways, Partitions, LineSize, Sets, CacheSize, Level);

            switch (Level)
            {
                case 1:
                {
                    Type = GetBitsDWORD(CPUInfo[0], 0, 4);

                    if (Type == 1)
                    {
                        Info->L1DataSize  = CacheSize;
                        Info->L1DataWays  = Ways;
                        Info->L1DataLines = LineSize;
                    }
                    else if (Type == 2)
                    {
                        Info->L1InstSize  = CacheSize;
                        Info->L1InstWays  = Ways;
                        Info->L1InstLines = LineSize;
                    }
                }
                break;

                case 2:
                    Info->L2Size  = CacheSize;
                    Info->L2Ways  = Ways;
                    Info->L2Lines = LineSize;
                    break;

                case 3:
                    Info->L3Size  = CacheSize;
                    Info->L3Ways  = Ways;
                    Info->L3Lines = LineSize;
                    break;
            }
        }
    }
}

typedef struct
{
    WCHAR szMicroarch[MAX_STR_LEN];
    WCHAR szPackage[MAX_STR_LEN];
    UINT Technology;
} INTEL_CPU_INFO;

VOID
GetCpuInfoIntel(INTEL_CACHE_INFO CacheInfo, INTEL_CPU_INFO *CpuInfo)
{
    BYTE PlatformId, Family, Model, Stepping, ExtFamily,
         ExtModel, BrandId = 0;
    WCHAR *pMicroarch = L"", *pPackage = L"";
    WCHAR szCpuName[MAX_STR_LEN];
    INT LogicalCpuCount, PhysicalCpuCount;
    UINT Technology = 0;
    INT CPUInfo[4] = {-1};

    __cpuid(CPUInfo, 1);

    Family = (CPUInfo[0] >> 8) & 0xf;
    Model = (CPUInfo[0] >> 4) & 0xf;
    Stepping = CPUInfo[0] & 0xf;

    ExtFamily = (CPUInfo[0] >> 20) & 0xff;
    ExtModel = (CPUInfo[0] >> 16) & 0xf;

    BrandId = (CPUInfo[1] & 0xff);

    PlatformId = (CPUInfo[0] >> 4) & 0x3;

    GetCPUName(szCpuName, sizeof(szCpuName));

    PhysicalCpuCount = GetPhysicalProcessorsCount();
    LogicalCpuCount = GetLogicalProcessorsCount();

    DebugTrace(L"Family = 0x%02X, Model = 0x%02X, Stepping = 0x%02X",
               Family, Model, Stepping);
    DebugTrace(L"ExtFamily = 0x%02X, ExtModel = 0x%02X",
               ExtFamily, ExtModel);
    DebugTrace(L"BrandId = 0x%02X, PlatformId = %d, szCpuName = %s",
               BrandId, PlatformId, szCpuName);
    DebugTrace(L"PhysicalCpuCount = %d, LogicalCpuCount = %d",
               PhysicalCpuCount, LogicalCpuCount);

    switch (Family)
    {
        case 0x06:
        {
            if ((ExtModel * 16 + Model) >= 0x10)
            {
                switch (ExtModel * 16 + Model)
                {
                    case 0x16:
                    {
                        Technology = 65;

                        if (PhysicalCpuCount == 4)
                        {
                            if (wcsstr(szCpuName, L"Xeon"))
                            {
                                pMicroarch = L"Clovertown";
                                pPackage = L"LGA771";
                            }
                            else
                            {
                                pMicroarch = L"Kentsfield";
                                pPackage = L"LGA775";
                            }
                        }
                        else if (wcsstr(szCpuName, L"Xeon") && wcsstr(szCpuName, L" 30"))
                        {
                            pMicroarch = L"Conroe";
                            pPackage = L"LGA775";
                        }
                        else if (PlatformId == 2 || LogicalCpuCount == 4 || wcsstr(szCpuName, L"Xeon"))
                        {
                            pMicroarch = L"Woodcrest";
                            pPackage = L"LGA771";
                        }
                        else if (wcsstr(szCpuName, L"2.93"))
                        {
                            pMicroarch = L"Conroe";
                            pPackage = L"LGA775";
                        }
                        else if (PhysicalCpuCount == 2)
                        {
                            if (PlatformId == 5)
                            {
                                pMicroarch = L"Merom";
                                pPackage = L"Socket 479";
                            }
                            else
                            {
                                pPackage = L"LGA775";

                                if (CacheInfo.L2Size <= 1024)
                                {
                                    pMicroarch = L"Conroe-1M";
                                }
                                else if (CacheInfo.L2Size <= 2048)
                                {
                                    pMicroarch = L"Allendale";
                                }
                                else
                                {
                                    pMicroarch = L"Conroe";
                                }
                            }
                        }
                        else
                        {
                            if (PlatformId == 5)
                            {
                                pMicroarch = L"Merom-L";
                                pPackage = L"Socket 479";
                            }
                            else
                            {
                                pMicroarch = L"Conroe-L";
                                pPackage = L"LGA775";
                            }
                        }
                    }
                    break;

                    case 0x17:
                    {
                        Technology = 45;

                        if (PhysicalCpuCount == 4)
                        {
                            if (wcsstr(szCpuName, L"Xeon") || PlatformId == 6)
                            {
                                pMicroarch = L"Harpertown";
                                pPackage = L"LGA771";
                            }
                            else
                            {
                                pMicroarch = L"Yorkfield";
                                pPackage = L"LGA775";
                            }
                        }
                        else if (PlatformId == 7)
                        {
                            pMicroarch = L"Penryn";
                            pPackage = L"Socket P";
                        }
                        else
                        {
                            pMicroarch = L"Wolfdale";
                            pPackage = L"LGA775";
                        }
                    }
                    break;

                    case 0x1A: /* Intel Core i7 LGA1366 (45nm) */
                    case 0x1E: /* Intel Core i5, i7 LGA1156 (45nm) */
                    case 0x1F: /* Intel Core i5, i7  */
                    {
                        Technology = 45;

                        pMicroarch = L"Nehalem";
                        pPackage = L"LGA1366 or LGA1156";
                    }
                    break;

                    case 0x25: /* Intel Core i3, i5, i7 LGA1156 (32nm) */
                    case 0x2C: /* Intel Core i7 LGA1366 (32nm) 6 Core */
                    case 0x2E: /* Intel Xeon Processor 7500 series */
                    {
                        Technology = 32;

                        pMicroarch = L"Nehalem";
                        pPackage = L"LGA1366 or LGA1156";
                    }
                    break;

                    case 0x1C:
                        Technology = 45;
                        break;

                    case 0x2A:
                    case 0x2D:
                    {
                        Technology = 32;

                        if (PlatformId == 2)
                        {
                            pMicroarch = L"SandyBridge";
                            pPackage = L"LGA1155";
                        }
                    }
                    break;
                }
            }
            else
            {
                switch (Model)
                {
                    case 0x05:
                    {
                        Technology = 250;

                        if (CacheInfo.L2Size > 512)
                        {
                            pMicroarch = L"Deschutes";
                            pPackage = L"Slot 2";
                        }
                        else if (CacheInfo.L2Size == 512)
                        {
                            pMicroarch = L"Deschutes";
                            pPackage = L"Slot 1";
                        }
                        else
                        {
                            pMicroarch = L"Covinton";
                            pPackage = L"Slot 1";
                        }
                    }
                    break;

                    case 0x06:
                    {
                        Technology = 250;

                        if ((Stepping == 0x0A || Stepping == 0x0D) && CacheInfo.L2Size == 256)
                        {
                            pMicroarch = L"Dixon";
                            pPackage = L"Mobile Module";
                        }
                        else if ((Stepping == 0x0A || Stepping == 0x0D) && CacheInfo.L2Size == 128)
                        {
                            pMicroarch = L"Dixon-128K";
                            pPackage = L"Mobile Module";
                        }
                        else
                        {
                            pMicroarch = L"Mendocino";
                            pPackage = L"Socket 370";
                        }
                    }
                    break;

                    case 0x07:
                    {
                        Technology = 250;

                        if (CacheInfo.L2Size == 1024)
                        {
                            pMicroarch = L"Tanner";
                            pPackage = L"Slot 2";
                        }
                        else
                        {
                            pMicroarch = L"Katmai";
                            pPackage = L"Slot 1";
                        }
                    }
                    break;

                    case 0x08:
                    {
                        Technology = 180;

                        if (BrandId == 0x03)
                        {
                            pMicroarch = L"Cascades";
                            pPackage = L"Slot 2";
                        }
                        else if (CacheInfo.L2Size == 256 && PlatformId % 2)
                        {
                            pMicroarch = L"Coppermine";
                            pPackage = L"Mobile Module";
                        }
                        else if (CacheInfo.L2Size == 256 && PlatformId == 0)
                        {
                            pMicroarch = L"Coppermine";
                            pPackage = L"Slot 1";
                        }
                        else if (CacheInfo.L2Size == 256)
                        {
                            pMicroarch = L"Coppermine";
                            pPackage = L"Socket 370";
                        }
                        else if (CacheInfo.L2Size <= 128 && PlatformId % 2)
                        {
                            pMicroarch = L"Coppermine-128K";
                            pPackage = L"Mobile Module";
                        }
                        else if (CacheInfo.L2Size <= 128)
                        {
                            pMicroarch = L"Coppermine-128K";
                            pPackage = L"Socket 370";
                        }
                    }
                    break;

                    case 0x09:
                    {
                        Technology = 130;
                        pPackage = L"Socket 479";

                        if (CacheInfo.L2Size == 1024)
                            pMicroarch = L"Banias";
                        else
                            pMicroarch = L"Banias-512K";
                    }
                    break;

                    case 0x0A:
                        Technology = 180;
                        pMicroarch = L"Cascades";
                        break;

                    case 0x0B:
                    {
                        Technology = 130;

                        if (BrandId == 0x07 || BrandId == 0x06)
                        {
                            pMicroarch = L"Tualatin";
                            pPackage = L"Mobile Module";
                        }
                        else
                        {
                            pMicroarch = L"Tualatin";
                            pPackage = L"Socket 370";
                        }
                    }
                    break;

                    case 0x0C:
                    case 0x0D:
                    {
                        Technology = 90;

                        pPackage = L"Socket 479";

                        if (CacheInfo.L2Size == 1024)
                            pMicroarch = L"Dothan-1024K";
                        else
                            pMicroarch = L"Dothan";
                    }
                    break;

                    case 0x0E:
                    {
                        Technology = 65;

                        pPackage = L"Socket 479";

                        if (LogicalCpuCount == 4 || wcsstr(szCpuName, L"Xeon"))
                            pMicroarch = L"Sossaman";
                        else if (CacheInfo.L2Size == 1024)
                            pMicroarch = L"Yonah";
                        else if (PhysicalCpuCount == 2)
                            pMicroarch = L"Yonah DC";
                        else
                            pMicroarch = L"Yonah SC";
                    }
                    break;

                    case 0x0F:
                    {
                        Technology = 65;

                        if (PhysicalCpuCount == 4)
                        {
                            if (wcsstr(szCpuName, L"Xeon"))
                            {
                                pMicroarch = L"Clovertown";
                                pPackage = L"LGA771";
                            }
                            else
                            {
                                pMicroarch = L"Kentsfield";
                                pPackage = L"LGA775";
                            }
                        }
                        else if (wcsstr(szCpuName, L"Xeon") && wcsstr(szCpuName, L" 30"))
                        {
                            pMicroarch = L"Conroe";
                            pPackage = L"LGA775";
                        }
                        else if (PlatformId == 2 || LogicalCpuCount == 4 || wcsstr(szCpuName, L"Xeon"))
                        {
                            pMicroarch = L"Woodcrest";
                            pPackage = L"LGA771";
                        }
                        else if (wcsstr(szCpuName, L"2.93"))
                        {
                            pMicroarch = L"Conroe";
                            pPackage = L"LGA775";
                        }
                        else if (PhysicalCpuCount == 2)
                        {
                            if (PlatformId == 5 || PlatformId == 7)
                            {
                                pMicroarch = L"Merom";
                                pPackage = L"Socket 479";
                            }
                            else
                            {
                                pPackage = L"LGA775";

                                if (CacheInfo.L2Size <= 1024)
                                    pMicroarch = L"Conroe-1M";
                                else if (CacheInfo.L2Size <= 2048)
                                    pMicroarch = L"Allendale";
                                else
                                    pMicroarch = L"Conroe";
                            }
                        }
                        else
                        {
                            if (PlatformId == 5 || PlatformId == 7)
                            {
                                pMicroarch = L"Merom-L";
                                pPackage = L"Socket 479";
                            }
                            else
                            {
                                pMicroarch = L"Conroe-L";
                                pPackage = L"LGA775";
                            }
                        }
                    }
                    break;
                }
            }
        }
        break;

        case 0x07:
            pMicroarch = L"Merced";
            break;

        case 0x0F:
        {
            switch (Model)
            {
                case 0x00:
                case 0x01:
                {
                    Technology = 180;

                    if ((BrandId == 0x0B && Model <= 1 && Stepping < 3) || BrandId == 0x0C)
                    {
                        pMicroarch = L"Foster MP";
                        pPackage = L"Socket 603";
                    }
                    else if (BrandId == 0x0E || BrandId == 0x0B)
                    {
                        pMicroarch = L"Foster";
                        pPackage = L"Socket 603";
                    }
                    else if (CacheInfo.L2Size == 128)
                    {
                        pMicroarch = L"Willamette-128K";
                        pPackage = L"Socket 478";
                    }
                    else
                    {
                        if (Model == 1)
                        {
                            pMicroarch = L"Willamette";
                            pPackage = L"Socket 478";
                        }
                        else
                        {
                            pMicroarch = L"Willamette";
                            pPackage = L"Socket 423";
                        }
                    }
                }
                break;

                case 0x02:
                {
                    Technology = 130;

                    if (PlatformId == 2 || PlatformId == 3)
                    {
                        pPackage = L"Socket 478";
                    }
                    else if (PlatformId == 4)
                    {
                        pPackage = L"LGA775";
                    }
                    else
                    {
                        pPackage = L"Socket 478 or LGA775";
                    }

                    if (BrandId == 0x0C)
                    {
                        pMicroarch = L"Gallatin";
                        pPackage = L"Socket 603/604";
                    }
                    else if (BrandId == 0x0B)
                    {
                        pMicroarch = L"Prestonia";
                        pPackage = L"Socket 603/604";
                    }
                    else if (BrandId == 0x09 && CacheInfo.L3Size == 2048)
                    {
                        pMicroarch = L"Gallatin";
                    }
                    else if (CacheInfo.L2Size == 256)
                    {
                        pMicroarch = L"Northwood-256K";
                        pPackage = L"Socket 478";
                    }
                    else if (CacheInfo.L2Size == 128 && wcsstr(szCpuName, L"Pentium"))
                    {
                        pMicroarch = L"Northwood-128K";
                    }
                    else if (CacheInfo.L2Size == 128)
                    {
                        pMicroarch = L"Northwood-128K";
                    }
                    else
                    {
                        pMicroarch = L"Northwood";
                    }
                }
                break;

                case 0x03:
                case 0x04:
                {
                    Technology = 90;

                    if (PlatformId == 2 || PlatformId == 3)
                    {
                        pPackage = L"Socket 478";
                    }
                    else if (PlatformId == 4)
                    {
                        pPackage = L"LGA775";
                    }
                    else
                    {
                        pPackage = L"Socket 478 or LGA775";
                    }

                    if (wcsstr(szCpuName, L"Pentium") && CacheInfo.L2Size >= 1024 && BrandId != 0xC &&
                        (PlatformId == 0 || BrandId == 0x0B || wcsstr(szCpuName, L"Xeon")))
                    {
                        pPackage = L"Socket 604";

                        if (PhysicalCpuCount == 2)
                            pMicroarch = L"Paxville DP";
                        else if (CacheInfo.L2Size == 2048)
                            pMicroarch = L"Irwindale";
                        else
                            pMicroarch = L"Nocona";
                    }
                    else if (BrandId == 0x0C || wcsstr(szCpuName, L"Xeon"))
                    {
                        pPackage = L"Socket 604";

                        if (PhysicalCpuCount == 2)
                            pMicroarch = L"Paxville";
                        else if (CacheInfo.L3Size == 0)
                            pMicroarch = L"Cranford";
                        else
                            pMicroarch = L"Potomac";
                    }
                    else if (PhysicalCpuCount == 2)
                    {
                        pMicroarch = L"Smithfield";
                    }
                    else if (CacheInfo.L2Size == 2048)
                    {
                        pMicroarch = L"Prescott-2M";
                    }
                    else if (wcsstr(szCpuName, L"Celeron") && CacheInfo.L2Size == 256)
                    {
                        pMicroarch = L"Prescott-256K";
                    }
                    else if (wcsstr(szCpuName, L"Mobile Celeron") && CacheInfo.L2Size == 512)
                    {
                        pMicroarch = L"Prescott-512K";
                    }
                    else if (wcsstr(szCpuName, L"Celeron") && CacheInfo.L2Size == 128)
                    {
                        pMicroarch = L"Prescott-128K";
                    }
                    else
                    {
                        pMicroarch = L"Prescott";
                    }
                }
                break;

                case 0x05:
                {
                    Technology = 90;
                    pPackage = L"LGA775";
                    pMicroarch = L"Tejas";
                }
                break;

                case 0x06:
                {
                    Technology = 65;

                    if (wcsstr(szCpuName, L"Xeon"))
                    {
                        pPackage = L"LGA771";
                        pMicroarch = L"Dempsey";
                    }
                    else
                    {
                        pPackage = L"LGA775";

                        if (PhysicalCpuCount == 2)
                        {
                            pMicroarch = L"Presler";
                        }
                        else if (CacheInfo.L2Size <= 512)
                        {
                            pMicroarch = L"Cedar Mill-V";
                        }
                        else
                        {
                            pMicroarch = L"Cedar Mill";
                        }
                    }
                }
                break;
            }
        }
        break;
    }

    wcscpy(CpuInfo->szMicroarch, pMicroarch);
    wcscpy(CpuInfo->szPackage, pPackage);
    CpuInfo->Technology  = Technology;
}

VOID
CPUIDInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    DWORD dwECX, dwEDX;
    INT CPUInfo[4] = {-1};
    CPU_IDS CpuIds = {0};

    IoAddIcon(IDI_CPU);
    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);
    IoAddIcon(IDI_TEMPERATURE);

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

    /* Family / Model / Stepping */
    IoAddValueName(1, 0, IDS_CPUID_MODEL);
    IoSetItemText(L"%u (%Xh) / %u (%Xh) / %u (%Xh)",
                  CpuIds.Family, CpuIds.Family,
                  CpuIds.Model, CpuIds.Model,
                  CpuIds.Stepping, CpuIds.Stepping);

    /* Cores count */
    IoAddValueName(1, 0, IDS_CPUID_CPU_COUNT);
    IoSetItemText(L"%d / %d",
                  GetPhysicalProcessorsCount(),
                  GetLogicalProcessorsCount());

    GetCPUVendor(szText, sizeof(szText));
    if (wcscmp(szText, L"GenuineIntel") == 0)
    {
        DOUBLE Multiplier = GetCpuMultiplier();
        DOUBLE BusSpeed = GetCpuBusSpeed(0);
        LONG MicrocodeRev = GetMicrocodeRevision();
        BYTE PlatformId = GetPlatformId();
        INTEL_CACHE_INFO CacheInfo = {0};
        INTEL_CPU_INFO CpuInfo = {0};

        GetIntelCpuCacheInfo(&CacheInfo);
        GetCpuInfoIntel(CacheInfo, &CpuInfo);

        /* Package */
        if (CpuInfo.szPackage[0] != 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_SOCKET);
            IoSetItemText(L"%s", CpuInfo.szPackage);
        }

        /* Technology */
        if (CpuInfo.Technology != 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_TECHNOLOGY);
            IoSetItemText(L"%u nm", CpuInfo.Technology);
        }

        /* Microarchitecture */
        if (CpuInfo.szMicroarch[0] != 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_MICROARCH);
            IoSetItemText(L"%s", CpuInfo.szMicroarch);
        }

        /* Platform Id */
        if (PlatformId > 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_PLATFORM_ID);
            IoSetItemText(L"%02Xh", PlatformId);
        }

        /* Microcode Update Revision */
        if (MicrocodeRev > 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_MICROCODE_REV);
            IoSetItemText(L"%X", MicrocodeRev);
        }

        /* Multiplier */
        if (Multiplier > 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_MULTIPLIER);
            IoSetItemText(L"%.1fx", Multiplier);
        }

        /* Bus Speed */
        if (BusSpeed > 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_BUS_SPEED);
            IoSetItemText(L"%.2f MHz", BusSpeed);
        }

        /* Speed */
        IoAddValueName(1, 0, IDS_CPUID_SPEED);
        IoSetItemText(L"%.2f MHz", GetCpuSpeed(0));

        /* L1 Instruction Cache */
        if (CacheInfo.L1InstSize > 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_L1_CODE_CACHE);
            IoSetItemText(L"%u kB (%u-ways, %u-byte line size)",
                          CacheInfo.L1InstSize, CacheInfo.L1InstWays, CacheInfo.L1InstLines);
        }

        /* L1 Data Cache */
        if (CacheInfo.L1DataSize > 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_L1_DATA_CACHE);
            IoSetItemText(L"%u kB (%u-ways, %u-byte line size)",
                          CacheInfo.L1DataSize, CacheInfo.L1DataWays, CacheInfo.L1DataLines);
        }

        /* L2 Cache */
        if (CacheInfo.L2Size > 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_L2_CACHE);
            IoSetItemText(L"%u kB (%u-ways, %u-byte line size)",
                          CacheInfo.L2Size, CacheInfo.L2Ways, CacheInfo.L2Lines);
        }

        /* L3 Cache */
        if (CacheInfo.L3Size > 0)
        {
            IoAddValueName(1, 0, IDS_CPUID_L3_CACHE);
            IoSetItemText(L"%u kB (%u-ways, %u-byte line size)",
                          CacheInfo.L3Size, CacheInfo.L3Ways, CacheInfo.L3Lines);
        }

        /* Tjmax */
        IoAddValueName(1, 3, IDS_CPUID_TJMAX);
        IoSetItemText(L"%d °C", GetTjmaxTemperature(0));
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
