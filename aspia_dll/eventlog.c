/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/eventlog.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


#define EVENT_MESSAGE_FILE_BUFFER       1024*10
#define EVENT_DLL_SEPARATOR             L";"
#define EVENT_MESSAGE_FILE              L"EventMessageFile"
#define EVENT_CATEGORY_MESSAGE_FILE     L"CategoryMessageFile"
#define EVENT_PARAMETER_MESSAGE_FILE    L"ParameterMessageFile"
static PEVENTLOGRECORD *g_RecordPtrs = NULL;
static DWORD g_TotalRecords = 0;
static LPWSTR lpSourceLogName = NULL;
static LPWSTR lpComputerName  = NULL;

static const LPWSTR EVENT_SOURCE_APPLICATION = L"Application";
static const LPWSTR EVENT_SOURCE_SECURITY    = L"Security";
static const LPWSTR EVENT_SOURCE_SYSTEM      = L"System";


static VOID
EventTimeToSystemTime(DWORD EventTime,
                      SYSTEMTIME *pSystemTime)
{
    SYSTEMTIME st1970 = { 1970, 1, 0, 1, 0, 0, 0, 0 };
    FILETIME ftLocal;
    union
    {
        FILETIME ft;
        ULONGLONG ll;
    } u1970, uUCT;

    uUCT.ft.dwHighDateTime = 0;
    uUCT.ft.dwLowDateTime = EventTime;
    SystemTimeToFileTime(&st1970, &u1970.ft);
    uUCT.ll = uUCT.ll * 10000000 + u1970.ll;
    FileTimeToLocalFileTime(&uUCT.ft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, pSystemTime);
}

static BOOL
GetEventUserName(EVENTLOGRECORD *pelr,
                 OUT LPWSTR pszUser,
                 IN SIZE_T Size)
{
    PSID lpSid;
    WCHAR szName[1024];
    WCHAR szDomain[1024];
    SID_NAME_USE peUse;
    DWORD cbName = 1024;
    DWORD cbDomain = 1024;

    /* Point to the SID. */
    lpSid = (PSID)((LPBYTE)pelr + pelr->UserSidOffset);

    /* User SID */
    if (pelr->UserSidLength > 0)
    {
        if (LookupAccountSid(NULL,
                             lpSid,
                             szName,
                             &cbName,
                             szDomain,
                             &cbDomain,
                             &peUse))
        {
            StringCbCopy(pszUser, Size, szName);
            return TRUE;
        }
    }

    StringCbCopy(pszUser, Size, L"N/A");
    return FALSE;
}

static VOID
GetEventType(IN WORD dwEventType,
             OUT LPWSTR eventTypeText,
             IN INT EventTypeLen)
{
    UINT uID;

    switch (dwEventType)
    {
        case EVENTLOG_ERROR_TYPE:
            uID = IDS_ELOG_ERROR_TYPE;
            break;
        case EVENTLOG_WARNING_TYPE:
            uID = IDS_ELOG_WARNING_TYPE;
            break;
        case EVENTLOG_INFORMATION_TYPE:
            uID = IDS_ELOG_INFO_TYPE;
            break;
        case EVENTLOG_AUDIT_SUCCESS:
            uID = IDS_ELOG_AUDIT_SUCCESS;
            break;
        case EVENTLOG_AUDIT_FAILURE:
            uID = IDS_ELOG_AUDIT_FAILURE;
            break;
        case EVENTLOG_SUCCESS:
            uID = IDS_ELOG_SUCCESS;
            break;
        default:
            uID = IDS_ELOG_UNKNOWN_TYPE;
            break;
    }

    LoadMUIString(uID, eventTypeText, EventTypeLen);
}

static VOID
TrimNulls(LPWSTR s)
{
    WCHAR *c;

    if (s != NULL)
    {
        c = s + SafeStrLen(s) - 1;
        while (c >= s && iswspace(*c))
            --c;
        *++c = L'\0';
    }
}

