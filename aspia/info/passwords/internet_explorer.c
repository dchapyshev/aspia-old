/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/passwords/internet_explorer.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../../main.h"

#include <wininet.h>


VOID
GetHashStr(WCHAR *Input, WCHAR *Hash, SIZE_T HashSize)
{
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    INT i;

    Hash[0] = 0;

    CryptAcquireContext(&hProv, 0, 0, PROV_RSA_FULL, 0);

    if (CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
    {
        if (CryptHashData(hHash,(UCHAR *)Input,(lstrlenW(Input) + 1)*2, 0))
        {
            BYTE Buffer[20];
            DWORD Length = 20;

            if (CryptGetHashParam(hHash, HP_HASHVAL, Buffer, &Length, 0))
            {
                WCHAR Temporary[128];
                UCHAR Tail = 0;

                CryptDestroyHash(hHash);
                CryptReleaseContext(hProv, 0);

                for (i = 0; i < 20; i++)
                {
                    UCHAR c = Buffer[i];
                    Tail += c;
                    StringCbPrintf(Temporary, sizeof(Temporary), L"%s%2.2X", Hash, c);
                    StringCbCopy(Hash, HashSize, Temporary);
                }
                StringCbPrintf(Temporary, sizeof(Temporary), L"%s%2.2X", Hash, Tail);
                StringCbCopy(Hash, HashSize, Temporary);
            }
        }
    }
}

VOID
ShowIEPasswords(HKEY hKey)
{
    LPINTERNET_CACHE_ENTRY_INFO pCacheInfo = NULL;
    DWORD dwEntrySize = 0;
    HANDLE hHandle = NULL;
    INT Index;

    FindFirstUrlCacheEntry(0, 0, &dwEntrySize);

    pCacheInfo = (LPINTERNET_CACHE_ENTRY_INFO)Alloc(dwEntrySize);
    if (!pCacheInfo) return;

    pCacheInfo->dwStructSize = dwEntrySize;

    hHandle = FindFirstUrlCacheEntry(0, pCacheInfo, &dwEntrySize);
    if (!hHandle) goto Exit;

    do
    {
        if (pCacheInfo->CacheEntryType == (URLHISTORY_CACHE_ENTRY | NORMAL_CACHE_ENTRY))
        {
            WCHAR szHash[MAX_STR_LEN], szURL[MAX_STR_LEN * 2] = {0};
            DWORD dwBufferSize = 0;
            BYTE *pBuffer;
            INT i, j = 0;

            for (i = 0; i < SafeStrLen(pCacheInfo->lpszSourceUrlName); i++)
                if (pCacheInfo->lpszSourceUrlName[i] == L'@') break;

            for (i++; i < SafeStrLen(pCacheInfo->lpszSourceUrlName); i++)
                szURL[j++] = pCacheInfo->lpszSourceUrlName[i];

            szURL[j] = L'/';

            GetHashStr(szURL, szHash, sizeof(szHash));
                    //Index = IoAddItem(0, szURL);
                    //IoSetItemText(Index, 1, szHash);
            RegQueryValueEx(hKey, szHash, 0, 0, 0, &dwBufferSize);

            pBuffer = (BYTE*)Alloc(dwBufferSize);
            if (pBuffer)
            {
                if (RegQueryValueEx(hKey, szHash, 0, 0, pBuffer, &dwBufferSize) == ERROR_SUCCESS)
                {
                    Index = IoAddItem(0, szURL);
                    IoSetItemText(Index, 1, szHash);
                }
                Free(pBuffer);
            }
        }

        Free(pCacheInfo);

        dwEntrySize = 0;
        FindNextUrlCacheEntry(hHandle, 0, &dwEntrySize);

        pCacheInfo = (LPINTERNET_CACHE_ENTRY_INFO)Alloc(dwEntrySize);
        if (!pCacheInfo) break;
    }
    while (FindNextUrlCacheEntry(hHandle, pCacheInfo, &dwEntrySize)); 

Exit:
    if (hHandle) FindCloseUrlCache(hHandle);
    if (pCacheInfo) Free(pCacheInfo);
}

VOID
SOFTWARE_PasswdIEInfo(VOID)
{
    HKEY hKey = NULL;

    IoAddIcon(IDI_IE);

    if (RegCreateKeyEx(HKEY_CURRENT_USER,
                       L"Software\\Microsoft\\Internet Explorer\\IntelliForms\\Storage1",
                       0, 0, REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS,
                       NULL, &hKey, 0) == ERROR_SUCCESS)
    {
        ShowIEPasswords(hKey);
        RegCloseKey(hKey);
    }

    if (RegCreateKeyEx(HKEY_CURRENT_USER,
                       L"Software\\Microsoft\\Internet Explorer\\IntelliForms\\Storage2",
                       0, 0, REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS,
                       NULL, &hKey, 0) == ERROR_SUCCESS)
    {
        ShowIEPasswords(hKey);
        RegCloseKey(hKey);
    }
}
