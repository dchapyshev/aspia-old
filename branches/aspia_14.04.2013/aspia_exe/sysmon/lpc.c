/*
 * PROJECT:         Aspia System Monitor
 * FILE:            aspia_exe/sysmon/lpc.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include <aspia.h>
#include "../aspia_exe.h"
#include "sysmon.h"


static CRITICAL_SECTION LpcCriticalSection;


BOOL
InitLpcSensors(VOID)
{
    if (!ParamsInfo.IsDriverInitialized)
        return FALSE;

    InitializeCriticalSection(&LpcCriticalSection);

    return FALSE;
}

VOID
EnumLpcSensors(PLPC_SENSORS_ENUMPROC lpEnumProc)
{
    if (!TryEnterCriticalSection(&LpcCriticalSection))
        return;

    LeaveCriticalSection(&LpcCriticalSection);
}

VOID
FreeLpcSensors(VOID)
{
    DeleteCriticalSection(&LpcCriticalSection);
}