static BOOL
GetEventMessageFileDLL(IN LPCWSTR lpLogName,
                       IN LPCWSTR SourceName,
                       IN LPCWSTR EntryName,
                       OUT LPWSTR ExpandedName)
{
    DWORD dwSize;
    WCHAR szModuleName[MAX_PATH];
    WCHAR szKeyName[MAX_PATH];
    HKEY hAppKey = NULL;
    HKEY hSourceKey = NULL;
    BOOL bReturn = FALSE;

    StringCbPrintf(szKeyName, sizeof(szKeyName),
                   L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s",
                   lpLogName);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     szKeyName,
                     0, KEY_READ | KEY_WOW64_64KEY,
                     &hAppKey) == ERROR_SUCCESS)
    {
        if (RegOpenKeyEx(hAppKey,
                         SourceName,
                         0, KEY_READ | KEY_WOW64_64KEY,
                         &hSourceKey) == ERROR_SUCCESS)
        {
            dwSize = MAX_PATH;
            if (RegQueryValueEx(hSourceKey,
                                EntryName,
                                NULL,
                                NULL,
                                (LPBYTE)szModuleName,
                                &dwSize) == ERROR_SUCCESS)
            {
                /* Returns a string containing the requested substituted environment variable. */
                ExpandEnvironmentStrings(szModuleName, ExpandedName, MAX_PATH);

                /* Succesfull */
                bReturn = TRUE;
            }
        }
    }

    if (hSourceKey != NULL)
        RegCloseKey(hSourceKey);

    if (hAppKey != NULL)
        RegCloseKey(hAppKey);

    return bReturn;
}

static BOOL
GetEventMessage(IN LPCTSTR KeyName,
                IN LPCTSTR SourceName,
                IN EVENTLOGRECORD *pevlr,
                OUT LPWSTR EventText,
                IN SIZE_T EventTextSize)
{
    SIZE_T i;
    HANDLE hLibrary = NULL;
    WCHAR SourceModuleName[1000];
    WCHAR ParameterModuleName[1000];
    LPWSTR lpMsgBuf = NULL;
    LPWSTR szDll;
    LPWSTR szMessage;
    LPWSTR *szArguments;
    BOOL bDone = FALSE;
    DWORD dwRet;

    /* TODO : GetEventMessageFileDLL can return a comma separated list of DLLs */
    if (GetEventMessageFileDLL(KeyName, SourceName, EVENT_MESSAGE_FILE, SourceModuleName))
    {
        /* Get the event message */
        szMessage = (LPWSTR)((LPBYTE)pevlr + pevlr->StringOffset);

        /* Allocate space for parameters */
        szArguments = (LPWSTR*)Alloc(sizeof(LPVOID) * pevlr->NumStrings);
        if (!szArguments)
        {
            DebugAllocFailed();
            return FALSE;
        }

        for (i = 0; i < pevlr->NumStrings; i++)
        {
            if (wcsstr(szMessage, L"%%"))
            {
                if (GetEventMessageFileDLL(KeyName, SourceName, EVENT_PARAMETER_MESSAGE_FILE, ParameterModuleName))
                {
                    /* Not yet support for reading messages from parameter message DLL */
                }

                szArguments[i] = szMessage;
                szMessage += SafeStrLen(szMessage) + 1;
            }
            else
            {
                szArguments[i] = szMessage;
                szMessage += SafeStrLen(szMessage) + 1;
            }
        }

        szDll = wcstok(SourceModuleName, EVENT_DLL_SEPARATOR);
        while ((szDll != NULL) && (!bDone))
        {
            hLibrary = LoadLibraryEx(szDll,
                                     NULL,
                                     DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
            if (hLibrary == NULL)
            {
                /* The DLL could not be loaded try the next one (if any) */
                szDll = wcstok(NULL, EVENT_DLL_SEPARATOR);
            }
            else
            {
                __try
                {
                    /* Retrieve the message string. */
                    dwRet = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                                          FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                          FORMAT_MESSAGE_FROM_HMODULE |
                                          FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                          hLibrary,
                                          pevlr->EventID,
                                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                          (LPWSTR)&lpMsgBuf,
                                          0,
                                          (va_list*)szArguments);
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    dwRet = 0;
                }

                if (dwRet == 0)
                {
                    /* We haven't found the string , get next DLL (if any) */
                    szDll = wcstok(NULL, EVENT_DLL_SEPARATOR);
                }
                else
                {
                    if (lpMsgBuf)
                    {
                        /* The ID was found and the message was formated */
                        bDone = TRUE;

                        /* Trim the string */
                        TrimNulls((LPWSTR)lpMsgBuf);

                        /* Copy the event text */
                        StringCbCopy(EventText, EventTextSize, lpMsgBuf);
                    }
                }

                if (lpMsgBuf) LocalFree(lpMsgBuf);
                lpMsgBuf = NULL;

                FreeLibrary(hLibrary);
            }
        }

        Free(szArguments);

        /* No more dlls to try, return result */
        return bDone;
    }

    StringCbCopy(EventText, EventTextSize, L"N/A");
    return FALSE;
}

