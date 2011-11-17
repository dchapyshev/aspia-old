/*
 * PROJECT:         Aspia
 * FILE:            aspia/main.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include "targetver.h"
#include "version.h"

#include "resource.h"
#include "info/smbios.h"

#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
#include <math.h>
#include <time.h>
#include <strsafe.h>
#include <lm.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <powrprof.h>
#include <setupapi.h>
#include <devguid.h>
#include <psapi.h>
#include <locale.h>
#include <process.h>


#define MAX_STR_LEN        256
#define TOOLBAR_HEIGHT     24
#define SPLIT_WIDTH        3
#define MAX_SENSORS_REFRESH_RATE 20864

#ifndef TVM_SETEXTENDEDSTYLE
#define TVM_SETEXTENDEDSTYLE (TV_FIRST + 44)
#endif

#ifndef TVS_EX_DOUBLEBUFFER
#define TVS_EX_DOUBLEBUFFER 0x0004
#endif

#define WM_NOTIFYICONMSG_SENSORS (WM_USER + 286)
#define WM_NOTIFYICONMSG_WINDOW  (WM_USER + 287)

#define Alloc(a) HeapAlloc(hProcessHeap, 0, a)
#define Free(a) HeapFree(hProcessHeap, 0, a)
#define ReAlloc(a, b) HeapReAlloc(hProcessHeap, 0, a, b)

/* winmain.c */
typedef struct
{
    BOOL DebugMode;

    INT SxSmIcon;
    INT SySmIcon;
    INT SysColorDepth;

    WCHAR szCurrentPath[MAX_PATH];
} PARAMS_STRUCT, *PPARAMS_STRUCT;

extern HINSTANCE hInstance;
extern HINSTANCE hIconsInst;
extern HINSTANCE hLangInst;
extern HWND hMainWnd;
extern UINT CurrentCategory;
extern HIMAGELIST hListViewImageList;
extern HANDLE hProcessHeap;
extern CRITICAL_SECTION CriticalSection;
extern BOOL IsLoadingDone;
extern BOOL IsCanceled;
extern PARAMS_STRUCT ParamsInfo;

VOID GUIInfoThread(LPVOID lpParameter);
BOOL IsRootCategory(UINT Category, PVOID List);
VOID LoadLanguage(VOID);
VOID LoadIcons(VOID);

/* controls.c */
typedef struct _COLUMN_LIST
{
    UINT StringID;
    INT Width;
} COLUMN_LIST;

extern HWND hToolBar;
extern HWND hTreeView;
extern HWND hListView;
extern HWND hSplitter;

extern UINT ColumnCount;

extern HIMAGELIST hImageTreeView;

INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
VOID MainWndOnSize(LPARAM);
VOID ToolBarOnGetDispInfo(LPTOOLTIPTEXT);
VOID InitControls(HWND);
INT AddImageToImageList(HIMAGELIST, UINT);
VOID ReInitControls(VOID);

__inline VOID
AddColumn(HWND hList, SIZE_T Index, INT Width, LPWSTR lpszText)
{
    LV_COLUMN Column = {0};

    Column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    Column.iSubItem = Index;
    Column.pszText = lpszText;
    Column.cx  = Width;
    Column.fmt = LVCFMT_LEFT;

    ListView_InsertColumn(hList, Index, &Column);
}

__inline INT
AddItem(HWND hList, INT IconIndex, LPWSTR lpText, LPARAM lParam)
{
    LV_ITEM Item = {0};

    Item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
    Item.pszText = lpText;
    Item.iItem = ListView_GetItemCount(hList);
    Item.iImage = IconIndex;
    Item.lParam = lParam;

    return ListView_InsertItem(hList, &Item);
}

/* TreeView */
HTREEITEM AddCategory(HWND, HIMAGELIST, HTREEITEM, UINT, UINT);
VOID InitColumnsList(COLUMN_LIST*);

