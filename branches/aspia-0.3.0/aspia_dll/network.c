/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/network.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)


static VOID
CardsTypeToText(DWORD Type, LPWSTR Str, SIZE_T Size)
{
    LPWSTR lpszValue;

    switch (Type)
    {
        case IF_TYPE_OTHER:
            lpszValue = L"Other";
            break;
        case IF_TYPE_ETHERNET_CSMACD:
            lpszValue = L"Ethernet";
            break;
        case IF_TYPE_ISO88025_TOKENRING:
            lpszValue = L"Token Ring";
            break;
        case IF_TYPE_PPP:
            lpszValue = L"PPP";
            break;
        case IF_TYPE_SOFTWARE_LOOPBACK:
            lpszValue = L"Software Lookback";
            break;
        case IF_TYPE_ATM:
            lpszValue = L"ATM";
            break;
        case IF_TYPE_IEEE80211:
            lpszValue = L"IEEE 802.11 Wireless";
            break;
        case IF_TYPE_TUNNEL:
            lpszValue = L"Tunnel type encapsulation";
            break;
        case IF_TYPE_IEEE1394:
            lpszValue = L"IEEE 1394 Firewire";
            break;
        default:
            lpszValue = L"Unknown";
            break;
    }
    StringCbCopy(Str, Size, lpszValue);
}

BOOL
GetAdapterFriendlyName(LPWSTR lpszKey, LPWSTR lpszName, INT NameLen)
{
    SP_DEVINFO_DATA DeviceInfoData = {0};
    WCHAR DeviceID[MAX_PATH], AdapterID[MAX_PATH];
    HDEVINFO hDevInfo;
    INT DeviceIndex = 0;
    BOOL Result = FALSE;

    hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_NET,
                                   NULL,
                                   NULL,
                                   DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"SetupDiGetClassDevs() failed!");
        return FALSE;
    }

    if (!GetStringFromRegistry(TRUE,
                               HKEY_LOCAL_MACHINE,
                               lpszKey, L"PnpInstanceID",
                               AdapterID,
                               MAX_PATH))
    {
        DebugTrace(L"GetStringFromRegistry() failed!");
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return FALSE;
    }

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(hDevInfo,
                                 DeviceIndex,
                                 &DeviceInfoData))
    {
        ++DeviceIndex;

        if (!SetupDiGetDeviceInstanceId(hDevInfo,
                                        &DeviceInfoData,
                                        DeviceID,
                                        MAX_PATH,
                                        NULL))
        {
            DebugTrace(L"SetupDiGetDeviceInstanceId() failed! Error code = 0x%x",
                       GetLastError());
            continue;
        }

        if (wcscmp(DeviceID, AdapterID) == 0)
        {
            /* get the device's friendly name */
            if (!SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_FRIENDLYNAME,
                                                  0,
                                                  (BYTE*)lpszName,
                                                  NameLen,
                                                  NULL))
            {
                DebugTrace(L"SetupDiGetDeviceRegistryProperty() failed! Error code = 0x%x",
                           GetLastError());

                /* if the friendly name fails, try the description instead */
                if (SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                     &DeviceInfoData,
                                                     SPDRP_DEVICEDESC,
                                                     0,
                                                     (BYTE*)lpszName,
                                                     NameLen,
                                                     NULL))
                {
                    Result = TRUE;
                }
                else
                {
                    DebugTrace(L"SetupDiGetDeviceRegistryProperty() failed! Error code = 0x%x",
                               GetLastError());
                }
            }
            else
            {
                Result = TRUE;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return Result;
}

