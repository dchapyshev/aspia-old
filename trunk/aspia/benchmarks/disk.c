/*
 * PROJECT:         Aspia
 * FILE:            aspia/benchmarks/disk.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include "driver.h"


static HWND hDiskToolBar = NULL;
static HWND hDriveCombo = NULL;
static HIMAGELIST hDiskImgList = NULL;
static HICON hDialogIcon = NULL;

static WCHAR szStart[MAX_STR_LEN] = {0};
static WCHAR szStop[MAX_STR_LEN] = {0};
static WCHAR szSave[MAX_STR_LEN] = {0};
static WCHAR szClear[MAX_STR_LEN] = {0};

/* Toolbar buttons */
static const TBBUTTON Buttons[] =
{   /* iBitmap, idCommand, fsState, fsStyle, bReserved[2], dwData, iString */
    { 0, ID_DISK_START,TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szStart},
    { 1, ID_DISK_STOP, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szStop},
    {-1, 0,            TBSTATE_ENABLED, BTNS_SEP,                    {0}, 0, 0},
    { 2, ID_DISK_SAVE, TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szSave},
    { 3, ID_DISK_CLEAR,TBSTATE_ENABLED, BTNS_BUTTON | BTNS_AUTOSIZE, {0}, 0, (INT_PTR)szClear},
};


VOID
AddStringToComboBox(HWND hCombo, UINT StringID)
{
    WCHAR szText[MAX_STR_LEN];

    LoadMUIString(StringID, szText, MAX_STR_LEN);

    SendMessage(hCombo,
                CB_ADDSTRING, 0,
                (LPARAM)szText);
}

VOID
InitPhysicalDriverList(HWND hCombo)
{
    WCHAR szText[MAX_STR_LEN];
    IDSECTOR DriveInfo = {0};
    HANDLE hHandle;
    BYTE bIndex;

    for (bIndex = 0; bIndex <= 32; ++bIndex)
    {
        hHandle = drv_open_smart(bIndex);
        if (hHandle == INVALID_HANDLE_VALUE) continue;

        if (drv_read_smart_info(hHandle, bIndex, &DriveInfo))
        {
            ChangeByteOrder((PCHAR)DriveInfo.sModelNumber,
                            sizeof(DriveInfo.sModelNumber));

            StringCbPrintf(szText, sizeof(szText),
                           L"%S", DriveInfo.sModelNumber);
            ChopSpaces(szText, sizeof(szText));

            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szText);
        }

        drv_close_smart(hHandle);
    }

    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

VOID
InitLogicDrivesList(HWND hCombo)
{
    WCHAR szDrives[MAX_PATH], szDrive[4], szText[MAX_STR_LEN];
    LPWSTR lpType;
    UINT DriveType;
    INT Count;

    if (!GetLogicalDriveStrings(sizeof(szDrives)/sizeof(WCHAR), szDrives))
        return;

    for (Count = 0; szDrives[Count] != 0; Count += 4)
    {
        if (*(szDrives) > L'Z')
        {
            if (szDrives[Count] <= L'Z') szDrives[Count] += 32;
        }
        else
        {
            if (szDrives[Count] > L'Z') szDrives[Count] -= 32;
        }

        StringCbPrintf(szDrive,
                       sizeof(szDrive),
                       L"%c:\\",
                       szDrives[Count]);

        DriveType = GetDriveType(szDrive);

        switch (DriveType)
        {
            case DRIVE_REMOVABLE:
                lpType = L"Removable Disk";
                break;

            case DRIVE_FIXED:
                lpType = L"Hard Disk";
                break;

            case DRIVE_REMOTE:
                lpType = L"Remote Disk";
                break;

            case DRIVE_RAMDISK:
                lpType = L"Ram Disk";
                break;

            case DRIVE_CDROM:
            case DRIVE_UNKNOWN:
            case DRIVE_NO_ROOT_DIR:
            default:
                continue;
        }

        StringCbPrintf(szText, sizeof(szText), L"%s (%s)", szDrive, lpType);

        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szText);
    }

    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

VOID
ClearDrivesCombo(HWND hCombo)
{
    INT count = SendMessage(hCombo, CB_GETCOUNT, 0, 0);

    while (count--)
    {
        SendMessage(hCombo, CB_DELETESTRING, count, 0);
    }
}