static BOOL
GetEventCategory(IN LPCTSTR KeyName,
                 IN LPCTSTR SourceName,
                 IN EVENTLOGRECORD *pevlr,
                 OUT LPWSTR CategoryName,
                 IN SIZE_T CatNameSize)
{
    HANDLE hLibrary = NULL;
    WCHAR szMessageDLL[MAX_PATH];
    LPVOID lpMsgBuf = NULL;

    if (GetEventMessageFileDLL(KeyName, SourceName, EVENT_CATEGORY_MESSAGE_FILE, szMessageDLL))
    {
        hLibrary = LoadLibraryEx(szMessageDLL,
                                 NULL,
                                 DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
        if (hLibrary)
        {
            /* Retrieve the message string. */
            if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
                              FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                              hLibrary,
                              pevlr->EventCategory,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (LPWSTR)&lpMsgBuf,
                              EVENT_MESSAGE_FILE_BUFFER,
                              NULL) != 0)
            {
                if (lpMsgBuf)
                {
                    /* Trim the string */
                    TrimNulls(lpMsgBuf);

                    /* Copy the category name */
                    StringCbCopy(CategoryName, CatNameSize, lpMsgBuf);
                }
                else
                {
                    StringCbCopy(CategoryName, CatNameSize, lpMsgBuf);
                }
            }
            else
            {
                /* FIXME: Localization */
                StringCbCopy(CategoryName, CatNameSize, L"None");
            }

            if (hLibrary)
                FreeLibrary((HMODULE)hLibrary);

            /* Free the buffer allocated by FormatMessage */
            if (lpMsgBuf) LocalFree(lpMsgBuf);

            return TRUE;
        }
    }

    /* FIXME: Localization */
    StringCbCopy(CategoryName, CatNameSize, L"None");

    return FALSE;
}

