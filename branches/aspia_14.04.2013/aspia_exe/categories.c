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
    { IDS_SYSFILES_VERSION, 120  },
    { IDS_SYSFILES_MANUF,   150 },
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
    { L"dmi_bios",             IDS_CAT_HW_DMI_BIOS,               IDI_CPU,        IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_BIOSInfo,             NULL, StdColumnList, NULL },
    { L"dmi_bios_lang",        IDS_CAT_HW_DMI_BIOS_LANGUAGE,      IDI_CPU,        IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_BiosLanguageInfo,     NULL, StdColumnList, NULL },
    { L"dmi_system",           IDS_CAT_HW_DMI_SYSTEM,             IDI_COMPUTER,   IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_SystemInfo,           NULL, StdColumnList, NULL },
    { L"dmi_board",            IDS_CAT_HW_DMI_BOARD,              IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_BoardInfo,            NULL, StdColumnList, NULL },
    { L"dmi_enclosure",        IDS_CAT_HW_DMI_ENCLOSURE,          IDI_ENCLOSURE,  IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_EnclosureInfo,        NULL, StdColumnList, NULL },
    { L"dmi_cache",            IDS_CAT_HW_DMI_CACHE,              IDI_SENSOR,     IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_CacheInfo,            NULL, StdColumnList, NULL },
    { L"dmi_cpu",              IDS_CAT_HW_DMI_PROCESSOR,          IDI_CPU,        IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_CPUInfo,              NULL, StdColumnList, NULL },
    { L"dmi_mem",              IDS_CAT_HW_DMI_MEM,                IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_RAMInfo,              NULL, StdColumnList, NULL },
    { L"dmi_mem_ctrl",         IDS_CAT_HW_DMI_MEM_CTRL,           IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_MemCtrlInfo,          NULL, StdColumnList, NULL },
    { L"dmi_mem_module",       IDS_CAT_HW_DMI_MEM_MODULE,         IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_MemModuleInfo,        NULL, StdColumnList, NULL },
    { L"dmi_mem_array",        IDS_CAT_HW_DMI_MEM_ARRAY,          IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_MemArrayInfo,         NULL, StdColumnList, NULL },
    { L"dmi_physic_mem_array", IDS_CAT_HW_DMI_PHYSIC_MEM_ARRAY,   IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_PhysicMemArrayInfo,   NULL, StdColumnList, NULL },
    { L"dmi_manag_dev",        IDS_CAT_HW_DMI_MANAGEMENT_DEV,     IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_ManagDevInfo,         NULL, StdColumnList, NULL },
    { L"dmi_manag_dev_comp",   IDS_CAT_HW_DMI_MANAG_DEV_COMPONENT,IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_ManagDevComponentInfo,NULL, StdColumnList, NULL },
    { L"dmi_manag_dev_thresh", IDS_CAT_HW_DMI_MANAG_DEV_THRESHOLD,IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_ManagDevThresholdInfo,NULL, StdColumnList, NULL },
    { L"dmi_manag_ctrl_host",  IDS_CAT_HW_DMI_MANAG_CTRL_HOST,    IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_ManagCtrlHostInfo,    NULL, StdColumnList, NULL },
    { L"dmi_slot",             IDS_CAT_HW_DMI_SLOT,               IDI_PORT,       IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_SlotInfo,             NULL, StdColumnList, NULL },
    { L"dmi_ports",            IDS_CAT_HW_DMI_PORTS,              IDI_PORT,       IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_PortsInfo,            NULL, StdColumnList, NULL },
    { L"dmi_onboard",          IDS_CAT_HW_DMI_ONBOARD,            IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_OnboardInfo,          NULL, StdColumnList, NULL },
    { L"dmi_pointing",         IDS_CAT_HW_DMI_POINTING,           IDI_MOUSE,      IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_PointingInfo,         NULL, StdColumnList, NULL },
    { L"dmi_battery",          IDS_CAT_HW_DMI_BATTERY,            IDI_BATTERY,    IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_BatteryInfo,          NULL, StdColumnList, NULL },
    { L"dmi_temp_probe",       IDS_CAT_HW_DMI_TEMP_PROBE,         IDI_TEMPERATURE,IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_TempProbeInfo,        NULL, StdColumnList, NULL },
    { L"dmi_electr_probe",     IDS_CAT_HW_DMI_ELECTR_PROBE,       IDI_VOLTAGE,    IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_ElectrProbeInfo,      NULL, StdColumnList, NULL },
    { L"dmi_sys_pwr_supply",   IDS_CAT_HW_DMI_SYS_POWER_SUPPLY,   IDI_VOLTAGE,    IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_SysPowerSupplyInfo,   NULL, StdColumnList, NULL },
    { L"dmi_voltage_probe",    IDS_CAT_HW_DMI_VOLTAGE_PROBE,      IDI_VOLTAGE,    IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_VoltageProbeInfo,     NULL, StdColumnList, NULL },
    { L"dmi_cooling_dev",      IDS_CAT_HW_DMI_COOLING_DEVICE,     IDI_FAN,        IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_CoolingDeviceInfo,    NULL, StdColumnList, NULL },
    { L"dmi_sys_event_log",    IDS_CAT_HW_DMI_SYS_EVENT_LOG,      IDI_EVENTVWR,   IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_SysEventLogInfo,      NULL, StdColumnList, NULL },
    { L"dmi_group_assoc",      IDS_CAT_HW_DMI_GROUP_ASSOCIATIONS, IDI_COMPUTER,   IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_GroupAssocInfo,       NULL, StdColumnList, NULL },
    { L"dmi_sys_config_opt",   IDS_CAT_HW_DMI_SYS_CONFIG_OPT,     IDI_COMPUTER,   IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_SysConfigOptInfo,     NULL, StdColumnList, NULL },
    { L"dmi_sys_power_ctrls",  IDS_CAT_HW_DMI_SYS_POWER_CTRLS,    IDI_COMPUTER,   IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_SysPowerCtrlsInfo,    NULL, StdColumnList, NULL },
    { L"dmi_sys_reset",        IDS_CAT_HW_DMI_SYS_RESET,          IDI_COMPUTER,   IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_SysResetInfo,         NULL, StdColumnList, NULL },
    { L"dmi_sys_boot",         IDS_CAT_HW_DMI_SYS_BOOT,           IDI_COMPUTER,   IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_SysBootInfo,          NULL, StdColumnList, NULL },
    { L"dmi_hw_sec",           IDS_CAT_HW_DMI_HARDWARE_SEC,       IDI_SECURITY,   IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_HardwareSecInfo,      NULL, StdColumnList, NULL },
    { L"dmi_remote_access",    IDS_CAT_HW_DMI_REMOTE_ACCESS,      IDI_MONITOR,    IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_RemoteAccessInfo,     NULL, StdColumnList, NULL },
    { L"dmi_ipmi_dev",         IDS_CAT_HW_DMI_IPMI_DEVICE,        IDI_HW,         IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_IPMIDeviceInfo,       NULL, StdColumnList, NULL },
    { L"dmi_oem_strings",      IDS_CAT_HW_DMI_OEM_STRINGS,        IDI_TEXT,       IDR_POPUP, FALSE, FALSE, FALSE, NULL, DMI_OemStringsInfo,       NULL, StdColumnList, NULL },
    { 0 }
};

