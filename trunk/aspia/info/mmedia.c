/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/hw.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include "driver.h"


VOID
HW_MediaDevicesInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_AUDIO);
    IoAddIcon(IDI_MICROPHONE);

    DebugEndReceiving();
}

VOID
HW_MediaACodecsInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_AUDIO);

    DebugEndReceiving();
}

VOID
HW_MediaVCodecsInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_AUDIO);

    DebugEndReceiving();
}

VOID
HW_MediaMCIInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_AUDIO);

    DebugEndReceiving();
}
