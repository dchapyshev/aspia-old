/*
 * PROJECT:         Aspia (Helper DLL)
 * FILE:            dll/aspia_dll.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include <windows.h>

extern HINSTANCE hInst;

typedef struct
{
    DWORD dwID;
    BOOL bCritical;
    LPWSTR lpszName;
} INFO_STRUCT;

extern INFO_STRUCT SmartAttribList[];

BOOL SMART_IDToText(INFO_STRUCT *InfoStruct, DWORD dwIndex, LPWSTR lpszText, SIZE_T Size);
