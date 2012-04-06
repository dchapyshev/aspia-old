/*
 * PROJECT:         Aspia (Helper DLL)
 * FILE:            io_dll/io_dll.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include "aspia.h"

extern HINSTANCE hInst;
extern ASPIA_DLL_PARAMS DllParams;
extern BOOL IsCanceled;

extern PIO_ADD_FOOTER        IoAddFooter;
extern PIO_ADD_VALUE_NAME    IoAddValueName;
extern PIO_SET_ITEM_TEXT     IoSetItemText;
extern PIO_ADD_ICON          IoAddIcon;
extern PIO_ADD_HEADER        IoAddHeader;
extern PIO_ADD_ITEM          IoAddItem;
extern PIO_GET_TARGET        IoGetTarget;
extern PIO_ADD_HEADER_STRING IoAddHeaderString;
extern PIO_GET_CURRENT_ITEM_INDEX IoGetCurrentItemIndex;

BOOL GetAdapterFriendlyName(LPWSTR lpszKey, LPWSTR lpszName, INT NameLen);
VOID ShowInstallDate(VOID);
INT GetLogicalProcessorsCount(VOID);
INT GetPhysicalProcessorsCount(VOID);
VOID ShowIEShortInfo(INT IconIndex);
BOOL GetCPUName(LPWSTR lpszCpuName, SIZE_T Size);
VOID GetCPUVendor(LPWSTR lpszCpuVendor, SIZE_T Size);
BOOL Is64BitCpu(VOID);
BOOL SMBIOS_GetMainboardName(LPWSTR lpName, SIZE_T NameSize, LPWSTR lpManuf, SIZE_T ManufSize);
BOOL GetMSProductKey(BOOL is64, LPSTR lpszKeyPath, LPWSTR lpszKey, INT iSize);
BOOL GetBatteryInformation(VOID);
VOID GetLPCSensorsInfo(VOID);

VOID ListViewSetItemParam(HWND hList, INT Index, LPARAM lParam);
PVOID ListViewGetlParam(HWND hList, INT Index);
