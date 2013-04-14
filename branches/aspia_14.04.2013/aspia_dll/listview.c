/*
 * PROJECT:         Aspia
 * FILE:            aspia_dll/listview.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"


VOID
ListViewSetItemParam(HWND hList, INT Index, LPARAM lParam)
{
    LV_ITEM Item = {0};

    Item.mask = LVIF_PARAM;
    Item.iItem = Index;
    Item.lParam = lParam;

    ListView_SetItem(hList, &Item);
}

PVOID
ListViewGetlParam(HWND hList, INT Index)
{
    INT ItemIndex;
    LVITEM Item = {0};

    if (Index == -1)
    {
        ItemIndex = ListView_GetNextItem(hList, -1, LVNI_FOCUSED);
        if (ItemIndex == -1)
            return NULL;
    }
    else
    {
        ItemIndex = Index;
    }

    Item.mask = LVIF_PARAM;
    Item.iItem = ItemIndex;
    if (!ListView_GetItem(hList, &Item))
        return NULL;

    return (PVOID)Item.lParam;
}