/* ListView */
VOID ListViewDelAllColumns(VOID);
VOID ListViewClear(VOID);
PVOID ListViewGetlParam(HWND, INT);

/* categories.c */
typedef VOID (*PINFOFUNC)(VOID);

typedef struct _CATEGORY_LIST
{
    UINT StringID;
    UINT IconID;
    BOOL Expanded;
    BOOL Checked;
    HTREEITEM hTreeItem;
    PINFOFUNC InfoFunc;
    struct _COLUMN_LIST *ColumnList;
    struct _CATEGORY_LIST *Child;
} CATEGORY_LIST;

extern CATEGORY_LIST RootCategoryList[];

/* battery.c */
BOOL GetBatteryInformation(VOID);

/* cpuid.c */
typedef struct
{
    DWORD Family;
    DWORD Model;
    DWORD Stepping;
}
CPU_IDS, *PCPU_IDS;

VOID CPUIDInfo(VOID);
VOID GetProcessorIDs(PCPU_IDS Id);
VOID GetCPUVendor(LPWSTR, SIZE_T);
INT GetLogicalProcessorsCount(VOID);
BOOL GetCPUName(LPWSTR, SIZE_T);
BOOL Is64BitCpu(VOID);

/* devices.c */
VOID HW_DevicesInfo(VOID);
VOID HW_UnknownDevicesInfo(VOID);
BOOL IsPCIDevice(LPWSTR lpszDevice);
BOOL GetPCIVendorID(LPWSTR lpszDevice, LPWSTR lpszVendorID, SIZE_T Size);
BOOL GetPCIDeviceID(LPWSTR lpszDevice, LPWSTR lpszDeviceID, SIZE_T Size);
BOOL IsUSBDevice(LPWSTR lpszDevice);
BOOL GetUSBVendorID(LPWSTR lpszDevice, LPWSTR lpszVendorID, SIZE_T Size);
BOOL GetUSBDeviceID(LPWSTR lpszDevice, LPWSTR lpszDeviceID, SIZE_T Size);
BOOL IsMonitorDevice(LPWSTR lpszDevice);
BOOL GetMonitorID(LPWSTR lpszDevice, LPWSTR lpszID, SIZE_T Size);

/* os.c */
VOID OS_EnvironInfo(VOID);
VOID OS_RegInformation(VOID);
VOID OS_UsersInfo(VOID);
VOID OS_UsersGroupsInfo(VOID);
VOID OS_ActiveUsersInfo(VOID);
VOID OS_FontsInfo(VOID);
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

/* report.c */
VOID CreateReportWindow(VOID);
BOOL CreateReport(LPWSTR);
VOID CloseReport(VOID);
VOID ReportBeginColumn(VOID);
VOID ReportEndColumn(VOID);
VOID ReportEndValues(VOID);
VOID WriteReportItemString(LPWSTR, BOOL);
VOID WriteReportValueString(LPWSTR);
VOID WriteReportColumnString(LPWSTR);
VOID ReportSaveAll(BOOL, LPWSTR, BOOL);
VOID ReportSavePage(LPWSTR, UINT);
BOOL ReportSaveFileDialog(HWND, LPWSTR, SIZE_T);

/* services.c */
VOID SOFTWARE_ServicesInfo(VOID);
VOID SOFTWARE_DriversInfo(VOID);

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
VOID DMI_BatteryInfo(VOID);

/* eventlog.c */
VOID OS_SecurityEventsInfo(VOID);
VOID OS_SystemEventsInfo(VOID);
VOID OS_AppEventsInfo(VOID);

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
VOID HW_PowerInfo(VOID);
VOID HW_PrintersInfo(VOID);
VOID HW_SPDInfo(VOID);

/* netstat.c */
VOID NETWORK_NetStatInfo(VOID);

