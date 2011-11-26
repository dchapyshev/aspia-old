/*
 * PROJECT:         Aspia
 * FILE:            aspia/benchmarks/disk.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include "driver.h"


#define IDT_SECONDS_COUNTER_TIMER 3476
#define IDT_UPDATE_INFO_TIMER     3477

#define TEST_TYPE_RAMDOM_READ    1
#define TEST_TYPE_BUFFERED_READ  2
#define TEST_TYPE_RANDOM_WRITE   3
#define TEST_TYPE_BUFFERED_WRITE 4

#define TEST_FILE_SIZE_50MB   (50  * 1024 * 1024)
#define TEST_FILE_SIZE_100MB  (100 * 1024 * 1024)
#define TEST_FILE_SIZE_300MB  (300 * 1024 * 1024)
#define TEST_FILE_SIZE_500MB  (500 * 1024 * 1024)
#define TEST_FILE_SIZE_700MB  (700 * 1024 * 1024)
#define TEST_FILE_SIZE_1GB    (1   * 1024 * 1024 * 1024)
#define TEST_FILE_SIZE_3GB    (3   * 1024 * 1024 * 1024)
#define TEST_FILE_SIZE_5GB    (5   * 1024 * 1024 * 1024)
#define TEST_FILE_SIZE_10GB   (10  * 1024 * 1024 * 1024)

#define TEST_BLOCK_SIZE_4KB      (4    * 1024)
#define TEST_BLOCK_SIZE_8KB      (8    * 1024)
#define TEST_BLOCK_SIZE_16KB     (16   * 1024)
#define TEST_BLOCK_SIZE_32KB     (32   * 1024)
#define TEST_BLOCK_SIZE_64KB     (64   * 1024)
#define TEST_BLOCK_SIZE_128KB    (128  * 1024)
#define TEST_BLOCK_SIZE_256KB    (256  * 1024)
#define TEST_BLOCK_SIZE_512KB    (512  * 1024)
#define TEST_BLOCK_SIZE_1MB      (1024 * 1024)

VOID DrawDiagram(HWND hwnd, double x, double y);
VOID GraphOnPaint(HWND hwnd);
VOID GraphSetCoordNames(WCHAR *x, WCHAR *y);
VOID GraphSetCoordMaxValues(HWND hwnd, double x, double y);
VOID GraphSetBeginCoord(int x, int y);
VOID GraphClear(HWND hwnd);


static HWND hDiskToolBar = NULL;
static HWND hDriveCombo = NULL;
static HIMAGELIST hDiskImgList = NULL;
static HICON hDialogIcon = NULL;

static WCHAR szStart[MAX_STR_LEN] = {0};
static WCHAR szStop[MAX_STR_LEN] = {0};
static WCHAR szSave[MAX_STR_LEN] = {0};
static WCHAR szClear[MAX_STR_LEN] = {0};

static BOOL IsDiskBenchLaunched = FALSE;
static DWORD SecondsCount = 0;
static DWORD gFileSize = 0;
static DWORD gBlockSize = 0;
static DWORD gCurrentSpeed = 0;
static INT gReadingCount = 0;
static INT gReadingTotal = 0;
static INT gReadingPercent = 0;

static double gMaxSpeed = 0;
static double gMinSpeed = 0;

static BOOL IsTestCanceled = FALSE;


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
UpdateStatusTitle(HWND hDlg, UINT StringId)
{
    WCHAR szText[MAX_STR_LEN], szName[MAX_STR_LEN],
          szTitle[MAX_STR_LEN];

    LoadMUIString(IDS_ASPIA_DISK_BENCH, szName, MAX_STR_LEN);

    if (StringId != 0)
    {
        LoadMUIString(StringId, szText, MAX_STR_LEN);
        StringCbPrintf(szTitle, sizeof(szTitle),
                       L"%s - %s", szName, szText);
        SetWindowText(hDlg, szTitle);
    }
    else
    {
        SetWindowText(hDlg, szName);
    }
}

VOID
InitLogicDrivesList(HWND hCombo)
{
    WCHAR szDrives[MAX_PATH], szDrive[4], szText[MAX_STR_LEN], *p;
    LPWSTR lpType;
    UINT DriveType;
    INT Count, Index;

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

        Index = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szText);

        p = (WCHAR*)Alloc((wcslen(szDrive) + 1) * sizeof(WCHAR));
        if (p) wcscpy(p, szDrive);
        SendMessage(hCombo, CB_SETITEMDATA, Index, (LPARAM)p);
    }

    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

VOID
ClearDrivesCombo(HWND hCombo)
{
    INT count = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
    WCHAR *p;

    while (count--)
    {
        p = (WCHAR*)SendMessage(hCombo, CB_GETITEMDATA, count, 0);
        if (p) Free(p);
        SendMessage(hCombo, CB_DELETESTRING, count, 0);
    }
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
AddStringIdToCombo(HWND hCombo, LPARAM lParam, UINT StringID)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    LoadMUIString(StringID, szText, MAX_STR_LEN);

    Index = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szText);
    SendMessage(hCombo, CB_SETITEMDATA, Index, lParam);
}

VOID
AddStringToCombo(HWND hCombo, LPARAM lParam, LPWSTR lpStr)
{
    INT Index = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)lpStr);
    SendMessage(hCombo, CB_SETITEMDATA, Index, lParam);
}

VOID
DiskClearResults(HWND hDlg)
{
    UpdateStatusTitle(hDlg, 0);

    SetWindowText(GetDlgItem(hDlg, IDC_DISK_CURRENT), L"0.00 MB/s");
    SetWindowText(GetDlgItem(hDlg, IDC_DISK_MINIMUM), L"0.00 MB/s");
    SetWindowText(GetDlgItem(hDlg, IDC_DISK_MAXIMUM), L"0.00 MB/s");

    SetWindowText(GetDlgItem(hDlg, IDC_CPU_CURRENT), L"0 %");
    SetWindowText(GetDlgItem(hDlg, IDC_CPU_MINIMUM), L"0 %");
    SetWindowText(GetDlgItem(hDlg, IDC_CPU_MAXIMUM), L"0 %");

    SecondsCount = 0;
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

    AddStringToCombo(hBlockCombo, TEST_BLOCK_SIZE_4KB, L"4 KB");
    AddStringToCombo(hBlockCombo, TEST_BLOCK_SIZE_8KB, L"8 KB");
    AddStringToCombo(hBlockCombo, TEST_BLOCK_SIZE_16KB, L"16 KB");
    AddStringToCombo(hBlockCombo, TEST_BLOCK_SIZE_32KB, L"32 KB");
    AddStringToCombo(hBlockCombo, TEST_BLOCK_SIZE_64KB, L"64 KB");
    AddStringToCombo(hBlockCombo, TEST_BLOCK_SIZE_128KB, L"128 KB");
    AddStringToCombo(hBlockCombo, TEST_BLOCK_SIZE_256KB, L"256 KB");
    AddStringToCombo(hBlockCombo, TEST_BLOCK_SIZE_512KB, L"512 KB");
    AddStringToCombo(hBlockCombo, TEST_BLOCK_SIZE_1MB, L"1 MB");
    SendMessage(hBlockCombo, CB_SETCURSEL, 0, 0);

    AddStringIdToCombo(hTestCombo, TEST_TYPE_RAMDOM_READ, IDS_DISK_RANDOM_READ);
    AddStringIdToCombo(hTestCombo, TEST_TYPE_BUFFERED_READ, IDS_DISK_BUFFRED_READ);
    AddStringIdToCombo(hTestCombo, TEST_TYPE_RANDOM_WRITE, IDS_DISK_RANDOM_WRITE);
    AddStringIdToCombo(hTestCombo, TEST_TYPE_BUFFERED_WRITE, IDS_DISK_BUFFRED_WRITE);
    SendMessage(hTestCombo, CB_SETCURSEL, 0, 0);

    AddStringToCombo(hFileSizeCombo, TEST_FILE_SIZE_50MB, L"50 MB");
    AddStringToCombo(hFileSizeCombo, TEST_FILE_SIZE_100MB, L"100 MB");
    AddStringToCombo(hFileSizeCombo, TEST_FILE_SIZE_300MB, L"300 MB");
    AddStringToCombo(hFileSizeCombo, TEST_FILE_SIZE_500MB, L"500 MB");
    AddStringToCombo(hFileSizeCombo, TEST_FILE_SIZE_700MB, L"700 MB");
    AddStringToCombo(hFileSizeCombo, TEST_FILE_SIZE_1GB, L"1 GB");
    AddStringToCombo(hFileSizeCombo, TEST_FILE_SIZE_3GB, L"3 GB");
    AddStringToCombo(hFileSizeCombo, TEST_FILE_SIZE_5GB, L"5 GB");
    AddStringToCombo(hFileSizeCombo, TEST_FILE_SIZE_10GB, L"10 GB");
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

    GraphSetCoordNames(L"%", L"MB/s");

    DiskClearResults(hDlg);
}

BOOL
CreateFileForReadingTest(WCHAR *pDrive,
                         WCHAR *pFilePath,
                         DWORD dwFileSize)
{
    DWORD dwFileSystemFlags, dwBytesWritten;
    char *pData;
    INT i, count;
    HANDLE hFile;

    /* Создаем файл для тестирования чтения */
    hFile = CreateFile(pFilePath,
                       GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING |
                       FILE_FLAG_SEQUENTIAL_SCAN,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"CreateFile() failed. Error code = %d",
                   GetLastError());
        return FALSE;
    }

    /* Получаем информацию о разделе диска */
    if (!GetVolumeInformation(pDrive,
                              NULL, NULL, NULL, NULL,
                              &dwFileSystemFlags,
                              NULL, NULL))
    {
        DebugTrace(L"GetVolumeInformation() failed!");
        return FALSE;
    }

    /* Проверяем включено ли сжатие для данных на разделе */
    if (dwFileSystemFlags & FILE_VOLUME_IS_COMPRESSED)
    {
        DWORD lpInBuffer = COMPRESSION_FORMAT_NONE;
        DWORD dwBytesReturned;

        DebugTrace(L"Compressed file system.");

        /* Снимаем сжатие с файла, если оно включено */
        if (!DeviceIoControl(hFile,
                             FSCTL_SET_COMPRESSION,
                             (LPVOID)&lpInBuffer,
                             sizeof(USHORT),
                             NULL, 0,
                             (LPDWORD)&dwBytesReturned,
                             NULL))
        {
            DebugTrace(L"DeviceIoControl() failed!");
            CloseHandle(hFile);
            return FALSE;
        }
    }

    /* Выделяем буфер с размером блока */
    pData = (char*)VirtualAlloc(NULL,
                                TEST_BLOCK_SIZE_1MB,
                                MEM_COMMIT,
                                PAGE_READWRITE);
    if (!pData)
    {
        DebugTrace(L"VirtualAlloc() failed!");
        CloseHandle(hFile);
        return FALSE;
    }

    /* Заполняем буфер */
    memset(pData, 0xFF, TEST_BLOCK_SIZE_1MB);

    /* Определяем количество повторений записи */
    count = (INT)(dwFileSize / TEST_BLOCK_SIZE_1MB);

    DebugTrace(L"Start file creating. count = %d", count);

    /* Пишем файл заданного размера */
    for (i = 0; i < count; i++)
    {
        WriteFile(hFile, pData,
                  TEST_BLOCK_SIZE_1MB,
                  &dwBytesWritten,
                  NULL);
    }

    DebugTrace(L"End file creating.");

    /* Освобождаем буфер и закрываем файл */
    VirtualFree(pData, TEST_BLOCK_SIZE_1MB, MEM_DECOMMIT);
    CloseHandle(hFile);

    return TRUE;
}

