/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/passwords/ras.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../../main.h"

#include <ras.h>
#include <raserror.h>
#include <ntsecapi.h>
#include <sddl.h>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)


BOOL
GetLSASid(LPWSTR lpOutSid, SIZE_T Size)
{
    DWORD UserNameSize, SidSize, DomainSize;
    WCHAR UserName[MAX_STR_LEN];
    LPWSTR pDomain, pSidStr;
    SID_NAME_USE Snu;
    PSID pSid;
    BOOL Result = FALSE;

    UserNameSize = MAX_STR_LEN;
    if (!GetUserName(UserName, &UserNameSize))
        return FALSE;

    SidSize = 0;
    DomainSize = 0;

    LookupAccountName(NULL, UserName, NULL, &SidSize, NULL, &DomainSize, &Snu);

    pDomain = (LPWSTR)Alloc(DomainSize * sizeof(WCHAR));
    pSid = (PSID)Alloc(SidSize);
    if (!pDomain || !pSid)
        return FALSE;

    if (!LookupAccountName(NULL, UserName, pSid, &SidSize, pDomain, &DomainSize, &Snu))
        goto Exit;

    if (!IsValidSid(pSid))
        goto Exit;

    Result = ConvertSidToStringSid(pSid, &pSidStr);

    StringCbCopy(lpOutSid, Size, pSidStr);

Exit:
    Free(pDomain);
    Free(pSid);

    return Result;
}

VOID
InitUnicodeString(IN OUT PLSA_UNICODE_STRING DestinationString,
                  IN PCWSTR SourceString)
{
    ULONG DestSize;

    if (SourceString)
    {
        DestSize = wcslen(SourceString) * sizeof(WCHAR);
        DestinationString->Length = (USHORT)DestSize;
        DestinationString->MaximumLength = (USHORT)DestSize + sizeof(WCHAR);
    }
    else
    {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
    }

    DestinationString->Buffer = (PWSTR)SourceString;
}

BOOL
GetLSAData(LPWSTR lpKeyName, PLSA_UNICODE_STRING *lpOutData)
{
    LSA_OBJECT_ATTRIBUTES LsaObjectAttributes = {0};
    LSA_HANDLE LsaHandle;
    NTSTATUS Status;
    LSA_UNICODE_STRING KeyName;
    SIZE_T KeySize = (SafeStrLen(lpKeyName) + 1) * sizeof(WCHAR);

    KeyName.Buffer = (PWSTR)Alloc(KeySize);
    if (!KeyName.Buffer)
        return FALSE;

    InitUnicodeString(&KeyName, lpKeyName);

    Status = LsaOpenPolicy(NULL,
                           &LsaObjectAttributes,
                           POLICY_GET_PRIVATE_INFORMATION,
                           &LsaHandle);
    if (!NT_SUCCESS(Status))
        return FALSE;

    Status = LsaRetrievePrivateData(LsaHandle, &KeyName, lpOutData);

    LsaClose(&LsaHandle);
    Free(KeyName.Buffer);

    if (!NT_SUCCESS(Status))
        return FALSE;

    return TRUE;
}

BOOL
GetPasswordByUID(PLSA_UNICODE_STRING Data,
                 DWORD dwUID,
                 LPWSTR lpPassword,
                 SIZE_T PassSize)
{
    DWORD len = 0, cur = 0, off = 0;
    WCHAR szUid[MAX_STR_LEN];
    BOOL Result = FALSE;
    char *p;

    lpPassword[0] = 0;

    p = (char*)Data->Buffer;

    while (off < Data->Length)
    {
        if ((cur % 9) == 0)
        {
            len = cur / 9;

            if (SafeStrLen((PWSTR)(p + off)) == 0) break;

            StringCbPrintf(szUid, PassSize, L"%ws", p + off);
            if (dwUID == (DWORD)_wtol(szUid))
                Result = TRUE;
        }

        if ((((cur - 6) % 9) == 0) && (((cur - 6) / 9) == len) && Result)
        {
            StringCbPrintf(lpPassword, PassSize, L"%ws", p + off);
            return TRUE;
        }

        off += (SafeStrLen((PWSTR)(p + off)) + 1) * 2;
        ++cur;
    }

    return FALSE;
}