CATEGORY_LIST DataCategoryList[] =
{
    { L"cd",    IDS_CAT_HW_DATA_CD,    IDI_CD,  IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_CDInfo,       NULL, StdColumnList,   NULL },
    //{ L"scsi", IDS_CAT_HW_DATA_SCSI,  IDI_HDD, IDR_POPUP, FALSE, FALSE, NULL, HW_HDDSCSIInfo, NULL, NULL },
    { L"ata",   IDS_CAT_HW_DATA_ATA,   IDI_HDD, IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_HDDATAInfo,   NULL, StdColumnList,   NULL },
    { L"smart", IDS_CAT_HW_DATA_SMART, IDI_HDD, IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_HDDSMARTInfo, NULL, SMARTColumnList, NULL },
    { 0 }
};

CATEGORY_LIST DevicesCategoryList[] =
{
    { L"all_devices",     IDS_CAT_HW_ALL_DEVICES,     IDI_HW,          IDR_DEVICES, FALSE, FALSE, TRUE, NULL, HW_DevicesInfo,        HW_DevicesFree, DevicesColumnList,        NULL },
    { L"unknown_devices", IDS_CAT_HW_UNKNOWN_DEVICES, IDI_DISABLED_HW, IDR_DEVICES, FALSE, FALSE, TRUE, NULL, HW_UnknownDevicesInfo, HW_DevicesFree, UnknownDevicesColumnList, NULL },
    { 0 }
};

