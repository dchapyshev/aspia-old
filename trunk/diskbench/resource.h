/*
 * PROJECT:         Aspia
 * FILE:            diskbench/resource.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include <windows.h>
#include <wchar.h>

/* Icons */
#define IDI_MAIN  100
#define IDI_CLEAR 101
#define IDI_SAVE  102
#define IDI_START 103
#define IDI_STOP  104

#define ID_DISK_START     1000
#define ID_DISK_STOP      1001
#define ID_DISK_SAVE      1002
#define ID_DISK_CLEAR     1003

/* Timers */
#define IDT_SECONDS_COUNTER_TIMER  3476
#define IDT_CPU_USAGE_UPDATE_TIMER 3477

/* Dialogs */
#define IDD_DISK_BENCH_DIALOG    5006

/* Controls */
#define IDC_TEST_TYPE           146
#define IDC_TEST_FILE_SIZE      147
#define IDC_BLOCK_SIZE          148
#define IDC_GRAPH_WND           149
#define IDC_DISK_CURRENT        150
#define IDC_DISK_MINIMUM        151
#define IDC_DISK_MAXIMUM        152
#define IDC_CPU_CURRENT         153
#define IDC_CPU_MINIMUM         154
#define IDC_CPU_MAXIMUM         155
#define IDC_DISK_TIME           156

/* Strings */
#define IDS_ASPIA_DISK_BENCH             739
#define IDS_DISK_SAVE                    740
#define IDS_DISK_CLEAR                   741
#define IDS_DISK_START                   742
#define IDS_DISK_STOP                    743
#define IDS_DISK_RANDOM_READ             744
#define IDS_DISK_SEQ_READ                745
#define IDS_DISK_RANDOM_WRITE            746
#define IDS_DISK_SEQ_WRITE               747
#define IDS_DISK_READONLY                750
#define IDS_DISK_NO_FREE_SPACE           751
#define IDS_DISK_FILECREATE              752
#define IDS_DISK_TESTING                 753
#define IDS_DISK_TEST_COMPLETE           754