/* Функция рисует полигон для построения графика (оси, сетка, подписи к осям) */
VOID
DrawPolygon(HWND hwnd, HDC hdc)
{
    INT Height, Width, count, i;
    HBRUSH hBrush, hOldBrush;
    HPEN hPen, hOldPen;
    WCHAR buf[3];

    Height = GetClientWindowHeight(hwnd);
    Width = GetClientWindowWidth(hwnd);

#define LEFT_INDENT   40 /* Отступ слева */
#define RIGHT_INDENT  0  /* Отступ справа */
#define TOP_INDENT    0  /* Отступ сверху */
#define BOTTOM_INDENT 30 /* Отступ снизу */
#define X_GRID        10 /* Количество шагов сетки по оси X */
#define Y_GRID        10 /* Количество шагов сетки по оси Y */

#define X_MAX_NUM     100 /* Максимальная единица измерения по оси X */
#define Y_MAX_NUM     100 /* Максимальная единица измерения по оси Y */

    /* Задается отображение логических единиц, как физических */
    SetMapMode(hdc, MM_ISOTROPIC);
    /* Длина осей */
    SetWindowExtEx(hdc, Width, Height, NULL);
    /* Задаем область вывода */
    SetViewportExtEx(hdc, Width, Height, NULL);
    /* Устанавливаем начало координат */
    SetViewportOrgEx(hdc, 0, 0, NULL);
    /* Устанавливаем режим фона */
    SetBkMode(hdc, TRANSPARENT);

    /* Рисуем черный прямоугольник с серой окантовкой */
    hPen = CreatePen(PS_SOLID, 1, RGB(130, 130, 130));
    hBrush = CreateSolidBrush(RGB(0, 0, 0));

    hOldPen = SelectObject(hdc, hPen);
    hOldBrush = SelectObject(hdc, hBrush);

    Rectangle(hdc, 0, 0, Width, Height);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);

    /* Начинаем рисовать оси координат */
    hPen = CreatePen(PS_SOLID, 1, RGB(145, 145, 145));
    hOldPen = SelectObject(hdc, hPen);

    /* Рисуем ось Y */
    MoveToEx(hdc, LEFT_INDENT, Height - BOTTOM_INDENT, NULL);
    LineTo(hdc, LEFT_INDENT, TOP_INDENT);

    /* Рисуем ось X */
    MoveToEx(hdc, LEFT_INDENT, Height - BOTTOM_INDENT, NULL);
    LineTo(hdc, Width - RIGHT_INDENT, Height - BOTTOM_INDENT);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    /* Подготавливаемся к рисованию сеток */
    hPen = CreatePen(PS_DOT, 1, RGB(145, 145, 145));
    hOldPen = SelectObject(hdc, hPen);

    /* Рисуем сетку по оси X */
    count = (Width - LEFT_INDENT - RIGHT_INDENT) / X_GRID;

    /* Устанавливаем цвет шрифта */
    SetTextColor(hdc, RGB(255, 255, 255));
    /* Устанавливаем шрифт из системы */
    SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

    for (i = 0; i < X_GRID; i++)
    {
        INT x = (count * i) + LEFT_INDENT;

        MoveToEx(hdc, x, 0, NULL);
        LineTo(hdc, x, Height - TOP_INDENT - BOTTOM_INDENT);

        _itow((X_MAX_NUM / X_GRID) * i, buf, 10);
        TextOut(hdc, x - 5, Height - BOTTOM_INDENT + 5, buf, wcslen(buf));
    }

    /* Рисуем сетку по оси Y */
    count = (Height - TOP_INDENT - BOTTOM_INDENT) / Y_GRID;

    for (i = 1; i < Y_GRID; i++)
    {
        INT y = Height - (count * i) - BOTTOM_INDENT;

        MoveToEx(hdc, LEFT_INDENT, y, NULL);
        LineTo(hdc, Width - RIGHT_INDENT, y);

        _itow((Y_MAX_NUM / Y_GRID) * i, buf, 10);
        TextOut(hdc, LEFT_INDENT - 25, y - 7, buf, wcslen(buf));
    }

    /* Удаляем объекты после рисования сеток */
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    /* Выводим подписи с единицами измерений к осям */
    TextOut(hdc, LEFT_INDENT - 30, TOP_INDENT + 5, L"MB/s", 5); /* Y */
    TextOut(hdc, Width - RIGHT_INDENT - 15, Height - BOTTOM_INDENT + 5, L"%", 2); /* X */
}