CATEGORY_LIST DisplayCategoryList[] =
{
    { L"winvideo", IDS_CAT_HW_WIN_VIDEO, IDI_MONITOR, IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_WinVideoInfo, NULL, StdColumnList, NULL },
    { L"monitors", IDS_CAT_HW_MONITOR,   IDI_MONITOR, IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_MonitorInfo,  NULL, StdColumnList, NULL },
    { L"opengl",   IDS_CAT_HW_OPENGL,    IDI_OPENGL,  IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_OpenGlInfo,   NULL, StdColumnList, NULL },
    { 0 }
};

CATEGORY_LIST MMediaCategoryList[] =
{
    { L"mmedia_devices", IDS_CAT_HW_MMEDIA_DEV,    IDI_AUDIO, IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_MediaDevicesInfo, NULL, MMDevsColumnList, NULL },
    { L"mmedia_acodec",  IDS_CAT_HW_MMEDIA_ACODEC, IDI_AUDIO, IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_MediaACodecsInfo, NULL, StdColumnList,    NULL },
    { L"mmedia_vcodec",  IDS_CAT_HW_MMEDIA_VCODEC, IDI_AUDIO, IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_MediaVCodecsInfo, NULL, StdColumnList,    NULL },
    { L"mmedia_mci",     IDS_CAT_HW_MMEDIA_MCI,    IDI_AUDIO, IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_MediaMCIInfo,     NULL, StdColumnList,    NULL },
    { 0 }
};

CATEGORY_LIST HWCategoryList[] =
{
    { L"dmi",      IDS_CAT_HW_DMI,      IDI_COMPUTER, 0,         FALSE, FALSE, FALSE, NULL, NULL,            NULL, NULL,          DMICategoryList     },
    { L"spd",      IDS_CAT_HW_SPD,      IDI_HW,       IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_SPDInfo,      NULL, StdColumnList, NULL                },
    { L"cpu",      IDS_CAT_HW_CPU,      IDI_CPU,      IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_CPUInfo,      NULL, StdColumnList, NULL                },
    { L"stogage",  IDS_CAT_HW_DATA,     IDI_HDD,      0,         FALSE, FALSE, FALSE, NULL, NULL,            NULL, NULL,          DataCategoryList    },
    { L"display",  IDS_CAT_HW_DISPLAY,  IDI_MONITOR,  0,         FALSE, FALSE, FALSE, NULL, NULL,            NULL, NULL,          DisplayCategoryList },
    //{ L"mmedia", IDS_CAT_HW_MMEDIA,   IDI_AUDIO,    0,         FALSE, FALSE, FALSE, NULL, NULL,            NULL, NULL,          MMediaCategoryList  },
    { L"power",    IDS_CAT_HW_POWER,    IDI_POWERICO, IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_PowerInfo,    NULL, StdColumnList, NULL                },
    { L"printers", IDS_CAT_HW_PRINTERS, IDI_PRINTER,  IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_PrintersInfo, NULL, StdColumnList, NULL                },
    { L"devices",  IDS_CAT_HW_DEVICES,  IDI_HW,       0,         FALSE, FALSE, FALSE, NULL, NULL,            NULL, NULL,          DevicesCategoryList },
    { L"sensors",  IDS_CAT_HW_SENSOR,   IDI_SENSOR,   IDR_POPUP, FALSE, FALSE, FALSE, NULL, HW_SensorInfo,   NULL, StdColumnList, NULL                },
    { 0 }
};

