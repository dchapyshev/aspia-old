/*
 * PROJECT:         Aspia
 * FILE:            aspia_exe/categories.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_exe.h"

COLUMN_LIST StdColumnList[] =
{
    { IDS_PARAM, 260 },
    { IDS_VALUE, 270 },
    { 0 }
};

COLUMN_LIST SMARTColumnList[] =
{
    { IDS_SMART_ATTR_ID,    42  },
    { IDS_SMART_ATTR_NAME,  190 },
    { IDS_SMART_THRESHOLD,  70  },
    { IDS_SMART_ATTR_VALUE, 70  },
    { IDS_SMART_ATTR_WORST, 90  },
    { IDS_SMART_DATA,       70  },
    { 0 }
};

COLUMN_LIST DevicesColumnList[] =
{
    { IDS_DEVICE_NAME,             250 },
    { IDS_DEVICE_CLASS_NAME,       200 },
    { IDS_DEVICE_VERSION,          100 },
    { IDS_DEVICE_DRIVER_DATE,      90  },
    { IDS_DEVICE_DRIVER_VENDOR,    100 },
    { IDS_DEVICE_VENDOR_ID,        70  },
    { IDS_DEVICE_ESTIMATED_VENDOR, 150 },
    { IDS_DEVICE_ESTIMATED,        200 },
    { 0 }
};

COLUMN_LIST UnknownDevicesColumnList[] =
{
    { IDS_DEVICE_INSTANCE_ID,      250 },
    { IDS_DEVICE_VENDOR_ID,        70  },
    { IDS_DEVICE_ESTIMATED_VENDOR, 150 },
    { IDS_DEVICE_ESTIMATED,        200 },
    { 0 }
};

COLUMN_LIST UsersColumnList[] =
{
    { IDS_USER_NAME,               140 },
    { IDS_FULL_USER_NAME,          120 },
    { IDS_USER_DESC,               200 },
    { IDS_USER_ACCOUNT_DISABLED,   80  },
    { IDS_USER_PASSWD_CANT_CHANGE, 100 },
    { IDS_USER_PASSWORD_EXPIRED,   100 },
    { IDS_USER_DONT_EXPIRE_PASSWD, 100 },
    { IDS_USER_LOCKOUT,            80  },
    { IDS_USER_LAST_LOGON,         110 },
    { IDS_USER_NUM_LOGONS,         100 },
    { IDS_USER_BAD_PW_COUNT,       90  },
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
    { IDS_USER_NAME,           130 },
    { IDS_ACTUSERS_DOMAIN,     100 },
    { IDS_ACTUSERS_USERID,     30  },
    { IDS_ACTUSERS_STATE,      90  },
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
    { IDS_FONT_NAME,     260 },
    { IDS_FONT_FILENAME, 150 },
    { IDS_FONT_SIZE,     100 },
    { 0 }
};

COLUMN_LIST EventViewColumnList[] =
{
    { IDS_TYPE,          90  },
    { IDS_DATE,          70  },
    { IDS_TIME,          70  },
    { IDS_ELOG_SOURCE,   150 },
    { IDS_ELOG_CATEGORY, 100 },
    { IDS_ELOG_EVENT,    60  },
    { IDS_ELOG_USER,     120 },
    { IDS_ELOG_COMPUTER, 100 },
    { IDS_DESC,          450 },
    { 0 }
};

COLUMN_LIST InstalledAppColumnList[] =
{
    { IDS_NAME,                250 },
    { IDS_VERSION,             100 },
    { IDS_APP_PUBLISHER,       100 },
    { IDS_APP_HELPLINK,        100 },
    { IDS_APP_HELPPHONE,       100 },
    { IDS_APP_URL_UPDATEINFO,  100 },
    { IDS_APP_URL_INFOABOUT,   100 },
    { IDS_APP_INSTALLDATE,     70  },
    { IDS_APP_INSTALLLOCATION, 200 },
    { IDS_APP_UNINSTALL_STR,   200 },
    { IDS_APP_MODIFY_PATH,     200 },
    { 0 }
};

COLUMN_LIST InstalledUpdColumnList[] =
{
    { IDS_NAME,              250 },
    { IDS_APP_URL_INFOABOUT, 250 },
    { IDS_APP_UNINSTALL_STR, 250 },
    { 0 }
};

COLUMN_LIST ServicesColumnList[] =
{
    { IDS_SERVICE_DISP_NAME,  250 },
    { IDS_NAME,               150 },
    { IDS_DESC,               250 },
    { IDS_STATUS,             150 },
    { IDS_SERVICE_START_TYPE, 150 },
    { IDS_SERVICE_USER_NAME,  150 },
    { IDS_SERVICE_EX_FILE,    250 },
    { 0 }
};

COLUMN_LIST DriversColumnList[] =
{
    { IDS_SERVICE_DISP_NAME,  250 },
    { IDS_NAME,               150 },
    { IDS_DESC,               250 },
    { IDS_STATUS,             150 },
    { IDS_SERVICE_START_TYPE, 150 },
    { IDS_SERVICE_EX_FILE,    250 },
    { 0 }
};

COLUMN_LIST TaskMgrColumnList[] =
{
    { IDS_TASK_NAME,          120 },
    { IDS_TASK_FILEPATH,      250 },
    { IDS_TASK_USED_MEM,      70  },
    { IDS_TASK_USED_PAGEFILE, 70  },
    { IDS_TASK_DESC,          250 },
    { 0 }
};

COLUMN_LIST LicensesColumnList[] =
{
    { IDS_LICENSE_APP_NAME,    270 },
    { IDS_LICENSE_PRODUCT_KEY, 250 },
    { 0 }
};

COLUMN_LIST NetStatColumnList[] =
{
    { IDS_NETSTAT_PROC_NAME,      150 },
    { IDS_NETSTAT_PROTOCOL,       60  },
    { IDS_NETSTAT_LOCAL_ADDRESS,  160 },
    { IDS_NETSTAT_REMOTE_ADDRESS, 140 },
    { IDS_NETSTAT_STATE,          130 },
    { 0 }
};

COLUMN_LIST SharedColumnList[] =
{
    { IDS_NAME,               150 },
    { IDS_TYPE,               130 },
    { IDS_DESC,               230 },
    { IDS_SHARE_PATH,         250 },
    { IDS_SHARE_CURRENT_USES, 100 },
    { IDS_SHARE_MAX_USES,     150 },
    { 0 }
};

COLUMN_LIST RouteColumnList[] =
{
    { IDS_ROUTE_DEST_IP,     150 },
    { IDS_ROUTE_SUBNET_MASK, 150 },
    { IDS_ROUTE_GATEWAY,     150 },
    { IDS_ROUTE_METRIC,      70  },
    { 0 }
};

COLUMN_LIST IEHistoryColumnList[] =
{
    { IDS_IE_HISTORY_TIME,  70  },
    { IDS_IE_HISTORY_DATE,  70  },
    { IDS_IE_HISTORY_TITLE, 250 },
    { IDS_IE_HISTORY_URL,   300 },
    { 0 }
};

COLUMN_LIST IECookiesColumnList[] =
{
    { IDS_IE_COOKIE_URL_NAME,    250 },
    { IDS_IE_COOKIE_LAST_ACCESS, 120 },
    { IDS_IE_COOKIE_USE_COUNT,   50  },
    { IDS_IE_COOKIE_LAST_MODIFY, 120 },
    { IDS_IE_COOKIE_LAST_SYNC,   120 },
    { IDS_IE_COOKIE_EXPIRE,      120 },
    { IDS_IE_COOKIE_FILE_NAME,   500 },
    { 0 }
};

COLUMN_LIST OpenFilesColumnList[] =
{
    { IDS_OPENFILE_ID,    90  },
    { IDS_USER_NAME,      120 },
    { IDS_OPENFILE_COUNT, 80  },
    { IDS_OPENFILE_PATH,  250 },
    { 0 }
};

COLUMN_LIST FirewallColumnList[] =
{
    { IDS_FIREWALL_NAME,   90  },
    { IDS_FIREWALL_APP,    250 },
    { IDS_FIREWALL_STATE,  100 },
    { IDS_FIREWALL_ACTION, 100 },
    { IDS_FIREWALL_DIR,    110 },
    { IDS_FIREWALL_PROTO,  80  },
    { IDS_FIREWALL_SCOPE,  80  },
    { 0 }
};

COLUMN_LIST SysFilesColumnList[] =
{
    { IDS_SYSFILES_NAME,    110 },
    { IDS_SYSFILES_SIZE,    70  },
    { IDS_SIGNED_FILE,      40  },
    { IDS_SYSFILES_VERSION, 90  },
    { IDS_SYSFILES_MANUF,   110 },
    { IDS_SYSFILES_DESC,    230 },
    { 0 }
};

COLUMN_LIST FileTypesColumnList[] =
{
    { IDS_FILETYPES_EXT,  90  },
    { IDS_FILETYPES_DESC, 220 },
    { IDS_FILETYPES_TYPE, 170 },
    { 0 }
};

COLUMN_LIST MMDevsColumnList[] =
{
    { IDS_MMDEV_NAME,     90  },
    { IDS_MMDEV_ID,       220 },
    { IDS_MMDEV_FORMATS,  170 },
    { IDS_MMDEV_CHANNELS, 170 },
    { IDS_MMDEV_SUPPORTS, 170 },
    { 0 }
};

COLUMN_LIST CPLAppletsColumnList[] =
{
    { IDS_CPL_NAME, 270 },
    { IDS_CPL_DESC, 250 },
    { 0 }
};

CATEGORY_LIST DMICategoryList[] =
{
    { IDS_CAT_HW_DMI_BIOS,      IDI_CPU,      IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_BIOSInfo,      NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_SYSTEM,    IDI_COMPUTER, IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_SystemInfo,    NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_BOARD,     IDI_HW,       IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_BoardInfo,     NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_ENCLOSURE, IDI_ENCLOSURE,IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_EnclosureInfo, NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_CACHE,     IDI_SENSOR,   IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_CacheInfo,     NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_PROCESSOR, IDI_CPU,      IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_CPUInfo,       NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_MEM,       IDI_HW,       IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_RAMInfo,       NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_SLOT,      IDI_PORT,     IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_SlotInfo,      NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_PORTS,     IDI_PORT,     IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_PortsInfo,     NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_ONBOARD,   IDI_HW,       IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_OnboardInfo,   NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_POINTING,  IDI_MOUSE,    IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_PointingInfo,  NULL, StdColumnList, NULL },
    { IDS_CAT_HW_DMI_BATTERY,   IDI_BATTERY,  IDR_POPUP, FALSE, TRUE, FALSE, NULL, DMI_BatteryInfo,   NULL, StdColumnList, NULL },
    { 0 }
};

CATEGORY_LIST DataCategoryList[] =
{
    { IDS_CAT_HW_DATA_CD,    IDI_CD,  IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_CDInfo,       NULL, StdColumnList,   NULL },
    //{ IDS_CAT_HW_DATA_SCSI,  IDI_HDD, IDR_POPUP, FALSE, FALSE, NULL, HW_HDDSCSIInfo, NULL, NULL },
    { IDS_CAT_HW_DATA_ATA,   IDI_HDD, IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_HDDATAInfo,   NULL, StdColumnList,   NULL },
    { IDS_CAT_HW_DATA_SMART, IDI_HDD, IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_HDDSMARTInfo, NULL, SMARTColumnList, NULL },
    { 0 }
};

CATEGORY_LIST DevicesCategoryList[] =
{
    { IDS_CAT_HW_ALL_DEVICES,     IDI_HW,          IDR_DEVICES, FALSE, TRUE, TRUE, NULL, HW_DevicesInfo,        HW_DevicesFree, DevicesColumnList,        NULL },
    { IDS_CAT_HW_UNKNOWN_DEVICES, IDI_DISABLED_HW, IDR_DEVICES, FALSE, TRUE, TRUE, NULL, HW_UnknownDevicesInfo, HW_DevicesFree, UnknownDevicesColumnList, NULL },
    { 0 }
};

CATEGORY_LIST DisplayCategoryList[] =
{
    { IDS_CAT_HW_WIN_VIDEO, IDI_MONITOR, IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_WinVideoInfo, NULL, StdColumnList, NULL },
    { IDS_CAT_HW_MONITOR,   IDI_MONITOR, IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_MonitorInfo,  NULL, StdColumnList, NULL },
    { IDS_CAT_HW_OPENGL,    IDI_OPENGL,  IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_OpenGlInfo,   NULL, StdColumnList, NULL },
    { 0 }
};

CATEGORY_LIST MMediaCategoryList[] =
{
    { IDS_CAT_HW_MMEDIA_DEV,    IDI_AUDIO, IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_MediaDevicesInfo, NULL, MMDevsColumnList, NULL },
    { IDS_CAT_HW_MMEDIA_ACODEC, IDI_AUDIO, IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_MediaACodecsInfo, NULL, StdColumnList,    NULL },
    { IDS_CAT_HW_MMEDIA_VCODEC, IDI_AUDIO, IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_MediaVCodecsInfo, NULL, StdColumnList,    NULL },
    { IDS_CAT_HW_MMEDIA_MCI,    IDI_AUDIO, IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_MediaMCIInfo,     NULL, StdColumnList,    NULL },
    { 0 }
};

CATEGORY_LIST HWCategoryList[] =
{
    { IDS_CAT_HW_DMI,      IDI_COMPUTER, 0,         FALSE, TRUE, FALSE, NULL, NULL,            NULL, NULL,          DMICategoryList     },
    { IDS_CAT_HW_SPD,      IDI_HW,       IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_SPDInfo,      NULL, StdColumnList, NULL                },
    { IDS_CAT_HW_CPU,      IDI_CPU,      IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_CPUInfo,      NULL, StdColumnList, NULL                },
    { IDS_CAT_HW_DATA,     IDI_HDD,      0,         FALSE, TRUE, FALSE, NULL, NULL,            NULL, NULL,          DataCategoryList    },
    { IDS_CAT_HW_DISPLAY,  IDI_MONITOR,  0,         FALSE, TRUE, FALSE, NULL, NULL,            NULL, NULL,          DisplayCategoryList },
    //{ IDS_CAT_HW_MMEDIA,   IDI_AUDIO,    0,         FALSE, TRUE, FALSE, NULL, NULL,            NULL, NULL,          MMediaCategoryList  },
    { IDS_CAT_HW_POWER,    IDI_POWERICO, IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_PowerInfo,    NULL, StdColumnList, NULL                },
    { IDS_CAT_HW_PRINTERS, IDI_PRINTER,  IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_PrintersInfo, NULL, StdColumnList, NULL                },
    { IDS_CAT_HW_DEVICES,  IDI_HW,       0,         FALSE, TRUE, FALSE, NULL, NULL,            NULL, NULL,          DevicesCategoryList },
    { IDS_CAT_HW_SENSOR,   IDI_SENSOR,   IDR_POPUP, FALSE, TRUE, FALSE, NULL, HW_SensorInfo,   NULL, StdColumnList, NULL                },
    { 0 }
};

CATEGORY_LIST UpdSftCategoryList[] =
{
    { IDS_CAT_SOFTWARE_UPDATES,  IDI_UPDATES,  IDR_INSTALLED_SOFTWARE, FALSE, TRUE, TRUE, NULL, SOFTWARE_InstalledUpdInfo, SOFTWARE_InstalledAppsFree, InstalledUpdColumnList, NULL },
    { IDS_CAT_SOFTWARE_PROGRAMS, IDI_SOFTWARE, IDR_INSTALLED_SOFTWARE, FALSE, TRUE, TRUE, NULL, SOFTWARE_InstalledAppInfo, SOFTWARE_InstalledAppsFree, InstalledAppColumnList, NULL },
    { 0 }
};

CATEGORY_LIST SoftwareCategoryList[] =
{
    { IDS_CAT_SOFTWARE_UPD_SFT,  IDI_SOFTWARE, 0,           FALSE, TRUE, FALSE, NULL, NULL,                  NULL, NULL,               UpdSftCategoryList },
    { IDS_CAT_SOFTWARE_SERVICES, IDI_SERVICES, IDR_POPUP,   FALSE, TRUE, TRUE,  NULL, SOFTWARE_ServicesInfo, NULL, ServicesColumnList, NULL },
    { IDS_CAT_SOFTWARE_DRIVERS,  IDI_HW,       IDR_POPUP,   FALSE, TRUE, TRUE,  NULL, SOFTWARE_DriversInfo,  NULL, DriversColumnList,  NULL },
    { IDS_CAT_SOFTWARE_FILETYPES,IDI_APPS,     IDR_POPUP,   FALSE, TRUE, TRUE,  NULL, SOFTWARE_FileTypesInfo,NULL, FileTypesColumnList,NULL },
    { IDS_CAT_SOFTWARE_TASKMGR,  IDI_TASKMGR,  IDR_TASKMGR, FALSE, TRUE, TRUE,  NULL, SOFTWARE_TaskMgr,      NULL, TaskMgrColumnList,  NULL },
    { IDS_CAT_SOFTWARE_LICENSES, IDI_CONTACT,  IDR_POPUP,   FALSE, TRUE, TRUE,  NULL, SOFTWARE_LicensesInfo, NULL, LicensesColumnList, NULL },
    { 0 }
};

CATEGORY_LIST IECategoryList[] =
{
    { IDS_CAT_NETWORK_IE_PARAMS,  IDI_SERVICES, IDR_POPUP, FALSE, TRUE, FALSE, NULL, NETWORK_IEParamsInfo,  NULL, StdColumnList,       NULL },
    { IDS_CAT_NETWORK_IE_HISTORY, IDI_IE,       IDR_POPUP, FALSE, TRUE, TRUE,  NULL, NETWORK_IEHistoryInfo, NULL, IEHistoryColumnList, NULL },
    { IDS_CAT_NETWORK_IE_COOKIE,  IDI_IE,       IDR_POPUP, FALSE, TRUE, TRUE,  NULL, NETWORK_IECookieInfo,  NULL, IECookiesColumnList, NULL },
    { 0 }
};

CATEGORY_LIST NetworkCategoryList[] =
{
    { IDS_CAT_NETWORK_CARDS,   IDI_HW,      IDR_POPUP, FALSE, TRUE, FALSE, NULL, NETWORK_CardsInfo,     NULL, StdColumnList,       NULL },
    { IDS_CAT_NETWORK_RAS,     IDI_RAS,     IDR_POPUP, FALSE, TRUE, FALSE, NULL, NETWORK_RasInfo,       NULL, StdColumnList,       NULL },
    { IDS_CAT_NETWORK_NETSTAT, IDI_NETSTAT, IDR_POPUP, FALSE, TRUE, TRUE,  NULL, NETWORK_NetStatInfo,   NULL, NetStatColumnList,   NULL },
    { IDS_CAT_NETWORK_SHARED,  IDI_SHARED,  IDR_POPUP, FALSE, TRUE, TRUE,  NULL, NETWORK_SharedInfo,    NULL, SharedColumnList,    NULL },
    { IDS_CAT_NETWORK_FILES,   IDI_SHARED,  IDR_POPUP, FALSE, TRUE, TRUE,  NULL, NETWORK_OpenFilesInfo, NULL, OpenFilesColumnList, NULL },
    { IDS_CAT_NETWORK_ROUTE,   IDI_ROUTE,   IDR_POPUP, FALSE, TRUE, TRUE,  NULL, NETWORK_RouteInfo,     NULL, RouteColumnList,     NULL },
    //{ IDS_CAT_NETWORK_FIREWALL,IDI_FIREWALL,IDR_POPUP, FALSE, TRUE, FALSE, NULL, NETWORK_FirewallInfo,  NULL, FirewallColumnList,  NULL },
    { IDS_CAT_NETWORK_IE,      IDI_IE,      0,         FALSE, TRUE, FALSE, NULL, NULL,                  NULL, NULL,                IECategoryList },
    { 0 }
};

CATEGORY_LIST UsersCategoryList[] =
{
    { IDS_CAT_OS_USERS,       IDI_USER,  IDR_POPUP, FALSE, TRUE, TRUE, NULL, OS_UsersInfo,       NULL, UsersColumnList,       NULL },
    { IDS_CAT_OS_USERGROUP,   IDI_USERS, IDR_POPUP, FALSE, TRUE, TRUE, NULL, OS_UsersGroupsInfo, NULL, UsersGroupsColumnList, NULL },
    { IDS_CAT_OS_ACTIVEUSERS, IDI_USERS, IDR_POPUP, FALSE, TRUE, TRUE, NULL, OS_ActiveUsersInfo, NULL, ActiveUsersColumnList, NULL },
    { 0 }
};

CATEGORY_LIST EventsCategoryList[] =
{
    { IDS_ELOG_APPLICATIONS, IDI_APPS,     IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_AppEventsInfo,      NULL, EventViewColumnList, NULL },
    { IDS_ELOG_SECURITY,     IDI_SECURITY, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SecurityEventsInfo, NULL, EventViewColumnList, NULL },
    { IDS_ELOG_SYSTEM,       IDI_COMPUTER, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SystemEventsInfo,   NULL, EventViewColumnList, NULL },
    { 0 }
};

CATEGORY_LIST SysFilesCategoryList[] =
{
    { IDS_CAT_SYSFILES_KNOWN, IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesKnownInfo, NULL, StdColumnList,      NULL },
    { IDS_CAT_SYSFILES_DLL,   IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesDLLInfo,   NULL, SysFilesColumnList, NULL },
    { IDS_CAT_SYSFILES_AX,    IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesAXInfo,    NULL, SysFilesColumnList, NULL },
    { IDS_CAT_SYSFILES_EXE,   IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesEXEInfo,   NULL, SysFilesColumnList, NULL },
    { IDS_CAT_SYSFILES_SYS,   IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesSYSInfo,   NULL, SysFilesColumnList, NULL },
    { IDS_CAT_OS_FONTS,       IDI_FONTS,IDR_POPUP, FALSE, TRUE,  TRUE, NULL, OS_FontsInfo,         NULL, FontsColumnList,    NULL },
    { 0 }
};

CATEGORY_LIST OSCategoryList[] =
{
    { IDS_CAT_OS_REGDATA,      IDI_COMPUTER,   IDR_POPUP, FALSE, TRUE,  FALSE, NULL, OS_RegInformation,    NULL, StdColumnList,        NULL },
    { IDS_CAT_OS_DESKTOP,      IDI_DESKTOP,    IDR_POPUP, FALSE, TRUE,  FALSE, NULL, OS_DesktopInfo,       NULL, StdColumnList,        NULL },
    { IDS_CAT_OS_TASKSCHED,    IDI_TIME,       IDR_POPUP, FALSE, TRUE,  FALSE, NULL, OS_TaskSchedulerInfo, NULL, StdColumnList,        NULL },
    { IDS_CAT_OS_USERS_GROUPS, IDI_USERS,      0,         FALSE, TRUE,  FALSE, NULL, NULL,                 NULL, NULL,                 UsersCategoryList },
    { IDS_CAT_OS_REGPARAM,     IDI_LOCALES,    IDR_POPUP, FALSE, TRUE,  FALSE, NULL, OS_RegionalParamInfo, NULL, StdColumnList,        NULL },
    { IDS_CAT_OS_ENVIRON,      IDI_APPS,       IDR_POPUP, FALSE, TRUE,  TRUE,  NULL, OS_EnvironInfo,       NULL, EnvironColumnList,    NULL },
    { IDS_CAT_OS_SYSFOLDERS,   IDI_SYSFOLDERS, IDR_POPUP, FALSE, TRUE,  TRUE,  NULL, OS_SysFoldersInfo,    NULL, EnvironColumnList,    NULL },
    { IDS_CAT_OS_EVENTVIEW,    IDI_EVENTVWR,   0,         FALSE, FALSE, FALSE, NULL, NULL,                 NULL, NULL,                 EventsCategoryList },
    { IDS_CAT_OS_SYSFILES,     IDI_WINDOWS,    0,         FALSE, FALSE, FALSE, NULL, NULL,                 NULL, NULL,                 SysFilesCategoryList },
    //{ IDS_CAT_OS_CPL_APPLETS,  IDI_SETTINGS,   IDR_POPUP, FALSE, TRUE,  TRUE,  NULL, OS_CPLAppletsInfo,    NULL, CPLAppletsColumnList, NULL },
    { IDS_CAT_OS_AUTORUN,      IDI_SOFTWARE,   IDR_POPUP, FALSE, TRUE,  FALSE, NULL, OS_AutorunInfo,       NULL, StdColumnList,        NULL },
    { IDS_CAT_OS_PREVENTS,     IDI_PREVENT,    IDR_POPUP, FALSE, TRUE,  FALSE, NULL, OS_PreventsInfo,      NULL, StdColumnList,        NULL },
    { 0 }
};

CATEGORY_LIST RootCategoryList[] =
{
    { IDS_CAT_SUMMARY,  IDI_COMPUTER, IDR_POPUP, TRUE, TRUE, FALSE, NULL, ShowSummaryInfo, NULL, StdColumnList, NULL                 },
    { IDS_CAT_HW,       IDI_HW,       0,         TRUE, TRUE, FALSE, NULL, NULL,            NULL, NULL,          HWCategoryList       },
    { IDS_CAT_SOFTWARE, IDI_SOFTWARE, 0,         TRUE, TRUE, FALSE, NULL, NULL,            NULL, NULL,          SoftwareCategoryList },
    { IDS_CAT_NETWORK,  IDI_NETWORK,  0,         TRUE, TRUE, FALSE, NULL, NULL,            NULL, NULL,          NetworkCategoryList  },
    { IDS_CAT_OS,       IDI_WINDOWS,  0,         TRUE, TRUE, FALSE, NULL, NULL,            NULL, NULL,          OSCategoryList       },
    { 0 }
};