static BOOL
QueryEventMessages(LPWSTR lpMachineName,
                   LPWSTR lpLogName)
{
    HANDLE hEventLog;
    EVENTLOGRECORD *pevlr;
    DWORD dwRead, dwNeeded, dwThisRecord, dwTotalRecords = 0,
          dwRecordsToRead = 0, dwFlags;
    LPWSTR lpSourceName;
    LPWSTR lpComputerName;
    LPWSTR lpData;
    BOOL bResult = TRUE; /* Read succeeded. */
    WCHAR szLocalDate[MAX_PATH];
    WCHAR szLocalTime[MAX_PATH];
    WCHAR szEventID[MAX_PATH];
    WCHAR szEventTypeText[MAX_PATH];
    WCHAR szCategoryID[MAX_PATH];
    WCHAR szUsername[MAX_PATH];
    WCHAR szEventText[EVENT_MESSAGE_FILE_BUFFER];
    WCHAR szCategory[MAX_PATH];
    INT IconIndex;
    SIZE_T dwCurrentRecord = 0;
    SYSTEMTIME time;

    dwFlags = EVENTLOG_FORWARDS_READ | EVENTLOG_SEQUENTIAL_READ;

    lpSourceLogName = lpLogName;
    lpComputerName = lpMachineName;

    /* Open the event log. */
    hEventLog = OpenEventLog(lpMachineName,
                             lpLogName);
    if (!hEventLog)
    {
        g_TotalRecords = 0;
        g_RecordPtrs = NULL;
        return FALSE;
    }

    GetOldestEventLogRecord(hEventLog, &dwThisRecord);

    /* Get the total number of event log records. */
    GetNumberOfEventLogRecords(hEventLog, &dwTotalRecords);
    g_TotalRecords = dwTotalRecords;

    g_RecordPtrs = (PEVENTLOGRECORD*)Alloc(dwTotalRecords * sizeof(PEVENTLOGRECORD));
    if (!g_RecordPtrs)
    {
        DebugAllocFailed();
        g_TotalRecords = 0;
        g_RecordPtrs = NULL;
        CloseEventLog(hEventLog);
        return FALSE;
    }

    while (dwCurrentRecord < dwTotalRecords)
    {
        pevlr = (EVENTLOGRECORD*)Alloc(sizeof(EVENTLOGRECORD) * dwTotalRecords);
        g_RecordPtrs[dwCurrentRecord] = pevlr;

        bResult = ReadEventLog(hEventLog,  /* Event log handle */
                               dwFlags,    /* Sequential read */
                               0,          /* Ignored for sequential read */
                               pevlr,      /* Pointer to buffer */
                               sizeof(EVENTLOGRECORD),   /* Size of buffer */
                               &dwRead,    /* Number of bytes read */
                               &dwNeeded); /* Bytes in the next record */
        if ((!bResult) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
        {
            Free(pevlr);
            pevlr = (EVENTLOGRECORD*)Alloc(dwNeeded);
            g_RecordPtrs[dwCurrentRecord] = pevlr;

            ReadEventLog(hEventLog,  /* event log handle */
                         dwFlags,    /* read flags */
                         0,          /* offset; default is 0 */
                         pevlr,      /* pointer to buffer */
                         dwNeeded,   /* size of buffer */
                         &dwRead,    /* number of bytes read */
                         &dwNeeded); /* bytes in next record */
        }

        while (dwRead > 0)
        {
            switch (pevlr->EventType)
            {
                case EVENTLOG_INFORMATION_TYPE:
                case EVENTLOG_AUDIT_SUCCESS:
                case EVENTLOG_SUCCESS:
                    IconIndex = 0;
                    break;
                case EVENTLOG_ERROR_TYPE:
                case EVENTLOG_AUDIT_FAILURE:
                    IconIndex = 2;
                    break;
                case EVENTLOG_WARNING_TYPE:
                    IconIndex = 1;
                    break;
                default:
                    IconIndex = -1;
                    break;
            }

            if ((IconIndex == 0 && DllParams.ELogShowInfo) ||
                (IconIndex == 1 && DllParams.ELogShowWarning) ||
                (IconIndex == 2 && DllParams.ELogShowError))
            {
                /* Get the event source name */
                lpSourceName = (LPWSTR)((LPBYTE)pevlr + sizeof(EVENTLOGRECORD));

                /* Get the computer name */
                lpComputerName = (LPWSTR)((LPBYTE)pevlr +
                                      sizeof(EVENTLOGRECORD) +
                                      (SafeStrLen(lpSourceName) + 1) * sizeof(WCHAR));

                /* This ist the data section of the current event */
                lpData = (LPWSTR)((LPBYTE)pevlr + pevlr->DataOffset);

                /* Compute the event time */
                EventTimeToSystemTime(pevlr->TimeWritten, &time);

                /* Get the username that generated the event */
                GetEventUserName(pevlr, szUsername, sizeof(szUsername));

                GetDateFormat(LOCALE_USER_DEFAULT,
                              DATE_SHORTDATE,
                              &time, NULL,
                              szLocalDate,
                              MAX_PATH);
                GetTimeFormat(LOCALE_USER_DEFAULT,
                              TIME_NOSECONDS,
                              &time, NULL,
                              szLocalTime,
                              MAX_PATH);

                GetEventType(pevlr->EventType, szEventTypeText, MAX_PATH);

                GetEventCategory(lpLogName, lpSourceName,
                                 pevlr, szCategory,
                                 sizeof(szCategory));

                StringCbPrintf(szEventID,
                               sizeof(szEventID),
                               L"%u",
                               (DWORD)(pevlr->EventID & 0xFFFF));
                StringCbPrintf(szCategoryID,
                               sizeof(szCategoryID),
                               L"%u",
                               (DWORD)(pevlr->EventCategory));

                IoAddItem(0, IconIndex, szEventTypeText);

                IoSetItemText(szLocalDate);
                IoSetItemText(szLocalTime);
                IoSetItemText(lpSourceName);
                IoSetItemText(szCategory);
                IoSetItemText(szEventID);
                IoSetItemText(szUsername);
                IoSetItemText(lpComputerName);

                GetEventMessage(lpSourceLogName, lpSourceName,
                                pevlr, szEventText, sizeof(szEventText));
                IoSetItemText(szEventText);
            }

            dwRead -= pevlr->Length;
            pevlr = (EVENTLOGRECORD*)((LPBYTE)pevlr + pevlr->Length);
        }

        Free(g_RecordPtrs[dwCurrentRecord]);

        if (IsCanceled) break;

        --dwRecordsToRead;
        ++dwCurrentRecord;
    }

    Free(g_RecordPtrs);

    /* Close the event log */
    CloseEventLog(hEventLog);

    return TRUE;
}

static VOID
EventViewInfo(LPWSTR lpLogName)
{
    WCHAR szText[MAX_STR_LEN];
    DWORD dwSize;

    if (!DllParams.ELogShowError &&
        !DllParams.ELogShowWarning &&
        !DllParams.ELogShowInfo)
    {
        return;
    }

    IoAddIcon(IDI_INFOICO);
    IoAddIcon(IDI_BANG);
    IoAddIcon(IDI_ERRORICO);

    dwSize = MAX_STR_LEN;
    GetComputerName(szText, &dwSize);
    QueryEventMessages(szText, lpLogName);
}

VOID
OS_SystemEventsInfo(VOID)
{
    DebugStartReceiving();

    EventViewInfo(EVENT_SOURCE_SYSTEM);

    DebugEndReceiving();
}

VOID
OS_AppEventsInfo(VOID)
{
    DebugStartReceiving();

    EventViewInfo(EVENT_SOURCE_APPLICATION);

    DebugEndReceiving();
}

VOID
OS_SecurityEventsInfo(VOID)
{
    DebugStartReceiving();

    EventViewInfo(EVENT_SOURCE_SECURITY);

    DebugEndReceiving();
}
