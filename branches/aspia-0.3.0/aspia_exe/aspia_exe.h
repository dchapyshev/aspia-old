/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/aspia_exe.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include "targetver.h"
#include "version.h"

#include "resource.h"

#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
#include <math.h>
#include <time.h>
#include <strsafe.h>
#include <lm.h>
#include <powrprof.h>
#include <setupapi.h>
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

/* winmain.c */
typedef struct
{
    BOOL DebugMode;

    BOOL IsIoInitialized;
    BOOL IsPortable;

    INT SxSmIcon;
    INT SySmIcon;
    INT SysColorDepth;

    WCHAR szCurrentPath[MAX_PATH];
} PARAMS_STRUCT, *PPARAMS_STRUCT;

/* categories.c */
typedef VOID (*PINFOFUNC)(VOID);
typedef VOID (*PFREEFUNC)(VOID);

typedef struct _CATEGORY_LIST
{
    UINT StringID;
    UINT IconID;
    UINT MenuID;
    BOOL Expanded;
    BOOL Checked;
    BOOL Sorting;
    HTREEITEM hTreeItem;
    PINFOFUNC InfoFunc;
    PFREEFUNC FreeFunc;
    struct _COLUMN_LIST *ColumnList;
    struct _CATEGORY_LIST *Child;
} CATEGORY_LIST;

extern CATEGORY_LIST RootCategoryList[];

extern HINSTANCE hInstance;
extern HINSTANCE hIconsInst;
extern HINSTANCE hLangInst;
extern HWND hMainWnd;
extern UINT CurrentCategory;
extern UINT CurrentMenu;
extern BOOL IsSortingAllowed;
extern PFREEFUNC InfoFreeFunction;
extern HIMAGELIST hListViewImageList;
extern HANDLE hProcessHeap;
extern CRITICAL_SECTION CriticalSection;
extern BOOL IsLoadingDone;
extern PARAMS_STRUCT ParamsInfo;

VOID GUIInfoThread(LPVOID lpParameter);
BOOL IsRootCategory(UINT Category, PVOID List);
BOOL LoadLanguage(VOID);
BOOL LoadIcons(VOID);
BOOL InitInfoDll(VOID);

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
BOOL InitControls(HWND);
VOID ReInitControls(VOID);

__inline VOID
AddColumn(HWND hList, INT Index, INT Width, LPWSTR lpszText)
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
VOID ListViewSetItemParam(INT Index, LPARAM lParam);

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
VOID ReportSave(BOOL, BOOL, LPWSTR, BOOL);
VOID ReportSavePage(LPWSTR, UINT);
BOOL ReportSaveFileDialog(HWND, LPWSTR, SIZE_T);

/* misc.c */
VOID CopyTextToClipboard(LPCWSTR);
VOID IntSetWindowTheme(HWND hwnd);

/* io.c */
VOID IoSetTarget(UINT);
UINT IoGetTarget(VOID);
INT IoGetCurrentItemIndex(VOID);
BOOL IoCreateReport(LPWSTR);
VOID IoCloseReport(VOID);
VOID IoAddHeader(INT, INT, UINT);
VOID IoAddHeaderString(INT, INT, LPWSTR, ...);
VOID IoAddValueName(INT, INT, UINT);
VOID IoAddItem(INT, INT, LPWSTR, ...);
VOID IoAddFooter(VOID);
VOID IoSetItemText(LPWSTR, ...);
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

    /* View Settings */
    BOOL SaveWindowPos;
    BOOL StayOnTop;
    BOOL ShowAltRows;
    BOOL ShowWindowStyles;

    /* Startup Settings */
    BOOL Autorun;
    BOOL HideToTray;
    BOOL ShowProgIcon;
    BOOL ShowSensorIcons;
    BOOL AllowKmDriver;

    /* Sensor Colors */
    COLORREF HddBackground;
    COLORREF HddFontColor;
    COLORREF CpuBackground;
    COLORREF CpuFontColor;

    UINT SensorsRefreshRate;
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
BOOL WritePrivateProfileInt(IN LPCTSTR lpAppName, IN LPCTSTR lpKeyName, IN INT Value, IN LPCTSTR lpFileName);

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

/* sysmon.c */
VOID CreateSysMonWindow(VOID);