VOID
CreateFilePathForReadingTest(WCHAR *pDrive, WCHAR *pPath, SIZE_T Size)
{
    INT i = 0;

    do
    {
        if (i++ > 100) return;

        StringCbPrintf(pPath, Size,
                       L"%saspia_read_test-%d",
                       pDrive, i);
    }
    while (GetFileAttributes(pPath) != INVALID_FILE_ATTRIBUTES);
}

VOID CALLBACK
SecondsCounterProc(HWND hDlg, UINT msg, UINT id, DWORD systime)
{
    HWND hwnd = GetDlgItem(hDlg, IDC_GRAPH_WND);
    WCHAR szText[MAX_STR_LEN];
    double speed;

    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(systime);

    speed = ((double)(gReadingCount * gBlockSize) / (double)(1024 * 1024)) * 4.0;

    StringCbPrintf(szText, sizeof(szText), L"%.2f MB/s", speed);
    SetWindowText(GetDlgItem(hDlg, IDC_DISK_CURRENT), szText);

    if (speed > gMaxSpeed)
    {
        gMaxSpeed = speed;

        StringCbPrintf(szText, sizeof(szText), L"%.2f MB/s", gMaxSpeed);
        SetWindowText(GetDlgItem(hDlg, IDC_DISK_MAXIMUM), szText);
    }

    if (speed < gMinSpeed)
    {
        gMinSpeed = speed;

        StringCbPrintf(szText, sizeof(szText), L"%.2f MB/s", gMinSpeed);
        SetWindowText(GetDlgItem(hDlg, IDC_DISK_MINIMUM), szText);
    }

    if (SecondsCount < 2)
    {
        gMinSpeed = speed;
        gMaxSpeed = speed;

        if (SecondsCount == 1)
        {
            GraphSetCoordMaxValues(hwnd, 100.0, (speed * 3.0));
            GraphSetBeginCoord(0, speed);
        }
    }
    else
    {
        DrawDiagram(hwnd, gReadingPercent, speed);
    }

    SecondsCount++;
    /* Обнуляем счетчик количества чтений */
    gReadingCount = 0;
}

