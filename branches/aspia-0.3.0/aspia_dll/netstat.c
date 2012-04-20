/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/netstat.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"

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
    if (!hDLL)
    {
        DebugTrace(L"LoadLibrary() failed! Error code = 0x%x",
                   GetLastError());
        return NULL;
    }

    pAllocateAndGetTcpExTableFromStack =
        (ALLOCGETTCPEXTTABLE)GetProcAddress(hDLL, "AllocateAndGetTcpExTableFromStack");
    if (pAllocateAndGetTcpExTableFromStack)
    {
        if (pAllocateAndGetTcpExTableFromStack(&TcpTable,
                                               TRUE,
                                               GetProcessHeap(),
                                               2, 2) == ERROR_SUCCESS)
        {
            FreeLibrary(hDLL);
            return TcpTable;
        }
    }
    else
    {
        DebugTrace(L"GetProcAddress() failed! Error code = 0x%x",
                   GetLastError());
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
    if (!hDLL)
    {
        DebugTrace(L"LoadLibrary() failed! Error code = 0x%x",
                   GetLastError());
        return NULL;
    }

    pAllocateAndGetUdpExTableFromStack =
        (ALLOCGETUDPEXTTABLE)GetProcAddress(hDLL, "AllocateAndGetUdpExTableFromStack");
    if (pAllocateAndGetUdpExTableFromStack)
    {
        if (pAllocateAndGetUdpExTableFromStack(&UdpTable,
                                               TRUE,
                                               GetProcessHeap(),
                                               2, 2) == ERROR_SUCCESS)
        {
            FreeLibrary(hDLL);
            return UdpTable;
        }
    }
    else
    {
        DebugTrace(L"GetProcAddress() failed! Error code = 0x%x",
                   GetLastError());
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
    if (!hDLL)
    {
        DebugTrace(L"LoadLibrary() failed! Error code = 0x%x",
                   GetLastError());
        return NULL;
    }

    pGetExtendedTcpTable = (GETEXTTCPTABLE)GetProcAddress(hDLL, "GetExtendedTcpTable");
    if (!pGetExtendedTcpTable)
    {
        DebugTrace(L"GetProcAddress() failed! Error code = 0x%x",
                   GetLastError());
        FreeLibrary(hDLL);
        return NULL;
    }

    TcpTable = (PVOID)Alloc(dwSize);
    if (!TcpTable)
    {
        DebugTrace(L"Alloc(%d) failed!", dwSize);
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
            DebugTrace(L"Alloc(%d) failed!", dwSize);
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
    if (!hDLL)
    {
        DebugTrace(L"LoadLibrary() failed! Error code = 0x%x",
                   GetLastError());
        return NULL;
    }

    pGetExtendedUdpTable = (GETEXTUDPTABLE)GetProcAddress(hDLL, "GetExtendedUdpTable");
    if (!pGetExtendedUdpTable)
    {
        DebugTrace(L"GetProcAddress() failed! Error code = 0x%x",
                   GetLastError());
        FreeLibrary(hDLL);
        return NULL;
    }

    UdpTable = (PVOID)Alloc(dwSize);
    if (!UdpTable)
    {
        DebugTrace(L"Alloc(%d) failed!", dwSize);
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
            DebugTrace(L"Alloc(%d) failed!", dwSize);
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
    if (!TcpTable)
    {
        DebugTrace(L"Alloc(%d) failed!", dwSize);
        return NULL;
    }

    dwError = GetTcpTable(TcpTable, &dwSize, TRUE);
    if (dwError != NO_ERROR)
    {
        DebugTrace(L"GetTcpTable() failed! Error code = 0x%x", dwError);
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
    if (!UdpTable)
    {
        DebugTrace(L"Alloc(%d) failed!", dwSize);
        return NULL;
    }

    dwError = GetUdpTable(UdpTable, &dwSize, TRUE);
    if (dwError != NO_ERROR)
    {
        DebugTrace(L"GetUdpTable() failed! Error code = 0x%x", dwError);
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
    DWORD dwIndex;

    DebugStartReceiving();

    IoAddIcon(IDI_NETSTAT);

    LoadMUIString(IDS_UNKNOWN, szUnknown, MAX_STR_LEN);

    if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
    {
        DebugTrace(L"WSAStartup() failed!");
        return;
    }

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        DebugTrace(L"CreateToolhelp32Snapshot() failed! Error code = 0x%x",
                   GetLastError());
        return;
    }

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
            IoAddItem(0, 0, szText);

            IoSetItemText(L"TCP");

            GetIpHostName(TRUE, TcpTableEx->table[dwIndex].dwLocalAddr,
                          szAddress, sizeof(szAddress));
            GetPortName(TcpTableEx->table[dwIndex].dwLocalPort, "tcp", szPort, sizeof(szPort));
            IoSetItemText(L"%s:%s", szAddress, szPort);

            GetIpHostName(FALSE, TcpTableEx->table[dwIndex].dwRemoteAddr,
                          szAddress, sizeof(szAddress));
            GetPortName(TcpTableEx->table[dwIndex].dwRemotePort, "tcp", szPort, sizeof(szPort));
            IoSetItemText(L"%s:%s", szAddress, szPort);

            IoSetItemText(TcpState[TcpTableEx->table[dwIndex].dwState]);

            if (IsCanceled)
            {
                Free(TcpTableEx);
                goto Exit;
            }
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
                IoAddItem(0, 0, szUnknown);
                IoSetItemText(L"TCP");

                GetIpHostName(TRUE, TcpTable->table[dwIndex].dwLocalAddr,
                              szAddress, sizeof(szAddress));
                GetPortName(TcpTable->table[dwIndex].dwLocalPort, "tcp", szPort, sizeof(szPort));
                IoSetItemText(L"%s:%s", szAddress, szPort);

                GetIpHostName(FALSE, TcpTable->table[dwIndex].dwRemoteAddr,
                          szAddress, sizeof(szAddress));
                GetPortName(TcpTable->table[dwIndex].dwRemotePort, "tcp", szPort, sizeof(szPort));
                IoSetItemText(L"%s:%s", szAddress, szPort);

                IoSetItemText(TcpState[TcpTable->table[dwIndex].dwState]);

                if (IsCanceled)
                {
                    Free(TcpTable);
                    goto Exit;
                }
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
            IoAddItem(0, 0, szText);

            IoSetItemText(L"UDP");

            GetIpHostName(TRUE, UdpTableEx->table[dwIndex].dwLocalAddr,
                          szAddress, sizeof(szAddress));
            GetPortName(UdpTableEx->table[dwIndex].dwLocalPort, "udp", szPort, sizeof(szPort));
            IoSetItemText(L"%s:%s", szAddress, szPort);

            IoSetItemText(L"*.*.*.*:*");

            IoSetItemText(L"-");

            if (IsCanceled)
            {
                Free(UdpTableEx);
                goto Exit;
            }
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
                IoAddItem(0, 0, szUnknown);
                IoSetItemText(L"UDP");

                GetIpHostName(TRUE, UdpTable->table[dwIndex].dwLocalAddr,
                              szAddress, sizeof(szAddress));
                GetPortName(UdpTable->table[dwIndex].dwLocalPort, "udp", szPort, sizeof(szPort));
                IoSetItemText(L"%s:%s", szAddress, szPort);

                IoSetItemText(L"*.*.*.*:*");

                IoSetItemText(L"-");

                if (IsCanceled)
                {
                    Free(UdpTable);
                    goto Exit;
                }
            }

            Free(UdpTable);
        }
    }
Exit:
    CloseHandle(hProcessSnap);

    DebugEndReceiving();
}