CATEGORY_LIST UpdSftCategoryList[] =
{
    { L"updates",  IDS_CAT_SOFTWARE_UPDATES,  IDI_UPDATES,  IDR_INSTALLED_SOFTWARE, FALSE, FALSE, TRUE, NULL, SOFTWARE_InstalledUpdInfo, SOFTWARE_InstalledAppsFree, InstalledUpdColumnList, NULL },
    { L"programs", IDS_CAT_SOFTWARE_PROGRAMS, IDI_SOFTWARE, IDR_INSTALLED_SOFTWARE, FALSE, FALSE, TRUE, NULL, SOFTWARE_InstalledAppInfo, SOFTWARE_InstalledAppsFree, InstalledAppColumnList, NULL },
    { 0 }
};

CATEGORY_LIST SoftwareCategoryList[] =
{
    { L"upd_sft",   IDS_CAT_SOFTWARE_UPD_SFT,  IDI_SOFTWARE, 0,           FALSE, FALSE, FALSE, NULL, NULL,                  NULL, NULL,               UpdSftCategoryList },
    { L"services",  IDS_CAT_SOFTWARE_SERVICES, IDI_SERVICES, IDR_POPUP,   FALSE, FALSE, TRUE,  NULL, SOFTWARE_ServicesInfo, NULL, ServicesColumnList, NULL },
    { L"drivers",   IDS_CAT_SOFTWARE_DRIVERS,  IDI_HW,       IDR_POPUP,   FALSE, FALSE, TRUE,  NULL, SOFTWARE_DriversInfo,  NULL, DriversColumnList,  NULL },
    { L"filetypes", IDS_CAT_SOFTWARE_FILETYPES,IDI_APPS,     IDR_POPUP,   FALSE, FALSE, TRUE,  NULL, SOFTWARE_FileTypesInfo,NULL, FileTypesColumnList,NULL },
    { L"taskmgr",   IDS_CAT_SOFTWARE_TASKMGR,  IDI_TASKMGR,  IDR_TASKMGR, FALSE, FALSE, TRUE,  NULL, SOFTWARE_TaskMgr,      NULL, TaskMgrColumnList,  NULL },
    { L"licensies", IDS_CAT_SOFTWARE_LICENSES, IDI_CONTACT,  IDR_POPUP,   FALSE, FALSE, TRUE,  NULL, SOFTWARE_LicensesInfo, NULL, LicensesColumnList, NULL },
    { 0 }
};

CATEGORY_LIST IECategoryList[] =
{
    { L"ie_params",  IDS_CAT_NETWORK_IE_PARAMS,  IDI_SERVICES, IDR_POPUP, FALSE, FALSE, FALSE, NULL, NETWORK_IEParamsInfo,  NULL, StdColumnList,       NULL },
    { L"ie_history", IDS_CAT_NETWORK_IE_HISTORY, IDI_IE,       IDR_POPUP, FALSE, FALSE, TRUE,  NULL, NETWORK_IEHistoryInfo, NULL, IEHistoryColumnList, NULL },
    { L"ie_cookie",  IDS_CAT_NETWORK_IE_COOKIE,  IDI_IE,       IDR_POPUP, FALSE, FALSE, TRUE,  NULL, NETWORK_IECookieInfo,  NULL, IECookiesColumnList, NULL },
    { 0 }
};