/* software.c */
VOID SOFTWARE_InstalledUpdInfo(VOID);
VOID SOFTWARE_InstalledAppInfo(VOID);
VOID SOFTWARE_TaskMgr(VOID);
VOID SOFTWARE_LicensesInfo(VOID);
VOID SOFTWARE_FileTypesInfo(VOID);

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
VOID ShowIEShortInfo(INT IconIndex);
BOOL GetAdapterFriendlyName(LPWSTR lpszKey, LPWSTR lpszName, INT NameLen);

/* tests.c */
VOID TESTS_MemTest(VOID);
VOID TESTS_HDDTest(VOID);
VOID TESTS_CPUTest(VOID);
VOID TESTS_StressTest(VOID);

/* summary.c */
VOID ShowSummaryInfo(VOID);

/* misc.c */
BOOL IsUserAdmin(VOID);
SIZE_T StrToHex(LPWSTR, SIZE_T);
BOOL GetStringFromRegistry(BOOL, HKEY, LPWSTR, LPWSTR, LPWSTR, DWORD);
BOOL GetBinaryFromRegistry(HKEY, LPWSTR, LPWSTR, LPBYTE, DWORD);
HICON GetFolderAssocIcon(LPWSTR);
INT GetSystemColorDepth(VOID);
BOOL GetFileDescription(LPWSTR, LPWSTR, SIZE_T);
INT GetDesktopColorDepth(VOID);
VOID CopyTextToClipboard(LPCWSTR);
BOOL IsWin64System(VOID);
VOID ConvertSecondsToString(LONGLONG, LPWSTR, SIZE_T);
VOID ChopSpaces(LPWSTR, SIZE_T);
BOOL TimeToString(time_t, LPWSTR, SIZE_T);
BOOL GetMSProductKey(BOOL, LPSTR, LPWSTR, INT);
VOID ChangeByteOrder(PCHAR, USHORT);
INT LoadMUIString(UINT, LPWSTR, INT);
BOOL ExtractDLLFromResources(UINT, LPWSTR, LPWSTR, SIZE_T);
BOOL GetCurrentPath(LPWSTR, SIZE_T);
INT SafeStrLen(LPCWSTR lpString);
LPWSTR SafeStrCpyN(LPWSTR lpString1, LPCWSTR lpString2, INT iMaxLength);
INT SafeStrCmp(LPCWSTR lpString1, LPCWSTR lpString2);
VOID IntSetWindowTheme(HWND hwnd);
double Round(double Argument, int Precision);
BOOL CenterWindow(HWND hWnd, HWND hWndCenter);


__inline INT
GetClientWindowWidth(IN HWND hwnd)
{
    RECT Rect;

    GetClientRect(hwnd, &Rect);
    return (Rect.right - Rect.left);
}

__inline INT
GetClientWindowHeight(IN HWND hwnd)
{
    RECT Rect;

    GetClientRect(hwnd, &Rect);
    return (Rect.bottom - Rect.top);
}

__inline INT
GetWindowWidth(IN HWND hwnd)
{
    RECT Rect;

    GetWindowRect(hwnd, &Rect);
    return (Rect.right - Rect.left);
}

__inline INT
GetWindowHeight(IN HWND hwnd)
{
    RECT Rect;

    GetWindowRect(hwnd, &Rect);
    return (Rect.bottom - Rect.top);
}

/* io.c */

#define IO_TARGET_LISTVIEW 0
#define IO_TARGET_HTML     1
#define IO_TARGET_CSV      2
#define IO_TARGET_TXT      3
#define IO_TARGET_JSON     4
#define IO_TARGET_INI      5
#define IO_TARGET_RTF      6

