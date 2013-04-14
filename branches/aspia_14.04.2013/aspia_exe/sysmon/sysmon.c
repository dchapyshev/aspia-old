/*
 * PROJECT:         Aspia System Monitor
 * FILE:            aspia_exe/sysmon/sysmon.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../aspia_exe.h"
#include "aspia.h"
#include "sysmon.h"


HWND hSysMonWnd = NULL;
HWND hSysMonList = NULL;
HIMAGELIST hSysMonImgList = NULL;

#define IDT_SYSMON_UPDATE_TIMER 4234

INT CALLBACK
SysmonHddDrivesEnumProc(INT ItemIndex, LPWSTR SensorName, INT Current, INT Max, INT Mix);
INT CALLBACK
SysmonIntelCpusEnumProc(INT ItemIndex, LPWSTR CpuName, INT CoreIndex, INT Current, INT Max, INT Min);
INT CALLBACK
SysmonBatteryEnumProc(INT ItemIndex, INT BatteryIndex, DWORD Current, DWORD Max, DWORD Min);

SENSORS_LIST SysmonSensorsList[] =
{
    { TRUE, FALSE, InitHardDrivesSensors, EnumHardDrivesSensors, (PVOID)SysmonHddDrivesEnumProc, FreeHardDrivesSensors },
    { TRUE, FALSE, InitIntelCpuSensors,   EnumIntelCpuSensors,   (PVOID)SysmonIntelCpusEnumProc, FreeIntelCpuSensors   },
    { TRUE, FALSE, InitBatterySensors,    EnumBatterySensors,    (PVOID)SysmonBatteryEnumProc,   FreeBatterySensors    },
    { 0 }
};

VOID
SysMonAddColumn(SIZE_T Index, INT Width, UINT resIndex)
{
    WCHAR szText[MAX_STR_LEN];
    LV_COLUMN Column = {0};

    LoadMUIStringF(hLangInst, resIndex, szText, MAX_STR_LEN);

    Column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    Column.iSubItem = Index;
    Column.pszText = szText;
    Column.cx  = Width;
    Column.fmt = LVCFMT_LEFT;

    ListView_InsertColumn(hSysMonList, Index, &Column);
}

VOID
SysMonInitControls(HWND hwnd)
{
    hSysMonList = CreateWindowEx(WS_EX_CLIENTEDGE,
                                 WC_LISTVIEW,
                                 L"",
                                 WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_TABSTOP | LVS_SHOWSELALWAYS,
                                 205, 28, 465, 350,
                                 hwnd,
                                 NULL,
                                 hInstance,
                                 NULL);

    if (!hSysMonList) return;

    ListView_SetExtendedListViewStyle(hSysMonList,
                                      LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    hSysMonImgList = ImageList_Create(ParamsInfo.SxSmIcon,
                                      ParamsInfo.SySmIcon,
                                      ILC_MASK | ParamsInfo.SysColorDepth,
                                      1, 1);

    AddIconToImageList(hIconsInst, hSysMonImgList, 16, IDI_TEMPERATURE);
    AddIconToImageList(hIconsInst, hSysMonImgList, 16, IDI_VOLTAGE);
    AddIconToImageList(hIconsInst, hSysMonImgList, 16, IDI_FAN);

    ListView_SetImageList(hSysMonList,
                          hSysMonImgList,
                          LVSIL_SMALL);

    SysMonAddColumn(0, 250, IDS_SYSMON_SENSOR);
    SysMonAddColumn(1, 90, IDS_SYSMON_CURRENT);
    SysMonAddColumn(2, 90, IDS_SYSMON_MIN);
    SysMonAddColumn(3, 90, IDS_SYSMON_MAX);

    /* Hide main window */
    ShowWindow(hMainWnd, SW_HIDE);
}

VOID
SysMonOnSize(LPARAM lParam)
{
    HDWP hdwp = BeginDeferWindowPos(1);

    /* Size hSysMonList */
    DeferWindowPos(hdwp,
                   hSysMonList,
                   0,
                   0,
                   0,
                   LOWORD(lParam),
                   HIWORD(lParam),
                   SWP_NOZORDER|SWP_NOACTIVATE);

    EndDeferWindowPos(hdwp);
}