VOID
NETWORK_CardsInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szKey[MAX_PATH],
          szNo[MAX_STR_LEN];
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    PIP_PER_ADAPTER_INFO pPerInfo;
    ULONG ulOutBufLen, PerLen;
    INT Count = 0;
    MIB_IFROW *pIfRow;

    DebugStartReceiving();

    IoAddIcon(IDI_HW);
    IoAddIcon(IDI_NETWORK);

    LoadMUIString(IDS_NO, szNo, MAX_STR_LEN);

    pAdapterInfo = (PIP_ADAPTER_INFO)Alloc(sizeof(IP_ADAPTER_INFO));
    if (!pAdapterInfo)
    {
        DebugTrace(L"Alloc(%d) failed!", sizeof(IP_ADAPTER_INFO));
        goto Cleanup;
    }

    ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        Free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)Alloc(ulOutBufLen);
        if (!pAdapterInfo)
        {
            DebugTrace(L"Alloc(%d) failed!", ulOutBufLen);
            goto Cleanup;
        }
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != NOERROR)
    {
        DebugTrace(L"GetAdaptersInfo() failed!");
        goto Cleanup;
    }

    pAdapter = pAdapterInfo;

    while (pAdapter)
    {
        StringCbPrintf(szKey, sizeof(szKey),
                       L"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%S\\Connection",
                       pAdapter->AdapterName);

        StringCbPrintf(szText, sizeof(szText), L"%S", pAdapter->Description);
        if (szText[0] == 0)
        {
            if (!GetAdapterFriendlyName(szKey, szText, MAX_STR_LEN))
                StringCbCopy(szText, sizeof(szText), L"Unknown Adapter");
        }

        DebugTrace(L"Adapter Name: %s", szText);

        IoAddHeaderString(0, 0, szText);

        if (GetStringFromRegistry(TRUE,
                                  HKEY_LOCAL_MACHINE,
                                  szKey, L"Name",
                                  szText, MAX_STR_LEN))
        {
            IoAddValueName(1, 0, IDS_NIC_NAME);
            IoSetItemText(szText);
        }

        pIfRow = (MIB_IFROW*)Alloc(sizeof(MIB_IFROW));
        if (!pIfRow)
        {
            DebugTrace(L"Alloc(%d) failed!", sizeof(MIB_IFROW));
            goto Cleanup;
        }

        pIfRow->dwIndex = pAdapter->Index;
        if (GetIfEntry(pIfRow) != NO_ERROR)
        {
            DebugTrace(L"GetIfEntry() failed!");
            Free(pIfRow);
            goto Cleanup;
        }

        IoAddValueName(1, 0, IDS_NIC_TYPE);
        CardsTypeToText(pIfRow->dwType, szText, sizeof(szText));
        IoSetItemText(szText);

        IoAddValueName(1, 0, IDS_NIC_MTU);
        IoSetItemText(L"%ld byte", pIfRow->dwMtu);

        IoAddValueName(1, 0, IDS_NIC_SPEED);
        IoSetItemText(L"%ld Mbps", pIfRow->dwSpeed / (1000 * 1000));

        IoAddValueName(1, 0, IDS_NIC_MAC);
        IoSetItemText(L"%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
                      pIfRow->bPhysAddr[0],
                      pIfRow->bPhysAddr[1],
                      pIfRow->bPhysAddr[2],
                      pIfRow->bPhysAddr[3],
                      pIfRow->bPhysAddr[4],
                      pIfRow->bPhysAddr[5],
                      pIfRow->bPhysAddr[6]);

        IoAddValueName(1, 1, IDS_NIC_IP);
        if (pAdapter->IpAddressList.IpAddress.String[0] == '0')
        {
            LoadMUIString(IDS_NOT_CONNECTED, szText, MAX_STR_LEN);
        }
        else
        {
            StringCbPrintf(szText, sizeof(szText), L"%S",
                           pAdapter->IpAddressList.IpAddress.String);
        }
        IoSetItemText(szText);

        if (pAdapter->IpAddressList.IpMask.String[0] != '0')
        {
            IoAddValueName(1, 1, IDS_NIC_SUBNET_MASK);
            IoSetItemText(L"%S",
                          pAdapter->IpAddressList.IpMask.String);
        }

        if (pAdapter->GatewayList.IpAddress.String[0] != 0)
        {
            IoAddValueName(1, 1, IDS_NIC_GETEWAY);
            StringCbPrintf(szText, sizeof(szText), L"%S",
                           pAdapter->GatewayList.IpAddress.String);
            IoSetItemText(L"%S", pAdapter->GatewayList.IpAddress.String);
        }

        pPerInfo = (PIP_PER_ADAPTER_INFO)Alloc(sizeof(IP_PER_ADAPTER_INFO));
        if (!pPerInfo)
        {
            DebugTrace(L"Alloc(%d) failed!", sizeof(IP_PER_ADAPTER_INFO));
            goto Cleanup;
        }

        PerLen = sizeof(IP_PER_ADAPTER_INFO);

        if (GetPerAdapterInfo(pAdapter->Index, pPerInfo, &PerLen) ==
            ERROR_BUFFER_OVERFLOW)
        {
            Free(pPerInfo);
            pPerInfo = (PIP_PER_ADAPTER_INFO)Alloc(PerLen);
            if (!pPerInfo)
            {
                DebugTrace(L"Alloc(%d) failed!", PerLen);
                goto Cleanup;
            }
        }

        if (GetPerAdapterInfo(pAdapter->Index, pPerInfo, &PerLen) != NO_ERROR)
        {
            DebugTrace(L"GetPerAdapterInfo() failed!");
            Free(pPerInfo);
            goto Cleanup;
        }

        if (pPerInfo->DnsServerList.IpAddress.String[0] != '\0')
        {
            IoAddItem(1, 1, L"DNS 1");

            IoSetItemText(L"%S",
                          pPerInfo->DnsServerList.IpAddress.String);

            if (pPerInfo->DnsServerList.Next)
            {
                IoAddItem(1, 1, L"DNS 2");
                IoSetItemText(L"%S",
                              pPerInfo->DnsServerList.Next->IpAddress.String);
            }
        }

        if (pAdapter->DhcpEnabled)
        {
            IoAddValueName(1, 1, IDS_NIC_DHCP_STATUS);
            LoadMUIString(IDS_YES, szText, MAX_STR_LEN);
            IoSetItemText(szText);

            if (pAdapter->DhcpServer.IpAddress.String[0] != '\0')
            {
                IoAddValueName(1, 1, IDS_NIC_DHCP_ADDR);
                IoSetItemText(L"%S",
                              pAdapter->DhcpServer.IpAddress.String);

                if (TimeToString(pAdapter->LeaseObtained, szText, sizeof(szText)))
                {
                    IoAddValueName(1, 1, IDS_NIC_DHCP_OBTAINED);
                    IoSetItemText(szText);
                }

                if (TimeToString(pAdapter->LeaseExpires, szText, sizeof(szText)))
                {
                    IoAddValueName(1, 1, IDS_NIC_DHCP_EXPIRES);
                    IoSetItemText(szText);
                }
            }
        }
        else
        {
            IoAddValueName(1, 1, IDS_NIC_DHCP_STATUS);
            IoSetItemText(szNo);
        }

        if (pAdapter->HaveWins)
        {
            IoAddValueName(1, 1, IDS_NIC_WINS_STATUS);
            LoadMUIString(IDS_YES, szText, MAX_STR_LEN);
            IoSetItemText(szText);

            IoAddItem(1, 1, L"WINS 1");
            IoSetItemText(L"%S",
                          pAdapter->PrimaryWinsServer.IpAddress.String);

            if (pAdapter->SecondaryWinsServer.IpAddress.String[0] != 0)
            {
                IoAddItem(1, 1, L"WINS 2");
                IoSetItemText(L"%S",
                              pAdapter->SecondaryWinsServer.IpAddress.String);
            }
        }
        else
        {
            IoAddValueName(1, 1, IDS_NIC_WINS_STATUS);
            IoSetItemText(szNo);
        }

        IoAddFooter();

        pAdapter = pAdapter->Next;
        Free(pPerInfo);
        Free(pIfRow);
        ++Count;

        if (IsCanceled) break;
    }