VOID
SOFTWARE_PasswdRASInfo(VOID)
{
    DWORD dwIndex, dwErr, dwEntries, dwCb = 0;
    WCHAR szText[MAX_STR_LEN];
    RASENTRYNAME *pRasEntryName;
    WCHAR Book1[MAX_PATH], Book2[MAX_PATH];
    RASENTRY RasEntry;
    DWORD DialParamsUID;
    WCHAR szKeyName[MAX_STR_LEN], szSid[MAX_STR_LEN];
    PLSA_UNICODE_STRING Data1, Data2;
    RASDIALPARAMS RasDialParams;
    BOOL IsPasswordReturned;
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_RAS);

    if (!GetLSASid(szSid, sizeof(szSid)))
        return;

    StringCbPrintf(szKeyName, sizeof(szKeyName), L"RasDialParams!%s#0", szSid);

    GetLSAData(L"L$_RasDefaultCredentials#0", &Data1);
    GetLSAData(szKeyName, &Data2);

    SHGetSpecialFolderPath(NULL, Book1, CSIDL_APPDATA, FALSE);
    StringCbCat(Book1, sizeof(Book1), L"\\Microsoft\\Network\\Connections\\pbk\\rasphone.pbk");

    SHGetSpecialFolderPath(NULL, Book2, CSIDL_COMMON_APPDATA, FALSE);
    StringCbCat(Book2, sizeof(Book2), L"\\Microsoft\\Network\\Connections\\pbk\\rasphone.pbk");

    dwErr = RasEnumEntries(NULL, NULL, NULL,
                           &dwCb, &dwEntries);
    if (dwErr != ERROR_BUFFER_TOO_SMALL)
        return;

    pRasEntryName = (RASENTRYNAME*)Alloc(dwCb);
    if (!pRasEntryName)
        return;

    pRasEntryName->dwSize = sizeof(RASENTRYNAME);
    dwErr = RasEnumEntries(NULL, NULL,
                           pRasEntryName,
                           &dwCb, &dwEntries);
    if (dwErr != ERROR_SUCCESS)
    {
        Free(pRasEntryName);
        return;
    }

    for (dwIndex = 0; dwIndex < dwEntries; ++dwIndex)
    {
        RasEntry.dwSize = sizeof(RASENTRY);
        dwCb = sizeof(RASENTRY);

        dwErr = RasGetEntryProperties(pRasEntryName[dwIndex].szPhonebookPath,
                                      pRasEntryName[dwIndex].szEntryName,
                                      &RasEntry,
                                      &dwCb,
                                      NULL, NULL);
        if (dwErr != ERROR_SUCCESS)
            continue;

        ZeroMemory(&RasDialParams, sizeof(RASDIALPARAMS));
        RasDialParams.dwSize = sizeof(RASDIALPARAMS);
        StringCbCopy(RasDialParams.szEntryName,
                     sizeof(RasDialParams.szEntryName),
                     pRasEntryName[dwIndex].szEntryName);

        dwErr = RasGetEntryDialParams(pRasEntryName[dwIndex].szPhonebookPath,
                                      &RasDialParams,
                                      &IsPasswordReturned);
        if (dwErr != ERROR_SUCCESS)
            continue;

        DialParamsUID =
            GetPrivateProfileInt(pRasEntryName[dwIndex].szEntryName,
                                 L"DialParamsUID", 0, Book1);

        if (DialParamsUID == 0)
        {
            DialParamsUID =
                GetPrivateProfileInt(pRasEntryName[dwIndex].szEntryName,
                                     L"DialParamsUID", 0, Book2);

            if (DialParamsUID == 0)
                continue;
        }

        IoAddHeaderString(pRasEntryName[dwIndex].szEntryName, 0);

        Index = IoAddValueName(IDS_PASSWD_RAS_TYPE, -1);
        IoSetItemText(Index, 1, RasEntry.szDeviceType);

        if (SafeStrLen(RasEntry.szLocalPhoneNumber) > 0)
        {
            if (wcscmp(RasEntry.szDeviceType, L"vpn") == 0)
                Index = IoAddValueName(IDS_NETWORK_RAS_SERVER, -1);
            else
                Index = IoAddValueName(IDS_PASSWD_RAS_PHONE_NUMBER, -1);

            IoSetItemText(Index, 1, RasEntry.szLocalPhoneNumber);
        }

        if (SafeStrLen(RasEntry.szAreaCode) > 0)
        {
            Index = IoAddValueName(IDS_PASSWD_RAS_AREA_CODE, -1);
            IoSetItemText(Index, 1, RasEntry.szAreaCode);
        }

        Index = IoAddValueName(IDS_PASSWD_RAS_USERNAME, -1);
        IoSetItemText(Index, 1, RasDialParams.szUserName);

        Index = IoAddValueName(IDS_PASSWD_RAS_PASSWORD, -1);
        if (!GetPasswordByUID(Data1, DialParamsUID, szText, sizeof(szText)))
            GetPasswordByUID(Data2, DialParamsUID, szText, sizeof(szText));
        IoSetItemText(Index, 1, szText);

        IoAddFooter();
    }

    LsaFreeMemory(Data1);
    LsaFreeMemory(Data2);
    Free(pRasEntryName);

    DebugEndReceiving();
}
