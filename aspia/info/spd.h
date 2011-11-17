/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/spd.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include <windows.h>
#include <wchar.h>
#include <strsafe.h>


typedef struct
{
    DWORD dwID;
    LPWSTR lpszValue;
} SPD_INFO;

extern SPD_INFO ManufacturerInfo[];
extern SPD_INFO RamTypeInfo[];
extern SPD_INFO VoltageInfo[];
extern SPD_INFO RefreshRateInfo[];
extern SPD_INFO ErrorDetectInfo[];

VOID GetSpdModuleType(BYTE *Spd, LPWSTR lpszString, SIZE_T Size);
VOID GetSpdManufacturer(BYTE *Spd, LPWSTR lpszString, SIZE_T Size);
INT GetSpdDensity(BYTE v);
double GetSpdCycleTime(BYTE v);
VOID GetSpdVoltage(BYTE *Spd, LPWSTR lpszString, SIZE_T Size);
VOID GetSpdModuleWidth(BYTE *Spd, LPWSTR lpszString, SIZE_T Size);
VOID GetSpdErrorDetection(BYTE *Spd, LPWSTR lpszString, SIZE_T Size);
VOID GetSpdRefreshRate(BYTE *Spd, LPWSTR lpszString, SIZE_T Size);
double GetSpdTime(BYTE Byte);
VOID ShowSpdDataForDDR(BYTE *Spd);
VOID ShowSpdDataForDDR2(BYTE *Spd);
VOID ShowSpdDataForDDR3(BYTE *Spd);