Cleanup:
    if (pAdapterInfo) Free(pAdapterInfo);

    DebugEndReceiving();
}

static VOID
ShareTypeToText(DWORD ShareType, LPWSTR lpszText, SIZE_T Size)
{
    UINT uiID = 0;

    if (lpszText) lpszText[0] = 0;
    switch (ShareType)
    {
        case STYPE_DISKTREE:
            uiID = IDS_SHARE_DISKTREE;
            break;
        case STYPE_PRINTQ:
            uiID = IDS_SHARE_PRINTQ;
            break;
        case STYPE_DEVICE:
            uiID = IDS_SHARE_DEVICE;
            break;
        case STYPE_IPC:
            uiID = IDS_SHARE_IPC;
            break;
        case STYPE_SPECIAL:
            uiID = IDS_SHARE_SPECIAL;
            break;
        case STYPE_TEMPORARY:
            uiID = IDS_SHARE_TEMPORARY;
            break;
    }

    LoadMUIString(uiID, lpszText, (INT)Size);
}

static UINT
GetShareIconIndexByType(DWORD ShareType)
{
    switch (ShareType)
    {
        case STYPE_TEMPORARY:
        case STYPE_SPECIAL:
        case STYPE_DISKTREE:
        case STYPE_DEVICE:
            return 0;
        case STYPE_PRINTQ:
            return 1;
    }
    return 2;
}

VOID
NETWORK_SharedInfo(VOID)
{
    DWORD dwError = 0, dwTotal = 0, dwResume = 0, dwCount, dwSize;
    WCHAR szCompName[MAX_STR_LEN], szText[MAX_STR_LEN];
    NET_API_STATUS Status;
    PSHARE_INFO_502 pInfo, pPtr;

    DebugStartReceiving();

    IoAddIcon(IDI_SHARED); /* Folder */
    IoAddIcon(IDI_PRINTER); /* Printer */
    IoAddIcon(IDI_APPS); /* For IPC */

    dwSize = MAX_STR_LEN;
    if (!GetComputerName(szCompName, &dwSize))
    {
        DebugTrace(L"GetComputerName() failed! Error code 0x%x",
                   GetLastError());
        goto Cleanup;
    }

    do
    {
        Status = NetShareEnum(szCompName, 502,
                              (LPBYTE*)&pInfo,
                              (DWORD)-1, &dwError,
                              &dwTotal, &dwResume);

        if (Status == ERROR_SUCCESS || Status == ERROR_MORE_DATA)
        {
            pPtr = pInfo;

            for (dwCount = 1; dwCount <= dwError; ++dwCount)
            {
                /* Name */
                IoAddItem(0, GetShareIconIndexByType(pPtr->shi502_type),
                          pPtr->shi502_netname);

                /* Type */
                ShareTypeToText(pPtr->shi502_type,
                                szText,
                                MAX_STR_LEN);
                IoSetItemText((szText[0] != 0) ? szText : L"-");

                /* Description */
                IoSetItemText((SafeStrLen(pPtr->shi502_remark) > 1) ? pPtr->shi502_remark : L"-");

                /* Local path */
                IoSetItemText((SafeStrLen(pPtr->shi502_path) > 0) ? pPtr->shi502_path : L"-");

                /* Current uses */
                IoSetItemText(L"%ld", pPtr->shi502_current_uses);

                /* Maximum uses */
                if (pPtr->shi502_max_uses != -1)
                {
                    StringCbPrintf(szText, sizeof(szText),
                                   L"%ld", pPtr->shi502_max_uses);
                }
                else
                {
                    LoadMUIString(IDS_SHARE_UNLIMITED,
                                  szText, MAX_STR_LEN);
                }
                IoSetItemText(szText);

                ++pPtr;
            }
            NetApiBufferFree(pInfo);
        }

        if (IsCanceled) break;
    }
    while (Status == ERROR_MORE_DATA);

Cleanup:
    DebugEndReceiving();
}

static VOID
IpToStr(IN IN_ADDR in, LPWSTR p, SIZE_T size)
{
    WCHAR b[10];

    _itow(in.S_un.S_addr & 0xFF, b, 10);
    StringCbCopy(p, size, b);
    _itow((in.S_un.S_addr >> 8) & 0xFF, b, 10);
    StringCbCat(p, size, L".");
    StringCbCat(p, size, b);
    _itow((in.S_un.S_addr >> 16) & 0xFF, b, 10);
    StringCbCat(p, size, L".");
    StringCbCat(p, size, b);
    _itow((in.S_un.S_addr >> 24) & 0xFF, b, 10);
    StringCbCat(p, size, L".");
    StringCbCat(p, size, b);
}

VOID
NETWORK_RouteInfo(VOID)
{
    PMIB_IPFORWARDTABLE pIpForwardTable;
    DWORD dwSize = 0, dwRet = 0;
    WCHAR szText[MAX_STR_LEN];
    struct in_addr IpAddr;
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_ROUTE);

    pIpForwardTable = (MIB_IPFORWARDTABLE*)Alloc(sizeof(MIB_IPFORWARDTABLE));
    if (!pIpForwardTable)
    {
        DebugTrace(L"Alloc(%d) failed!", sizeof(MIB_IPFORWARDTABLE));
        goto Cleanup;
    }

    if (GetIpForwardTable(pIpForwardTable, &dwSize, 0) ==
        ERROR_INSUFFICIENT_BUFFER)
    {
        Free(pIpForwardTable);
        pIpForwardTable = (MIB_IPFORWARDTABLE*)Alloc(dwSize);
        if (!pIpForwardTable)
        {
            DebugTrace(L"Alloc(%d) failed!", dwSize);
            goto Cleanup;
        }
    }

    dwRet = GetIpForwardTable(pIpForwardTable, &dwSize, 0);

    if (dwRet != NO_ERROR)
    {
        DebugTrace(L"GetIpForwardTable() failed! Error code = 0x%x", dwRet);
        goto Cleanup;
    }

    for (Index = 0; Index < (INT)pIpForwardTable->dwNumEntries; ++Index)
    {
        IpAddr.S_un.S_addr =
            (u_long)pIpForwardTable->table[Index].dwForwardDest;
        IpToStr(IpAddr, szText, sizeof(szText));
        IoAddItem(0, 0, szText);

        IpAddr.S_un.S_addr =
            (u_long)pIpForwardTable->table[Index].dwForwardMask;
        IpToStr(IpAddr, szText, sizeof(szText));
        IoSetItemText(szText);

        IpAddr.S_un.S_addr =
            (u_long)pIpForwardTable->table[Index].dwForwardNextHop;
        IpToStr(IpAddr, szText, sizeof(szText));
        IoSetItemText(szText);

        IoSetItemText(L"%ld",
                      pIpForwardTable->table[Index].dwForwardMetric1);

        if (IsCanceled) break;
    }

