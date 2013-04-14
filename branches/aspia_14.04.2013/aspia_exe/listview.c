/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/listview.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"

INT
ListViewAddItem(INT Indent, INT IconIndex, LPWSTR lpText)
{
    INT ItemIndex = ListView_GetItemCount(hListView);
    LV_ITEM Item = {0};

    Item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_INDENT | LVIF_PARAM;
    Item.pszText = lpText;
    Item.iItem = ItemIndex;
    Item.iImage = IconIndex;
    Item.iIndent = Indent;
    Item.lParam = ItemIndex;

    return ListView_InsertItem(hListView, &Item);
}

VOID
ListViewSetItemText(INT i, INT iSubItem, LPWSTR pszText)
{
    ListView_SetItemText(hListView, i, iSubItem, pszText);
}

INT
ListViewAddHeaderString(INT Indent, LPWSTR lpszText, INT IconIndex)
{
    LV_ITEM Item = {0};
    INT Index;

    Item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_INDENT;
    Item.pszText = lpszText;
    Item.iItem = ListView_GetItemCount(hListView);
    Item.iImage = IconIndex;
    Item.iIndent = Indent;

    Index = ListView_InsertItem(hListView, &Item);

    ListViewSetItemText(Index, 1, L"\0");
    return Index;
}

VOID
ListViewAddHeader(UINT StringID, INT IconIndex)
{
    WCHAR szText[MAX_STR_LEN];

    LoadMUIStringF(hLangInst, StringID, szText, MAX_STR_LEN);
    ListViewAddHeaderString(0, szText, IconIndex);
}

INT
ListViewAddValueName(LPWSTR lpszName, INT IconIndex)
{
    return ListViewAddItem(0, IconIndex, lpszName);
}

INT
ListViewAddImageListIcon(UINT IconID)
{
    HICON hIcon;
    INT Index;

    hIcon = (HICON)LoadImage(hIconsInst,
                             MAKEINTRESOURCE(IconID),
                             IMAGE_ICON,
                             ParamsInfo.SxSmIcon,
                             ParamsInfo.SySmIcon,
                             LR_CREATEDIBSECTION);

    if (!hIcon) return -1;

    Index = ImageList_AddIcon(hListViewImageList, hIcon);
    DestroyIcon(hIcon);
    return Index;
}

VOID
ListViewAddColumn(SIZE_T Index, INT Width, LPWSTR lpszText)
{
    LV_COLUMN Column = {0};

    Column.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    Column.iSubItem = Index;
    Column.pszText = lpszText;
    Column.cx  = Width;
    Column.fmt = LVCFMT_LEFT;

    ListView_InsertColumn(hListView, Index, &Column);
}
