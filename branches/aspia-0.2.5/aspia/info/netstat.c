/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/netstat.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"

#include <iphlpapi.h>
#include <tlhelp32.h>

#define ANY_SIZE 1


typedef struct
{
    DWORD dwState;
    DWORD dwLocalAddr;
    DWORD dwLocalPort;
    DWORD dwRemoteAddr;
    DWORD dwRemotePort;
    DWORD dwProcessId;
} MIB_TCPROW_EX, *PMIB_TCPROW_EX;

typedef struct
{
    DWORD dwNumEntries;
    MIB_TCPROW_EX table[ANY_SIZE];
} MIB_TCPTABLE_EX, *PMIB_TCPTABLE_EX;

typedef struct
{
    DWORD dwLocalAddr;
    DWORD dwLocalPort;
    DWORD dwProcessId;
} MIB_UDPROW_EX, *PMIB_UDPROW_EX;

typedef struct
{
    DWORD dwNumEntries;
    MIB_UDPROW_EX table[ANY_SIZE];
} MIB_UDPTABLE_EX, *PMIB_UDPTABLE_EX;

/* Definition for the AllocateAndGetTcpExTableFromStack function */
typedef DWORD (WINAPI *ALLOCGETTCPEXTTABLE)(PVOID*, BOOL, HANDLE, DWORD, DWORD);
/* Definition for the GetExtendedTcpTable function */
typedef DWORD (WINAPI *GETEXTTCPTABLE)(PVOID, PDWORD, BOOL, ULONG, TCP_TABLE_CLASS, ULONG);

/* Definition for the AllocateAndGetUdpExTableFromStack function */
typedef DWORD (WINAPI *ALLOCGETUDPEXTTABLE)(PVOID*, BOOL, HANDLE, DWORD, DWORD);
/* Definition for the GetExtendedUdpTable function */
typedef DWORD (WINAPI *GETEXTUDPTABLE)(PVOID, PDWORD, BOOL, ULONG, UDP_TABLE_CLASS, ULONG);


/* TCP endpoint states */
WCHAR TcpState[][32] = {
    L"UNKNOWN",
    L"CLOSED",
    L"LISTENING",
    L"SYN_SENT",
    L"SYN_RCVD",
    L"ESTABLISHED",
    L"FIN_WAIT1",
    L"FIN_WAIT2",
    L"CLOSE_WAIT",
    L"CLOSING",
    L"LAST_ACK",
    L"TIME_WAIT",
    L"DELETE_TCB"
};


PVOID
GetExtTcpTableXP(VOID)
{
    ALLOCGETTCPEXTTABLE pAllocateAndGetTcpExTableFromStack = NULL;
    HINSTANCE hDLL = NULL;
    PVOID TcpTable = NULL;

    hDLL = LoadLibrary(L"IPHLPAPI.DLL");
    if (!hDLL) return NULL;

    pAllocateAndGetTcpExTableFromStack =
        (ALLOCGETTCPEXTTABLE)GetProcAddress(hDLL, "AllocateAndGetTcpExTableFromStack");
    if (pAllocateAndGetTcpExTableFromStack)
    {
        if (pAllocateAndGetTcpExTableFromStack(&TcpTable,
                                               TRUE,
                                               hProcessHeap,
                                               2, 2) == ERROR_SUCCESS)
        {
            FreeLibrary(hDLL);
            return TcpTable;
        }
    }

    FreeLibrary(hDLL);
    return NULL;
}

PVOID
GetExtUdpTableXP(VOID)
{
    ALLOCGETUDPEXTTABLE pAllocateAndGetUdpExTableFromStack = NULL;
    HINSTANCE hDLL = NULL;
    PVOID UdpTable = NULL;

    hDLL = LoadLibrary(L"IPHLPAPI.DLL");
    if (!hDLL) return NULL;

    pAllocateAndGetUdpExTableFromStack =
        (ALLOCGETUDPEXTTABLE)GetProcAddress(hDLL, "AllocateAndGetUdpExTableFromStack");
    if (pAllocateAndGetUdpExTableFromStack)
    {
        if (pAllocateAndGetUdpExTableFromStack(&UdpTable,
                                               TRUE,
                                               hProcessHeap,
                                               2, 2) == ERROR_SUCCESS)
        {
            FreeLibrary(hDLL);
            return UdpTable;
        }
    }

    FreeLibrary(hDLL);
    return NULL;
}