Cleanup:
    if (pIpForwardTable != NULL)
        Free(pIpForwardTable);

    DebugEndReceiving();
}

static VOID
ShowIERegInfo(UINT StringID,
              LPWSTR lpszPath,
              LPWSTR lpszKeyName,
              INT IconIndex)
{
    WCHAR szText[MAX_STR_LEN];

    if (GetStringFromRegistry(TRUE,
                              HKEY_CURRENT_USER,
                              lpszPath,
                              lpszKeyName,
                              szText,
                              MAX_STR_LEN))
    {
        IoAddValueName(1, IconIndex, StringID);
        IoSetItemText(L"%s", szText);
    }
}

VOID
ShowIEShortInfo(INT IconIndex)
{
    INTERNET_PER_CONN_OPTION_LIST Info;
    INTERNET_PER_CONN_OPTION Option[2];
    WCHAR szText[MAX_STR_LEN];
    DWORD dwSize;

    if (GetStringFromRegistry(TRUE,
                              HKEY_LOCAL_MACHINE,
                              L"Software\\Microsoft\\Internet Explorer",
                              L"Version", szText,
                              MAX_STR_LEN))
    {
        IoAddValueName(1, IconIndex, IDS_VERSION);
        IoSetItemText(szText);
    }

    ShowIERegInfo(IDS_IE_STARTPAGE,
                  L"Software\\Microsoft\\Internet Explorer\\Main",
                  L"Start Page",
                  IconIndex);

    Option[0].dwOption = INTERNET_PER_CONN_FLAGS;
    Option[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;

    Info.dwSize = sizeof(Info);
    Info.pszConnection = NULL;
    Info.dwOptionCount = 2;
    Info.dwOptionError = 0;
    Info.pOptions = Option;

    dwSize = sizeof(Info);
    if (InternetQueryOption(NULL,
                            INTERNET_OPTION_PER_CONNECTION_OPTION,
                            &Info,
                            &dwSize))
    {
        IoAddValueName(1, IconIndex, IDS_IE_USE_PROXY);

        if (Option[0].Value.dwValue & PROXY_TYPE_PROXY)
        {
            LoadMUIString(IDS_YES, szText, MAX_STR_LEN);
            IoSetItemText(szText);
        }
        else
        {
            LoadMUIString(IDS_NO, szText, MAX_STR_LEN);
            IoSetItemText(szText);
        }

        if (SafeStrLen(Option[1].Value.pszValue) != 0)
        {
            IoAddValueName(1, IconIndex, IDS_IE_PROXY_ADDR);
            IoSetItemText(Option[1].Value.pszValue);
            GlobalFree(Option[1].Value.pszValue);
        }
    }
    else
    {
        DebugTrace(L"InternetQueryOption() failed! Error code = 0x%x",
                   GetLastError());
    }
}

VOID
NETWORK_IEParamsInfo(VOID)
{
    DebugStartReceiving();

    IoAddIcon(IDI_IE);

    IoAddHeader(0, 0, IDS_CAT_NETWORK_IE_PARAMS);

    ShowIEShortInfo(0);

    ShowIERegInfo(IDS_IE_DOWNLOADDIR,
                  L"Software\\Microsoft\\Internet Explorer",
                  L"Download Directory", 0);
    ShowIERegInfo(IDS_IE_SEARCHPAGE,
                  L"Software\\Microsoft\\Internet Explorer\\Main",
                  L"Search Page", 0);
    ShowIERegInfo(IDS_IE_LOCALPAGE,
                  L"Software\\Microsoft\\Internet Explorer\\Main",
                  L"Local Page", 0);

    DebugEndReceiving();
}

VOID
NETWORK_IEHistoryInfo(VOID)
{
    IUrlHistoryStg2 *History;
    IEnumSTATURL *EnumPtr;
    WCHAR szText[MAX_STR_LEN];
    STATURL StatUrl;
    ULONG uFetched;
    SYSTEMTIME SysTime;
    HRESULT hr;

    DebugStartReceiving();

    if (!DllParams.IEShowFile &&
        !DllParams.IEShowFtp &&
        !DllParams.IEShowHttp)
    {
        DebugTrace(L"Information not selected!");
        goto Cleanup;
    }

    IoAddIcon(IDI_IE);

    CoInitialize(NULL);

    hr = CoCreateInstance(&CLSID_CUrlHistory,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_IUrlHistoryStg2,
                          (LPVOID*)&History);
    if (FAILED(hr))
    {
        DebugTrace(L"CoCreateInstance() failed! hr = %ld", hr);
        goto Cleanup;
    }

    hr = History->lpVtbl->EnumUrls(History, &EnumPtr);
    if (FAILED(hr))
    {
        DebugTrace(L"EnumUrls() failed! hr = %ld", hr);
        goto Cleanup;
    }

    while (SUCCEEDED(EnumPtr->lpVtbl->Next(EnumPtr, 1, &StatUrl, &uFetched)))
    {
        if (!uFetched) break;

        if (SafeStrLen(StatUrl.pwcsUrl) < 8) continue;

        if ((wcsncmp(StatUrl.pwcsUrl, L"http://", 7) == 0 && DllParams.IEShowHttp) ||
            (wcsncmp(StatUrl.pwcsUrl, L"ftp://", 6) == 0 && DllParams.IEShowFtp) ||
            (wcsncmp(StatUrl.pwcsUrl, L"file://", 7) == 0 && DllParams.IEShowFile))
        {
            if (FileTimeToSystemTime(&StatUrl.ftLastVisited, &SysTime))
            {
                /* Time */
                if (GetTimeFormat(LOCALE_USER_DEFAULT,
                                  0, &SysTime, NULL, szText,
                                  MAX_STR_LEN))
                {
                    IoAddItem(0, 0, szText);
                }
                else
                {
                    IoAddItem(0, 0, L"-");
                }

                /* Date */
                if (GetDateFormat(LOCALE_USER_DEFAULT,
                                  0, &SysTime, NULL, szText,
                                  MAX_STR_LEN))
                {
                    IoSetItemText(szText);
                }
                else
                {
                    IoSetItemText(L"-");
                }
            }
            else
            {
                DebugTrace(L"FileTimeToSystemTime() failed! Error code = 0x%x",
                           GetLastError());

                IoAddItem(0, 0, L"-");
                IoSetItemText(L"-");
            }

            /* Title */
            IoSetItemText((SafeStrLen(StatUrl.pwcsTitle) > 1) ? StatUrl.pwcsTitle : L"-");
            /* URL */
            __try
            {
                if (wcslen(StatUrl.pwcsUrl) > 4)
                {
                    IoSetItemText(L"%s", StatUrl.pwcsUrl);
                }
                else
                {
                    IoSetItemText(L"-");
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                IoSetItemText(L"-");
            }
        }

        if (IsCanceled) break;
    }

Cleanup:
    if (History)
        History->lpVtbl->Release(History);
    CoUninitialize();

    DebugEndReceiving();
}

static VOID
GetDateTimeString(FILETIME FileTime, LPWSTR lpszDateTime, SIZE_T Size)
{
    WCHAR szTime[MAX_STR_LEN] = {0}, szDate[MAX_STR_LEN] = {0};
    SYSTEMTIME SysTime;

    if (!FileTimeToLocalFileTime(&FileTime, &FileTime))
    {
        DebugTrace(L"FileTimeToLocalFileTime() failed! Error code = 0x%x",
                   GetLastError());
    }
    else
    {
        if (FileTimeToSystemTime(&FileTime, &SysTime))
        {
            if (!GetTimeFormat(LOCALE_USER_DEFAULT,
                               0, &SysTime, NULL, szTime,
                               MAX_STR_LEN))
            {
                DebugTrace(L"GetTimeFormat() failed! Error code = 0x%x",
                           GetLastError());
            }

            if (!GetDateFormat(LOCALE_USER_DEFAULT,
                               0, &SysTime, NULL, szDate,
                               MAX_STR_LEN))
            {
                DebugTrace(L"GetDateFormat() failed! Error code = 0x%x",
                           GetLastError());
            }
        }
        else
        {
            DebugTrace(L"FileTimeToSystemTime() failed! Error code = 0x%x",
                       GetLastError());
        }
    }

    StringCbPrintf(lpszDateTime, Size, L"%s %s",
        (szDate[0] != 0) ? szDate : L"-", (szTime[0] != 0) ? szTime : L"-");
}

static VOID
IECookieAdd(LPINTERNET_CACHE_ENTRY_INFO pCacheInfo)
{
    WCHAR szText[MAX_STR_LEN];

    IoAddItem(0, 0, pCacheInfo->lpszSourceUrlName);

    GetDateTimeString(pCacheInfo->LastAccessTime,
                      szText, sizeof(szText));
    IoSetItemText(szText);

    IoSetItemText(L"%ld", pCacheInfo->dwHitRate);

    GetDateTimeString(pCacheInfo->LastModifiedTime,
                      szText, sizeof(szText));
    IoSetItemText(szText);

    GetDateTimeString(pCacheInfo->LastSyncTime,
                      szText, sizeof(szText));
    IoSetItemText(szText);

    GetDateTimeString(pCacheInfo->ExpireTime,
                      szText, sizeof(szText));
    IoSetItemText(szText);

    IoSetItemText(pCacheInfo->lpszLocalFileName);
}

VOID
NETWORK_IECookieInfo(VOID)
{
    LPINTERNET_CACHE_ENTRY_INFO pCacheInfo;
    DWORD dwEntrySize = sizeof(INTERNET_CACHE_ENTRY_INFO);
    HANDLE hHandle = NULL;

    DebugStartReceiving();

    IoAddIcon(IDI_IE);

    pCacheInfo = (LPINTERNET_CACHE_ENTRY_INFO)Alloc(dwEntrySize);
    if (!pCacheInfo)
    {
        DebugTrace(L"Alloc(%d) failed!", dwEntrySize);
        goto Cleanup;
    }

    pCacheInfo->dwStructSize = dwEntrySize;

    hHandle = FindFirstUrlCacheEntry(L"cookie:", pCacheInfo, &dwEntrySize);
    if (!hHandle)
    {
        Free(pCacheInfo);

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            goto Cleanup;

        pCacheInfo = (LPINTERNET_CACHE_ENTRY_INFO)Alloc(dwEntrySize);
        if (!pCacheInfo)
        {
            DebugTrace(L"Alloc(%d) failed!", dwEntrySize);
            goto Cleanup;
        }

        pCacheInfo->dwStructSize = dwEntrySize;
        hHandle = FindFirstUrlCacheEntry(L"cookie:", pCacheInfo, &dwEntrySize);
        if (!hHandle)
        {
            DebugTrace(L"FindFirstUrlCacheEntry() failed! Error code = 0x%x",
                       GetLastError());
            Free(pCacheInfo);
            goto Cleanup;
        }
    }

    IECookieAdd(pCacheInfo);
    Free(pCacheInfo);

    for (;;)
    {
        pCacheInfo = (LPINTERNET_CACHE_ENTRY_INFO)Alloc(dwEntrySize);
        if (!pCacheInfo)
        {
            DebugTrace(L"Alloc(%d) failed!", dwEntrySize);
            break;
        }

        if (!FindNextUrlCacheEntry(hHandle, pCacheInfo, &dwEntrySize))
        {
            Free(pCacheInfo);

            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                break;

            pCacheInfo = (LPINTERNET_CACHE_ENTRY_INFO)Alloc(dwEntrySize);
            if (!pCacheInfo)
            {
                DebugTrace(L"Alloc(%d) failed!", dwEntrySize);
                break;
            }

            if (!FindNextUrlCacheEntry(hHandle, pCacheInfo, &dwEntrySize))
            {
                DebugTrace(L"FindNextUrlCacheEntry() failed! Error code = 0x%x",
                           GetLastError());
                Free(pCacheInfo);
                break;
            }
        }

        IECookieAdd(pCacheInfo);
        Free(pCacheInfo);

        if (IsCanceled) break;
    }

Cleanup:
    if (hHandle != NULL)
        FindCloseUrlCache(hHandle);

    DebugEndReceiving();
}

VOID
RasAddParam(UINT StringID, BOOL State)
{
    WCHAR szText[MAX_STR_LEN];

    LoadMUIString(StringID, szText, MAX_STR_LEN);
    IoAddItem(1, State ? 1 : 2, szText);

    LoadMUIString(State ? IDS_YES : IDS_NO, szText, MAX_STR_LEN);
    IoSetItemText(szText);
}

VOID
NETWORK_RasInfo(VOID)
{
    DWORD dwErr, dwEntries, dwCb = 0;
    WCHAR szText[MAX_STR_LEN], szNo[MAX_STR_LEN];
    SIZE_T dwIndex;
    RASDIALPARAMS RasDialParams;
    RASENTRYNAME *pRasEntryName;
    BOOL IsPasswordReturned;
    RASENTRY RasEntry;

    DebugStartReceiving();

    IoAddIcon(IDI_RAS);
    IoAddIcon(IDI_CHECKED);
    IoAddIcon(IDI_UNCHECKED);

    LoadMUIString(IDS_NO, szNo, MAX_STR_LEN);

    dwErr = RasEnumEntries(NULL, NULL, NULL,
                           &dwCb, &dwEntries);
    if (dwErr != ERROR_BUFFER_TOO_SMALL)
    {
        DebugTrace(L"RasEnumEntries() failed! Error code = 0x%x", dwErr);
        return;
    }

    pRasEntryName = (RASENTRYNAME*)Alloc(dwCb);
    if (!pRasEntryName)
    {
        DebugTrace(L"Alloc(%d) failed!", dwCb);
        goto Cleanup;
    }

    pRasEntryName->dwSize = sizeof(RASENTRYNAME);
    dwErr = RasEnumEntries(NULL, NULL,
                           pRasEntryName,
                           &dwCb, &dwEntries);
    if (dwErr != ERROR_SUCCESS)
    {
        DebugTrace(L"RasEnumEntries() failed! Error code = 0x%x", dwErr);
        goto Cleanup;
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

        /* Connection name */
        IoAddHeaderString(0, 0, pRasEntryName[dwIndex].szEntryName);

        /* Device name */
        IoAddValueName(1, 0, IDS_RAS_DEVICE_NAME);
        IoSetItemText(RasEntry.szDeviceName);

        /* Device type */
        IoAddValueName(1, 0, IDS_RAS_DEVICE_TYPE);
        IoSetItemText(RasEntry.szDeviceType);

        /* Country Code */
        if (RasEntry.dwCountryCode > 0)
        {
            IoAddValueName(1, 0, IDS_RAS_COUNTRY_CODE);
            IoSetItemText(L"%ld", RasEntry.dwCountryCode);
        }

        /* Area code */
        if (SafeStrLen(RasEntry.szAreaCode) > 0)
        {
            IoAddValueName(1, 0, IDS_RAS_AREA_CODE);
            IoSetItemText(RasEntry.szAreaCode);
        }

        /* Local phone number */
        if (SafeStrLen(RasEntry.szLocalPhoneNumber) > 0)
        {
            if (wcscmp(RasEntry.szDeviceType, L"vpn") == 0)
                IoAddValueName(1, 0, IDS_NETWORK_RAS_SERVER);
            else
                IoAddValueName(1, 0, IDS_RAS_PHONE_NUMBER);

            IoSetItemText(RasEntry.szLocalPhoneNumber);
        }

        /* User name */
        IoAddValueName(1, 0, IDS_RAS_USERNAME);
        IoSetItemText((SafeStrLen(RasDialParams.szUserName) > 1) ? RasDialParams.szUserName : L"-");

        /* Domine */
        if (SafeStrLen(RasDialParams.szDomain) > 0)
        {
            IoAddValueName(1, 0, IDS_RAS_DOMAIN);
            IoSetItemText(RasDialParams.szDomain);
        }

        /* IP */
        IoAddValueName(1, 0, IDS_RAS_IP);
        if (RasEntry.ipaddr.a == 0 && RasEntry.ipaddr.b == 0 &&
            RasEntry.ipaddr.c == 0 && RasEntry.ipaddr.d == 0)
        {
            LoadMUIString(IDS_RAS_DINAMIC_ADDR, szText, MAX_STR_LEN);
        }
        else
        {
            StringCbPrintf(szText, sizeof(szText),
                           L"%d.%d.%d.%d",
                           RasEntry.ipaddr.a,
                           RasEntry.ipaddr.b,
                           RasEntry.ipaddr.c,
                           RasEntry.ipaddr.d);
        }
        IoSetItemText(szText);

        /* DNS */
        IoAddItem(1, 0, L"DNS 1");
        if (RasEntry.ipaddrDns.a == 0 && RasEntry.ipaddrDns.b == 0 &&
            RasEntry.ipaddrDns.c == 0 && RasEntry.ipaddrDns.d == 0)
        {
            LoadMUIString(IDS_RAS_DINAMIC_ADDR, szText, MAX_STR_LEN);
        }
        else
        {
            IoSetItemText(L"%d.%d.%d.%d",
                          RasEntry.ipaddrDns.a,
                          RasEntry.ipaddrDns.b,
                          RasEntry.ipaddrDns.c,
                          RasEntry.ipaddrDns.d);

            /* DNS Alt */
            IoAddItem(1, 0, L"DNS 2");
            StringCbPrintf(szText, sizeof(szText),
                           L"%d.%d.%d.%d",
                           RasEntry.ipaddrDnsAlt.a,
                           RasEntry.ipaddrDnsAlt.b,
                           RasEntry.ipaddrDnsAlt.c,
                           RasEntry.ipaddrDnsAlt.d);
        }
        IoSetItemText(szText);

        /* WINS */
        IoAddItem(1, 0, L"WINS 1");
        if (RasEntry.ipaddrWins.a == 0 && RasEntry.ipaddrWins.b == 0 &&
            RasEntry.ipaddrWins.c == 0 && RasEntry.ipaddrWins.d == 0)
        {
            LoadMUIString(IDS_RAS_DINAMIC_ADDR, szText, MAX_STR_LEN);
        }
        else
        {
            IoSetItemText(L"%d.%d.%d.%d",
                          RasEntry.ipaddrWins.a,
                          RasEntry.ipaddrWins.b,
                          RasEntry.ipaddrWins.c,
                          RasEntry.ipaddrWins.d);

            /* WINS Alt */
            IoAddItem(1, 0, L"WINS 2");
            StringCbPrintf(szText, sizeof(szText),
                           L"%d.%d.%d.%d",
                           RasEntry.ipaddrWinsAlt.a,
                           RasEntry.ipaddrWinsAlt.b,
                           RasEntry.ipaddrWinsAlt.c,
                           RasEntry.ipaddrWinsAlt.d);
        }
        IoSetItemText(szText);

        /* Framing protocol */
        IoAddValueName(1, 0, IDS_RAS_FRAMING_PROTOCOL);
        if (RasEntry.dwFramingProtocol == RASFP_Ppp)
        {
            StringCbCopy(szText, sizeof(szText), L"PPP");
        }
        else if (RasEntry.dwFramingProtocol == RASFP_Slip)
        {
            StringCbCopy(szText, sizeof(szText), L"SLIP");
        }
        else
        {
            StringCbCopy(szText, sizeof(szText), L"Unknown");
        }
        IoSetItemText(szText);

        /* Script */
        IoAddValueName(1, 0, IDS_RAS_SCRIPT);
        if (SafeStrLen(RasEntry.szScript) == 0)
        {
            IoSetItemText(szNo);
        }
        else
        {
            IoSetItemText(RasEntry.szScript);
        }

        /* IP Header compression */
        RasAddParam(IDS_RAS_IP_HEADER_COMPRES,
                    RasEntry.dwfOptions & RASEO_IpHeaderCompression);

        /* PPP LCP Extensions */
        RasAddParam(IDS_RAS_PPP_LCP_EXTENSIONS,
                    !(RasEntry.dwfOptions & RASEO_DisableLcpExtensions));

        /* SW Compression */
        RasAddParam(IDS_RAS_SW_COMPRESSION,
                    RasEntry.dwfOptions & RASEO_SwCompression);

        /* Require Encrypted password */
        RasAddParam(IDS_RAS_REQURE_ENCRYPT_PASS,
                    RasEntry.dwfOptions & RASEO_RequireEncryptedPw);

        /* Require MS Encrypted password */
        RasAddParam(IDS_RAS_REQURE_MS_ENCRYPT_PASS,
                    RasEntry.dwfOptions & RASEO_RequireMsEncryptedPw);

        /* Require Data Encryption */
        RasAddParam(IDS_RAS_REQURE_DATA_ENCRYPT,
                    RasEntry.dwfOptions & RASEO_RequireDataEncryption);

        /* Require EAP (Extensible Authentication Protocol) */
        RasAddParam(IDS_RAS_REQURE_EAP,
                    RasEntry.dwfOptions & RASEO_RequireEAP);

        /* Require PAP (Password Authentication Protocol) */
        RasAddParam(IDS_RAS_REQURE_PAP,
                    RasEntry.dwfOptions & RASEO_RequirePAP);

        /* Require SPAP (Shiva's Password Authentication Protocol) */
        RasAddParam(IDS_RAS_REQURE_SPAP,
                    RasEntry.dwfOptions & RASEO_RequireSPAP);

        /* Require CHAP (Challenge Handshake Authentication Protocol) */
        RasAddParam(IDS_RAS_REQURE_CHAP,
                    RasEntry.dwfOptions & RASEO_RequireCHAP);

        /* Require MSCHAP (Microsoft Challenge Handshake Authentication Protocol) */
        RasAddParam(IDS_RAS_REQURE_MSCHAP,
                    RasEntry.dwfOptions & RASEO_RequireMsCHAP);

        /* Require MSCHAP2 (Microsoft Challenge Handshake Authentication Protocol) */
        RasAddParam(IDS_RAS_REQURE_MSCHAP2,
                    RasEntry.dwfOptions & RASEO_RequireMsCHAP2);

        IoAddFooter();

        if (IsCanceled) break;
    }

Cleanup:
    if (pRasEntryName != NULL)
        Free(pRasEntryName);

    DebugEndReceiving();
}

VOID
NETWORK_OpenFilesInfo(VOID)
{
    DWORD dwError = 0, dwTotal = 0, dwResume = 0, dwCount;
    NET_API_STATUS Status;
    PFILE_INFO_3 pInfo, pPtr;

    DebugStartReceiving();

    IoAddIcon(IDI_SHARED);

    do
    {
        Status = NetFileEnum(NULL, NULL, NULL, 3,
                             (LPBYTE*)&pInfo,
                             (DWORD)-1, &dwError,
                             &dwTotal, &dwResume);

        if (Status == ERROR_SUCCESS || Status == ERROR_MORE_DATA)
        {
            pPtr = pInfo;

            for (dwCount = 1; dwCount <= dwError; ++dwCount)
            {
                /* ID */
                IoAddItem(0, 0, L"%ld", pPtr->fi3_id);

                /* User Name */
                IoSetItemText(pPtr->fi3_username);

                /* Locks Count */
                IoSetItemText(L"%ld", pPtr->fi3_num_locks);

                /* File Path */
                IoSetItemText(pPtr->fi3_pathname);

                ++pPtr;
            }
            NetApiBufferFree(pInfo);
        }

        if (IsCanceled) break;
    }
    while (Status == ERROR_MORE_DATA);

    DebugEndReceiving();
}

VOID
NETWORK_FirewallInfo(VOID)
{
    INetFwAuthorizedApplications *fwApps = NULL;
    INetFwAuthorizedApplication *fwApp = NULL;
    INetFwProfile *fwProfile = NULL;
    INetFwPolicy *fwPolicy = NULL;
    INetFwMgr *fwMgr = NULL;
    IEnumVARIANT* pEnum = NULL;
    IUnknown *pUnknown = NULL;
    LONG Index, Count = 0;
    INT IconIndex = -1;
    BSTR Path = NULL, Name = NULL, Dest = NULL;
    NET_FW_IP_VERSION Proto;
    NET_FW_SCOPE Scope;
    VARIANT_BOOL State;
    ULONG Counter;
    VARIANT Var;
    HRESULT hr;

    DebugStartReceiving();

    CoInitialize(NULL);

    hr = CoCreateInstance(&CLSID_NetFwMgr,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          &IID_INetFwMgr,
                          (LPVOID*)&fwMgr);
    if (FAILED(hr)) goto Cleanup;

    hr = fwMgr->lpVtbl->get_LocalPolicy(fwMgr, &fwPolicy);
    if (FAILED(hr)) goto Cleanup;

    hr = fwPolicy->lpVtbl->get_CurrentProfile(fwPolicy, &fwProfile);
    if (FAILED(hr)) goto Cleanup;

    hr = fwProfile->lpVtbl->get_AuthorizedApplications(fwProfile, &fwApps);
    if (FAILED(hr)) goto Cleanup;

    hr = fwApps->lpVtbl->get__NewEnum(fwApps, &pUnknown);
    if (FAILED(hr)) goto Cleanup;

    hr = fwApps->lpVtbl->get_Count(fwApps, &Count);
    if (FAILED(hr)) goto Cleanup;

    hr = pUnknown->lpVtbl->QueryInterface(pUnknown, &IID_IEnumVARIANT, (LPVOID*)&pEnum);
    if (FAILED(hr)) goto Cleanup;

    for (Index = 0; Index < Count; Index++)
    {
        HICON hIcon;

        hr = pEnum->lpVtbl->Next(pEnum, 1, &Var, &Counter);
        if (FAILED(hr)) break;

        fwApp = (INetFwAuthorizedApplication*)Var.byref;
        fwApp->lpVtbl->get_ProcessImageFileName(fwApp, &Path);
        fwApp->lpVtbl->get_Name(fwApp, &Name);
        fwApp->lpVtbl->get_RemoteAddresses(fwApp, &Dest);
        fwApp->lpVtbl->get_Enabled(fwApp, &State);
        fwApp->lpVtbl->get_IpVersion(fwApp, &Proto);
        fwApp->lpVtbl->get_Scope(fwApp, &Scope);

        if (IoGetTarget() == IO_TARGET_LISTVIEW)
        {
            hIcon = ExtractIcon(hInst, Path, 0);
            if (!hIcon)
            {
                IconIndex = IoAddIcon(IDI_APPS);
            }
            else
            {
                IconIndex = ImageList_AddIcon(*DllParams.hListImgList, hIcon);
                DestroyIcon(hIcon);
            }
        }

        IoAddItem(0, IconIndex, Name);
        IoSetItemText(Path);
        IoSetItemText(State ? L"Enabled" : L"Disabled");
        IoSetItemText(L"Allowed");
        IoSetItemText(Dest);

        if (Proto == NET_FW_IP_VERSION_V4)
            IoSetItemText(L"TCP/IP v4");
        else if (Proto == NET_FW_IP_VERSION_V6)
            IoSetItemText(L"TCP/IP v6");
        else if (Proto == NET_FW_IP_VERSION_ANY)
            IoSetItemText(L"Any");
        else
            IoSetItemText(L"Unknown");

        if (Scope == NET_FW_SCOPE_ALL)
            IoSetItemText(L"All");
        else if (Scope == NET_FW_SCOPE_LOCAL_SUBNET)
            IoSetItemText(L"Local Subnet");
        else if (Scope == NET_FW_SCOPE_CUSTOM)
            IoSetItemText(L"Custom");
        else
            IoSetItemText(L"Unknown");

        SysFreeString(Name);
        SysFreeString(Path);
        SysFreeString(Dest);
    }

Cleanup:
    if (fwPolicy) fwPolicy->lpVtbl->Release(fwPolicy);
    if (fwMgr) fwMgr->lpVtbl->Release(fwMgr);

    CoUninitialize();

    DebugEndReceiving();
}
