/*
 * PROJECT:         Aspia
 * FILE:            driver/dll/battery.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "driver.h"


HANDLE
drv_open_battery(LPWSTR lpszDevice)
{
    return CreateFile(lpszDevice,
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE, 
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_SYSTEM,
                      NULL);
}

BOOL
drv_close_battery(HANDLE hHandle)
{
    return CloseHandle(hHandle);
}

ULONG
drv_get_battery_tag(HANDLE hHandle)
{
    ULONG uBatteryTag = 0;
    DWORD cbBytesReturned, dwTmp = 0;

    if (!DeviceIoControl(hHandle,
                         IOCTL_BATTERY_QUERY_TAG,
                         &dwTmp,
                         sizeof(DWORD),
                         &uBatteryTag,
                         sizeof(ULONG),
                         &cbBytesReturned,
                         NULL))
    {
        return 0;
    }

    return uBatteryTag;
}

BOOL
drv_query_battery_info(HANDLE hHandle,
                       BATTERY_QUERY_INFORMATION_LEVEL InfoLevel,
                       LPVOID lpBuffer,
                       DWORD dwBufferSize)
{
    BATTERY_QUERY_INFORMATION BatteryQueryInfo;
    DWORD cbBytesReturned;

    BatteryQueryInfo.BatteryTag = drv_get_battery_tag(hHandle);
    if (!BatteryQueryInfo.BatteryTag)
        return FALSE;

    BatteryQueryInfo.InformationLevel = InfoLevel;
    return DeviceIoControl(hHandle,
                           IOCTL_BATTERY_QUERY_INFORMATION,
                           &BatteryQueryInfo,
                           sizeof(BATTERY_QUERY_INFORMATION),
                           lpBuffer,
                           dwBufferSize,
                           &cbBytesReturned,
                           NULL);
}

BOOL
drv_query_battery_status(HANDLE hHandle,
                         BATTERY_STATUS *lpBatteryStatus,
                         DWORD dwBufferSize)
{
    BATTERY_WAIT_STATUS BatteryWaitStatus = {0};
    DWORD cbBytesReturned;

    BatteryWaitStatus.BatteryTag = drv_get_battery_tag(hHandle);
    if (!BatteryWaitStatus.BatteryTag)
        return FALSE;

    return DeviceIoControl(hHandle,
                           IOCTL_BATTERY_QUERY_STATUS,
                           &BatteryWaitStatus,
                           sizeof(BATTERY_WAIT_STATUS),
                           lpBatteryStatus,
                           dwBufferSize,
                           &cbBytesReturned,
                           NULL);
}
