/*
 * PROJECT:         Aspia (Helper DLL)
 * FILE:            helper_dll/battery.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"


HANDLE
OpenBattery(LPWSTR lpszDevice)
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
CloseBattery(HANDLE hHandle)
{
    return CloseHandle(hHandle);
}

ULONG
GetBatteryTag(HANDLE hHandle)
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
QueryBatteryInfo(HANDLE hHandle,
                       BATTERY_QUERY_INFORMATION_LEVEL InfoLevel,
                       LPVOID lpBuffer,
                       DWORD dwBufferSize)
{
    BATTERY_QUERY_INFORMATION BatteryQueryInfo;
    DWORD cbBytesReturned;

    BatteryQueryInfo.BatteryTag = GetBatteryTag(hHandle);
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
QueryBatteryStatus(HANDLE hHandle,
                         BATTERY_STATUS *lpBatteryStatus,
                         DWORD dwBufferSize)
{
    BATTERY_WAIT_STATUS BatteryWaitStatus = {0};
    DWORD cbBytesReturned;

    BatteryWaitStatus.BatteryTag = GetBatteryTag(hHandle);
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
