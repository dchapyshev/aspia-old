/*
 * PROJECT:         Aspia
 * FILE:            include/aspia.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

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
#include <wintrust.h>
#include <wincrypt.h>
#include <iphlpapi.h>
#include <cpl.h>
#include <initguid.h>
#include <mstask.h>
#include <ntsecapi.h>
#include <wtsapi32.h>
#include <urlhist.h>
#include <iphlpapi.h>
#include <wininet.h>
#include <ras.h>
#include <raserror.h>
#include <netfw.h>
#include <sddl.h>
#include <intrin.h>
#include <gl/gl.h>

#include "version.h"
#include "io.h"
#include "helper.h"
#include "lang.h"
#include "icons.h"

#define MAX_STR_LEN        256

#define IO_TARGET_UNKNOWN  0
#define IO_TARGET_LISTVIEW 1
#define IO_TARGET_HTML     2
#define IO_TARGET_CSV      3
#define IO_TARGET_TXT      4
#define IO_TARGET_JSON     5
#define IO_TARGET_INI      6
#define IO_TARGET_RTF      7

#define Alloc(a) HeapAlloc(GetProcessHeap(), 0, a)
#define Free(a) HeapFree(GetProcessHeap(), 0, a)
#define ReAlloc(a, b) HeapReAlloc(GetProcessHeap(), 0, a, b)

typedef VOID (*PIO_ADD_FOOTER)(VOID);
typedef VOID (*PIO_ADD_VALUE_NAME)(INT, INT, UINT);
typedef VOID (*PIO_SET_ITEM_TEXT)(LPWSTR, ...);
typedef INT  (*PIO_ADD_ICON)(UINT);
typedef VOID (*PIO_ADD_HEADER)(INT, INT, UINT);
typedef VOID (*PIO_ADD_ITEM)(INT, INT, LPWSTR, ...);
typedef UINT (*PIO_GET_TARGET)(VOID);
typedef VOID (*PIO_ADD_HEADER_STRING)(INT, INT, LPWSTR, ...);
typedef INT (*PIO_GET_CURRENT_ITEM_INDEX)(VOID);

typedef struct
{
    HWND hMainWnd;

    HINSTANCE hLangInst;
    HINSTANCE hIconsInst;

    HWND hListView;
    HIMAGELIST *hListImgList;

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
    PIO_GET_CURRENT_ITEM_INDEX IoGetCurrentItemIndex;

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

/* os.c */
VOID OS_EnvironInfo(VOID);
VOID OS_RegInformation(VOID);
VOID OS_UsersInfo(VOID);
VOID OS_UsersGroupsInfo(VOID);
VOID OS_ActiveUsersInfo(VOID);
VOID OS_FontsInfo(VOID);
VOID OS_CPLAppletsInfo(VOID);
VOID OS_AutorunInfo(VOID);
VOID OS_SysFoldersInfo(VOID);
VOID OS_DesktopInfo(VOID);
VOID OS_TaskSchedulerInfo(VOID);
VOID OS_PreventsInfo(VOID);
VOID ShowInstallDate(VOID);
VOID OS_SysFilesDLLInfo(VOID);
VOID OS_SysFilesAXInfo(VOID);
VOID OS_SysFilesEXEInfo(VOID);
VOID OS_SysFilesSYSInfo(VOID);
VOID OS_SysFilesKnownInfo(VOID);

/* regional.c */
VOID OS_RegionalParamInfo(VOID);

/* dmi.c */
VOID DMI_CPUInfo(VOID);
VOID DMI_RAMInfo(VOID);
VOID DMI_BIOSInfo(VOID);
VOID DMI_SystemInfo(VOID);
VOID DMI_BoardInfo(VOID);
VOID DMI_CacheInfo(VOID);
VOID DMI_SlotInfo(VOID);
VOID DMI_PortsInfo(VOID);
VOID DMI_EnclosureInfo(VOID);
VOID DMI_OnboardInfo(VOID);
VOID DMI_PointingInfo(VOID);
VOID DMI_BatteryInfo(VOID);

/* mmedia.c */
VOID HW_MediaDevicesInfo(VOID);
VOID HW_MediaACodecsInfo(VOID);
VOID HW_MediaVCodecsInfo(VOID);
VOID HW_MediaMCIInfo(VOID);

/* hw.c */
ULONG GetTjmaxTemperature(DWORD CpuIndex);
ULONG GetPciAddress(BYTE bus, BYTE device, BYTE function);
VOID HW_SensorInfo(VOID);
VOID HW_CPUInfo(VOID);
VOID HW_CDInfo(VOID);
VOID HW_HDDSCSIInfo(VOID);
VOID HW_HDDATAInfo(VOID);
VOID HW_HDDSMARTInfo(VOID);
VOID HW_MonitorInfo(VOID);
VOID HW_WinVideoInfo(VOID);
VOID HW_OpenGlInfo(VOID);
VOID HW_PowerInfo(VOID);
VOID HW_PrintersInfo(VOID);
VOID HW_SPDInfo(VOID);

/* devices.c */
VOID HW_DevicesInfo(VOID);
VOID HW_UnknownDevicesInfo(VOID);
VOID HW_DevicesFree(VOID);

/* software.c */
VOID SOFTWARE_InstalledUpdInfo(VOID);
VOID SOFTWARE_InstalledAppInfo(VOID);
VOID SOFTWARE_InstalledAppsFree(VOID);
VOID SOFTWARE_TaskMgr(VOID);
VOID SOFTWARE_LicensesInfo(VOID);
VOID SOFTWARE_FileTypesInfo(VOID);

/* summary.c */
VOID ShowSummaryInfo(VOID);

/* services.c */
VOID SOFTWARE_ServicesInfo(VOID);
VOID SOFTWARE_DriversInfo(VOID);

/* netstat.c */
VOID NETWORK_NetStatInfo(VOID);

/* eventlog.c */
VOID OS_SecurityEventsInfo(VOID);
VOID OS_SystemEventsInfo(VOID);
VOID OS_AppEventsInfo(VOID);

ULONG GetTjmaxTemperature(DWORD CpuIndex);

VOID SetCanceledState(BOOL State);
BOOL GetCanceledState(VOID);