/* 
 *   Функция генерирует псевдослучайные числа
 *   Алгоритм найден на http://en.wikipedia.org/wiki/Xorshift
 */
DWORD xor128(VOID)
{
    static DWORD x = 123456789;
    static DWORD y = 362436069;
    static DWORD z = 521288629;
    static DWORD w = 88675123;
    DWORD t;
 
    t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}

VOID
ReadDiskTest(HWND hDlg, WCHAR *pDrive, INT TestType,
             DWORD dwFileSize, DWORD dwBlockSize)
{
    DWORD dwAttrib = FILE_ATTRIBUTE_NORMAL;
    DWORD dwBytesWritten;
    WCHAR szFilePath[MAX_PATH];
    LARGE_INTEGER MoveTo;
    INT i, count;
    HANDLE hFile;
    char *pData;

    switch (TestType)
    {
        case TEST_TYPE_RAMDOM_READ:
            dwAttrib |= FILE_FLAG_RANDOM_ACCESS;
            dwAttrib |= FILE_FLAG_NO_BUFFERING;
            break;
        case TEST_TYPE_BUFFERED_READ:
            dwAttrib |= FILE_FLAG_NO_BUFFERING;
            break;
        default:
            return;
    }

    /* Генерируем имя файла */
    CreateFilePathForReadingTest(pDrive, szFilePath,
                                 sizeof(szFilePath));

    DebugTrace(L"drive = %s, path = %s, file size = %ld, block size = %ld",
               pDrive, szFilePath, dwFileSize, dwBlockSize);

    /* Создаем файл для тестирования */
    UpdateStatusTitle(hDlg, IDS_DISK_FILECREATE);
    if (!CreateFileForReadingTest(pDrive, szFilePath,
                                  dwFileSize, dwBlockSize))
    {
        DeleteFile(szFilePath);
        return;
    }

    /* Начинаем тест */
    UpdateStatusTitle(hDlg, IDS_DISK_TESTING);

    /* Открываем файл, который будем читать */
    hFile = CreateFile(szFilePath,
                       GENERIC_READ | GENERIC_WRITE,
                       0, NULL, OPEN_EXISTING,
                       dwAttrib, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"CreateFile() failed. Error code = %d",
                   GetLastError());
        DeleteFile(szFilePath);
        return;
    }

    /* Выделяем буфер с размером блока */
    pData = (char*)VirtualAlloc(NULL,
                                dwBlockSize,
                                MEM_COMMIT,
                                PAGE_READWRITE);
    if (!pData)
    {
        DebugTrace(L"VirtualAlloc() failed!");
        CloseHandle(hFile);
        return;
    }

    IsTestCanceled = FALSE;

    gMaxSpeed = 0;
    gMinSpeed = 0;

    /* Создаем таймер, который будет считать количество секунд с начала теста */
    SecondsCount = 0;
    SetTimer(hDlg, IDT_SECONDS_COUNTER_TIMER, 250, SecondsCounterProc);

    count = (INT)(dwFileSize / dwBlockSize);
    DebugTrace(L"Start file reading. count = %d", count);

    gFileSize = dwFileSize;
    gBlockSize = dwBlockSize;
    gReadingTotal = count;

    i = 0;
    do
    {
        if (IsTestCanceled) break;

        MoveTo.QuadPart = xor128() % (count * dwBlockSize);

        if (SetFilePointerEx(hFile, MoveTo, NULL, FILE_BEGIN))
        {
            gReadingPercent = (i * 100) / count;
            ReadFile(hFile, pData, dwBlockSize, &dwBytesWritten, NULL);

            gReadingCount++;
            i++;
        }
    }
    while (i < count);

    SecondsCounterProc(hDlg, 0, 0, 0);

    DebugTrace(L"End file reading.");

    /* Пишем, что тест завершен */
    UpdateStatusTitle(hDlg, IDS_DISK_TEST_COMPLETE);

    /* Проводим очистку */
    VirtualFree(pData, dwBlockSize, MEM_DECOMMIT);
    KillTimer(hDlg, IDT_SECONDS_COUNTER_TIMER);
    CloseHandle(hFile);
    DeleteFile(szFilePath);
}