VOID IoSetTarget(UINT);
UINT IoGetTarget(VOID);
BOOL IoCreateReport(LPWSTR);
VOID IoCloseReport(VOID);
VOID IoAddHeader(INT, UINT, INT);
INT IoAddHeaderString(INT, LPWSTR, INT);
INT IoAddValueName(INT, UINT, INT);
INT IoAddItem(INT, INT, LPWSTR);
VOID IoAddFooter(VOID);
VOID IoSetItemText(INT, INT, LPWSTR);
VOID IoAddColumnsList(COLUMN_LIST*, LPWSTR lpCategoryName, LPWSTR lpIniPath);
INT IoAddIcon(UINT);
INT IoGetColumnsCount(VOID);
VOID IoSetColumnsCount(INT);
VOID IoWriteTableTitle(LPWSTR, UINT, BOOL);
VOID IoWriteBeginTable(VOID);
VOID IoWriteEndTable(VOID);
VOID IoWriteBeginContentTable(LPWSTR);
VOID IoWriteEndContentTable(VOID);
VOID IoWriteContentTableItem(UINT, LPWSTR, BOOL);
VOID IoWriteContentTableEndRootItem(VOID);
VOID IoRunInfoFunc(UINT, LPVOID);

/* report/listview.c */
INT ListViewAddItem(INT, INT, LPWSTR);
VOID ListViewAddHeader(UINT, INT);

/* settings.c */
typedef struct
{
    /* Save all windows position */
    BOOL SaveWindowPos;

    /* Main Window Position */
    BOOL IsMaximized;
    INT Left;
    INT Top;
    INT Right;
    INT Bottom;
    INT SplitterPos;

    /* Report Window Position */
    BOOL ReportIsMaximized;
    INT ReportLeft;
    INT ReportTop;
    INT ReportRight;
    INT ReportBottom;

    UINT StartupCategory;

    /* Report */
    WCHAR szReportPath[MAX_PATH];
    BOOL IsAddContent;
    UINT ReportFileType;

    /* Report: Content Filtering */
    BOOL ELogShowError;
    BOOL ELogShowWarning;
    BOOL ELogShowInfo;
    BOOL IEShowFile;
    BOOL IEShowHttp;
    BOOL IEShowFtp;

    /* Startup Settings */
    BOOL Autorun;
    BOOL HideToTray;
    BOOL ShowProgIcon;
    BOOL ShowSensorIcons;
    BOOL StayOnTop;
    BOOL AllowKmDriver;

    /* Sensor Colors */
    COLORREF HddBackground;
    COLORREF HddFontColor;
    COLORREF CpuBackground;
    COLORREF CpuFontColor;

    UINT SensorsRefreshRate;

    BOOL SendDevReport;
} SETTINGS_STRUCT, *PSETTINGS_STRUCT;

typedef struct
{
    /* Language */
    WCHAR szLangFile[MAX_PATH];
    WCHAR szIconsFile[MAX_PATH];
} THEMES_STRUCT, *PTHEMES_STRUCT;

typedef struct
{
    UINT LanguageId;
    LPWSTR lpszFileName;
    LPWSTR lpszLanguageName;
} LANGUAGES_LIST, *PLANGUAGES_LIST;

extern LANGUAGES_LIST LanguagesList[];
extern SETTINGS_STRUCT SettingsInfo;
extern THEMES_STRUCT ThemesInfo;

BOOL LoadSettings(VOID);
BOOL SaveSettings(VOID);
INT_PTR CALLBACK SettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL GetIniFilePath(OUT LPWSTR lpszPath, IN SIZE_T PathLen);

/* dev_report.c */
VOID DetectUnknownDevices(VOID);

/* systray.c */
extern BOOL IsMainWindowHiden;

HICON CreateTrayIcon(LPWSTR szText, COLORREF crBackground, COLORREF crFont);
VOID AddTraySensors(VOID);
VOID DeleteTraySensors(VOID);
VOID HideMainWindowToTray(VOID);
VOID RestoreMainWindowFromTray(VOID);
VOID DeleteMainWindowFromTray(VOID);
VOID AddMainWindowToTray(VOID);
VOID UpdateTrayIcons(VOID);

/* lpc.c */
VOID GetLPCSensorsInfo(VOID);

/* sysmon.c */
VOID CreateSysMonWindow(HWND hParentWnd);