INT
AddSensorItem(INT Indent, INT IconIndex, LPWSTR lpText)
{
    INT ItemIndex = ListView_GetItemCount(hSysMonList);
    LV_ITEM Item = {0};

    Item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM;
    Item.pszText = lpText;
    Item.iItem = ItemIndex;
    Item.iImage = IconIndex;
    Item.iIndent = Indent;
    Item.lParam = ItemIndex;

    return ListView_InsertItem(hSysMonList, &Item);
}

INT
FindSensorItem(LPWSTR lpText)
{
    LVFINDINFO FindInfo = {0};

    FindInfo.flags = LVFI_STRING;
    FindInfo.psz   = lpText;

    return ListView_FindItem(hSysMonList, -1, &FindInfo);
}

VOID
SetSensorValue(INT i, INT iSubItem, LPWSTR lpText)
{
    ListView_SetItemText(hSysMonList, i, iSubItem, lpText);
}

INT CALLBACK
SysmonIntelCpusEnumProc(INT ItemIndex, LPWSTR CpuName, INT CoreIndex, INT Current, INT Max, INT Min)
{
    WCHAR szValue[MAX_STR_LEN];
    INT Index;

    if (ItemIndex == -1)
    {
        WCHAR szFormat[MAX_STR_LEN];
        INT Old = FindSensorItem(CpuName);
        INT IconIndex;

        if (Old == -1)
        {
            IconIndex = AddIconToImageList(hIconsInst, hSysMonImgList, 16, IDI_CPU);
            AddSensorItem(0, IconIndex, CpuName);
        }

        LoadString(hLangInst, IDS_SENSOR_CORE_TEMPERATURE, szFormat, MAX_STR_LEN);
        StringCbPrintf(szValue, sizeof(szValue), szFormat, CoreIndex + 1);
        Index = AddSensorItem(1, 0, szValue);
    }
    else
    {
        Index = ItemIndex;
    }

    StringCbPrintf(szValue, sizeof(szValue), L"%d °C", Current);
    SetSensorValue(Index, 1, szValue);

    StringCbPrintf(szValue, sizeof(szValue), L"%d °C", Min);
    SetSensorValue(Index, 2, szValue);

    StringCbPrintf(szValue, sizeof(szValue), L"%d °C", Max);
    SetSensorValue(Index, 3, szValue);

    return Index;
}

INT CALLBACK
SysmonHddDrivesEnumProc(INT ItemIndex, LPWSTR SensorName, INT Current, INT Max, INT Min)
{
    WCHAR szValue[MAX_STR_LEN];
    INT Index;

    if (ItemIndex == -1)
    {
        INT IconIndex;

        IconIndex = AddIconToImageList(hIconsInst, hSysMonImgList, 16, IDI_HDD);
        AddSensorItem(0, IconIndex, SensorName);

        LoadString(hLangInst, IDS_SYSMON_TEMPERATURE, szValue, MAX_STR_LEN);
        Index = AddSensorItem(1, 0, szValue);
    }
    else
    {
        Index = ItemIndex;
    }

    StringCbPrintf(szValue, sizeof(szValue), L"%d °C", Current);
    SetSensorValue(Index, 1, szValue);

    StringCbPrintf(szValue, sizeof(szValue), L"%d °C", Min);
    SetSensorValue(Index, 2, szValue);

    StringCbPrintf(szValue, sizeof(szValue), L"%d °C", Max);
    SetSensorValue(Index, 3, szValue);

    return Index;
}

INT CALLBACK
SysmonBatteryEnumProc(INT ItemIndex, INT BatteryIndex, DWORD Current, DWORD Max, DWORD Min)
{
    WCHAR szValue[MAX_STR_LEN];
    INT Index;

    if (ItemIndex == -1)
    {
        WCHAR szFormat[MAX_STR_LEN];
        INT IconIndex;

        IconIndex = AddIconToImageList(hIconsInst, hSysMonImgList, 16, IDI_BATTERY);
        LoadString(hLangInst, IDS_SYSMON_BATTERY, szFormat, MAX_STR_LEN);

        StringCbPrintf(szValue, sizeof(szValue), szFormat, BatteryIndex + 1);
        AddSensorItem(0, IconIndex, szValue);

        LoadString(hLangInst, IDS_BAT_VOLTAGE, szValue, MAX_STR_LEN);
        Index = AddSensorItem(1, 1, szValue);
    }
    else
    {
        Index = ItemIndex;
    }

    StringCbPrintf(szValue, sizeof(szValue), L"%u mV", Current);
    SetSensorValue(Index, 1, szValue);

    StringCbPrintf(szValue, sizeof(szValue), L"%u mV", Min);
    SetSensorValue(Index, 2, szValue);

    StringCbPrintf(szValue, sizeof(szValue), L"%u mV", Max);
    SetSensorValue(Index, 3, szValue);

    return Index;
}