CATEGORY_LIST NetworkCategoryList[] =
{
    { L"netcards",     IDS_CAT_NETWORK_CARDS,   IDI_HW,      IDR_POPUP, FALSE, FALSE, FALSE, NULL, NETWORK_CardsInfo,     NULL, StdColumnList,       NULL },
    { L"ras",          IDS_CAT_NETWORK_RAS,     IDI_RAS,     IDR_POPUP, FALSE, FALSE, FALSE, NULL, NETWORK_RasInfo,       NULL, StdColumnList,       NULL },
    { L"netstat",      IDS_CAT_NETWORK_NETSTAT, IDI_NETSTAT, IDR_POPUP, FALSE, FALSE, TRUE,  NULL, NETWORK_NetStatInfo,   NULL, NetStatColumnList,   NULL },
    { L"shared",       IDS_CAT_NETWORK_SHARED,  IDI_SHARED,  IDR_POPUP, FALSE, FALSE, TRUE,  NULL, NETWORK_SharedInfo,    NULL, SharedColumnList,    NULL },
    { L"shared_files", IDS_CAT_NETWORK_FILES,   IDI_SHARED,  IDR_POPUP, FALSE, FALSE, TRUE,  NULL, NETWORK_OpenFilesInfo, NULL, OpenFilesColumnList, NULL },
    { L"routes",       IDS_CAT_NETWORK_ROUTE,   IDI_ROUTE,   IDR_POPUP, FALSE, FALSE, TRUE,  NULL, NETWORK_RouteInfo,     NULL, RouteColumnList,     NULL },
    //{ L"firewall", IDS_CAT_NETWORK_FIREWALL,IDI_FIREWALL,IDR_POPUP, FALSE, FALSE, FALSE, NULL, NETWORK_FirewallInfo,  NULL, FirewallColumnList,  NULL },
    { L"ie",           IDS_CAT_NETWORK_IE,      IDI_IE,      0,         FALSE, FALSE, FALSE, NULL, NULL,                  NULL, NULL,                IECategoryList },
    { 0 }
};

CATEGORY_LIST UsersCategoryList[] =
{
    { L"users_list",   IDS_CAT_OS_USERS,       IDI_USER,  IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_UsersInfo,       NULL, UsersColumnList,       NULL },
    { L"users_group",  IDS_CAT_OS_USERGROUP,   IDI_USERS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_UsersGroupsInfo, NULL, UsersGroupsColumnList, NULL },
    { L"users_active", IDS_CAT_OS_ACTIVEUSERS, IDI_USERS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_ActiveUsersInfo, NULL, ActiveUsersColumnList, NULL },
    { 0 }
};

CATEGORY_LIST EventsCategoryList[] =
{
    { L"eventlog_apps",     IDS_ELOG_APPLICATIONS, IDI_APPS,     IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_AppEventsInfo,      NULL, EventViewColumnList, NULL },
    { L"eventlog_security", IDS_ELOG_SECURITY,     IDI_SECURITY, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SecurityEventsInfo, NULL, EventViewColumnList, NULL },
    { L"eventlog_system",   IDS_ELOG_SYSTEM,       IDI_COMPUTER, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SystemEventsInfo,   NULL, EventViewColumnList, NULL },
    { 0 }
};

CATEGORY_LIST SysFilesCategoryList[] =
{
    { L"sysfiles_known", IDS_CAT_SYSFILES_KNOWN, IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesKnownInfo, NULL, StdColumnList,      NULL },
    { L"sysfiles_dll",   IDS_CAT_SYSFILES_DLL,   IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesDLLInfo,   NULL, SysFilesColumnList, NULL },
    { L"sysfiles_ax",    IDS_CAT_SYSFILES_AX,    IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesAXInfo,    NULL, SysFilesColumnList, NULL },
    { L"sysfiles_exe",   IDS_CAT_SYSFILES_EXE,   IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesEXEInfo,   NULL, SysFilesColumnList, NULL },
    { L"sysfiles_sys",   IDS_CAT_SYSFILES_SYS,   IDI_APPS, IDR_POPUP, FALSE, FALSE, TRUE, NULL, OS_SysFilesSYSInfo,   NULL, SysFilesColumnList, NULL },
    { L"fonts",          IDS_CAT_OS_FONTS,       IDI_FONTS,IDR_POPUP, FALSE, FALSE,  TRUE, NULL, OS_FontsInfo,         NULL, FontsColumnList,    NULL },
    { 0 }
};

