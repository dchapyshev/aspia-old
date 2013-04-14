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

/***************************************************************************/
/* DEFINES *****************************************************************/
/***************************************************************************/
#define MAX_STR_LEN        256
#define TOOLBAR_HEIGHT     24
#define SPLIT_WIDTH        3

#ifndef TVM_SETEXTENDEDSTYLE
#define TVM_SETEXTENDEDSTYLE (TV_FIRST + 44)
#endif

#ifndef TVS_EX_DOUBLEBUFFER
#define TVS_EX_DOUBLEBUFFER 0x0004
#endif

/***************************************************************************/
/* STRUCTURES **************************************************************/
/***************************************************************************/
typedef struct
{
    HINSTANCE hInstance;
    HINSTANCE hIconsInstance;
    HANDLE hProcessHeap;
    HWND hMainWindow;
    HWND hToolBar;
    HWND hListView;
    HWND hTreeView;
    HWND hSplitter;
    HIMAGELIST hTreeViewImageList;
    HIMAGELIST hListViewImageList;

    WCHAR szIconsFile[MAX_PATH];

    WORD wLanguage;

    INT SmallIconX;
    INT SmallIconY;
    INT SystemColorDepth;

    BOOL IsDriverInitialized;
    BOOL IsPortable;
    BOOL DebugMode;
} GLOBAL_VARS, *PGLOBAL_VARS;

typedef struct
{
    BOOL IsSaveWindowPosition;

    /* Main window position */
    BOOL IsMainWindowStayOnTop;
    BOOL IsMainWindowMaximized;
    INT MainWindowLeft;
    INT MainWindowTop;
    INT MainWindowRight;
    INT MainWindowBottom;
    INT MainWindowSplitter;

    INT ToolBarIconsSize;

} SETTINGS_INFO, *PSETTINGS_INFO;


/***************************************************************************/
/* VARIABLES ***************************************************************/
/***************************************************************************/

extern GLOBAL_VARS GlobalVars;
extern SETTINGS_INFO SettingsInfo;

/***************************************************************************/
/* FUNCTIONS ***************************************************************/
/***************************************************************************/

/* winmain.c */

/* misc.c */
BOOL IsDebugModeEnabled(VOID);
BOOL IsPortable(VOID);
VOID SetProcessPrivilege(LPCTSTR lpName);
INT StringLoad(UINT ResID, LPWSTR Buffer, INT BufLen);
HMENU MenuLoad(LPCWSTR lpMenuName);
INT_PTR DialogLoad(LPCWSTR lpDialogName, HWND hWndParent, DLGPROC lpDialogFunc);