VOID CALLBACK
SysmonUpdateProc(HWND hwnd, UINT msg, UINT id, DWORD systime)
{
    INT Index = 0;

    do
    {
        if (SysmonSensorsList[Index].IsEnabled &&
            !SysmonSensorsList[Index].IsInitialized)
        {
            if (SysmonSensorsList[Index].InitFunction)
                SysmonSensorsList[Index].IsInitialized =
                    SysmonSensorsList[Index].InitFunction();

            if (!SysmonSensorsList[Index].IsInitialized)
            {
                SysmonSensorsList[Index].IsEnabled = FALSE;
            }
        }

        if (SysmonSensorsList[Index].IsEnabled &&
            SysmonSensorsList[Index].IsInitialized)
        {
            if (SysmonSensorsList[Index].UpdateFunction)
                SysmonSensorsList[Index].UpdateFunction(SysmonSensorsList[Index].EnumProcFunction);
        }
    }
    while (SysmonSensorsList[++Index].UpdateFunction != NULL);
}

static VOID
FreeSensorsList(VOID)
{
    INT Index = 0;

    do
    {
        if (SysmonSensorsList[Index].IsEnabled &&
            SysmonSensorsList[Index].IsInitialized)
        {
            SysmonSensorsList[Index].IsInitialized = FALSE;

            if (SysmonSensorsList[Index].FreeFunction)
                SysmonSensorsList[Index].FreeFunction();
        }
    }
    while (SysmonSensorsList[++Index].FreeFunction != NULL);
}

VOID
MonitoringThread(LPVOID lpParameter)
{
    HWND hList = (HWND)lpParameter;

    SetTimer(hList, IDT_SYSMON_UPDATE_TIMER, 1000, SysmonUpdateProc);
}

LRESULT CALLBACK
SysMonWindowProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_CREATE:
        {
            SysMonInitControls(hwnd);

            SetTimer(hSysMonList, IDT_SYSMON_UPDATE_TIMER, 1000, SysmonUpdateProc);
            //_beginthread(MonitoringThread, 0, hSysMonList);
        }
        break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_SYSMON_START:
                {

                }
                break;
            }
            break;

        case WM_NOTIFY:
        {
            LPNMHDR data = (LPNMHDR)lParam;

            switch (data->code)
            {

            }
        }
        break;

        case WM_SIZE:
            SysMonOnSize(lParam);
            break;

        case WM_SIZING:
            break;

        case WM_DESTROY:
        {
            KillTimer(hSysMonList, IDT_SYSMON_UPDATE_TIMER);
            FreeSensorsList();

            ShowWindow(hMainWnd, SW_SHOW);
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, Msg, wParam, lParam);
}

VOID
CreateSysMonWindow(VOID)
{
    WNDCLASSEX WndClass = {0};
    WCHAR szWindowClass[] = L"ASPIAISSYSMON";
    WCHAR szWindowName[MAX_STR_LEN];
    MSG Msg;

    /* Create the window */
    WndClass.cbSize        = sizeof(WNDCLASSEX);
    WndClass.lpszClassName = szWindowClass;
    WndClass.lpfnWndProc   = SysMonWindowProc;
    WndClass.hInstance     = hInstance;
    WndClass.style         = CS_HREDRAW | CS_VREDRAW;
    WndClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    WndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    WndClass.lpszMenuName  = NULL;

    if (RegisterClassEx(&WndClass) == (ATOM)0)
        return;

    LoadMUIStringF(hLangInst, IDS_ASPIA_SYSMON, szWindowName, MAX_STR_LEN);

    /* Создаем главное окно программы */
    hSysMonWnd = CreateWindowEx(WS_EX_WINDOWEDGE,
                                szWindowClass,
                                szWindowName,
                                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                20, 20, 570, 500,
                                NULL, NULL, hInstance, NULL);

    if (!hSysMonWnd)
    {
        UnregisterClass(szWindowClass, hInstance);
        return;
    }

    CenterWindow(hSysMonWnd, NULL);

    /* Show it */
    ShowWindow(hSysMonWnd, SW_SHOW);
    UpdateWindow(hSysMonWnd);

    /* Message Loop */
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    UnregisterClass(szWindowClass, hInstance);
}