PVOID
GetExtTcpTableVista(VOID)
{
    DWORD dwResult, dwSize = sizeof(MIB_TCPTABLE_EX);
    GETEXTTCPTABLE pGetExtendedTcpTable = NULL;
    HINSTANCE hDLL = NULL;
    PVOID TcpTable = NULL;

    hDLL = LoadLibrary(L"IPHLPAPI.DLL");
    if (!hDLL) return NULL;

    pGetExtendedTcpTable = (GETEXTTCPTABLE)GetProcAddress(hDLL, "GetExtendedTcpTable");
    if (!pGetExtendedTcpTable)
    {
        FreeLibrary(hDLL);
        return NULL;
    }

    TcpTable = (PVOID)Alloc(dwSize);
    if (!TcpTable)
    {
        FreeLibrary(hDLL);
        return NULL;
    }

    dwResult = pGetExtendedTcpTable(TcpTable,
                                    &dwSize,
                                    TRUE, AF_INET,
                                    TCP_TABLE_OWNER_PID_ALL,
                                    0);
    if (dwResult == ERROR_INSUFFICIENT_BUFFER)
    {
        Free(TcpTable);

        TcpTable = (PVOID)Alloc(dwSize);
        if (!TcpTable)
        {
            FreeLibrary(hDLL);
            return NULL;
        }

        dwResult = pGetExtendedTcpTable(TcpTable,
                                        &dwSize,
                                        TRUE, AF_INET,
                                        TCP_TABLE_OWNER_PID_ALL,
                                        0);
        if (dwResult != NO_ERROR)
        {
            Free(TcpTable);
            FreeLibrary(hDLL);
            return NULL;
        }
    }
    else if (dwResult != NO_ERROR)
    {
        Free(TcpTable);
        FreeLibrary(hDLL);
        return NULL;
    }

    FreeLibrary(hDLL);
    return TcpTable;
}

PVOID
GetExtUdpTableVista(VOID)
{
    DWORD dwResult, dwSize = sizeof(MIB_UDPTABLE_EX);
    GETEXTUDPTABLE pGetExtendedUdpTable = NULL;
    HINSTANCE hDLL = NULL;
    PVOID UdpTable = NULL;

    hDLL = LoadLibrary(L"IPHLPAPI.DLL");
    if (!hDLL) return NULL;

    pGetExtendedUdpTable = (GETEXTUDPTABLE)GetProcAddress(hDLL, "GetExtendedUdpTable");
    if (!pGetExtendedUdpTable)
    {
        FreeLibrary(hDLL);
        return NULL;
    }

    UdpTable = (PVOID)Alloc(dwSize);
    if (!UdpTable)
    {
        FreeLibrary(hDLL);
        return NULL;
    }

    dwResult = pGetExtendedUdpTable(UdpTable,
                                    &dwSize,
                                    TRUE, AF_INET,
                                    UDP_TABLE_OWNER_PID,
                                    0);
    if (dwResult == ERROR_INSUFFICIENT_BUFFER)
    {
        Free(UdpTable);

        UdpTable = (PVOID)Alloc(dwSize);
        if (!UdpTable)
        {
            FreeLibrary(hDLL);
            return NULL;
        }

        dwResult = pGetExtendedUdpTable(UdpTable,
                                        &dwSize,
                                        TRUE, AF_INET,
                                        UDP_TABLE_OWNER_PID,
                                        0);
        if (dwResult != NO_ERROR)
        {
            Free(UdpTable);
            FreeLibrary(hDLL);
            return NULL;
        }
    }
    else if (dwResult != NO_ERROR)
    {
        Free(UdpTable);
        FreeLibrary(hDLL);
        return NULL;
    }

    FreeLibrary(hDLL);
    return UdpTable;
}

PVOID
GetTcpTableOld(VOID)
{
    DWORD dwError, dwSize = 0;
    PMIB_TCPTABLE TcpTable;

    dwError = GetTcpTable(NULL, &dwSize, TRUE);
    if (dwError != ERROR_INSUFFICIENT_BUFFER)
        return NULL;

    TcpTable = (PMIB_TCPTABLE)Alloc(dwSize);
    if (!TcpTable) return NULL;

    dwError = GetTcpTable(TcpTable, &dwSize, TRUE);
    if (dwError != NO_ERROR)
    {
        Free(TcpTable);
        return NULL;
    }

    return TcpTable;
}

PVOID
GetUdpTableOld(VOID)
{
    DWORD dwError, dwSize = 0;
    PMIB_UDPTABLE UdpTable;

    dwError = GetUdpTable(NULL, &dwSize, TRUE);
    if (dwError != ERROR_INSUFFICIENT_BUFFER)
        return NULL;

    UdpTable = (PMIB_UDPTABLE)Alloc(dwSize);
    if (!UdpTable) return NULL;

    dwError = GetUdpTable(UdpTable, &dwSize, TRUE);
    if (dwError != NO_ERROR)
    {
        Free(UdpTable);
        return NULL;
    }

    return UdpTable;
}