VOID
WriteDiskTest(HWND hDlg, WCHAR *pDrive, INT TestType,
              INT FileSize, INT BlockSize)
{

}

VOID
TestThread(LPVOID lpParameter)
{
    HWND hDlg = (HWND)lpParameter;
    DWORD dwFileSize, dwBlockSize;
    INT Index, TestType;
    WCHAR *pDrive;

    GraphClear(GetDlgItem(hDlg, IDC_GRAPH_WND));

    /* Получаем размер файла */
    Index = SendMessage(GetDlgItem(hDlg, IDC_TEST_FILE_SIZE),
                        CB_GETCURSEL, 0, 0);
    dwFileSize = SendMessage(GetDlgItem(hDlg, IDC_TEST_FILE_SIZE),
                             CB_GETITEMDATA, Index, 0);
    /* Получаем размер блока */
    Index = SendMessage(GetDlgItem(hDlg, IDC_BLOCK_SIZE),
                        CB_GETCURSEL, 0, 0);
    dwBlockSize = SendMessage(GetDlgItem(hDlg, IDC_BLOCK_SIZE),
                              CB_GETITEMDATA, Index, 0);
    /* Получаем тип теста */
    Index = SendMessage(GetDlgItem(hDlg, IDC_TEST_TYPE),
                        CB_GETCURSEL, 0, 0);
    TestType = SendMessage(GetDlgItem(hDlg, IDC_TEST_TYPE),
                           CB_GETITEMDATA, Index, 0);

    /* Получаем строку устройства (буква диска или точка монтирования) */
    Index = SendMessage(hDriveCombo, CB_GETCURSEL, 0, 0);
    pDrive = (WCHAR*)SendMessage(hDriveCombo, CB_GETITEMDATA, Index, 0);

    if (!pDrive || (INT)pDrive == CB_ERR || dwFileSize == CB_ERR ||
        TestType == CB_ERR || dwBlockSize == CB_ERR)
    {
        _endthread();
    }

    switch (TestType)
    {
        case TEST_TYPE_RAMDOM_READ:
        case TEST_TYPE_BUFFERED_READ:
            ReadDiskTest(hDlg, pDrive, TestType, dwFileSize, dwBlockSize);
            break;

        case TEST_TYPE_RANDOM_WRITE:
        case TEST_TYPE_BUFFERED_WRITE:
            WriteDiskTest(hDlg, pDrive, TestType, dwFileSize, dwBlockSize);
            break;
    }

    _endthread();
}

INT_PTR CALLBACK
DiskBenchDlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            CenterWindow(hDlg, NULL);
            InitDiskBenchDlg(hDlg);

            IsDiskBenchLaunched = TRUE;
        }
        break;

        case WM_PAINT:
            GraphOnPaint(GetDlgItem(hDlg, IDC_GRAPH_WND));
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_DISK_START:
                    _beginthread(TestThread, 0, (LPVOID)hDlg);
                    break;

                case ID_DISK_STOP:
                    IsTestCanceled = TRUE;
                    break;

                case ID_DISK_CLEAR:
                    DiskClearResults(hDlg);
                    break;
            }
            break;

        case WM_CLOSE:
        {
            ClearDrivesCombo(hDriveCombo);

            DestroyIcon(hDialogIcon);
            ImageList_Destroy(hDiskImgList);

            IsDiskBenchLaunched = FALSE;

            EndDialog(hDlg, LOWORD(wParam));
        }
        break;
    }

    return FALSE;
}

VOID
CreateDiskBenchWindow(VOID)
{
    if (IsDiskBenchLaunched)
        return;

    DialogBox(hLangInst,
              MAKEINTRESOURCE(IDD_DISK_BENCH_DIALOG),
              NULL,
              DiskBenchDlgProc);
}