VOID
DiskToolBarOnGetDispInfo(LPTOOLTIPTEXT lpttt)
{
    UINT idButton = (UINT)lpttt->hdr.idFrom;
    UINT StringID = 0;

    switch (idButton)
    {
        case ID_DISK_START:
            StringID = IDS_DISK_START;
            break;
        case ID_DISK_STOP:
            StringID = IDS_DISK_STOP;
            break;
        case ID_DISK_SAVE:
            StringID = IDS_DISK_SAVE;
            break;
        case ID_DISK_CLEAR:
            StringID = IDS_DISK_CLEAR;
            break;
    }

    LoadMUIString(StringID, lpttt->szText, 80);
}

VOID
InitDiskBenchDlg(HWND hDlg)
{
    SIZE_T NumButtons = sizeof(Buttons) / sizeof(Buttons[0]);
    HWND hBlockCombo = GetDlgItem(hDlg, IDC_BLOCK_SIZE);
    HWND hTestCombo = GetDlgItem(hDlg, IDC_TEST_TYPE);
    HWND hFileSizeCombo = GetDlgItem(hDlg, IDC_TEST_FILE_SIZE);

    hDialogIcon = LoadImage(hIconsInst,
                            MAKEINTRESOURCE(IDI_HDD),
                            IMAGE_ICON,
                            16, 16, 0);
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);

    LoadMUIString(IDS_DISK_START, szStart, MAX_STR_LEN);
    LoadMUIString(IDS_DISK_STOP, szStop, MAX_STR_LEN);
    LoadMUIString(IDS_DISK_SAVE, szSave, MAX_STR_LEN);
    LoadMUIString(IDS_DISK_CLEAR, szClear, MAX_STR_LEN);

    /* Create toolbar */
    hDiskToolBar = CreateWindowEx(0,
                                  TOOLBARCLASSNAME,
                                  NULL,
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBSTYLE_FLAT |
                                  TBSTYLE_TOOLTIPS | TBSTYLE_LIST | CCS_BOTTOM,
                                  0, 40,
                                  340, 30,
                                  hDlg,
                                  NULL,
                                  hInstance,
                                  NULL);
    if (!hDiskToolBar) return;

    SendMessage(hDiskToolBar,
                TB_BUTTONSTRUCTSIZE,
                sizeof(Buttons[0]),
                0);

    /* Create image list for ToolBar */
    hDiskImgList = ImageList_Create(TOOLBAR_HEIGHT,
                                    TOOLBAR_HEIGHT,
                                    ILC_MASK | ParamsInfo.SysColorDepth,
                                    1, 1);
    if (!hDiskImgList)
        return;

    AddImageToImageList(hDiskImgList, IDI_START);
    AddImageToImageList(hDiskImgList, IDI_STOP);
    AddImageToImageList(hDiskImgList, IDI_SAVE);
    AddImageToImageList(hDiskImgList, IDI_CLEAR);

    SendMessage(hDiskToolBar,
                TB_SETIMAGELIST,
                0,
                (LPARAM)hDiskImgList);

    SendMessage(hDiskToolBar, TB_ADDBUTTONS, NumButtons, (LPARAM)Buttons);

    /* Size tool bar */
    SendMessage(hDiskToolBar, TB_AUTOSIZE, 0, 0);

    SendMessage(hBlockCombo, CB_ADDSTRING, 0, (LPARAM)L"4 KB");
    SendMessage(hBlockCombo, CB_ADDSTRING, 0, (LPARAM)L"8 KB");
    SendMessage(hBlockCombo, CB_ADDSTRING, 0, (LPARAM)L"16 KB");
    SendMessage(hBlockCombo, CB_ADDSTRING, 0, (LPARAM)L"32 KB");
    SendMessage(hBlockCombo, CB_ADDSTRING, 0, (LPARAM)L"64 KB");
    SendMessage(hBlockCombo, CB_ADDSTRING, 0, (LPARAM)L"128 KB");
    SendMessage(hBlockCombo, CB_ADDSTRING, 0, (LPARAM)L"256 KB");
    SendMessage(hBlockCombo, CB_ADDSTRING, 0, (LPARAM)L"512 KB");
    SendMessage(hBlockCombo, CB_ADDSTRING, 0, (LPARAM)L"1 MB");
    SendMessage(hBlockCombo, CB_SETCURSEL, 0, 0);

    AddStringToComboBox(hTestCombo, IDS_DISK_RANDOM_READ);
    AddStringToComboBox(hTestCombo, IDS_DISK_BUFFRED_READ);
    AddStringToComboBox(hTestCombo, IDS_DISK_RANDOM_WRITE);
    AddStringToComboBox(hTestCombo, IDS_DISK_BUFFRED_WRITE);
    SendMessage(hTestCombo, CB_SETCURSEL, 0, 0);

    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"10 MB");
    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"50 MB");
    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"100 MB");
    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"300 MB");
    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"500 MB");
    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"700 MB");
    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"1 GB");
    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"3 GB");
    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"5 GB");
    SendMessage(hFileSizeCombo, CB_ADDSTRING, 0, (LPARAM)L"10 GB");
    SendMessage(hFileSizeCombo, CB_SETCURSEL, 0, 0);

    /* Drive select combobox */
    hDriveCombo = CreateWindow(L"COMBOBOX", L"",
                               CBS_DROPDOWNLIST | CBS_SORT | WS_CHILD |
                                   WS_VISIBLE | WS_TABSTOP | WS_VSCROLL,
                               GetClientWindowWidth(hDiskToolBar) - 300 - 10,
                               5,
                               300,
                               50,
                               hDiskToolBar,
                               0, hInstance, NULL);
    if (!hDriveCombo) return;

    SendMessage(hDriveCombo, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
    InitLogicDrivesList(hDriveCombo);

    SetWindowText(GetDlgItem(hDlg, IDC_DISK_CURRENT), L"0 MB/s");
    SetWindowText(GetDlgItem(hDlg, IDC_DISK_MINIMUM), L"0 MB/s");
    SetWindowText(GetDlgItem(hDlg, IDC_DISK_MAXIMUM), L"0 MB/s");

    SetWindowText(GetDlgItem(hDlg, IDC_CPU_CURRENT), L"0 %");
    SetWindowText(GetDlgItem(hDlg, IDC_CPU_MINIMUM), L"0 %");
    SetWindowText(GetDlgItem(hDlg, IDC_CPU_MAXIMUM), L"0 %");
}