BOOL
ProcessPidToName(HANDLE hProcessSnap,
                 DWORD ProcessId,
                 LPWSTR ProcessName,
                 SIZE_T Size)
{
    PROCESSENTRY32 ProcessEntry;

    if (ProcessName) ProcessName[0] = 0;

    ProcessEntry.dwSize = sizeof(ProcessEntry);
    if (!Process32First(hProcessSnap, &ProcessEntry))
        return FALSE;

    do
    {
        if (ProcessEntry.th32ProcessID == ProcessId)
        {
            StringCbCopy(ProcessName, Size, ProcessEntry.szExeFile);
            return TRUE;
        }
    }
    while (Process32Next(hProcessSnap, &ProcessEntry));

    return FALSE;
}

VOID
GetIpHostName(BOOL Local, UINT IpAddr, LPWSTR lpszName, SIZE_T NameSize)
{
    UINT nIpAddr;
    char Name[MAX_STR_LEN];

    nIpAddr = htonl(IpAddr);

    if (!IpAddr)
    {
        if (!Local)
        {
            StringCbPrintf(lpszName, NameSize,
                           L"%d.%d.%d.%d",
                           (nIpAddr >> 24) & 0xFF,
                           (nIpAddr >> 16) & 0xFF,
                           (nIpAddr >> 8) & 0xFF,
                           (nIpAddr) & 0xFF);
        }
        else
        {
            if (gethostname(Name, MAX_STR_LEN) == 0)
            {
                StringCbPrintf(lpszName, NameSize, L"%S", Name);
            }
        }
    }
    else if (IpAddr == 0x0100007f)
    {
        if (Local)
        {
            if (gethostname(Name, MAX_STR_LEN) == 0)
            {
                StringCbPrintf(lpszName, NameSize, L"%S", Name);
            }
        }
        else
        {
            StringCbCopy(lpszName, NameSize, L"localhost");
        }
    }
    else
    {
        StringCbPrintf(lpszName, NameSize,
                       L"%d.%d.%d.%d",
                       ((nIpAddr >> 24) & 0x000000FF),
                       ((nIpAddr >> 16) & 0x000000FF),
                       ((nIpAddr >> 8) & 0x000000FF),
                       ((nIpAddr) & 0x000000FF));
    }
}

VOID
GetPortName(UINT Port, PCSTR Proto, LPWSTR lpszName, SIZE_T NameSize)
{
    struct servent *pSrvent;
    USHORT uPort = htons((WORD)Port);

    /* Try to translate to a name */
    pSrvent = getservbyport(Port, Proto);
    if (pSrvent)
        StringCbPrintf(lpszName, NameSize, L"%ld (%S)", uPort, pSrvent->s_name);
    else
        StringCbPrintf(lpszName, NameSize, L"%ld", uPort);
}

