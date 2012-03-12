/*
 * PROJECT:         Aspia
 * FILE:            diskbench/diskbench.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include <windows.h>
#include <wchar.h>
#include <strsafe.h>
#include <commctrl.h>
#include <process.h>

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

#define MAX_STR_LEN 256

#define TOOLBAR_HEIGHT 24

#define TEST_TYPE_RAMDOM_READ    1
#define TEST_TYPE_SEQ_READ       2
#define TEST_TYPE_RANDOM_WRITE   3
#define TEST_TYPE_SEQ_WRITE      4

#define TEST_FILE_SIZE_50MB   1
#define TEST_FILE_SIZE_100MB  2
#define TEST_FILE_SIZE_300MB  3
#define TEST_FILE_SIZE_500MB  4
#define TEST_FILE_SIZE_700MB  5
#define TEST_FILE_SIZE_1GB    6
#define TEST_FILE_SIZE_3GB    7
#define TEST_FILE_SIZE_5GB    8
#define TEST_FILE_SIZE_10GB   9

#define TEST_BLOCK_SIZE_4KB      (4    * 1024)
#define TEST_BLOCK_SIZE_8KB      (8    * 1024)
#define TEST_BLOCK_SIZE_16KB     (16   * 1024)
#define TEST_BLOCK_SIZE_32KB     (32   * 1024)
#define TEST_BLOCK_SIZE_64KB     (64   * 1024)
#define TEST_BLOCK_SIZE_128KB    (128  * 1024)
#define TEST_BLOCK_SIZE_256KB    (256  * 1024)
#define TEST_BLOCK_SIZE_512KB    (512  * 1024)
#define TEST_BLOCK_SIZE_1MB      (1024 * 1024)


#define Alloc(a) HeapAlloc(GetProcessHeap(), 0, a)
#define Free(a) HeapFree(GetProcessHeap(), 0, a)

VOID DrawDiagram(HWND hwnd, double x, double y);
VOID GraphOnPaint(HWND hwnd);
VOID GraphSetCoordNames(WCHAR *x, WCHAR *y);
VOID GraphSetCoordMaxValues(HWND hwnd, double x, double y);
VOID GraphSetBeginCoord(int x, int y);
VOID GraphClear(HWND hwnd);