INT_PTR CALLBACK
DiskBenchDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg, NULL);
            InitDiskBenchDlg(hDlg);
            break;

        case WM_PAINT:
        {
            HWND hGraph = GetDlgItem(hDlg, IDC_GRAPH_WND);
            PAINTSTRUCT ps;
            HDC hdc;

            hdc = BeginPaint(hGraph, &ps);

            DrawPolygon(hGraph, hdc);
            ValidateRect(hGraph, NULL);

            EndPaint(hGraph, &ps);
        }
        break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_LOW_LEVEL_TEST:
                {
                    if (IsDlgButtonChecked(hDlg, IDC_LOW_LEVEL_TEST) == BST_CHECKED)
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_TEST_FILE_SIZE), FALSE);
                        ClearDrivesCombo(hDriveCombo);
                        InitPhysicalDriverList(hDriveCombo);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_TEST_FILE_SIZE), TRUE);
                        ClearDrivesCombo(hDriveCombo);
                        InitLogicDrivesList(hDriveCombo);
                    }
                }
                break;
            }
            break;

        case WM_CLOSE:
            DestroyIcon(hDialogIcon);
            ImageList_Destroy(hDiskImgList);
            EndDialog(hDlg, LOWORD(wParam));
            break;
    }

    return FALSE;
}

VOID
CreateDiskBenchWindow(VOID)
{
    DialogBox(hLangInst,
              MAKEINTRESOURCE(IDD_DISK_BENCH_DIALOG),
              NULL,
              DiskBenchDlgProc);
}