VOID
NETWORK_NetStatInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szAddress[MAX_STR_LEN],
          szPort[MAX_STR_LEN], szUnknown[MAX_STR_LEN];
    PMIB_TCPTABLE_EX TcpTableEx;
    PMIB_UDPTABLE_EX UdpTableEx;
    PMIB_TCPTABLE TcpTable;
    PMIB_UDPTABLE UdpTable;
    HANDLE hProcessSnap;
    WSADATA WsaData;
    INT ItemIndex;
    DWORD dwIndex;

    DebugStartReceiving();

    IoAddIcon(IDI_NETSTAT);

    LoadMUIString(IDS_UNKNOWN, szUnknown, MAX_STR_LEN);

    if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
        return;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return;

    /* Try to get extended TCP table */
    TcpTableEx = GetExtTcpTableXP();
    if (!TcpTableEx)
        TcpTableEx = GetExtTcpTableVista();

    if (TcpTableEx)
    {
        for (dwIndex = 0; dwIndex < TcpTableEx->dwNumEntries; dwIndex++)
        {
            if (!ProcessPidToName(hProcessSnap,
                                  TcpTableEx->table[dwIndex].dwProcessId,
                                  szText, sizeof(szText)))
                StringCbCopy(szText, sizeof(szText), szUnknown);
            ItemIndex = IoAddItem(0, 0, szText);

            IoSetItemText(ItemIndex, 1, L"TCP");

            GetIpHostName(TRUE, TcpTableEx->table[dwIndex].dwLocalAddr,
                          szAddress, sizeof(szAddress));
            GetPortName(TcpTableEx->table[dwIndex].dwLocalPort, "tcp", szPort, sizeof(szPort));
            StringCbPrintf(szText, sizeof(szText), L"%s:%s",
                           szAddress, szPort);
            IoSetItemText(ItemIndex, 2, szText);

            GetIpHostName(FALSE, TcpTableEx->table[dwIndex].dwRemoteAddr,
                          szAddress, sizeof(szAddress));
            GetPortName(TcpTableEx->table[dwIndex].dwRemotePort, "tcp", szPort, sizeof(szPort));
            StringCbPrintf(szText, sizeof(szText), L"%s:%s",
                           szAddress, szPort);
            IoSetItemText(ItemIndex, 3, szText);

            IoSetItemText(ItemIndex, 4, TcpState[TcpTableEx->table[dwIndex].dwState]);
        }
        Free(TcpTableEx);
    }
    else /* Get old TCP table */
    {
        TcpTable = GetTcpTableOld();
        if (TcpTable)
        {
            for (dwIndex = 0; dwIndex < TcpTable->dwNumEntries; dwIndex++)
            {
                ItemIndex = IoAddItem(0, 0, szUnknown);
                IoSetItemText(ItemIndex, 1, L"TCP");

                GetIpHostName(TRUE, TcpTable->table[dwIndex].dwLocalAddr,
                              szAddress, sizeof(szAddress));
                GetPortName(TcpTable->table[dwIndex].dwLocalPort, "tcp", szPort, sizeof(szPort));
                StringCbPrintf(szText, sizeof(szText), L"%s:%s",
                               szAddress, szPort);
                IoSetItemText(ItemIndex, 2, szText);

                GetIpHostName(FALSE, TcpTable->table[dwIndex].dwRemoteAddr,
                          szAddress, sizeof(szAddress));
                GetPortName(TcpTable->table[dwIndex].dwRemotePort, "tcp", szPort, sizeof(szPort));
                StringCbPrintf(szText, sizeof(szText), L"%s:%s",
                               szAddress, szPort);
                IoSetItemText(ItemIndex, 3, szText);

                IoSetItemText(ItemIndex, 4, TcpState[TcpTable->table[dwIndex].dwState]);
            }

            Free(TcpTable);
        }
    }

    /* Try to get extended UDP table */
    UdpTableEx = GetExtUdpTableXP();
    if (!UdpTableEx)
        UdpTableEx = GetExtUdpTableVista();

    if (UdpTableEx)
    {
        for (dwIndex = 0; dwIndex < UdpTableEx->dwNumEntries; dwIndex++)
        {
            if (!ProcessPidToName(hProcessSnap,
                                  UdpTableEx->table[dwIndex].dwProcessId,
                                  szText, sizeof(szText)))
                StringCbCopy(szText, sizeof(szText), szUnknown);
            ItemIndex = IoAddItem(0, 0, szText);

            IoSetItemText(ItemIndex, 1, L"UDP");

            GetIpHostName(TRUE, UdpTableEx->table[dwIndex].dwLocalAddr,
                          szAddress, sizeof(szAddress));
            GetPortName(UdpTableEx->table[dwIndex].dwLocalPort, "udp", szPort, sizeof(szPort));
            StringCbPrintf(szText, sizeof(szText), L"%s:%s",
                           szAddress, szPort);
            IoSetItemText(ItemIndex, 2, szText);

            IoSetItemText(ItemIndex, 3, L"*.*.*.*:*");

            IoSetItemText(ItemIndex, 4, L"-");
        }
        Free(UdpTableEx);
    }
    else /* Get old UDP table */
    {
        UdpTable = GetUdpTableOld();
        if (UdpTable)
        {
            for (dwIndex = 0; dwIndex < UdpTable->dwNumEntries; dwIndex++)
            {
                ItemIndex = IoAddItem(0, 0, szUnknown);
                IoSetItemText(ItemIndex, 1, L"UDP");

                GetIpHostName(TRUE, UdpTable->table[dwIndex].dwLocalAddr,
                              szAddress, sizeof(szAddress));
                GetPortName(UdpTable->table[dwIndex].dwLocalPort, "udp", szPort, sizeof(szPort));
                StringCbPrintf(szText, sizeof(szText), L"%s:%s",
                               szAddress, szPort);
                IoSetItemText(ItemIndex, 2, szText);

                IoSetItemText(ItemIndex, 3, L"*.*.*.*:*");

                IoSetItemText(ItemIndex, 4, L"-");
            }

            Free(UdpTable);
        }
    }

    CloseHandle(hProcessSnap);

    DebugEndReceiving();
}