CATEGORY_LIST OSCategoryList[] =
{
    { L"regdata",    IDS_CAT_OS_REGDATA,      IDI_COMPUTER,   IDR_POPUP, FALSE, FALSE,  FALSE, NULL, OS_RegInformation,    NULL, StdColumnList,        NULL },
    { L"desktop",    IDS_CAT_OS_DESKTOP,      IDI_DESKTOP,    IDR_POPUP, FALSE, FALSE,  FALSE, NULL, OS_DesktopInfo,       NULL, StdColumnList,        NULL },
    { L"tasksched",  IDS_CAT_OS_TASKSCHED,    IDI_TIME,       IDR_POPUP, FALSE, FALSE,  FALSE, NULL, OS_TaskSchedulerInfo, NULL, StdColumnList,        NULL },
    { L"usergroups", IDS_CAT_OS_USERS_GROUPS, IDI_USERS,      0,         FALSE, FALSE,  FALSE, NULL, NULL,                 NULL, NULL,                 UsersCategoryList },
    { L"regparam",   IDS_CAT_OS_REGPARAM,     IDI_LOCALES,    IDR_POPUP, FALSE, FALSE,  FALSE, NULL, OS_RegionalParamInfo, NULL, StdColumnList,        NULL },
    { L"environ",    IDS_CAT_OS_ENVIRON,      IDI_APPS,       IDR_POPUP, FALSE, FALSE,  TRUE,  NULL, OS_EnvironInfo,       NULL, EnvironColumnList,    NULL },
    { L"sysfolders", IDS_CAT_OS_SYSFOLDERS,   IDI_SYSFOLDERS, IDR_POPUP, FALSE, FALSE,  TRUE,  NULL, OS_SysFoldersInfo,    NULL, EnvironColumnList,    NULL },
    { L"eventview",  IDS_CAT_OS_EVENTVIEW,    IDI_EVENTVWR,   0,         FALSE, FALSE, FALSE, NULL, NULL,                 NULL, NULL,                 EventsCategoryList },
    { L"sysfiles",   IDS_CAT_OS_SYSFILES,     IDI_WINDOWS,    0,         FALSE, FALSE, FALSE, NULL, NULL,                 NULL, NULL,                 SysFilesCategoryList },
    //{ L"applets", IDS_CAT_OS_CPL_APPLETS,  IDI_SETTINGS,   IDR_POPUP, FALSE, FALSE,  TRUE,  NULL, OS_CPLAppletsInfo,    NULL, CPLAppletsColumnList, NULL },
    { L"autorun",    IDS_CAT_OS_AUTORUN,      IDI_SOFTWARE,   IDR_POPUP, FALSE, FALSE,  FALSE, NULL, OS_AutorunInfo,       NULL, StdColumnList,        NULL },
    { L"prevents",   IDS_CAT_OS_PREVENTS,     IDI_PREVENT,    IDR_POPUP, FALSE, FALSE,  FALSE, NULL, OS_PreventsInfo,      NULL, StdColumnList,        NULL },
    { 0 }
};

CATEGORY_LIST RootCategoryList[] =
{
    { L"summary",  IDS_CAT_SUMMARY,  IDI_COMPUTER, IDR_POPUP, TRUE, FALSE, FALSE, NULL, ShowSummaryInfo, NULL, StdColumnList, NULL                 },
    { L"hardware", IDS_CAT_HW,       IDI_HW,       0,         TRUE, FALSE, FALSE, NULL, NULL,            NULL, NULL,          HWCategoryList       },
    { L"software", IDS_CAT_SOFTWARE, IDI_SOFTWARE, 0,         TRUE, FALSE, FALSE, NULL, NULL,            NULL, NULL,          SoftwareCategoryList },
    { L"network",  IDS_CAT_NETWORK,  IDI_NETWORK,  0,         TRUE, FALSE, FALSE, NULL, NULL,            NULL, NULL,          NetworkCategoryList  },
    { L"os",       IDS_CAT_OS,       IDI_WINDOWS,  0,         TRUE, FALSE, FALSE, NULL, NULL,            NULL, NULL,          OSCategoryList       },
    { 0 }
};
