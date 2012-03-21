/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/regional.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


static VOID
AddLocaleInfoString2(INT Icon, UINT StringID, LCTYPE Type1, LCTYPE Type2)
{
    WCHAR szText[MAX_STR_LEN], szTemp[MAX_STR_LEN];
    INT Index;

    if (GetLocaleInfo(LOCALE_USER_DEFAULT, Type1,
                      szText, MAX_STR_LEN))
    {
        if (GetLocaleInfo(LOCALE_USER_DEFAULT, Type2,
                          szTemp, MAX_STR_LEN))
        {
            StringCbCat(szText, sizeof(szText), L" / ");
            StringCbCat(szText, sizeof(szText), szTemp);
        }
        Index = IoAddValueName(1, StringID, Icon);
        IoSetItemText(Index, 1, szText);
    }
}

static VOID
AddLocaleInfoString1(INT Icon, UINT StringID, LCTYPE Type)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index;

    if (GetLocaleInfo(LOCALE_USER_DEFAULT, Type,
                      szText, MAX_STR_LEN))
    {
        Index = IoAddValueName(1, StringID, Icon);
        IoSetItemText(Index, 1, szText);
    }
}

VOID
OS_RegionalParamInfo(VOID)
{
    LCID UserLcid = GetUserDefaultLCID();
    WCHAR szText[MAX_STR_LEN];
    TIME_ZONE_INFORMATION TimeZone = {0};
    LPWSTR lpszValue;
    DWORD dwRet;
    INT Index;

    DebugStartReceiving();

    IoAddIcon(IDI_LOCALES);
    IoAddIcon(IDI_CALENDAR);
    IoAddIcon(IDI_TEXT);

    dwRet = GetTimeZoneInformation(&TimeZone);
    if (dwRet != TIME_ZONE_ID_INVALID)
    {
        IoAddHeader(0, IDS_REGION_TIME_ZONE, 1);
        Index = IoAddValueName(1, IDS_REGION_CUR_TIMEZONE, 1);
        if (dwRet == TIME_ZONE_ID_DAYLIGHT)
            IoSetItemText(Index, 1, TimeZone.DaylightName);
        else
            IoSetItemText(Index, 1, TimeZone.StandardName);

        if (dwRet != TIME_ZONE_ID_UNKNOWN)
        {
            LPWSTR szFormat = L"%02d.%02d (day.month) %02d:%02d (hour:minute)";

            Index = IoAddValueName(1, IDS_REGION_CHANGE_TO_STD_TIME, 1);
            IoSetItemText(Index, 1, szFormat,
                          TimeZone.StandardDate.wDay,
                          TimeZone.StandardDate.wMonth,
                          TimeZone.StandardDate.wHour,
                          TimeZone.StandardDate.wMinute);

            Index = IoAddValueName(1, IDS_REGION_CHANGE_TO_DL_TIME, 1);
            IoSetItemText(Index, 1, szFormat,
                          TimeZone.DaylightDate.wDay,
                          TimeZone.DaylightDate.wMonth,
                          TimeZone.DaylightDate.wHour,
                          TimeZone.DaylightDate.wMinute);
        }

        IoAddFooter();
    }

    IoAddHeader(0, IDS_REGION_LANGUAGE, 0);
    AddLocaleInfoString1(0, IDS_REGION_LANG_NAME_NATIVE, LOCALE_SNATIVELANGNAME);
    AddLocaleInfoString1(0, IDS_REGION_LANG_NAME_ENGLISH, LOCALE_SENGLANGUAGE);
    AddLocaleInfoString1(0, IDS_REGION_LANG_NAME_ISO639, LOCALE_SISO639LANGNAME);
    IoAddFooter();

    IoAddHeader(0, IDS_REGION_COUNTRY, 0);
    AddLocaleInfoString1(0, IDS_REGION_COUNTRY_NAME_NATIVE, LOCALE_SNATIVECTRYNAME);
    AddLocaleInfoString1(0, IDS_REGION_COUNTRY_NAME_ENGLISH, LOCALE_SENGCOUNTRY);
    AddLocaleInfoString1(0, IDS_REGION_COUNTRY_NAME_ISO3166, LOCALE_SISO3166CTRYNAME);
    AddLocaleInfoString1(0, IDS_REGION_COUNTRY_CODE, LOCALE_ICOUNTRY);
    IoAddFooter();

    IoAddHeader(0, IDS_REGION_CURRENCY, 0);
    AddLocaleInfoString1(0, IDS_REGION_CUR_NAME_NATIVE, LOCALE_SNATIVECURRNAME);
    AddLocaleInfoString1(0, IDS_REGION_CUR_NAME_ENGLISH, LOCALE_SENGCURRNAME);
    AddLocaleInfoString1(0, IDS_REGION_CUR_SYMBOL_NATIVE, LOCALE_SCURRENCY);
    AddLocaleInfoString1(0, IDS_REGION_CUR_SYMBOL_ISO4217, LOCALE_SINTLSYMBOL);

    if (GetCurrencyFormat(UserLcid, 0, L"123456789.00",
                          NULL, szText, MAX_STR_LEN))
    {
        Index = IoAddValueName(1, IDS_REGION_CUR_FORMAT, 0);
        IoSetItemText(Index, 1, szText);
    }

    if (GetCurrencyFormat(UserLcid, 0, L"-123456789.00",
                          NULL, szText, MAX_STR_LEN))
    {
        Index = IoAddValueName(1, IDS_REGION_CUR_NEG_FORMAT, 0);
        IoSetItemText(Index, 1, szText);
    }
    IoAddFooter();

    IoAddHeader(0, IDS_REGION_FORMATTING, 2);
    AddLocaleInfoString1(2, IDS_REGION_FRMT_TIME_FORMAT, LOCALE_STIMEFORMAT);
    AddLocaleInfoString1(2, IDS_REGION_FRMT_SDATE_FORMAT, LOCALE_SSHORTDATE);
    AddLocaleInfoString1(2, IDS_REGION_FRMT_LDATE_FORMAT, LOCALE_SLONGDATE);

    if (GetNumberFormat(UserLcid, 0, L"123456789",
                        NULL, szText, MAX_STR_LEN))
    {
        Index = IoAddValueName(1, IDS_REGION_FRMT_NUMBER_FORMAT, 2);
        IoSetItemText(Index, 1, szText);
    }
    if (GetNumberFormat(UserLcid, 0, L"-123456789",
                        NULL, szText, MAX_STR_LEN))
    {
        Index = IoAddValueName(1, IDS_REGION_FRMT_NEG_NUM_FORMAT, 2);
        IoSetItemText(Index, 1, szText);
    }
    if (GetLocaleInfo(UserLcid, LOCALE_SLIST,
                      szText, MAX_STR_LEN))
    {
        WCHAR szResult[MAX_STR_LEN];

        StringCbPrintf(szResult, sizeof(szResult),
                       L"first%s second%s third", szText, szText);
        Index = IoAddValueName(1, IDS_REGION_FRMT_LIST_FORMAT, 2);
        IoSetItemText(Index, 1, szResult);
    }
    if (GetLocaleInfo(UserLcid, LOCALE_SNATIVEDIGITS,
                      szText, MAX_STR_LEN))
    {
        Index = IoAddValueName(1, IDS_REGION_FRMT_NATIVE_DIGITS, 2);
        IoSetItemText(Index, 1, szText);
    }
    IoAddFooter();

    IoAddHeader(0, IDS_REGION_DAYS_OF_WEEK, 1);

    AddLocaleInfoString2(1, IDS_REGION_DOW_MONDAY, LOCALE_SDAYNAME1, LOCALE_SABBREVDAYNAME1);
    AddLocaleInfoString2(1, IDS_REGION_DOW_TUESDAY, LOCALE_SDAYNAME2, LOCALE_SABBREVDAYNAME2);
    AddLocaleInfoString2(1, IDS_REGION_DOW_WEDNESDAY, LOCALE_SDAYNAME3, LOCALE_SABBREVDAYNAME3);
    AddLocaleInfoString2(1, IDS_REGION_DOW_THURSDAY, LOCALE_SDAYNAME4, LOCALE_SABBREVDAYNAME4);
    AddLocaleInfoString2(1, IDS_REGION_DOW_FRIDAY, LOCALE_SDAYNAME5, LOCALE_SABBREVDAYNAME5);
    AddLocaleInfoString2(1, IDS_REGION_DOW_SATURDAY, LOCALE_SDAYNAME6, LOCALE_SABBREVDAYNAME6);
    AddLocaleInfoString2(1, IDS_REGION_DOW_SUNDAY, LOCALE_SDAYNAME7, LOCALE_SABBREVDAYNAME7);

    IoAddFooter();

    IoAddHeader(0, IDS_REGION_MONTHS, 1);

    AddLocaleInfoString2(1, IDS_REGION_MNS_JANUARY, LOCALE_SMONTHNAME1, LOCALE_SABBREVMONTHNAME1);
    AddLocaleInfoString2(1, IDS_REGION_MNS_FEBRUARY, LOCALE_SMONTHNAME2, LOCALE_SABBREVMONTHNAME2);
    AddLocaleInfoString2(1, IDS_REGION_MNS_MARCH, LOCALE_SMONTHNAME3, LOCALE_SABBREVMONTHNAME3);
    AddLocaleInfoString2(1, IDS_REGION_MNS_APRIL, LOCALE_SMONTHNAME4, LOCALE_SABBREVMONTHNAME4);
    AddLocaleInfoString2(1, IDS_REGION_MNS_MAY, LOCALE_SMONTHNAME5, LOCALE_SABBREVMONTHNAME5);
    AddLocaleInfoString2(1, IDS_REGION_MNS_JUNE, LOCALE_SMONTHNAME6, LOCALE_SABBREVMONTHNAME6);
    AddLocaleInfoString2(1, IDS_REGION_MNS_JULE, LOCALE_SMONTHNAME7, LOCALE_SABBREVMONTHNAME7);
    AddLocaleInfoString2(1, IDS_REGION_MNS_AUGUST, LOCALE_SMONTHNAME8, LOCALE_SABBREVMONTHNAME8);
    AddLocaleInfoString2(1, IDS_REGION_MNS_SEPTEMBER, LOCALE_SMONTHNAME9, LOCALE_SABBREVMONTHNAME9);
    AddLocaleInfoString2(1, IDS_REGION_MNS_OCTOBER, LOCALE_SMONTHNAME10, LOCALE_SABBREVMONTHNAME10);
    AddLocaleInfoString2(1, IDS_REGION_MNS_NOVEMBER, LOCALE_SMONTHNAME11, LOCALE_SABBREVMONTHNAME11);
    AddLocaleInfoString2(1, IDS_REGION_MNS_DECEMBER, LOCALE_SMONTHNAME12, LOCALE_SABBREVMONTHNAME12);

    IoAddFooter();

    IoAddHeader(0, IDS_REGION_MISCELLANEOUS, 0);
    if (GetLocaleInfo(UserLcid, LOCALE_ICALENDARTYPE,
                      szText, MAX_STR_LEN))
    {
        INT CalId = _wtoi(szText);

        switch (CalId)
        {
            case 1:
                lpszValue = L"Gregorian (localized)";
                break;
            case 2:
                lpszValue = L"Gregorian (English strings always)";
                break;
            case 3:
                lpszValue = L"Japanese Emperor Era";
                break;
            case 4:
                lpszValue = L"Taiwan Calendar";
                break;
            case 5:
                lpszValue = L"Korean Tangun Era";
                break;
            case 6:
                lpszValue = L"Hijri (Arabic Lunar)";
                break;
            case 7:
                lpszValue = L"Thai";
                break;
            case 8:
                lpszValue = L"Hebrew (Lunar)";
                break;
            case 9:
                lpszValue = L"Gregorian Middle East French";
                break;
            case 10:
                lpszValue = L"Gregorian Arabic";
                break;
            case 11:
                lpszValue = L"Gregorian transliterated English";
                break;
            case 12:
                lpszValue = L"Gregorian transliterated French";
                break;
            case 23:
                lpszValue = L"Um Al Qura (Arabic lunar)";
                break;
            default:
                lpszValue = L"Unknown";
                break;
        }
        Index = IoAddValueName(1, IDS_REGION_CALENDAR_TYPE, 0);
        IoSetItemText(Index, 1, lpszValue);
    }
    if (GetLocaleInfo(UserLcid, LOCALE_IPAPERSIZE,
                      szText, MAX_STR_LEN))
    {
        INT PaperId = _wtoi(szText);

        switch (PaperId)
        {
            case 1:
                lpszValue = L"US Letter";
                break;
            case 5:
                lpszValue = L"US legal";
                break;
            case 8:
                lpszValue = L"A3";
                break;
            case 9:
                lpszValue = L"A4";
                break;
            default:
                lpszValue = L"Unknown";
                break;
        }

        Index = IoAddValueName(1, IDS_REGION_PAPER_SIZE, 0);
        IoSetItemText(Index, 1, lpszValue);
    }
    if (GetLocaleInfo(UserLcid, LOCALE_IMEASURE,
                      szText, MAX_STR_LEN))
    {
        INT Id = _wtoi(szText);

        switch (Id)
        {
            case 0:
                lpszValue = L"Metric";
                break;
            case 1:
                lpszValue = L"US System";
                break;
            default:
                lpszValue = L"Unknown";
                break;
        }

        Index = IoAddValueName(1, IDS_REGION_MEASUREMENT_SYSTEM, 0);
        IoSetItemText(Index, 1, lpszValue);
    }

    AddLocaleInfoString1(0, IDS_REGION_ANSI_CODEPAGE, LOCALE_IDEFAULTANSICODEPAGE);
    AddLocaleInfoString1(0, IDS_REGION_OEM_CODEPAGE, LOCALE_IDEFAULTCODEPAGE);
    AddLocaleInfoString1(0, IDS_REGION_MAC_CODEPAGE, LOCALE_IDEFAULTMACCODEPAGE);
    AddLocaleInfoString1(0, IDS_REGION_LANG_CODE, LOCALE_ILANGUAGE);

    DebugEndReceiving();
}
