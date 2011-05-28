/*
 * PROJECT:         Aspia
 * FILE:            aspia/categories.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "main.h"


COLUMN_LIST StdColumnList[] =
{
    { IDS_PARAM, 260 },
    { IDS_VALUE, 270 },
    { 0 }
};

COLUMN_LIST SMARTColumnList[] =
{
    { IDS_SMART_ATTR_ID, 38 },
    { IDS_SMART_ATTR_NAME, 190 },
    { IDS_SMART_THRESHOLD, 70 },
    { IDS_SMART_ATTR_VALUE, 70 },
    { IDS_SMART_ATTR_WORST, 90 },
    { IDS_SMART_DATA, 70 },
    { 0 }
};

COLUMN_LIST DevicesColumnList[] =
{
    { IDS_DEVICE_NAME, 250 },
    { IDS_DEVICE_CLASS_NAME, 200 },
    { IDS_DEVICE_VERSION, 100 },
    { IDS_DEVICE_DRIVER_DATE, 90 },
    { IDS_DEVICE_DRIVER_VENDOR, 100 },
    { IDS_DEVICE_VENDOR_ID, 70 },
    { IDS_DEVICE_ESTIMATED_VENDOR, 150 },
    { IDS_DEVICE_ESTIMATED, 200 },
    { 0 }
};

COLUMN_LIST UnknownDevicesColumnList[] =
{
    { IDS_DEVICE_INSTANCE_ID, 250 },
    { IDS_DEVICE_VENDOR_ID, 70 },
    { IDS_DEVICE_ESTIMATED_VENDOR, 150 },
    { IDS_DEVICE_ESTIMATED, 200 },
    { 0 }
};

COLUMN_LIST UsersColumnList[] =
{
    { IDS_USER_NAME, 140 },
    { IDS_FULL_USER_NAME, 120 },
    { IDS_USER_DESC, 200 },
    { IDS_USER_ACCOUNT_DISABLED, 80 },
    { IDS_USER_PASSWD_CANT_CHANGE, 100 },
    { IDS_USER_PASSWORD_EXPIRED, 100 },
    { IDS_USER_DONT_EXPIRE_PASSWD, 100 },
    { IDS_USER_LOCKOUT, 80 },
    { IDS_USER_LAST_LOGON, 110 },
    { IDS_USER_NUM_LOGONS, 100 },
    { IDS_USER_BAD_PW_COUNT, 90 },
    { 0 }
};

COLUMN_LIST UsersGroupsColumnList[] =
{
    { IDS_USER_GROUP_NAME, 260 },
    { IDS_USER_GROUP_DESC, 240 },
    { 0 }
};

COLUMN_LIST ActiveUsersColumnList[] =
{
    { IDS_USER_NAME, 130 },
    { IDS_ACTUSERS_DOMAIN, 100 },
    { IDS_ACTUSERS_USERID, 30 },
    { IDS_ACTUSERS_STATE, 90 },
    { IDS_ACTUSERS_CLIENTNAME, 120 },
    { IDS_ACTUSERS_LOGON_TYPE, 100 },
    { 0 }
};

COLUMN_LIST EnvironColumnList[] =
{
    { IDS_FOLDER_NAME, 200 },
    { IDS_FOLDER_PATH, 300 },
    { 0 }
};

COLUMN_LIST FontsColumnList[] =
{
    { IDS_FONT_NAME, 260 },
    { IDS_FONT_FILENAME, 150 },
    { IDS_FONT_SIZE, 100 },
    { 0 }
};

COLUMN_LIST EventViewColumnList[] =
{
    { IDS_TYPE, 90 },
    { IDS_DATE, 70 },
    { IDS_TIME, 70 },
    { IDS_ELOG_SOURCE, 150 },
    { IDS_ELOG_CATEGORY, 100 },
    { IDS_ELOG_EVENT, 60 },
    { IDS_ELOG_USER, 120 },
    { IDS_ELOG_COMPUTER, 100 },
    { IDS_DESC, 450 },
    { 0 }
};

COLUMN_LIST InstalledAppColumnList[] =
{
    { IDS_NAME, 250 },
    { IDS_VERSION, 100 },
    { IDS_APP_PUBLISHER, 100 },
    { IDS_APP_HELPLINK, 100 },
    { IDS_APP_HELPPHONE, 100 },
    { IDS_APP_URL_UPDATEINFO, 100 },
    { IDS_APP_URL_INFOABOUT, 100 },
    { IDS_APP_INSTALLDATE, 70 },
    { IDS_APP_INSTALLLOCATION, 200 },
    { IDS_APP_UNINSTALL_STR, 200 },
    { IDS_APP_MODIFY_PATH, 200 },
    { 0 }
};

COLUMN_LIST InstalledUpdColumnList[] =
{
    { IDS_NAME, 250 },
    { IDS_APP_URL_INFOABOUT, 250 },
    { IDS_APP_UNINSTALL_STR, 250 },
    { 0 }
};

COLUMN_LIST ServicesColumnList[] =
{
    { IDS_SERVICE_DISP_NAME, 250 },
    { IDS_NAME, 150 },
    { IDS_DESC, 250 },
    { IDS_STATUS, 150 },
    { IDS_SERVICE_START_TYPE, 150 },
    { IDS_SERVICE_USER_NAME, 150 },
    { IDS_SERVICE_EX_FILE, 250 },
    { 0 }
};

COLUMN_LIST DriversColumnList[] =
{
    { IDS_SERVICE_DISP_NAME, 250 },
    { IDS_NAME, 150 },
    { IDS_DESC, 250 },
    { IDS_STATUS, 150 },
    { IDS_SERVICE_START_TYPE, 150 },
    { IDS_SERVICE_EX_FILE, 250 },
    { 0 }
};

COLUMN_LIST TaskMgrColumnList[] =
{
    { IDS_TASK_NAME, 120 },
    { IDS_TASK_FILEPATH, 250 },
    { IDS_TASK_USED_MEM, 70 },
    { IDS_TASK_USED_PAGEFILE, 70 },
    { IDS_TASK_DESC, 250 },
    { 0 }
};

COLUMN_LIST LicensesColumnList[] =
{
    { IDS_LICENSE_APP_NAME, 270 },
    { IDS_LICENSE_PRODUCT_KEY, 250 },
    { 0 }
};

COLUMN_LIST NetStatColumnList[] =
{
    { IDS_NETSTAT_PROC_NAME, 150 },
    { IDS_NETSTAT_PROTOCOL, 60 },
    { IDS_NETSTAT_LOCAL_ADDRESS, 160 },
    { IDS_NETSTAT_REMOTE_ADDRESS, 140 },
    { IDS_NETSTAT_STATE, 130 },
    { 0 }
};

COLUMN_LIST SharedColumnList[] =
{
    { IDS_NAME, 150 },
    { IDS_TYPE, 130 },
    { IDS_DESC, 230 },
    { IDS_SHARE_PATH, 250 },
    { IDS_SHARE_CURRENT_USES, 100 },
    { IDS_SHARE_MAX_USES, 150 },
    { 0 }
};

COLUMN_LIST RouteColumnList[] =
{
    { IDS_ROUTE_DEST_IP, 150 },
    { IDS_ROUTE_SUBNET_MASK, 150 },
    { IDS_ROUTE_GATEWAY, 150 },
    { IDS_ROUTE_METRIC, 70 },
    { 0 }
};

COLUMN_LIST IEHistoryColumnList[] =
{
    { IDS_IE_HISTORY_TIME, 70 },
    { IDS_IE_HISTORY_DATE, 70 },
    { IDS_IE_HISTORY_TITLE, 250 },
    { IDS_IE_HISTORY_URL, 300 },
    { 0 }
};

COLUMN_LIST IECookiesColumnList[] =
{
    { IDS_IE_COOKIE_URL_NAME, 250 },
    { IDS_IE_COOKIE_LAST_ACCESS, 120 },
    { IDS_IE_COOKIE_USE_COUNT, 50 },
    { IDS_IE_COOKIE_LAST_MODIFY, 120 },
    { IDS_IE_COOKIE_LAST_SYNC, 120 },
    { IDS_IE_COOKIE_EXPIRE, 120 },
    { IDS_IE_COOKIE_FILE_NAME, 500 },
    { 0 }
};

COLUMN_LIST OpenFilesColumnList[] =
{
    { IDS_OPENFILE_ID, 90 },
    { IDS_USER_NAME, 120 },
    { IDS_OPENFILE_COUNT, 80 },
    { IDS_OPENFILE_PATH, 250 },
    { 0 }
};

COLUMN_LIST FirewallColumnList[] =
{
    { IDS_FIREWALL_NAME, 90 },
    { IDS_FIREWALL_APP, 250 },
    { IDS_FIREWALL_STATE, 100 },
    { IDS_FIREWALL_ACTION, 100 },
    { IDS_FIREWALL_DIR, 110 },
    { IDS_FIREWALL_PROTO, 80 },
    { IDS_FIREWALL_SCOPE, 80 },
    { 0 }
};

COLUMN_LIST PasswdIEColumnList[] =
{
    { IDS_PASSWD_IE_URL, 250 },
    { IDS_PASSWD_IE_USERNAME, 150 },
    { IDS_PASSWD_IE_PASSWORD, 150 },
    { 0 }
};

CATEGORY_LIST DMICategoryList[] =
{
    { IDS_CAT_HW_DMI_BIOS,      IDI_CPU,      FALSE, TRUE, NULL, DMI_BIOSInfo,      StdColumnList, NULL },
    { IDS_CAT_HW_DMI_SYSTEM,    IDI_COMPUTER, FALSE, TRUE, NULL, DMI_SystemInfo,    StdColumnList, NULL },
    { IDS_CAT_HW_DMI_BOARD,     IDI_HW,       FALSE, TRUE, NULL, DMI_BoardInfo,     StdColumnList, NULL },
    { IDS_CAT_HW_DMI_ENCLOSURE, IDI_COMPUTER, FALSE, TRUE, NULL, DMI_EnclosureInfo, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_CACHE,     IDI_SENSOR,   FALSE, TRUE, NULL, DMI_CacheInfo,     StdColumnList, NULL },
    { IDS_CAT_HW_DMI_PROCESSOR, IDI_CPU,      FALSE, TRUE, NULL, DMI_CPUInfo,       StdColumnList, NULL },
    { IDS_CAT_HW_DMI_MEM,       IDI_HW,       FALSE, TRUE, NULL, DMI_RAMInfo,       StdColumnList, NULL },
    { IDS_CAT_HW_DMI_SLOT,      IDI_PORT,     FALSE, TRUE, NULL, DMI_SlotInfo,      StdColumnList, NULL },
    { IDS_CAT_HW_DMI_PORTS,     IDI_PORT,     FALSE, TRUE, NULL, DMI_PortsInfo,     StdColumnList, NULL },
    { IDS_CAT_HW_DMI_ONBOARD,   IDI_HW,       FALSE, TRUE, NULL, DMI_OnboardInfo,   StdColumnList, NULL },
    { IDS_CAT_HW_DMI_BATTERY,   IDI_BATTERY,  FALSE, TRUE, NULL, DMI_BatteryInfo,   StdColumnList, NULL },
    { 0 }
};

CATEGORY_LIST DataCategoryList[] =
{
    { IDS_CAT_HW_DATA_CD,    IDI_CD,  FALSE, TRUE, NULL, HW_CDInfo,       StdColumnList,   NULL },
    //{ IDS_CAT_HW_DATA_SCSI,  IDI_HDD, FALSE, NULL, HW_HDDSCSIInfo, NULL },
    { IDS_CAT_HW_DATA_ATA,   IDI_HDD, FALSE, TRUE, NULL, HW_HDDATAInfo,   StdColumnList,   NULL },
    { IDS_CAT_HW_DATA_SMART, IDI_HDD, FALSE, TRUE, NULL, HW_HDDSMARTInfo, SMARTColumnList, NULL },
    { 0 }
};

CATEGORY_LIST DevicesCategoryList[] =
{
    { IDS_CAT_HW_ALL_DEVICES,      IDI_HW,          FALSE, TRUE, NULL, HW_DevicesInfo,        DevicesColumnList,        NULL },
    { IDS_CAT_HW_UNKNOWN_DEVICES,  IDI_DISABLED_HW, FALSE, TRUE, NULL, HW_UnknownDevicesInfo, UnknownDevicesColumnList, NULL },
    { 0 }
};

CATEGORY_LIST DisplayCategoryList[] =
{
    { IDS_CAT_HW_WIN_VIDEO, IDI_MONITOR, FALSE, TRUE, NULL, HW_WinVideoInfo, StdColumnList, NULL },
    { IDS_CAT_HW_MONITOR,   IDI_MONITOR, FALSE, TRUE, NULL, HW_MonitorInfo, StdColumnList, NULL },
    { 0 }
};

CATEGORY_LIST HWCategoryList[] =
{
    { IDS_CAT_HW_DMI,      IDI_COMPUTER, FALSE, TRUE, NULL, NULL,            NULL,               DMICategoryList },
    { IDS_CAT_HW_SPD,      IDI_HW,       FALSE, TRUE, NULL, HW_SPDInfo,      StdColumnList,      NULL },
    { IDS_CAT_HW_CPU,      IDI_CPU,      FALSE, TRUE, NULL, HW_CPUInfo,      StdColumnList,      NULL },
    { IDS_CAT_HW_DATA,     IDI_HDD,      FALSE, TRUE, NULL, NULL,            NULL,               DataCategoryList },
    { IDS_CAT_HW_DISPLAY,  IDI_MONITOR,  FALSE, TRUE, NULL, NULL,            NULL,               DisplayCategoryList },
    { IDS_CAT_HW_POWER,    IDI_POWERICO, FALSE, TRUE, NULL, HW_PowerInfo,    StdColumnList,      NULL },
    { IDS_CAT_HW_PRINTERS, IDI_PRINTER,  FALSE, TRUE, NULL, HW_PrintersInfo, StdColumnList,      NULL },
    { IDS_CAT_HW_DEVICES,  IDI_HW,       FALSE, TRUE, NULL, NULL,            NULL,               DevicesCategoryList },
    { IDS_CAT_HW_SENSOR,   IDI_SENSOR,   FALSE, TRUE, NULL, HW_SensorInfo,   StdColumnList,      NULL },
    { 0 }
};

CATEGORY_LIST UpdSftCategoryList[] =
{
    { IDS_CAT_SOFTWARE_UPDATES,  IDI_UPDATES,  FALSE, TRUE, NULL, SOFTWARE_InstalledUpdInfo, InstalledUpdColumnList, NULL },
    { IDS_CAT_SOFTWARE_PROGRAMS, IDI_SOFTWARE, FALSE, TRUE, NULL, SOFTWARE_InstalledAppInfo, InstalledAppColumnList, NULL },
    { 0 }
};

CATEGORY_LIST PasswdCategoryList[] =
{
    { IDS_CAT_PASSWD_IE,         IDI_IE,         FALSE, TRUE, NULL, SOFTWARE_PasswdIEInfo,         PasswdIEColumnList, NULL },
    { IDS_CAT_PASSWD_OUTLOOK_E,  IDI_SOFTWARE,   FALSE, TRUE, NULL, SOFTWARE_PasswdOutlookInfo,    StdColumnList, NULL },
    { IDS_CAT_PASSWD_OUTLOOK,    IDI_SOFTWARE,   FALSE, TRUE, NULL, SOFTWARE_PasswdOutlookInfo,    StdColumnList, NULL },
    { IDS_CAT_PASSWD_CHROME,     IDI_SOFTWARE,   FALSE, TRUE, NULL, SOFTWARE_PasswdChromeInfo,     StdColumnList, NULL },
    { IDS_CAT_PASSWD_FIREFOX,    IDI_FIREFOX,    FALSE, TRUE, NULL, SOFTWARE_PasswdFirefoxInfo,    StdColumnList, NULL },
    { IDS_CAT_PASSWD_THUNDERBIRD,IDI_THUNDERBIRD,FALSE, TRUE, NULL, SOFTWARE_PasswdThunderbirdInfo,StdColumnList, NULL },
    { IDS_CAT_PASSWD_OPERA,      IDI_OPERA,      FALSE, TRUE, NULL, SOFTWARE_PasswdOperaInfo,      StdColumnList, NULL },
    { IDS_CAT_PASSWD_RAS,        IDI_RAS,        FALSE, TRUE, NULL, SOFTWARE_PasswdRASInfo,        StdColumnList, NULL },
    { IDS_CAT_PASSWD_MSRDP,      IDI_COMPUTER,   FALSE, TRUE, NULL, SOFTWARE_PasswdMsRDPInfo,      StdColumnList, NULL },
    { 0 }
};

CATEGORY_LIST SoftwareCategoryList[] =
{
    { IDS_CAT_SOFTWARE_UPD_SFT,  IDI_SOFTWARE, FALSE, TRUE, NULL, NULL,                  NULL,               UpdSftCategoryList },
    { IDS_CAT_SOFTWARE_SERVICES, IDI_SERVICES, FALSE, TRUE, NULL, SOFTWARE_ServicesInfo, ServicesColumnList, NULL },
    { IDS_CAT_SOFTWARE_DRIVERS,  IDI_HW,       FALSE, TRUE, NULL, SOFTWARE_DriversInfo,  DriversColumnList,  NULL },
    { IDS_CAT_SOFTWARE_PASSWORDS,IDI_PASSWORDS,FALSE, TRUE, NULL, NULL,                  NULL,               PasswdCategoryList },
    { IDS_CAT_SOFTWARE_TASKMGR,  IDI_TASKMGR,  FALSE, TRUE, NULL, SOFTWARE_TaskMgr,      TaskMgrColumnList,  NULL },
    { IDS_CAT_SOFTWARE_LICENSES, IDI_CONTACT,  FALSE, TRUE, NULL, SOFTWARE_LicensesInfo, LicensesColumnList, NULL },
    { 0 }
};

CATEGORY_LIST IECategoryList[] =
{
    { IDS_CAT_NETWORK_IE_PARAMS,  IDI_SERVICES, FALSE, TRUE, NULL, NETWORK_IEParamsInfo,  StdColumnList,       NULL },
    { IDS_CAT_NETWORK_IE_HISTORY, IDI_IE,       FALSE, TRUE, NULL, NETWORK_IEHistoryInfo, IEHistoryColumnList, NULL },
    { IDS_CAT_NETWORK_IE_COOKIE,  IDI_IE,       FALSE, TRUE, NULL, NETWORK_IECookieInfo,  IECookiesColumnList, NULL },
    { 0 }
};

CATEGORY_LIST NetworkCategoryList[] =
{
    { IDS_CAT_NETWORK_CARDS,   IDI_HW,      FALSE, TRUE, NULL, NETWORK_CardsInfo,     StdColumnList,       NULL },
    { IDS_CAT_NETWORK_RAS,     IDI_RAS,     FALSE, TRUE, NULL, NETWORK_RasInfo,       StdColumnList,       NULL },
    { IDS_CAT_NETWORK_NETSTAT, IDI_NETSTAT, FALSE, TRUE, NULL, NETWORK_NetStatInfo,   NetStatColumnList,   NULL },
    { IDS_CAT_NETWORK_SHARED,  IDI_SHARED,  FALSE, TRUE, NULL, NETWORK_SharedInfo,    SharedColumnList,    NULL },
    { IDS_CAT_NETWORK_FILES,   IDI_SHARED,  FALSE, TRUE, NULL, NETWORK_OpenFilesInfo, OpenFilesColumnList, NULL },
    { IDS_CAT_NETWORK_ROUTE,   IDI_ROUTE,   FALSE, TRUE, NULL, NETWORK_RouteInfo,     RouteColumnList,     NULL },
    { IDS_CAT_NETWORK_FIREWALL,IDI_FIREWALL,FALSE, TRUE, NULL, NETWORK_FirewallInfo,  FirewallColumnList,  NULL },
    { IDS_CAT_NETWORK_IE,      IDI_IE,      FALSE, TRUE, NULL, NULL,                  NULL,                IECategoryList },
    { 0 }
};

CATEGORY_LIST UsersCategoryList[] =
{
    { IDS_CAT_OS_USERS,       IDI_USER,  FALSE, TRUE, NULL, OS_UsersInfo,       UsersColumnList,       NULL },
    { IDS_CAT_OS_USERGROUP,   IDI_USERS, FALSE, TRUE, NULL, OS_UsersGroupsInfo, UsersGroupsColumnList, NULL },
    { IDS_CAT_OS_ACTIVEUSERS, IDI_USERS, FALSE, TRUE, NULL, OS_ActiveUsersInfo, ActiveUsersColumnList, NULL },
    { 0 }
};

CATEGORY_LIST EventsCategoryList[] =
{
    { IDS_ELOG_APPLICATIONS, IDI_APPS,     FALSE, FALSE, NULL, OS_AppEventsInfo,      EventViewColumnList, NULL },
    { IDS_ELOG_SECURITY,     IDI_SECURITY, FALSE, FALSE, NULL, OS_SecurityEventsInfo, EventViewColumnList, NULL },
    { IDS_ELOG_SYSTEM,       IDI_COMPUTER, FALSE, FALSE, NULL, OS_SystemEventsInfo,   EventViewColumnList, NULL },
    { 0 }
};

CATEGORY_LIST OSCategoryList[] =
{
    { IDS_CAT_OS_REGDATA,      IDI_COMPUTER,   FALSE, TRUE, NULL, OS_RegInformation,    StdColumnList,      NULL },
    { IDS_CAT_OS_DESKTOP,      IDI_DESKTOP,    FALSE, TRUE, NULL, OS_DesktopInfo,       StdColumnList,      NULL },
    { IDS_CAT_OS_TASKSCHED,    IDI_TIME,       FALSE, TRUE, NULL, OS_TaskSchedulerInfo, StdColumnList,      NULL },
    { IDS_CAT_OS_USERS_GROUPS, IDI_USERS,      FALSE, TRUE, NULL, NULL,                 NULL,               UsersCategoryList },
    { IDS_CAT_OS_REGPARAM,     IDI_LOCALES,    FALSE, TRUE, NULL, OS_RegionalParamInfo, StdColumnList,      NULL },
    { IDS_CAT_OS_ENVIRON,      IDI_APPS,       FALSE, TRUE, NULL, OS_EnvironInfo,       EnvironColumnList,  NULL },
    { IDS_CAT_OS_SYSFOLDERS,   IDI_SYSFOLDERS, FALSE, TRUE, NULL, OS_SysFoldersInfo,    EnvironColumnList,  NULL },
    { IDS_CAT_OS_FONTS,        IDI_FONTS,      FALSE, TRUE, NULL, OS_FontsInfo,         FontsColumnList,    NULL },
    { IDS_CAT_OS_EVENTVIEW,    IDI_EVENTVWR,   FALSE, FALSE, NULL, NULL,                NULL,               EventsCategoryList },
    { IDS_CAT_OS_AUTORUN,      IDI_SOFTWARE,   FALSE, TRUE, NULL, OS_AutorunInfo,       StdColumnList,      NULL },
    { IDS_CAT_OS_PREVENTS,     IDI_PREVENT,    FALSE, TRUE, NULL, OS_PreventsInfo,      StdColumnList,      NULL },
    { 0 }
};

CATEGORY_LIST RootCategoryList[] =
{
    { IDS_CAT_SUMMARY,  IDI_COMPUTER, TRUE, TRUE, NULL, ShowSummaryInfo, StdColumnList,      NULL },
    { IDS_CAT_HW,       IDI_HW,       TRUE, TRUE, NULL, NULL,            NULL,               HWCategoryList },
    { IDS_CAT_SOFTWARE, IDI_SOFTWARE, TRUE, TRUE, NULL, NULL,            NULL,               SoftwareCategoryList },
    { IDS_CAT_NETWORK,  IDI_NETWORK,  TRUE, TRUE, NULL, NULL,            NULL,               NetworkCategoryList },
    { IDS_CAT_OS,       IDI_WINDOWS,  TRUE, TRUE, NULL, NULL,            NULL,               OSCategoryList },
    { 0 }
};
