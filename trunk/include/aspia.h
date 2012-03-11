#pragma once

#include <windows.h>
#include <wchar.h>
#include <strsafe.h>
#include <commctrl.h>
#include <setupapi.h>
#include <devguid.h>
#include <batclass.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <lm.h>
#include <cfgmgr32.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <shobjidl.h>

#include "version.h"
#include "io.h"
#include "helper.h"
#include "lang.h"
#include "icons.h"

#define MAX_STR_LEN        256

#define IO_TARGET_LISTVIEW 0
#define IO_TARGET_HTML     1
#define IO_TARGET_CSV      2
#define IO_TARGET_TXT      3
#define IO_TARGET_JSON     4
#define IO_TARGET_INI      5
#define IO_TARGET_RTF      6

#define Alloc(a) HeapAlloc(GetProcessHeap(), 0, a)
#define Free(a) HeapFree(GetProcessHeap(), 0, a)
#define ReAlloc(a, b) HeapReAlloc(GetProcessHeap(), 0, a, b)

typedef VOID (*PIO_ADD_FOOTER)(VOID);
typedef INT  (*PIO_ADD_VALUE_NAME)(INT, UINT, INT);
typedef VOID (*PIO_SET_ITEM_TEXT)(INT, INT, LPWSTR);
typedef INT  (*PIO_ADD_ICON)(UINT);
typedef VOID (*PIO_ADD_HEADER)(INT, UINT, INT);
typedef INT  (*PIO_ADD_ITEM)(INT, INT, LPWSTR);
typedef UINT (*PIO_GET_TARGET)(VOID);
typedef INT  (*PIO_ADD_HEADER_STRING)(INT, LPWSTR, INT);

typedef struct
{
    HWND hMainWnd;

    HINSTANCE hLangInst;
    HINSTANCE hIconsInst;

    HWND hListView;
    HIMAGELIST hListImgList;

    WCHAR szCurrentPath[MAX_PATH];

    INT SxSmIcon;
    INT SySmIcon;
    INT SysColorDepth;

    BOOL ELogShowError;
    BOOL ELogShowWarning;
    BOOL ELogShowInfo;

    BOOL IEShowHttp;
    BOOL IEShowFtp;
    BOOL IEShowFile;

    PIO_ADD_FOOTER        IoAddFooter;
    PIO_ADD_VALUE_NAME    IoAddValueName;
    PIO_SET_ITEM_TEXT     IoSetItemText;
    PIO_ADD_ICON          IoAddIcon;
    PIO_ADD_HEADER        IoAddHeader;
    PIO_ADD_ITEM          IoAddItem;
    PIO_GET_TARGET        IoGetTarget;
    PIO_ADD_HEADER_STRING IoAddHeaderString;

} ASPIA_DLL_PARAMS, *PASPIA_DLL_PARAMS;

BOOL AspiaDllInitialize(ASPIA_DLL_PARAMS *Params);

typedef struct
{
    HKEY hRootKey;
    HKEY hAppKey;
    WCHAR szKeyName[MAX_PATH];
} INST_APP_INFO;

typedef struct
{
    DWORD Family;
    DWORD Model;
    DWORD Stepping;
}
CPU_IDS, *PCPU_IDS;

/* cpuid.c */
VOID CPUIDInfo(VOID);
VOID GetProcessorIDs(PCPU_IDS Id);
VOID GetCPUVendor(LPWSTR, SIZE_T);
INT GetLogicalProcessorsCount(VOID);
BOOL GetCPUName(LPWSTR, SIZE_T);
BOOL Is64BitCpu(VOID);

/* network.c */
VOID NETWORK_CardsInfo(VOID);
VOID NETWORK_SharedInfo(VOID);
VOID NETWORK_OpenFilesInfo(VOID);
VOID NETWORK_RouteInfo(VOID);
VOID NETWORK_FirewallInfo(VOID);
VOID NETWORK_IEParamsInfo(VOID);
VOID NETWORK_IEHistoryInfo(VOID);
VOID NETWORK_IECookieInfo(VOID);
VOID NETWORK_RasInfo(VOID);

ULONG GetTjmaxTemperature(DWORD CpuIndex);
