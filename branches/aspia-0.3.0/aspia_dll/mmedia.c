/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/mmedia.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


VOID
HW_MediaDevicesInfo(VOID)
{
    WAVEOUTCAPS waveOutputPaps;
    UINT DevsNum, i;

    DebugStartReceiving();

    IoAddIcon(IDI_AUDIO);
    IoAddIcon(IDI_MICROPHONE);

    DevsNum = waveOutGetNumDevs();
    if (DevsNum > 0)
    {
        for (i = 0; i < DevsNum; i++)
        {
            if (waveOutGetDevCaps(i,
                                  &waveOutputPaps,
                                  sizeof(waveOutputPaps)) != MMSYSERR_NOERROR)
            {
                continue;
            }

            IoAddItem(0, 0, waveOutputPaps.szPname);
        }
    }

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
