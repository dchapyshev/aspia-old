/*
 * PROJECT:         Aspia (Info Helper DLL)
 * FILE:            aspia_dll/licenses.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "aspia.h"
#include "aspia_dll.h"


VOID
GetVMWareWorkstationLicenses(VOID)
{
    WCHAR szKeyName[MAX_PATH], szSerial[MAX_PATH];
    DWORD dwType, dwSize = MAX_PATH;
    INT ItemIndex = 0;
    HKEY hKey, hSubKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   L"Software\\VMware, Inc.\\VMware Workstation",
                   &hKey) != ERROR_SUCCESS)
    {
        return;
    }

    while (RegEnumKeyEx(hKey, ItemIndex, szKeyName, &dwSize,
           NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        if (RegOpenKey(hKey, szKeyName, &hSubKey) == ERROR_SUCCESS)
        {
            if (wcsncmp(szKeyName, L"License.ws", 10) == 0)
            {
                dwType = REG_SZ;
                dwSize = MAX_PATH;

                if (RegQueryValueEx(hSubKey,
                                    L"Serial",
                                    NULL,
                                    &dwType,
                                    (LPBYTE)szSerial,
                                    &dwSize) == ERROR_SUCCESS)
                {
                    IoAddItem(0, 0, L"VMWare Workstation");
                    IoSetItemText(szSerial);
                }
            }

            RegCloseKey(hSubKey);
        }

        dwSize = MAX_PATH;
        ++ItemIndex;
    }

    RegCloseKey(hKey);
}

VOID
GetVMWareServerLicenses(VOID)
{
    WCHAR szKeyName[MAX_PATH], szSerial[MAX_PATH];
    DWORD dwType, dwSize = MAX_PATH;
    INT ItemIndex = 0;
    HKEY hKey, hSubKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   L"Software\\VMware, Inc.\\VMware Server",
                   &hKey) != ERROR_SUCCESS)
    {
        return;
    }

    while (RegEnumKeyEx(hKey, ItemIndex, szKeyName, &dwSize,
           NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        if (RegOpenKey(hKey, szKeyName, &hSubKey) == ERROR_SUCCESS)
        {
            if (wcsncmp(szKeyName, L"License.vs.", 11) == 0)
            {
                dwType = REG_SZ;
                dwSize = MAX_PATH;

                if (RegQueryValueEx(hSubKey,
                                    L"Serial",
                                    NULL,
                                    &dwType,
                                    (LPBYTE)szSerial,
                                    &dwSize) == ERROR_SUCCESS)
                {
                    IoAddItem(0, 0, L"VMWare Server");
                    IoSetItemText(szSerial);
                }
            }

            RegCloseKey(hSubKey);
        }

        dwSize = MAX_PATH;
        ++ItemIndex;
    }

    RegCloseKey(hKey);
}

VOID
Get3PlanesoftKeys(VOID)
{
    WCHAR szKeyName[MAX_PATH], szValue[MAX_PATH];
    DWORD dwSize = MAX_PATH, dwType;
    INT KeyIndex = 0;
    HKEY hKey, hSubKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   L"SOFTWARE\\3Planesoft",
                   &hKey) != ERROR_SUCCESS)
    {
        return;
    }

    while (RegEnumKeyEx(hKey, KeyIndex, szKeyName, &dwSize,
           NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        if (RegOpenKey(hKey, szKeyName, &hSubKey) != ERROR_SUCCESS)
        {
            continue;
        }

        dwType = REG_SZ;
        dwSize = MAX_PATH;

        if (RegQueryValueEx(hSubKey,
                            L"RegisteredTo",
                            NULL,
                            &dwType,
                            (LPBYTE)szValue,
                            &dwSize) == ERROR_SUCCESS)
        {
            IoAddItem(0, 0, L"%s (Name)", szKeyName);
            IoSetItemText(szValue);
        }

        dwType = REG_SZ;
        dwSize = MAX_PATH;

        if (RegQueryValueEx(hSubKey,
                            L"SerNum",
                            NULL,
                            &dwType,
                            (LPBYTE)szValue,
                            &dwSize) == ERROR_SUCCESS)
        {
            IoAddItem(0, 0, L"%s (Key)", szKeyName);
            IoSetItemText(szValue);
        }

        RegCloseKey(hSubKey);

        dwSize = MAX_PATH;
        ++KeyIndex;
    }

    RegCloseKey(hKey);
}

VOID
GetAheadNeroLicensies(VOID)
{
    WCHAR szKeyName[MAX_PATH], szValueName[MAX_PATH],
          szValue[MAX_PATH];
    DWORD dwSize = MAX_PATH, dwValueSize;
    INT KeyIndex = 0, ValIndex;
    HKEY hKey, hSubKey;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   L"Software\\Nero\\Shared",
                   &hKey) != ERROR_SUCCESS)
    {
        return;
    }

    while (RegEnumKeyEx(hKey, KeyIndex, szKeyName, &dwSize,
           NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        if (wcsncmp(szKeyName, L"NL", 2) == 0)
        {
            if (RegOpenKey(hKey, szKeyName, &hSubKey) != ERROR_SUCCESS)
            {
                continue;
            }

            dwValueSize = MAX_PATH;
            dwSize = MAX_PATH;
            ValIndex = 0;

            while (RegEnumValue(hSubKey,
                                ValIndex,
                                szValueName,
                                &dwSize,
                                0, NULL,
                                (LPBYTE)szValue,
                                &dwValueSize) == ERROR_SUCCESS)
            {
                if (wcsncmp(szValueName, L"Serial", 6) == 0)
                {
                    IoAddItem(0, 0, L"Ahead Nero");
                    IoSetItemText(szValue);
                }

                dwValueSize = MAX_PATH;
                dwSize = MAX_PATH;
                ++ValIndex;
            }

            RegCloseKey(hSubKey);
        }

        dwSize = MAX_PATH;
        ++KeyIndex;
    }

    RegCloseKey(hKey);
}

typedef struct
{
    LPWSTR lpProductName;
    HKEY hRootKey;
    LPWSTR lpKeyPath;
    LPWSTR lpKeyName;
} LICENSIES_INFO;

LICENSIES_INFO LicensiesInfo[] =
{
    { L"3D Mark 2001 (Name)",                  HKEY_LOCAL_MACHINE, L"MadOnion.com\\Registration2001",                L"3DMarkRegName" },
    { L"3D Mark 2001 (Key)",                   HKEY_LOCAL_MACHINE, L"MadOnion.com\\Registration2001",                L"3DMarkRegKey"  },
    { L"Adobe Acrobat 7",                      HKEY_LOCAL_MACHINE, L"Adobe\\Adobe Acrobat\\7.0\\Registration",       L"SERIAL"        },
    { L"Adobe Acrobat 8",                      HKEY_LOCAL_MACHINE, L"Adobe\\Adobe Acrobat\\8.0\\Registration",       L"SERIAL"        },
    { L"Adobe Photoshop 7",                    HKEY_LOCAL_MACHINE, L"Adobe\\Photoshop\\7.0\\Registration",           L"SERIAL"        },
    { L"Advanced Direct Remailer",             HKEY_LOCAL_MACHINE, L"Tweak Marketing\\Advanced Direct Remailer\\Registration", L"code"},
    { L"Advanced Direct Remailer",             HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Direct Remailer\\Registration", L"code"          },
    { L"After Effects 7 (Name)",               HKEY_LOCAL_MACHINE, L"Adobe\\After Effects\\7.0\\Registration",       L"NAME"          },
    { L"After Effects 7 (Company)",            HKEY_LOCAL_MACHINE, L"Adobe\\After Effects\\7.0\\Registration",       L"COMPAN"        },
    { L"After Effects 7 (Key)",                HKEY_LOCAL_MACHINE, L"Adobe\\After Effects\\7.0\\Registration",       L"Serial"        },
    { L"Alcohol 120% (Name)",                  HKEY_CURRENT_USER,  L"Alcohol Soft\\Alcohol 120%\\Info",              L"UserName"      },
    { L"Alcohol 120% (Key)",                   HKEY_CURRENT_USER,  L"Alcohol Soft\\Alcohol 120%\\Info",              L"SerialNo"     },
    { L"Alcohol 120% (Key)",                   HKEY_CURRENT_USER,  L"Alcohol Soft\\Alcohol 120%\\Info",              L"ServerKey"     },
    { L"Axailis IconWorkshop 6.0",             HKEY_CURRENT_USER,  L"Axialis\\IconWorkshop\\registration",           L"ProductKey"    },
    { L"Beyond TV 4",                          HKEY_LOCAL_MACHINE, L"SnapStream Media\\Beyond TV",                   L"ProductKey"    },
    { L"Beyond TV 4 Link",                     HKEY_LOCAL_MACHINE, L"SnapStream Media\\Beyond TV",                   L"NetworkLicense"},
    { L"Beyond Media",                         HKEY_LOCAL_MACHINE, L"SnapStream Media\\Beyond Media",                L"ProductKey"    },
    { L"Borland Delphi 6 (Serial)",            HKEY_LOCAL_MACHINE, L"Borland\\Delphi\\6.0",                          L"LMLIC"         },
    { L"Borland Delphi 6 (Key)",               HKEY_LOCAL_MACHINE, L"Borland\\Delphi\\6.0",                          L"LMKEY"         },
    { L"Borland Delphi 7 (Serial)",            HKEY_LOCAL_MACHINE, L"Borland\\Delphi\\7.0",                          L"LMLIC"         },
    { L"Borland Delphi 7 (Key)",               HKEY_LOCAL_MACHINE, L"Borland\\Delphi\\7.0",                          L"LMKEY"         },
    { L"Call of Duty 2",                       HKEY_LOCAL_MACHINE, L"Activision\\Call of Duty 2",                    L"codkey"        },
    { L"Company of Heroes (Key)",              HKEY_LOCAL_MACHINE, L"THQ\\Company of Heroes",                        L"ProductKey"    },
    { L"Cyberlink PowerDVD (Key)",             HKEY_LOCAL_MACHINE, L"Cyberlink\\PowerDVD",                           L"CDKey"         },
    { L"Dell Service Tag",                     HKEY_LOCAL_MACHINE, L"Dell Computer Corporation\\SysInfo",            L"SerialNumber"  },
    { L"DVD Profiler (First Name)",            HKEY_CURRENT_USER,  L"InterVocative Software\\DVD Profiler",          L"RegFName"      },
    { L"DVD Profiler (Last Name)",             HKEY_CURRENT_USER,  L"InterVocative Software\\DVD Profiler",          L"RegLName"      },
    { L"DVD Profiler (Key)",                   HKEY_CURRENT_USER,  L"InterVocative Software\\DVD Profiler",          L"RegKey"        },
    { L"3DMark 2003",                          HKEY_LOCAL_MACHINE, L"Futuremark\\3DMark03",                          L"KeyCode"       },
    { L"3DMark 2005",                          HKEY_LOCAL_MACHINE, L"Futuremark\\3DMark05",                          L"KeyCode"       },
    { L"3DMark 2006",                          HKEY_LOCAL_MACHINE, L"Futuremark\\3DMark06",                          L"KeyCode"       },
    { L"PCMark 2005",                          HKEY_LOCAL_MACHINE, L"Futuremark\\PCMark05",                          L"KeyCode"       },
    { L"HDD State Inspector",                  HKEY_LOCAL_MACHINE, L"AltrixSoft\\HDD State Inspector",               L"Key"           },
    { L"James Bond 007 Nightfire",             HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\James Bond 007 Nightfire\\ergc", L""  },
    { L"mIRC (User Name)",                     HKEY_CURRENT_USER,  L"mIRC\\UserName",                                L""              },
    { L"mIRC (Key)",                           HKEY_CURRENT_USER,  L"mIRC\\License",                                 L""              },
    { L"Naturally Speaking 8",                 HKEY_LOCAL_MACHINE, L"ScanSoft\\NaturallySpeaking8\\Activation",      L"SerialNumber"  },
    { L"Nero Burning Rom 6",                   HKEY_LOCAL_MACHINE, L"Ahead\\Nero - Burning Rom\\Info",               L"serial6"       },
    { L"Nero 7 (Name)",                        HKEY_LOCAL_MACHINE, L"Ahead\\Installation\\Families\\Nero 7\\Info",   L"User" },
    { L"Nero 7 (Company)",                     HKEY_LOCAL_MACHINE, L"Ahead\\Installation\\Families\\Nero 7\\Info",   L"Company" },
    { L"Nero 7 (Key)",                         HKEY_LOCAL_MACHINE, L"Ahead\\Installation\\Families\\Nero 7\\Info",   L"Serial7_*" },
    { L"Nero 8 (Name)",                        HKEY_LOCAL_MACHINE, L"Nero\\Installation\\Families\\Nero 8\\Info",    L"User" },
    { L"Nero 8 (Company)",                     HKEY_LOCAL_MACHINE, L"Nero\\Installation\\Families\\Nero 8\\Info",    L"Company" },
    { L"Nero 8 (Key)",                         HKEY_LOCAL_MACHINE, L"Nero\\Installation\\Families\\Nero 8\\Info",    L"Serial8_*" },
    { L"Nero BackItUp",                        HKEY_LOCAL_MACHINE, L"Ahead\\Nero BackItUp\\Preferences\\General",    L"Serial" },
    { L"NewsBin Pro 5 (First Name)",           HKEY_CURRENT_USER,  L"DJI Interprises\\Newsbin50\\RegNew",            L"FirstName"     },
    { L"NewsBin Pro 5 (Last Name)",            HKEY_CURRENT_USER,  L"DJI Interprises\\Newsbin50\\RegNew",            L"LastName"      },
    { L"NewsBin Pro 5 (Key)",                  HKEY_CURRENT_USER,  L"DJI Interprises\\Newsbin50\\RegNew",            L"Code1"         },
    { L"Norton PartitionMagic 8 (Name)",       HKEY_LOCAL_MACHINE, L"Symantec\\Norton PartitionMagic\\8.0\\UserInfo",L"Name"          },
    { L"Norton PartitionMagic 8 (Company)",    HKEY_LOCAL_MACHINE, L"Symantec\\Norton PartitionMagic\\8.0\\UserInfo",L"Company"       },
    { L"Norton PartitionMagic 8 (Key)",        HKEY_LOCAL_MACHINE, L"Symantec\\Norton PartitionMagic\\8.0\\UserInfo",L"SerialNumber"  },
    { L"Symantec ACT! 6",                      HKEY_LOCAL_MACHINE, L"Symantec\\ACT!\\install", L"SerialNumber"                        },
    { L"Symantec Norton Internet Security 2007",  HKEY_LOCAL_MACHINE, L"Symantec\\CCPD-LC\\KStore\\00000082\\00000049\\000000b9", L"Key" },
    { L"Symantec Norton SystemWorks 2008 11.0.1", HKEY_LOCAL_MACHINE, L"Symantec\\CCPD-LC\\KStore\\00000082\\000000d2\\0000025f", L"Key" },
    { L"Norton Antivirus 2006",                HKEY_LOCAL_MACHINE, L"Symantec\\CCPD-LC\\KStore\\00000082\\0000001e\\0000004a",    L"Key" },
    { L"O&O CleverCache 6 (Name)",             HKEY_LOCAL_MACHINE, L"O&O\\O&O CleverCache\\6.0",                     L"User"          },
    { L"O&O CleverCache 6 (Company)",          HKEY_LOCAL_MACHINE, L"O&O\\O&O CleverCache\\6.0",                     L"Company"       },
    { L"O&O CleverCache 6 (Key)",              HKEY_LOCAL_MACHINE, L"O&O\\O&O CleverCache\\6.0",                     L"SerialNo"      },
    { L"O&O Defrag 8 (Name)",                  HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\8.0\\Pro\\licenses",           L"User"          },
    { L"O&O Defrag 8 (Company)",               HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\8.0\\Pro\\licenses",           L"Company"       },
    { L"O&O Defrag 8 (Key)",                   HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\8.0\\Pro\\licenses",           L"SerialNo"      },
    { L"O&O DriveLED 2 (Name)",                HKEY_LOCAL_MACHINE, L"O&O\\O&O DriveLED\\2.0",                        L"User"          },
    { L"O&O DriveLED 2 (Company)",             HKEY_LOCAL_MACHINE, L"O&O\\O&O DriveLED\\2.0",                        L"Company"       },
    { L"O&O DriveLED 2 (Key)",                 HKEY_LOCAL_MACHINE, L"O&O\\O&O DriveLED\\2.0",                        L"SerialNo"      },
    { L"Quake 4",                              HKEY_LOCAL_MACHINE, L"id\\Quake 4",                                   L"CDKey"         },
    { L"PC Icon Editor (Name)",                HKEY_CURRENT_USER,  L"Program4Pc\\PC Icon Editor",                    L"Name"          },
    { L"PC Icon Editor (Key)",                 HKEY_CURRENT_USER,  L"Program4Pc\\PC Icon Editor",                    L"Key"           },
    { L"ReplayConverter",                      HKEY_LOCAL_MACHINE, L"ReplayConverter",                               L"RegCode"       },
    { L"SmartVersion",                         HKEY_CURRENT_USER,  L"SmartVersion",                                  L"RegistrationCode" },
    { L"Splinter Cell - Chaos Theory",         HKEY_LOCAL_MACHINE, L"Ubisoft\\Splinter Cell Chaos Theory\\Keys",     L"DiscKey_SCCT"  },
    { L"Stardock",                             HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\odp",     L"Serial No"     },
    { L"SuperCleaner (Name)",                  HKEY_CURRENT_USER,  L"SuperCleaner\\Registration",                    L"Name"          },
    { L"SuperCleaner (Key)",                   HKEY_CURRENT_USER,  L"SuperCleaner\\Registration",                    L"Code"          },
    { L"Tag&Rename (Name)",                    HKEY_CURRENT_USER,  L"Softpointer\\Tag&Rename\\Config",               L"Name"          },
    { L"Tag&Rename (Key)",                     HKEY_CURRENT_USER,  L"Softpointer\\Tag&Rename\\Config",               L"cbVQFFtoTagReplaseUnde" },
    { L"Techsmith Camtasia Studio 3.0 (Name)", HKEY_LOCAL_MACHINE, L"TechSmith\\Camtasia Studio\\3.0",               L"RegisteredTo"    },
    { L"Techsmith Camtasia Studio 3.0 (Key)",  HKEY_LOCAL_MACHINE, L"TechSmith\\Camtasia Studio\\3.0",               L"RegistrationKey" },
    { L"Techsmith Camtasia Studio 4.0 (Name)", HKEY_LOCAL_MACHINE, L"TechSmith\\Camtasia Studio\\4.0",               L"RegisteredTo"    },
    { L"Techsmith Camtasia Studio 4.0 (Key)",  HKEY_LOCAL_MACHINE, L"TechSmith\\Camtasia Studio\\4.0",               L"RegistrationKey" },
    { L"Techsmith Camtasia Studio 5.0 (Name)", HKEY_LOCAL_MACHINE, L"TechSmith\\Camtasia Studio\\5.0",               L"RegisteredTo"    },
    { L"Techsmith Camtasia Studio 5.0 (Key)",  HKEY_LOCAL_MACHINE, L"TechSmith\\Camtasia Studio\\5.0",               L"RegistrationKey" },
    { L"Techsmith Camtasia Studio 6.0 (Name)", HKEY_LOCAL_MACHINE, L"TechSmith\\Camtasia Studio\\6.0",               L"RegisteredTo"    },
    { L"Techsmith Camtasia Studio 6.0 (Key)",  HKEY_LOCAL_MACHINE, L"TechSmith\\Camtasia Studio\\6.0",               L"RegistrationKey" },
    { L"Techsmith SnagIt 7.0 (Name)",          HKEY_LOCAL_MACHINE, L"TechSmith\\SnagIt\\7",                          L"RegisteredTo"    },
    { L"Techsmith SnagIt 7.0 (Key)",           HKEY_LOCAL_MACHINE, L"TechSmith\\SnagIt\\7",                          L"RegistrationKey" },
    { L"Techsmith SnagIt 8.0 (Name)",          HKEY_CURRENT_USER,  L"TechSmith\\SnagIt\\8",                          L"RegisteredTo"    },
    { L"Techsmith SnagIt 8.0 (Key)",           HKEY_CURRENT_USER,  L"TechSmith\\SnagIt\\8",                          L"RegistrationKey" },
    { L"Techsmith SnagIt 8.1 (Name)",          HKEY_LOCAL_MACHINE, L"TechSmith\\SnagIt\\8",                          L"RegisteredTo"    },
    { L"Techsmith SnagIt 8.1 (Key)",           HKEY_LOCAL_MACHINE, L"TechSmith\\SnagIt\\8",                          L"RegistrationKey" },
    { L"Techsmith SnagIt 9.0 (Name)",          HKEY_LOCAL_MACHINE, L"TechSmith\\SnagIt\\9",                          L"RegisteredTo"    },
    { L"Techsmith SnagIt 9.0 (Key)",           HKEY_LOCAL_MACHINE, L"TechSmith\\SnagIt\\9",                          L"RegistrationKey" },
    { L"TGTSoft StyleXP",                      HKEY_LOCAL_MACHINE, L"TGT Soft\\StyleXP",                             L"RegKey"          },
    { L"Trend Micro PC-cillin Antivirus 11",   HKEY_LOCAL_MACHINE, L"TrendMicro\\PC-cillin",                         L"register no."    },
    { L"Trend Micro PC-cillin Antivirus 2007", HKEY_LOCAL_MACHINE, L"TrendMicro\\AntiVirus\\15",                     L"SerialNo"        },
    { L"TuneUP 2006 (Name)",                   HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\5.0",                        L"UserName"        },
    { L"TuneUP 2006 (Company)",                HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\5.0",                        L"Company"         },
    { L"TuneUP 2006 (Key)",                    HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\5.0",                        L"RegCode"         },
    { L"TuneUP 2007 (Name)",                   HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\6.0",                        L"UserName"        },
    { L"TuneUP 2007 (Company)",                HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\6.0",                        L"Company"         },
    { L"TuneUP 2007 (Key)",                    HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\6.0",                        L"RegCode"         },
    { L"TuneUp 2009 (Name)",                   HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\8.0",                        L"UserName" },
    { L"TuneUp 2009 (Company)",                HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\8.0",                        L"Company" },
    { L"TuneUp 2009 (Key)",                    HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\8.0",                        L"ProductKey" },
    { L"TuneUp 2010 (Name)",                   HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\9.0",                        L"UserName" },
    { L"TuneUp 2010 (Company)",                HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\9.0",                        L"Company" },
    { L"TuneUp 2010 (Key)",                    HKEY_LOCAL_MACHINE, L"TuneUp\\Utilities\\9.0",                        L"ProductKey" },
    { L"Unreal Tournament 2003",               HKEY_LOCAL_MACHINE, L"Unreal Technology\\Installed Apps\\UT2003",     L"CDKey"           },
    { L"Unreal Tournament 2004",               HKEY_LOCAL_MACHINE, L"Unreal Technology\\Installed Apps\\UT2004",     L"CDKey"           },
    { L"VSO BlindWrite 6",                     HKEY_CURRENT_USER,  L"VSO\\BW6",                                        L"LicenseKey"      },
    { L"VSO ConvertX to DVD",                  HKEY_LOCAL_MACHINE, L"VSO\\ConvertXtoDVD",                            L"LicenseKey"      },
    { L"VSO ConvertXtoDVD",                    HKEY_CURRENT_USER,  L"VSO\\ConvertXToDVD",                            L"LicenseKey"      },
    { L"Westwood Alarmstufe Rot 2",            HKEY_LOCAL_MACHINE, L"Westwood\\Red Alert 2",                         L"Serial"          },
    { L"Westwood Alarmstufe Rot 2 Yuri's Revenge", HKEY_LOCAL_MACHINE, L"Westwood\\Yuri's Revenge",                  L"Serial"          },
    { L"Westwood Tiberian Sun",                HKEY_LOCAL_MACHINE, L"Westwood\\Tiberian Sun",                        L"Serial"          },
    { L"Winamp (Name)",                        HKEY_LOCAL_MACHINE, L"Nullsoft\\Winamp",                              L"regname"         },
    { L"Winamp (Key)",                         HKEY_LOCAL_MACHINE, L"Nullsoft\\Winamp",                              L"regkey"          },
    { L"WinPatrol",                            HKEY_LOCAL_MACHINE, L"BillP Studios\\WinPatrol",                      L"RegNumber"       },
    { L"WS FTP",                               HKEY_LOCAL_MACHINE, L"Ipswitch\\WS_FTP",                              L"SerialNumber"    },

    { L"AutoCAD 2000, 2002",                   HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R15.0\\ACAD-1:409",          L"SerialNumber" },
    { L"AutoCAD 2000, 2002",                   HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R15.0\\ACAD-1:419",          L"SerialNumber" },
    { L"AutoCAD 2004",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.0\\ACAD-201:409",        L"SerialNumber" },
    { L"AutoCAD 2004",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.0\\ACAD-201:419",        L"SerialNumber" },
    { L"AutoCAD 2005",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-301:409",        L"SerialNumber" },
    { L"AutoCAD 2005",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-301:419",        L"SerialNumber" },
    { L"AutoCAD 2006",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4001:409",       L"SerialNumber" },
    { L"AutoCAD 2006",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4001:419",       L"SerialNumber" },
    { L"AutoCAD 2007",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5001:409",       L"SerialNumber" },
    { L"AutoCAD 2007",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5001:419",       L"SerialNumber" },
    { L"AutoCAD 2008",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6001:409",       L"SerialNumber" },
    { L"AutoCAD 2008",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6001:419",       L"SerialNumber" },
    { L"AutoCAD 2009",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7001:409",       L"SerialNumber" },
    { L"AutoCAD 2009",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7001:419",       L"SerialNumber" },
    { L"AutoCAD 2010",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8001:409",       L"SerialNumber" },
    { L"AutoCAD 2010",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8001:419",       L"SerialNumber" },
    { L"AutoCAD 2011",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9001:409",       L"SerialNumber" },
    { L"AutoCAD 2011",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9001:419",       L"SerialNumber" },
    { L"AutoCAD 2012",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A001:409",       L"SerialNumber" },
    { L"AutoCAD 2012",                         HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A001:419",       L"SerialNumber" },

    { L"AutoCAD Architecture 2005",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-304:409",        L"SerialNumber" },
    { L"AutoCAD Architecture 2005",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-304:419",        L"SerialNumber" },
    { L"AutoCAD Architecture 2006",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4004:409",       L"SerialNumber" },
    { L"AutoCAD Architecture 2006",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4004:419",       L"SerialNumber" },
    { L"AutoCAD Architecture 2007",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5004:409",       L"SerialNumber" },
    { L"AutoCAD Architecture 2007",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5004:419",       L"SerialNumber" },
    { L"AutoCAD Architecture 2008",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6004:409",       L"SerialNumber" },
    { L"AutoCAD Architecture 2008",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6004:419",       L"SerialNumber" },
    { L"AutoCAD Architecture 2009",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7004:409",       L"SerialNumber" },
    { L"AutoCAD Architecture 2009",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7004:419",       L"SerialNumber" },
    { L"AutoCAD Architecture 2010",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8004:409",       L"SerialNumber" },
    { L"AutoCAD Architecture 2010",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8004:419",       L"SerialNumber" },
    { L"AutoCAD Architecture 2011",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9004:409",       L"SerialNumber" },
    { L"AutoCAD Architecture 2011",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9004:419",       L"SerialNumber" },
    { L"AutoCAD Architecture 2012",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A004:409",       L"SerialNumber" },
    { L"AutoCAD Architecture 2012",            HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A004:419",       L"SerialNumber" },

    { L"AutoCAD Building Systems 2004",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.0\\ACAD-206:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2004",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.0\\ACAD-206:419",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2005",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-306:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2005",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-306:419",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2006",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4006:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2006",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4006:419",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2007",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5006:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2007",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5006:419",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2008",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6006:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2008",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6006:419",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2009",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7006:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2009",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7006:419",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2010",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8006:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2010",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8006:419",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2011",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9006:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2011",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9006:419",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2012",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A006:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2012",        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A006:419",       L"SerialNumber" },

    { L"AutoCAD Electrical 2005",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-307:409",        L"SerialNumber" },
    { L"AutoCAD Electrical 2005",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-307:419",        L"SerialNumber" },
    { L"AutoCAD Electrical 2006",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4007:409",       L"SerialNumber" },
    { L"AutoCAD Electrical 2006",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4007:419",       L"SerialNumber" },
    { L"AutoCAD Electrical 2007",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5007:409",       L"SerialNumber" },
    { L"AutoCAD Electrical 2007",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5007:419",       L"SerialNumber" },
    { L"AutoCAD Electrical 2008",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6007:409",       L"SerialNumber" },
    { L"AutoCAD Electrical 2008",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6007:419",       L"SerialNumber" },
    { L"AutoCAD Electrical 2009",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7007:409",       L"SerialNumber" },
    { L"AutoCAD Electrical 2009",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7007:419",       L"SerialNumber" },
    { L"AutoCAD Electrical 2010",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8007:409",       L"SerialNumber" },
    { L"AutoCAD Electrical 2010",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8007:419",       L"SerialNumber" },
    { L"AutoCAD Electrical 2011",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9007:409",       L"SerialNumber" },
    { L"AutoCAD Electrical 2011",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9007:419",       L"SerialNumber" },
    { L"AutoCAD Electrical 2012",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A007:409",       L"SerialNumber" },
    { L"AutoCAD Electrical 2012",              HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A007:419",       L"SerialNumber" },

    { L"AutoCAD LT 2000",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R2000\\ACLT-1:409",       L"SerialNumber" },
    { L"AutoCAD LT 2000",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R2000\\ACLT-1:419",       L"SerialNumber" },
    { L"AutoCAD LT 2002i",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R7\\ACLT-1:409",          L"SerialNumber" },
    { L"AutoCAD LT 2002i",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R7\\ACLT-1:419",          L"SerialNumber" },
    { L"AutoCAD LT 2002",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R8\\ACLT-1:409",          L"SerialNumber" },
    { L"AutoCAD LT 2002",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R8\\ACLT-1:419",          L"SerialNumber" },
    { L"AutoCAD LT 2004",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R9\\ACLT-201:409",        L"SerialNumber" },
    { L"AutoCAD LT 2004",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R9\\ACLT-201:419",        L"SerialNumber" },
    { L"AutoCAD LT 2005",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R10\\ACLT-301:409",       L"SerialNumber" },
    { L"AutoCAD LT 2005",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R10\\ACLT-301:419",       L"SerialNumber" },
    { L"AutoCAD LT 2006",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R11\\ACLT-4001:409",      L"SerialNumber" },
    { L"AutoCAD LT 2006",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R11\\ACLT-4001:419",      L"SerialNumber" },
    { L"AutoCAD LT 2007",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R12\\ACADLT-5001:409",    L"SerialNumber" },
    { L"AutoCAD LT 2007",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R12\\ACADLT-5001:419",    L"SerialNumber" },
    { L"AutoCAD LT 2008",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R13\\ACADLT-6001:409",    L"SerialNumber" },
    { L"AutoCAD LT 2008",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R13\\ACADLT-6001:419",    L"SerialNumber" },
    { L"AutoCAD LT 2009",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R14\\ACADLT-7001:409",    L"SerialNumber" },
    { L"AutoCAD LT 2009",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R14\\ACADLT-7001:419",    L"SerialNumber" },
    { L"AutoCAD LT 2010",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R15\\ACADLT-8001:409",    L"SerialNumber" },
    { L"AutoCAD LT 2010",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R15\\ACADLT-8001:419",    L"SerialNumber" },
    { L"AutoCAD LT 2011",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R16\\ACADLT-9001:409",    L"SerialNumber" },
    { L"AutoCAD LT 2011",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R16\\ACADLT-9001:419",    L"SerialNumber" },
    { L"AutoCAD LT 2012",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R17\\ACADLT-A001:409",    L"SerialNumber" },
    { L"AutoCAD LT 2012",                      HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R17\\ACADLT-A001:419",    L"SerialNumber" },
    { L"AutoCAD LT 98",                        HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD LT\\R5.0\\ACLT-2452551:43869540", L"SerialNumber" },

    { L"AutoCAD MEP 2005",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-306:409",        L"SerialNumber" },
    { L"AutoCAD MEP 2005",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.1\\ACAD-306:419",        L"SerialNumber" },
    { L"AutoCAD MEP 2006",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4006:409",       L"SerialNumber" },
    { L"AutoCAD MEP 2006",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R16.2\\ACAD-4006:419",       L"SerialNumber" },
    { L"AutoCAD MEP 2007",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5006:409",       L"SerialNumber" },
    { L"AutoCAD MEP 2007",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5006:419",       L"SerialNumber" },
    { L"AutoCAD MEP 2008",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6006:409",       L"SerialNumber" },
    { L"AutoCAD MEP 2008",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.1\\ACAD-6006:419",       L"SerialNumber" },
    { L"AutoCAD MEP 2009",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7006:409",       L"SerialNumber" },
    { L"AutoCAD MEP 2009",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.2\\ACAD-7006:419",       L"SerialNumber" },
    { L"AutoCAD MEP 2010",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8006:409",       L"SerialNumber" },
    { L"AutoCAD MEP 2010",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.0\\ACAD-8006:419",       L"SerialNumber" },
    { L"AutoCAD MEP 2011",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9006:409",       L"SerialNumber" },
    { L"AutoCAD MEP 2011",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.1\\ACAD-9006:419",       L"SerialNumber" },
    { L"AutoCAD MEP 2012",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A006:409",       L"SerialNumber" },
    { L"AutoCAD MEP 2012",                     HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R18.2\\ACAD-A006:419",       L"SerialNumber" },

    { L"AutoCAD Mechanical 2000i",             HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R15.0\\ACAD-7:409",          L"SerialNumber" },
    { L"AutoCAD Mechanical 2000i",             HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R15.0\\ACAD-7:419",          L"SerialNumber" },

    { L"Autodesk 3ds Max 8",                   HKEY_LOCAL_MACHINE, L"Autodesk\\3dsmax\\8.0",                         L"SerialNumber" },
    { L"Autodesk Inventor",                    HKEY_LOCAL_MACHINE, L"Inventor\\System\\License",                               L"SerialNumber" },
    { L"Autodesk Raster Design 2007",          HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5xxx:409\\AddOns\\RasterDesign", L"SerialNumber" },
    { L"Autodesk Raster Design 2007",          HKEY_LOCAL_MACHINE, L"Autodesk\\AutoCAD\\R17.0\\ACAD-5xxx:419\\AddOns\\RasterDesign", L"SerialNumber" },
    { L"Autodesk Volo View 2002-627",          HKEY_LOCAL_MACHINE, L"Autodesk\\Volo View\\2002-627",                 L"SerialNumber" },
    { L"Adobe Acrobat 5.0",                    HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\Adobe Acrobat 5.0", L"ProductID" },
    { L"Adobe Acrobat 5.x",                    HKEY_CURRENT_USER,  L"Adobe\\Acrobat\\5.0\\Registration",             L"SERIAL" },
    { L"Adobe Acrobat 6.0",                    HKEY_LOCAL_MACHINE, L"Adobe\\Adobe Acrobat\\6.0\\Registration",       L"SERIAL" },
    { L"Adobe Acrobat 7.x",                    HKEY_CURRENT_USER,  L"Adobe\\Acrobat\\7.0\\Registration",             L"SERIAL" },
    { L"Adobe Acrobat 9.x",                    HKEY_LOCAL_MACHINE, L"Adobe\\Adobe Acrobat\\9.0\\Registration",       L"SERIAL" },
    { L"Adobe Acrobat Distiller 6.0",          HKEY_LOCAL_MACHINE, L"Adobe\\Acrobat Distiller\\6.0\\Registration",   L"SERIAL" },
    { L"Adobe Acrobat Distiller 7.0",          HKEY_LOCAL_MACHINE, L"Adobe\\Acrobat Distiller\\7.0\\Registration",   L"SERIAL" },
    { L"Adobe Acrobat Distiller 8.0",          HKEY_LOCAL_MACHINE, L"Adobe\\Acrobat Distiller\\8.0\\Registration",   L"SERIAL" },
    { L"Adobe Creative Suite 2",               HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{0134A1A1-C283-4A47-91A1-92F19F960372}", L"EPIC_SERIAL" },
    { L"Adobe Illustrator CS2 (Name)",         HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{B2F5D08C-7E79-4FCD-AAF4-57AD35FF0601}", L"EPIC_NAME" },
    { L"Adobe Illustrator CS2 (Key)",          HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{B2F5D08C-7E79-4FCD-AAF4-57AD35FF0601}", L"EPIC_SERIAL" },
    { L"Adobe InDesign CS2 (Name)",            HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F4C8163-F259-49A0-A018-2857A90578BC}", L"EPIC_NAME" },
    { L"Adobe InDesign CS2 (Key)",             HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F4C8163-F259-49A0-A018-2857A90578BC}", L"EPIC_SERIAL" },
    { L"Adobe Lightroom 1.0",                  HKEY_LOCAL_MACHINE, L"Adobe\\Lightroom\\1.0\\Registration",           L"serial_number" },
    { L"Adobe Photoshop CS2 (Name)",           HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{236BB7C4-4419-42FD-0409-1E257A25E34D}", L"EPIC_NAME" },
    { L"Adobe Photoshop CS2 (Key)",            HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{236BB7C4-4419-42FD-0409-1E257A25E34D}", L"EPIC_SERIAL" },
    { L"Adobe Photoshop CS (Name)",            HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{EFB21DE7-8C19-4A88-BB28-A766E16493BC}", L"RegOwner" },
    { L"Adobe Photoshop CS (Key)",             HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{EFB21DE7-8C19-4A88-BB28-A766E16493BC}", L"Serial" },
    { L"Adobe Premiere Pro (Name)",            HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{A14F7508-B784-40B8-B11A-E0E2EEB7229F}", L"NAME" },
    { L"Adobe Premiere Pro (Key)",             HKEY_LOCAL_MACHINE, L"Microsoft\\Windows\\CurrentVersion\\Uninstall\\{A14F7508-B784-40B8-B11A-E0E2EEB7229F}", L"SERIAL" },
    { L"The Sims 2 Apartment Life",            HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Apartment Life\\ergc", L"" },
    { L"The Sims 2 Bon Voyage",                HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Bon Voyage\\ergc", L"" },
    { L"The Sims 2 Celebration Stuff",         HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Celebration Stuff\\ergc", L"" },
    { L"The Sims 2 Deluxe",                    HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Deluxe\\ergc", L"" },
    { L"The Sims 2 Double Deluxe",             HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Double Deluxe\\ergc", L"" },
    { L"The Sims 2 Family Fun Stuff",          HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Family Fun Stuff\\ergc", L"" },
    { L"The Sims 2 Free Time",                 HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 FreeTime\\ergc", L"" },
    { L"The Sims 2 Glamour Life Stuff",        HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Glamour Life Stuff\\ergc", L"" },
    { L"The Sims 2 H M Fashion Stuff",         HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 H M Fashion Stuff\\ergc", L"" },
    { L"The Sims 2 IKEA Home Stuff",           HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 IKEA Home Stuff\\ergc", L"" },
    { L"The Sims 2 Kitchen & Bath Interior Design Stuff", HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Kitchen & Bath Interior Design Stuff\\ergc", L"" },
    { L"The Sims 2 Nightlife",                 HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Nightlife\\ergc", L"" },
    { L"The Sims 2 Open for Business",         HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Open for Business\\ergc", L"" },
    { L"The Sims 2 Pets",                      HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Pets\\ergc", L"" },
    { L"The Sims 2 Seasons",                   HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Seasons\\ergc", L"" },
    { L"The Sims 2 Teen Style Stuff",          HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 Teen Style Stuff\\ergc", L"" },
    { L"The Sims 2 University",                HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2 University\\ergc", L"" },
    { L"The Sims 2",                           HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Sims 2\\ergc", L"" },
    { L"The Sims Castaway Stories",            HKEY_LOCAL_MACHINE, L"Electronic Arts\\The Sims Castaway Stories\\ergc", L"" },
    { L"The Sims Deluxe",                      HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\The Sims Deluxe\\ergc", L"" },
    { L"The Sims Hot Date",                    HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\The Sims Hot Date\\ergc", L"" },
    { L"The Sims House Party",                 HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\The Sims House Party\\ergc", L"" },
    { L"The Sims Life Stories",                HKEY_LOCAL_MACHINE, L"Electronic Arts\\The Sims Life Stories\\ergc", L"" },
    { L"The Sims Livin’ Large",                HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\The Sims Livin' Large\\ergc", L"" },
    { L"The Sims Makin’ Magic",                HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\The Sims Makin' Magic\\ergc", L"" },
    { L"The Sims Pet Stories",                 HKEY_LOCAL_MACHINE, L"Electronic Arts\\The Sims Pet Stories\\ergc", L"" },
    { L"The Sims Superstar",                   HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\The Sims Superstar\\ergc", L"" },
    { L"The Sims Unleashed",                   HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\The Sims Unleashed\\ergc", L"" },
    { L"The Sims Vacation",                    HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\The Sims Vacation\\ergc", L"" },
    { L"The Sims",                             HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\The Sims\\ergc", L"" },
    { L"FIFA 2002",                            HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Sports\\FIFA 2002\\ergc", L"" },
    { L"FIFA 2003",                            HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Sports\\FIFA 2003\\ergc", L"" },
    { L"FIFA 2007",                            HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Sports\\FIFA 07\\ergc", L"" },
    { L"Freedom Force",                        HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Distribution\\Freedom Force\\ergc", L"" },
    { L"Global Operations",                    HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Global Operations\\ergc", L"" },
    { L"Medal of Honor Airborne",              HKEY_LOCAL_MACHINE, L"Electronic Arts\\Electronic Arts\\Medal of Honor Airborne\\ergc", L"" },
    { L"Medal of Honor: Allied Assault: Breakthrough", HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Medal of Honor Allied Assault Breakthrough\\ergc", L"" },
    { L"Medal of Honor: Allied Assault: Spearhead",    HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Medal of Honor Allied Assault Spearhead\\ergc", L"" },
    { L"Medal of Honor: Allied Assault",       HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Medal of Honor Allied Assault\\egrc", L"" },
    { L"Medal of Honor: Pacific Assault",      HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\Medal of Honor Pacific Assault\\ergc", L"" },
    { L"Medal of Honor",                       HKEY_LOCAL_MACHINE, L"Electronic Arts\\Electronic Arts\\Medal of Honor\\ergc", L"" },
    { L"NHL 2002",                             HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Sports\\NHL 2002\\ergc", L"" },
    { L"NHL 2003",                             HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Sports\\NHL 2003\\ergc", L"" },
    { L"Nascar Racing 2002",                   HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Sports\\Nascar Racing 2002\\ergc", L"" },
    { L"Nascar Racing 2003",                   HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Sports\\Nascar Racing 2003\\ergc", L"" },
    { L"Need For Speed Hot Pursuit",           HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Need For Speed Hot Pursuit\\egrc", L"" },
    { L"Need for Speed Carbon",                HKEY_LOCAL_MACHINE, L"Electronic Arts\\Electronic Arts\\Need for Speed Carbon\\ergc", L"" },
    { L"Need for Speed Most Wanted",           HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\Need for Speed Most Wanted\\ergc", L"" },
    { L"Need for Speed Underground",           HKEY_LOCAL_MACHINE, L"Electronic Arts\\Electronic Arts\\Need for Speed Underground\\ergc", L"" },
    { L"SPORE: Creepy and Cute Parts Pack",    HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\SPORE Creepy and Cute Parts Pack\\ergc", L"" },
    { L"SPORE",                                HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\SPORE(TM)\\ergc", L"" },
    { L"Shogun: Total War: Warlord Edition",   HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Shogun Total War - WarlordEdition\\ergc", L"" },
    { L"SimCity 4 Deluxe",                     HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\SimCity 4 Deluxe\\ergc", L"" },
    { L"SimCity 4 Rush Hour",                  HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\SimCity 4 Rush Hour\\ergc", L"" },
    { L"SimCity 4",                            HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\SimCity 4\\ergc", L"" },
    { L"Battle for Middle Earth",              HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\The Battle for Middle-earth\\ergc", L"" },
    { L"Battlefield 1942: Road To Rome",       HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Battlefield 1942 The Road to Rome\\egrc", L"" },
    { L"Battlefield 1942: Secret Weapons of WWII", HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Battlefield 1942 Secret Weapons of WWII\\ergc", L"" },
    { L"Battlefield 1942",                     HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Battlefield 1942\\ergc", L"" },
    { L"Battlefield 2 Demo",                   HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Battlefield 2 Demo\\ergc", L"" },
    { L"Battlefield 2 Special Forces",         HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\Battlefield 2 Special Forces\\ergc", L"" },
    { L"Battlefield 2142",                     HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Battlefield 2142\\ergc", L"" },
    { L"Battlefield 2",                        HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\Battlefield 2\\ergc", L"" },
    { L"Battlefield Vietnam",                  HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Battlefield Vietnam\\ergc", L"" },
    { L"Black and White",                      HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA GAMES\\Black and White\\ergc", L"" },
    { L"Command & Conquer 3: Kane’s Wrath",    HKEY_LOCAL_MACHINE, L"Electronic Arts\\Electronic Arts\\Command and Conquer 3 Kanes Wrath\\ergc", L"" },
    { L"Command & Conquer 3: Tiberium Wars",   HKEY_LOCAL_MACHINE, L"Electronic Arts\\Electronic Arts\\Command and Conquer 3\\ergc", L"" },
    { L"Command & Conquer: Generals Zero Hour",HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour\\ergc", L"" },
    { L"Command & Conquer: Generals",          HKEY_LOCAL_MACHINE, L"Electronic Arts\\EA Games\\Generals\\ergc", L"" },
    { L"Command & Conquer: Red Alert 3",       HKEY_LOCAL_MACHINE, L"Electronic Arts\\Electronic Arts\\RedAlert 3\\ergc", L"" },
    { L"Crysis",                               HKEY_LOCAL_MACHINE, L"Electronic Arts\\Electronic Arts\\Crysis\\ergc", L"" },
    { L"SimCity 3000 Unlimited",               HKEY_LOCAL_MACHINE, L"Electronic Arts\\Maxis\\SimCity 3000 Unlimited", L"Serial Number" },
    { L"Command & Conquer: Red Alert 2 Yuri's Revenge", HKEY_LOCAL_MACHINE, L"Westwood\\Yuri's Revenge", L"Serial" },
    { L"Command & Conquer: Red Alert 2",       HKEY_LOCAL_MACHINE, L"Westwood\\Red Alert 2", L"Serial" },
    { L"Command & Conquer: Red Alert",         HKEY_LOCAL_MACHINE, L"Westwood\\Red Alert", L"Serial" },
    { L"Command & Conquer: Tiberian Sun",      HKEY_LOCAL_MACHINE, L"Westwood\\Tiberian Sun", L"Serial" },
    { L"Counter-Strike",                       HKEY_CURRENT_USER,  L"Valve\\CounterStrike\\Settings", L"CDKey" },
    { L"Counter-Strike",                       HKEY_CURRENT_USER,  L"Valve\\CounterStrike\\Settings", L"Key" },
    { L"Gunman Chronicles",                    HKEY_CURRENT_USER,  L"Valve\\Gunman\\Settings", L"Key" },
    { L"Half-Life",                            HKEY_CURRENT_USER,  L"Valve\\Half-Life\\Settings", L"Key" },
    { L"Call of Duty 2",                       HKEY_LOCAL_MACHINE, L"Activision\\Call of Duty 2", L"codkey" },
    { L"Call of Duty 4",                       HKEY_LOCAL_MACHINE, L"Activision\\Call of Duty 4", L"codkey" },
    { L"Call of Duty",                         HKEY_LOCAL_MACHINE, L"Activision\\Call of Duty", L"codkey" },
    { L"Call of Duty: United Offensive",       HKEY_LOCAL_MACHINE, L"Activision\\Call of Duty United Offensive", L"key" },
    { L"Call of Duty: World at War",           HKEY_LOCAL_MACHINE, L"Activision\\Call of Duty WAW", L"codkey" },
    { L"Corel Paint Shop Pro photo X2",        HKEY_LOCAL_MACHINE, L"Corel\\Paint Shop Pro\\12\\Installer", L"SerialNumber" },
    { L"Corel VideoStudio 12 Pro",             HKEY_LOCAL_MACHINE, L"Ulead Systems\\Corel VideoStudio\\12.0\\Installer", L"Serial Number" },
    { L"CorelDRAW Graphics Suite 12",          HKEY_LOCAL_MACHINE, L"Corel\\CorelDRAW\\12.0", L"Serial" },
    { L"CorelDRAW Graphics Suite X3",          HKEY_LOCAL_MACHINE, L"Corel\\CorelDRAW\\13.0", L"Serial" },
    { L"CorelDRAW Graphics Suite X4",          HKEY_LOCAL_MACHINE, L"Corel\\Setup\\CorelDRAW Graphics Suite 14", L"SerialNumber" },
    { L"Cyberlink CDS",                        HKEY_LOCAL_MACHINE, L"CyberLink\\CDS", L"CDKey" },
    { L"Cyberlink LabelPrint (Name)",          HKEY_LOCAL_MACHINE, L"CyberLink\\LabelPrint\\2.0", L"UserName" },
    { L"Cyberlink LabelPrint (Company)",       HKEY_LOCAL_MACHINE, L"CyberLink\\LabelPrint\\2.0", L"Company" },
    { L"Cyberlink LabelPrint (Key)",           HKEY_LOCAL_MACHINE, L"CyberLink\\LabelPrint\\2.0", L"CDKey" },
    { L"Cyberlink LabelPrint (Serial)",        HKEY_LOCAL_MACHINE, L"CyberLink\\LabelPrint\\2.0\\BuildInfo", L"SR_No" },
    { L"Cyberlink PCM4Everio",                 HKEY_LOCAL_MACHINE, L"CyberLink\\PCM4Everio\\UserReg", L"SR_No" },
    { L"Cyberlink Power2Go",                   HKEY_LOCAL_MACHINE, L"CyberLink\\Power2Go\\5.0\\UserReg", L"SR_No" },
    { L"Cyberlink PowerDVD DX MyMovie",        HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDVD DX\\MyMovie", L"CDkey" },
    { L"Cyberlink PowerDVD DX (User)",         HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDVD DX", L"UserName" },
    { L"Cyberlink PowerDVD DX (Company)",      HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDVD DX", L"Company" },
    { L"Cyberlink PowerDVD DX (Key)",          HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDVD DX", L"CDKey" },
    { L"Cyberlink PowerDVD DX (Serial)",       HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDVD DX\\UserReg", L"SR_NO" },
    { L"Cyberlink PowerDVD (Key)",             HKEY_LOCAL_MACHINE, L"Cyberlink\\PowerDVD", L"CDKey" },
    { L"Cyberlink PowerDVD (Serial)",          HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDVD\\UserReg", L"SR_No" },
    { L"Cyberlink PowerDVD 10 (Name)",         HKEY_LOCAL_MACHINE, L"Cyberlink\\PowerDVD10", L"UserName" },
    { L"Cyberlink PowerDVD 10 (Key)",          HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDVD10", L"CDKey" },
    { L"Cyberlink PowerDirector 6.5 Uniplayer",HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDirector\\6.5\\UniPlayer", L"CDKey" },
    { L"Cyberlink PowerDirector Express 5.0",  HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDirector Express\\5.0\\UserReg", L"SR_No" },
    { L"Cyberlink PowerDirector 6.5",          HKEY_LOCAL_MACHINE, L"CyberLink\\PowerDirector\\6.5\\UserReg", L"SR_No" },
    { L"Cyberlink PowerProducer 3.0",          HKEY_LOCAL_MACHINE, L"CyberLink\\PowerProducer\\3.0\\UserReg", L"SR_No" },
    { L"Cyberlink PowerStarter",               HKEY_LOCAL_MACHINE, L"CyberLink\\PowerStarter\\UserReg", L"SR_No" },
    { L"Advanced ACT! Password Recovery",      HKEY_CURRENT_USER,  L"Elcom\\Advanced ACT! Password Recovery\\Registration", L"Code" },
    { L"Advanced ART Password Recovery",       HKEY_CURRENT_USER,  L"Elcom\\ART\\Registration", L"Code" },
    { L"Advanced Access Password Recovery",    HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Access Password Recovery\\Registration", L"Code" },
    { L"Advanced Archive Password Recovery",   HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Archive Password Recovery\\Registration", L"Code" },
    { L"Advanced Disk Catalog",                HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Disk Catalog\\Registration", L"Code" },
    { L"Advanced EFS Data Recovery",           HKEY_LOCAL_MACHINE, L"Elcom\\Advanced EFS Data Recovery\\Registration", L"Code" },
    { L"Advanced IE Password Recovery",        HKEY_LOCAL_MACHINE, L"Elcom\\Advanced IE Password Recovery\\Registration", L"Code" },
    { L"Advanced IM Password Recovery",        HKEY_CURRENT_USER,  L"Elcom\\Advanced IM Password Recovery\\Registration", L"Code" },
    { L"Advanced Intuit Password Recovery",    HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Intuit Password Recovery\\Registration", L"Code" },
    { L"Advanced Lotus Password Recovery",     HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Lotus Password Recovery\\Registration", L"Code" },
    { L"Advanced Mailbox Password Recovery",   HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Mailbox Password Recovery\\Registration", L"Code" },
    { L"Advanced Office Key Recovery",         HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Office Key Recovery\\Registration", L"Code" },
    { L"Advanced Office Password Recovery",    HKEY_LOCAL_MACHINE, L"ElcomSoft\\Advanced Office Password Recovery\\Registration", L"Code" },
    { L"Advanced Outlook Express Password Recovery", HKEY_LOCAL_MACHINE, L"Elcom\\Advanced OE Password Recovery\\Registration", L"Code" },
    { L"Advanced Outlook Password Recovery",   HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Outlook Password Recovery\\Registration", L"Code" },
    { L"Advanced PDF Password Recovery",       HKEY_LOCAL_MACHINE, L"Elcom\\Advanced PDF Password Recovery\\Registration", L"Code" },
    { L"Advanced Proactive Windows Security Explorer", HKEY_LOCAL_MACHINE, L"Elcom\\Proactive Windows Security Explorer\\Registration", L"Code" },
    { L"Advanced RAR Password Recovery",       HKEY_LOCAL_MACHINE, L"Elcom\\Advanced RAR Password Recovery\\Registration", L"Code" },
    { L"Advanced VBA Password Recovery",       HKEY_LOCAL_MACHINE, L"ElcomSoft\\Advanced VBA Password Recovery\\Registration", L"Code" },
    { L"Advanced Windows Password Recovery",   HKEY_LOCAL_MACHINE, L"Elcom\\Advanced Windows Password Recovery\\Registration", L"Code" },
    { L"Advanced Wireless Security Auditor",   HKEY_CURRENT_USER,  L"ElcomSoft\\Elcomsoft Wireless Security Auditor\\Registration", L"Code" },
    { L"Advanced ZIP Password Recovery",       HKEY_LOCAL_MACHINE, L"Elcom\\Advanced ZIP Password Recovery\\Registration", L"Code" },
    { L"Advanced eBook Inscriber",             HKEY_LOCAL_MACHINE, L"Elcom\\Advanced eBook Inscriber\\Registration", L"Code" },
    { L"Advanced eBook Processor",             HKEY_LOCAL_MACHINE, L"Elcom\\Advanced eBook Processor\\Registration", L"Code" },
    { L"The Bat!",                             HKEY_CURRENT_USER,  L"RIT\\The Bat!", L"RegistrationBlock" },
    { L"S.T.A.L.K.E.R. - Shadow of Chernobyl (Name)", HKEY_LOCAL_MACHINE, L"GSC Game World\\STALKER-SHOC", L"InstallUserName" },
    { L"S.T.A.L.K.E.R. - Shadow of Chernobyl (Key)",  HKEY_LOCAL_MACHINE, L"GSC Game World\\STALKER-SHOC", L"InstallCDKEY" },
    { L"IsoBuster (Key 1)",                    HKEY_CURRENT_USER, L"Smart Projects\\IsoBuster", L"ID" },
    { L"IsoBuster (Key 2)",                    HKEY_CURRENT_USER, L"Smart Projects\\IsoBuster", L"ID1" },
    { L"IsoBuster (Key 3)",                    HKEY_CURRENT_USER, L"Smart Projects\\IsoBuster", L"ID2" },
    { L"O&O Defrag 10 Pro (Name)",             HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\10.0\\Pro\\licenses", L"User" },
    { L"O&O Defrag 10 Pro (Company)",          HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\10.0\\Pro\\licenses", L"Company" },
    { L"O&O Defrag 10 Pro (Key)",              HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\10.0\\Pro\\licenses", L"SerialNo" },
    { L"O&O Defrag 11 Pro (Name)",             HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\11.0\\Pro\\licenses", L"User" },
    { L"O&O Defrag 11 Pro (Company)",          HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\11.0\\Pro\\licenses", L"Company" },
    { L"O&O Defrag 11 Pro (Key)",              HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\11.0\\Pro\\licenses", L"SerialNo" },
    { L"O&O Defrag 12 Pro (Name)",             HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\12.0\\Pro\\licenses", L"User" },
    { L"O&O Defrag 12 Pro (Company)",          HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\12.0\\Pro\\licenses", L"Company" },
    { L"O&O Defrag 12 Pro (Key)",              HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\12.0\\Pro\\licenses", L"SerialNo" },
    { L"O&O Defrag 9 Pro (Name)",              HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\9.0\\Pro\\licenses", L"User" },
    { L"O&O Defrag 9 Pro (Company)",           HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\9.0\\Pro\\licenses", L"Company" },
    { L"O&O Defrag 9 Pro (Key)",               HKEY_LOCAL_MACHINE, L"O&O\\O&O Defrag\\9.0\\Pro\\licenses", L"SerialNo" },
    { L"O&O DiskImage 1 (Name)",               HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskImage\\1.0\\Pro\\licenses", L"User" },
    { L"O&O DiskImage 1 (Company)",            HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskImage\\1.0\\Pro\\licenses", L"Company" },
    { L"O&O DiskImage 1 (Key)",                HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskImage\\1.0\\Pro\\licenses", L"SerialNo" },
    { L"O&O DiskImage 2 (Name)",               HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskImage\\2.0\\licenses", L"User" },
    { L"O&O DiskImage 2 (Company)",            HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskImage\\2.0\\licenses", L"Company" },
    { L"O&O DiskImage 2 (Key)",                HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskImage\\2.0\\licenses", L"SerialNo" },
    { L"O&O DiskRecovery 3 (Name)",            HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskRecovery\\3.0", L"User" },
    { L"O&O DiskRecovery 3 (Company)",         HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskRecovery\\3.0", L"Company" },
    { L"O&O DiskRecovery 3 (Key)",             HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskRecovery\\3.0", L"SerialNo" },
    { L"O&O DiskRecovery 4 (Name)",            HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskRecovery\\4.0", L"User" },
    { L"O&O DiskRecovery 4 (Company)",         HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskRecovery\\4.0", L"Company" },
    { L"O&O DiskRecovery 4 (Key)",             HKEY_LOCAL_MACHINE, L"O&O\\O&O DiskRecovery\\4.0", L"SerialNo" },
    { L"O&O FormatRecovery 4 (Name)",          HKEY_LOCAL_MACHINE, L"O&O\\O&O FormatRecovery\\4.0", L"User" },
    { L"O&O FormatRecovery 4 (Company)",       HKEY_LOCAL_MACHINE, L"O&O\\O&O FormatRecovery\\4.0", L"Company" },
    { L"O&O FormatRecovery 4 (Key)",           HKEY_LOCAL_MACHINE, L"O&O\\O&O FormatRecovery\\4.0", L"SerialNo" },
    { L"O&O MediaRecovery 4 (Name)",           HKEY_LOCAL_MACHINE, L"O&O\\O&O MediaRecovery\\4.0", L"User" },
    { L"O&O MediaRecovery 4 (Company)",        HKEY_LOCAL_MACHINE, L"O&O\\O&O MediaRecovery\\4.0", L"Company" },
    { L"O&O MediaRecovery 4 (Key)",            HKEY_LOCAL_MACHINE, L"O&O\\O&O MediaRecovery\\4.0", L"SerialNo" },
    { L"O&O SafeErase 2 (Name)",               HKEY_LOCAL_MACHINE, L"O&O\\O&O Safeerase\\2.0", L"User" },
    { L"O&O SafeErase 2 (Company)",            HKEY_LOCAL_MACHINE, L"O&O\\O&O Safeerase\\2.0", L"Company" },
    { L"O&O SafeErase 2 (Key)",                HKEY_LOCAL_MACHINE, L"O&O\\O&O Safeerase\\2.0", L"SerialNo" },
    { L"O&O SafeUnErase 2 (Name)",             HKEY_LOCAL_MACHINE, L"O&O\\O&O UnErase\\2.0", L"User" },
    { L"O&O SafeUnErase 2 (Company)",          HKEY_LOCAL_MACHINE, L"O&O\\O&O UnErase\\2.0", L"Company" },
    { L"O&O SafeUnErase 2 (Key)",              HKEY_LOCAL_MACHINE, L"O&O\\O&O UnErase\\2.0", L"SerialNo" },
    { L"O&O UnErase 4 (Name)",                 HKEY_LOCAL_MACHINE, L"O&O\\O&O UnErase\\4.0", L"User" },
    { L"O&O UnErase 4 (Company)",              HKEY_LOCAL_MACHINE, L"O&O\\O&O UnErase\\4.0", L"Company" },
    { L"O&O UnErase 4 (Key)",                  HKEY_LOCAL_MACHINE, L"O&O\\O&O UnErase\\4.0", L"SerialNo" },
    { L"Macromedia Contribute Studio MX (Serial)",          HKEY_LOCAL_MACHINE, L"Macromedia\\Contribute\\1\\Registration", L"Serial Number" },
    { L"Macromedia Contribute Studio MX (Validation Key)",  HKEY_LOCAL_MACHINE, L"Macromedia\\Contribute\\1\\Registration", L"Validation" },
    { L"Macromedia Dreamweaver 8",                          HKEY_LOCAL_MACHINE, L"Macromedia\\Dreamweaver\\8\\Registration", L"Cached Serial Number" },
    { L"Macromedia Dreamweaver MX 2004 (First Name)",       HKEY_LOCAL_MACHINE, L"Macromedia\\Dreamweaver\\7\\Registration", L"First Name" },
    { L"Macromedia Dreamweaver MX 2004 (Last Name)",        HKEY_LOCAL_MACHINE, L"Macromedia\\Dreamweaver\\7\\Registration", L"Last Name" },
    { L"Macromedia Dreamweaver MX 2004 (Serial)",           HKEY_LOCAL_MACHINE, L"Macromedia\\Dreamweaver\\7\\Registration", L"Serial Number" },
    { L"Macromedia Dreamweaver MX 2004 (Validation Key)",   HKEY_LOCAL_MACHINE, L"Macromedia\\Dreamweaver\\7\\Registration", L"Validation" },
    { L"Macromedia Dreamweaver Studio MX (Serial)",         HKEY_LOCAL_MACHINE, L"Macromedia\\Dreamweaver\\6\\Registration", L"Serial Number" },
    { L"Macromedia Dreamweaver Studio MX (Validation Key)", HKEY_LOCAL_MACHINE, L"Macromedia\\Dreamweaver\\6\\Registration", L"Validation" },
    { L"Macromedia Fireworks MX 2004 (First Name)",         HKEY_LOCAL_MACHINE, L"Macromedia\\Fireworks\\7\\Registration", L"First Name" },
    { L"Macromedia Fireworks MX 2004 (Last Name)",          HKEY_LOCAL_MACHINE, L"Macromedia\\Fireworks\\7\\Registration", L"Last Name" },
    { L"Macromedia Fireworks MX 2004 (Serial)",             HKEY_LOCAL_MACHINE, L"Macromedia\\Fireworks\\7\\Registration", L"Serial Number" },
    { L"Macromedia Fireworks MX 2004 (Validation Key)",     HKEY_LOCAL_MACHINE, L"Macromedia\\Fireworks\\7\\Registration", L"Validation" },
    { L"Macromedia Fireworks Studio MX (Serial)",           HKEY_LOCAL_MACHINE, L"Macromedia\\Fireworks\\6\\Registration", L"Serial Number" },
    { L"Macromedia Fireworks Studio MX (Validation Key)",   HKEY_LOCAL_MACHINE, L"Macromedia\\Fireworks\\6\\Registration", L"Validation" },
    { L"Macromedia Flash MX 2004 (First Name)",             HKEY_LOCAL_MACHINE, L"Macromedia\\Flash\\7\\Registration", L"First Name" },
    { L"Macromedia Flash MX 2004 (Last Name)",              HKEY_LOCAL_MACHINE, L"Macromedia\\Flash\\7\\Registration", L"Last Name" },
    { L"Macromedia Flash MX 2004 (Serial)",                 HKEY_LOCAL_MACHINE, L"Macromedia\\Flash\\7\\Registration", L"Serial Number" },
    { L"Macromedia Flash MX 2004 (Validation Key)",         HKEY_LOCAL_MACHINE, L"Macromedia\\Flash\\7\\Registration", L"Validation" },
    { L"Macromedia Flash Studio MX (Serial)",               HKEY_LOCAL_MACHINE, L"Macromedia\\Flash\\6\\Registration", L"Serial Number" },
    { L"Macromedia Flash Studio MX (Validation Key)",       HKEY_LOCAL_MACHINE, L"Macromedia\\Flash\\6\\Registration", L"Validation" },
    { L"Macromedia FreeHand MX 2004 (Serial)",              HKEY_LOCAL_MACHINE, L"Macromedia\\FreeHand\\111\\Registration", L"Serial Number" },
    { L"Macromedia FreeHand MX 2004 (Validation Key)",      HKEY_LOCAL_MACHINE, L"Macromedia\\FreeHand\\111\\Registration", L"Validation" },
    { L"Macromedia FreeHand Studio MX (Serial)",            HKEY_LOCAL_MACHINE, L"Macromedia\\FreeHand\\11\\Registration", L"Serial Number" },
    { L"Macromedia FreeHand Studio MX (Validation Key)",    HKEY_LOCAL_MACHINE, L"Macromedia\\FreeHand\\11\\Registration", L"Validation" },
    { L"Macromedia HomeSite+ 55 (Serial)",                  HKEY_LOCAL_MACHINE, L"Macromedia\\HomeSite+\\55\\Registration", L"Serial Number" },
    { L"Macromedia HomeSite+ 55 (Validation Key)",          HKEY_LOCAL_MACHINE, L"Macromedia\\HomeSite+\\55\\Registration", L"Validation" },
    { L"ACDSee 3.1",                                        HKEY_LOCAL_MACHINE, L"ACD Systems\\ACDSeeG", L"LicenseNumber" },
    { L"ACDSee 9.x (User)",                                 HKEY_LOCAL_MACHINE, L"ACD Systems\\ACDSee\\90", L"UserName" },
    { L"ACDSee 9.x (Key)",                                  HKEY_LOCAL_MACHINE, L"ACD Systems\\ACDSee\\90", L"LicenseNumber" },
    { L"Acronis True Image",                                HKEY_LOCAL_MACHINE, L"Acronis\\TrueImage\\Registration", L"standard" },
    { L"Advanced Smartcheck 3.1",                           HKEY_LOCAL_MACHINE, L"URL TOY\\ASC", L"Key" },
    { L"Advanced SystemCare 3 (Name)",                      HKEY_LOCAL_MACHINE, L"IObit\\Advanced SystemCare 3", L"Account" },
    { L"Advanced SystemCare 3 (Key)",                       HKEY_LOCAL_MACHINE, L"IObit\\Advanced SystemCare 3", L"Code" },
    { L"Alien Skin Exposure 2.0 (Name)",                    HKEY_LOCAL_MACHINE, L"Alien Skin\\Exposure\\2.0", L"Name" },
    { L"Alien Skin Exposure 2.0 (Company)",                 HKEY_LOCAL_MACHINE, L"Alien Skin\\Exposure\\2.0", L"Company" },
    { L"Alien Skin Exposure 2.0 (Key)",                     HKEY_LOCAL_MACHINE, L"Alien Skin\\Exposure\\2.0", L"Registration Code" },
    { L"Alien Skin Image Doctor 2.0 (Name)",                HKEY_LOCAL_MACHINE, L"Alien Skin\\Image Doctor\\2.0", L"Name" },
    { L"Alien Skin Image Doctor 2.0 (Company)",             HKEY_LOCAL_MACHINE, L"Alien Skin\\Image Doctor\\2.0", L"Company" },
    { L"Alien Skin Image Doctor 2.0 (Key)",                 HKEY_LOCAL_MACHINE, L"Alien Skin\\Image Doctor\\2.0", L"Registration Code" },
    { L"Alien Skin Snap Art 1.0 (Name)",                    HKEY_LOCAL_MACHINE, L"Alien Skin\\Snap Art\\1.0", L"Name" },
    { L"Alien Skin Snap Art 1.0 (Company)",                 HKEY_LOCAL_MACHINE, L"Alien Skin\\Snap Art\\1.0", L"Company" },
    { L"Alien Skin Snap Art 1.0 (Key)",                     HKEY_LOCAL_MACHINE, L"Alien Skin\\Snap Art\\1.0", L"Registration Code" },
    { L"Alien Skin Xenofex 2.0 (Name)",                     HKEY_LOCAL_MACHINE, L"Alien Skin\\Xenofex\\2.0", L"Name" },
    { L"Alien Skin Xenofex 2.0 (Company)",                  HKEY_LOCAL_MACHINE, L"Alien Skin\\Xenofex\\2.0", L"Company" },
    { L"Alien Skin Xenofex 2.0 (Key)",                      HKEY_LOCAL_MACHINE, L"Alien Skin\\Xenofex\\2.0", L"Registration Code" },
    { L"All Image 1.2.3",                                   HKEY_CURRENT_USER,  L"Towodo Software\\All Image\\License", L"LicenseKey" },
    { L"Anno 1701",                                         HKEY_LOCAL_MACHINE, L"Sunflowers\\Anno 1701", L"SerialNo" },
    { L"Araxis Merge 2008 (Name)",                          HKEY_CURRENT_USER,  L"Araxis\\Merge\\7.0", L"LicensedUser" },
    { L"Araxis Merge 2008 (Company)",                       HKEY_CURRENT_USER,  L"Araxis\\Merge\\7.0", L"CompanyName" },
    { L"Araxis Merge 2008 (Key)",                           HKEY_CURRENT_USER,  L"Araxis\\Merge\\7.0", L"SerialNumber" },
    { L"Arcsoft Panorama Maker 3.0",                        HKEY_CURRENT_USER,  L"Arcsoft\\Panorama Maker 3.0\\USER", L"SERIALNUMBER" },
    { L"Arcsoft Panorama Maker 3.5.0",                      HKEY_CURRENT_USER,  L"Arcsoft\\Panorama Maker\\3.5.0\\USER", L"SERIALNUMBER" },
    { L"Ashampoo Burning Studio 9",                         HKEY_CURRENT_USER,  L"Ashampoo\\Ashampoo Burning Studio 9", L"RegKey" },
    { L"Ashampoo Core Tuner",                               HKEY_LOCAL_MACHINE, L"Ashampoo\\Ashampoo Core Tuner", L"Key" },
    { L"Ashampoo Cover Studio 1.01",                        HKEY_CURRENT_USER,  L"Ashampoo\\Ashampoo Cover Studio", L"regKey" },
    { L"Autoplay Menu Builder 5.1 (Name)",                  HKEY_CURRENT_USER,  L"AutoPlay Menu Builder\\Registration", L"Name" },
    { L"Autoplay Menu Builder 5.1 (Key)",                   HKEY_CURRENT_USER,  L"AutoPlay Menu Builder\\Registration", L"Code" },
    { L"Avast Antivirus 5 (Licence file path)",             HKEY_LOCAL_MACHINE, L"ALWIL Software\\Avast\\5.0", L"LicenseFile" },
    { L"Avira AntiVir Personal Edition Classic (Product ID 1)", HKEY_LOCAL_MACHINE, L"Avira\\AntiVir PersonalEdition Classic", L"ProductID" },
    { L"Avira AntiVir Personal Edition Classic (Product ID 2)", HKEY_LOCAL_MACHINE, L"Avira\\AntiVir PersonalEdition Classic", L"WSC_ProductId" },
    { L"Axialis Professional Screen Saver Producer 3.6",    HKEY_CURRENT_USER,  L"Axialis\\Professional Screen Saver Producer\\registration", L"ProductKey" },
    { L"Batch Image Resizer (E-mail)",                      HKEY_CURRENT_USER,  L"JKLNSoft\\BatchImageResizer\\Registration Info", L"Email" },
    { L"Batch Image Resizer (Key)",                         HKEY_CURRENT_USER,  L"JKLNSoft\\BatchImageResizer\\Registration Info", L"Registration Code" },
    { L"Belarc Advisor",                                    HKEY_LOCAL_MACHINE, L"Belarc\\Advisor\\2.0", L"Serial Number" },
    { L"Beyond Compare 3",                                  HKEY_CURRENT_USER,  L"Scooter Software\\Beyond Compare 3", L"CacheID" },
    { L"Beyond Compare",                                    HKEY_CURRENT_USER,  L"Scooter Software\\Beyond Compare", L"CertKey" },
    { L"BitComet Acceleration Patch",                       HKEY_LOCAL_MACHINE, L"BitComet Acceleration Patch", L"Serial" },
    { L"CD Labeler Gold",                                   HKEY_CURRENT_USER,  L"VB and VBA Program Settings\\CDLabelergold\\Options", L"RegCode" },
    { L"CachemanXP",                                        HKEY_CURRENT_USER,  L"CachemanXP", L"Serial" },
    { L"Casir 2.2",                                         HKEY_CURRENT_USER,  L"iSergiwa Software\\CaSIR2.2", L"CaSIR ID" },
    { L"Chainz (Name)",                                     HKEY_LOCAL_MACHINE, L"GameHouse\\chainz", L"RegName" },
    { L"Chainz (Key)",                                      HKEY_LOCAL_MACHINE, L"GameHouse\\chainz", L"Registration" },
    { L"Chrome",                                            HKEY_LOCAL_MACHINE, L"Techland\\Chrome", L"SerialNumber" },
    { L"Company of Heroes: Opposing Fronts",                HKEY_LOCAL_MACHINE, L"THQ\\Company of Heroes", L"CoHOFProductKey" },
    { L"Company of Heroes: Opposing Fronts (Bonus Map)",    HKEY_LOCAL_MACHINE, L"THQ\\Company of Heroes", L"CoHOFRetailerContentKey" },
    { L"DVDFab 5",                                          HKEY_LOCAL_MACHINE, L"DVDFab\\V5", L"Key" },
    { L"DVDFab",                                            HKEY_LOCAL_MACHINE, L"DVDFab\\Platinum", L"Key" },
    { L"DameWare NT Utilities (Name)",                      HKEY_CURRENT_USER,  L"DameWare Development\\NT Utilities", L"Last Set Name" },
    { L"DameWare NT Utilities (Company)",                   HKEY_CURRENT_USER,  L"DameWare Development\\NT Utilities", L"Last Set Company" },
    { L"DameWare NT Utilities (Key)",                       HKEY_CURRENT_USER,  L"DameWare Development\\NT Utilities", L"Last Set Reg" },
    { L"Dawn of War: Dark Crusade",                         HKEY_LOCAL_MACHINE, L"THQ\\Dawn of War - Dark Crusade", L"CDKEY" },
    { L"Dawn of War: Soulstorm",                            HKEY_LOCAL_MACHINE, L"THQ\\Dawn of War - Soulstorm", L"CDKEY" },
    { L"Dawn of War: Winter Assualt",                       HKEY_LOCAL_MACHINE, L"THQ\\Dawn of War", L"CDKEY_WXP" },
    { L"Dawn of War",                                       HKEY_LOCAL_MACHINE, L"THQ\\Dawn of War", L"CDKEY" },
    { L"Dekart SIM Manager (Name)",                         HKEY_LOCAL_MACHINE, L"Dekart\\SIM Manager", L"UserName" },
    { L"Dekart SIM Manager (Key)",                          HKEY_LOCAL_MACHINE, L"Dekart\\SIM Manager", L"SerialNumber" },
    { L"Dell Service Tag",                                  HKEY_LOCAL_MACHINE, L"Dell Computer Corporation\\SysInfo", L"SerialNumber" },
    { L"DiskData (License)",                                HKEY_CURRENT_USER,  L"Digital Information Gallery\\DiskData\\Settings", L"Licensee" },
    { L"DiskData (Key)",                                    HKEY_CURRENT_USER,  L"Digital Information Gallery\\DiskData\\Settings", L"Key" },
    { L"DonationCoder Processtamer",                        HKEY_LOCAL_MACHINE, L"DonationCoder\\processtamer", L"ID" },
    { L"Dragon Naturally Speaking 10",                      HKEY_LOCAL_MACHINE, L"ScanSoft\\NaturallySpeaking10\\Activation", L"SerialNumber" },
    { L"Dream Aquarium (Name)",                             HKEY_LOCAL_MACHINE, L"Spiral Monkey\\Dream Aquarium", L"ownerName" },
    { L"Dream Aquarium (User ID)",                          HKEY_LOCAL_MACHINE, L"Spiral Monkey\\Dream Aquarium", L"userID" },
    { L"Dream Aquarium (Key)",                              HKEY_LOCAL_MACHINE, L"Spiral Monkey\\Dream Aquarium", L"license" },
    { L"EMCO MSI Package Builder 3 Pro",                    HKEY_LOCAL_MACHINE, L"EMCO\\MSI Package Builder 3 Professional", L"Signature" },
    { L"EZB Systems - Easyboot (Name)",                     HKEY_CURRENT_USER,  L"EasyBoot Systems\\EasyBoot\\3.0", L"UserName" },
    { L"EZB Systems - Easyboot (Key)",                      HKEY_CURRENT_USER,  L"EasyBoot Systems\\EasyBoot\\3.0", L"Registration" },
    { L"EZB Systems - UltraISO (Name)",                     HKEY_CURRENT_USER,  L"EasyBoot Systems\\UltraISO\\5.0", L"UserName" },
    { L"EZB Systems - UltraISO (Key)",                      HKEY_CURRENT_USER,  L"EasyBoot Systems\\UltraISO\\5.0", L"Registration" },
    { L"Elaborate Bytes Clone DVD",                         HKEY_LOCAL_MACHINE, L"Elaborate Bytes\\CloneDVD\\Key", L"Key" },
    { L"Final Data (Name)",                                 HKEY_LOCAL_MACHINE, L"FinalData\\FinalData\\2.0.1.1028", L"name" },
    { L"Final Data (Company)",                              HKEY_LOCAL_MACHINE, L"FinalData\\FinalData\\2.0.1.1028", L"Company" },
    { L"Final Data (Key)",                                  HKEY_LOCAL_MACHINE, L"FinalData\\FinalData\\2.0.1.1028", L"Serial" },
    { L"Final Recovery (Name)",                             HKEY_LOCAL_MACHINE, L"MeetSoft\\FinalRecovery", L"Username" },
    { L"Final Recovery (Key)",                              HKEY_LOCAL_MACHINE, L"MeetSoft\\FinalRecovery", L"RegCode" },
    { L"FinePrint 5 (Name)",                                HKEY_LOCAL_MACHINE, L"FinePrint Software\\FinePrint5", L"RegName" },
    { L"FinePrint 5 (Key)",                                 HKEY_LOCAL_MACHINE, L"FinePrint Software\\FinePrint5", L"RegNum" },
    { L"FinePrint PdfFactory Pro 2 (Name)",                 HKEY_LOCAL_MACHINE, L"FinePrint Software\\pdfFactory2", L"RegName" },
    { L"FinePrint PdfFactory Pro 2 (Key)",                  HKEY_LOCAL_MACHINE, L"FinePrint Software\\pdfFactory2", L"RegNum" },
    { L"FinePrint pdfFactory 8 (Name)",                     HKEY_LOCAL_MACHINE, L"FinePrint Software\\pdfFactory3", L"RegName" },
    { L"FinePrint pdfFactory 8 (Key)",                      HKEY_LOCAL_MACHINE, L"FinePrint Software\\pdfFactory3", L"RegNum" },
    { L"Funk Software, Inc: Proxy 5, 6 and 7 Master",       HKEY_LOCAL_MACHINE, L"Funk Software, Inc.\\Proxy v5\\Proxy Master\\LicenseKeys", L"" },
    { L"GetDataBack for NTFS (Name)",                       HKEY_LOCAL_MACHINE, L"Runtime Software\\GetDataBackNT\\License", L"Name" },
    { L"GetDataBack for NTFS (Key)",                        HKEY_LOCAL_MACHINE, L"Runtime Software\\GetDataBackNT\\License", L"Key" },
    { L"GetDataBack (Name)",                                HKEY_LOCAL_MACHINE, L"Runtime Software\\GetDataBack\\License", L"Name" },
    { L"GetDataBack (Key)",                                 HKEY_LOCAL_MACHINE, L"Runtime Software\\GetDataBack\\License", L"Key" },
    { L"GetRight",                                          HKEY_LOCAL_MACHINE, L"Headlight\\GetRight", L"GRCode" },
    { L"Hidden & Dangerous 2",                              HKEY_LOCAL_MACHINE, L"Illusion Softworks\\Hidden & Dangerous 2", L"key" },
    { L"Hide Folders XP (Name)",                            HKEY_LOCAL_MACHINE, L"FSPro Labs\\Hide Folders XP 2", L"RegUserName" },
    { L"Hide Folders XP (Key)",                             HKEY_LOCAL_MACHINE, L"FSPro Labs\\Hide Folders XP 2", L"RegCode" },
    { L"IGI 2: Covert Strike",                              HKEY_LOCAL_MACHINE, L"IGI 2 Retail", L"CDKey" },
    { L"IconSaver 2.2 (Name)",                              HKEY_CURRENT_USER,  L"Mario Knok\\IconSaver", L"Registered Name" },
    { L"IconSaver 2.2 (Key)",                               HKEY_CURRENT_USER,  L"Mario Knok\\IconSaver", L"Registered Key" },
    { L"Industry Giant 2",                                  HKEY_CURRENT_USER,  L"JoWooD\\InstalledGames\\IG2", L"prvkey" },
    { L"Internet Download Manager (First Name)",            HKEY_LOCAL_MACHINE, L"Internet Download Manager", L"FName" },
    { L"Internet Download Manager (Last Name)",             HKEY_LOCAL_MACHINE, L"Internet Download Manager", L"LName" },
    { L"Internet Download Manager (Email)",                 HKEY_LOCAL_MACHINE, L"Internet Download Manager", L"Email" },
    { L"Internet Download Manager (Key)",                   HKEY_LOCAL_MACHINE, L"Internet Download Manager", L"Serial" },
    { L"Legends of Might and Magic",                        HKEY_CURRENT_USER,  L"3d0\\Status", L"CustomerNumber" },
    { L"LimeWire Acceleration Patch",                       HKEY_LOCAL_MACHINE, L"LimeWire Acceleration Patch", L"Serial" },
    { L"MagicISO (Name)",                                   HKEY_CURRENT_USER,  L"MagicISO", L"RegUserName" },
    { L"MagicISO (Key)",                                    HKEY_CURRENT_USER,  L"MagicISO", L"RegSerialKey" },
    { L"MagicISO (Name)",                                   HKEY_LOCAL_MACHINE, L"MagicISO", L"RegUserName" },
    { L"MagicISO (Key)",                                    HKEY_LOCAL_MACHINE, L"MagicISO", L"RegSerialKey" },
    { L"Magix Audio Cleaning Lab Deluxe",                   HKEY_LOCAL_MACHINE, L"Magix\\AACCodes", L"MAGIX_CODE" },
    { L"Malwarebytes Anti-Malware (ID)",                    HKEY_LOCAL_MACHINE, L"Malwarebytes' Anti-Malware", L"ID" },
    { L"Malwarebytes Anti-Malware (Key)",                   HKEY_LOCAL_MACHINE, L"Malwarebytes' Anti-Malware", L"key" },
    { L"MaxDSLR 4.0 (Name)",                                HKEY_LOCAL_MACHINE, L"Diffraction Limited\\MaxDSLR\\4.0", L"Name" },
    { L"MaxDSLR 4.0 (Key)",                                 HKEY_LOCAL_MACHINE, L"Diffraction Limited\\MaxDSLR\\4.0", L"Serial" },
    { L"Media Monkey 3.x (Name)",                           HKEY_CURRENT_USER,  L"MediaMonkey", L"Username" },
    { L"Media Monkey 3.x (Key)",                            HKEY_CURRENT_USER,  L"MediaMonkey", L"Password" },
    { L"Mediafour MacDrive 7.x",                            HKEY_LOCAL_MACHINE, L"Mediafour\\Common\\LM\\{B6916668-60EA-4EE6-AC3E-DA8989868EC0}\\md7", L"SerialNumber" },
    { L"Medieval II Total War",                             HKEY_LOCAL_MACHINE, L"SEGA\\Medieval II Total War", L"CDKey" },
    { L"MemoriesOnTV (Name)",                               HKEY_LOCAL_MACHINE, L"CodeJam\\MemoriesOnTV\\Settings", L"User" },
    { L"MemoriesOnTV (Key)",                                HKEY_LOCAL_MACHINE, L"CodeJam\\MemoriesOnTV\\Settings", L"Serial" },
    { L"Mindjet MindManager 7 Pro",                         HKEY_LOCAL_MACHINE, L"Mindjet\\MindManager\\7\\Registration", L"LicenseKey" },
    { L"MochaSoft mtn5250 (Company)",                       HKEY_CURRENT_USER,  L"MochaSoft\\mtn5250\\Version 1.0\\1", L"company" },
    { L"MochaSoft mtn5250 (Host)",                          HKEY_CURRENT_USER,  L"MochaSoft\\mtn5250\\Version 1.0\\1", L"lickey" },
    { L"MochaSoft mtn5250 (Key)",                           HKEY_CURRENT_USER,  L"MochaSoft\\mtn5250\\Version 1.0\\1", L"host_name" },
    { L"Monitor Control (Name)",                            HKEY_CURRENT_USER,  L"Softany\\Monitor Control", L"RegName" },
    { L"Monitor Control (Key)",                             HKEY_CURRENT_USER,  L"Softany\\Monitor Control", L"RegCode" },
    { L"NOX",                                               HKEY_LOCAL_MACHINE, L"Westwood\\NOX", L"Serial" },
    { L"NTI CD Maker",                                      HKEY_LOCAL_MACHINE, L"NewTech Infosystems\\NTI CD-Maker", L"ProductKey" },
    { L"Netopia Timbuktu Pro",                              HKEY_LOCAL_MACHINE, L"Netopia\\Timbuktu Pro", L"ProductCode" },
    { L"Company of Heroes",                                 HKEY_LOCAL_MACHINE, L"THQ\\Company of Heroes", L"CoHOFProductKey" },
    { L"Company of Heroes (Bonus Map)",                     HKEY_LOCAL_MACHINE, L"THQ\\Company of Heroes", L"CoHOFRetailerContentKey" },
    { L"Olympus DSS Player 2002 (Name)",                    HKEY_LOCAL_MACHINE, L"Olympus\\DSSPLayer2002\\Dictation Module\\UserInformation", L"UserName" },
    { L"Olympus DSS Player 2002 (Company)",                 HKEY_LOCAL_MACHINE, L"Olympus\\DSSPLayer2002\\Dictation Module\\UserInformation", L"CompanyName" },
    { L"Olympus DSS Player 2002 (Key)",                     HKEY_LOCAL_MACHINE, L"Olympus\\DSSPLayer2002\\Dictation Module\\UserInformation", L"SerialNumber" },
    { L"Olympus DSS Player Pro (Name)",                     HKEY_LOCAL_MACHINE, L"Olympus\\DSSPLayerPro\\Transcription Module\\UserInformation", L"UserName" },
    { L"Olympus DSS Player Pro (Company)",                  HKEY_LOCAL_MACHINE, L"Olympus\\DSSPLayerPro\\Transcription Module\\UserInformation", L"CompanyName" },
    { L"Olympus DSS Player Pro (Key)",                      HKEY_LOCAL_MACHINE, L"Olympus\\DSSPLayerPro\\Transcription Module\\UserInformation", L"SerialNumber" },
    { L"PCTools Registry Machanic (Name)",                  HKEY_LOCAL_MACHINE, L"PCTools\\Registry Mechanic\\Settings", L"Owner" },
    { L"PCTools Registry Machanic (Key)",                   HKEY_LOCAL_MACHINE, L"PCTools\\Registry Mechanic\\Settings", L"Licensekey" },
    { L"PGWARE GameGain (Name)",                            HKEY_LOCAL_MACHINE, L"PGWARE\\GameGain", L"Name" },
    { L"PGWARE GameGain (Key)",                             HKEY_LOCAL_MACHINE, L"PGWARE\\GameGain", L"Serial" },
    { L"Padus DiscJuggler (Name)",                          HKEY_LOCAL_MACHINE, L"Padus\\DiscJuggler\\Settings", L"User" },
    { L"Padus DiscJuggler (Key)",                           HKEY_LOCAL_MACHINE, L"Padus\\DiscJuggler\\Settings", L"Registration" },
    { L"PaperPort Professional (Name)",                     HKEY_LOCAL_MACHINE, L"ScanSoft\\PaperPort\\License", L"UserName" },
    { L"PaperPort Professional (Company)",                  HKEY_LOCAL_MACHINE, L"ScanSoft\\PaperPort\\License", L"UserOrganization" },
    { L"PaperPort Professional (Key)",                      HKEY_LOCAL_MACHINE, L"ScanSoft\\PaperPort\\License", L"Serial#" },
    { L"Passware 7.x (Name)",                               HKEY_CURRENT_USER,  L"Passware\\Passware Kit Enterprise\\7\\Registration", L"Name" },
    { L"Passware 7.x (License)",                            HKEY_CURRENT_USER,  L"Passware\\Passware Kit Enterprise\\7\\Registration", L"License" },
    { L"Passware 7.x (Serial)",                             HKEY_CURRENT_USER,  L"Passware\\Passware Kit Enterprise\\7\\Registration", L"Serial" },
    { L"Passware 8.x (Name)",                               HKEY_CURRENT_USER,  L"Passware\\Passware Kit Enterprise\\8\\Registration", L"Name" },
    { L"Passware 8.x (License)",                            HKEY_CURRENT_USER,  L"Passware\\Passware Kit Enterprise\\8\\Registration", L"License" },
    { L"Passware 8.x (Serial)",                             HKEY_CURRENT_USER,  L"Passware\\Passware Kit Enterprise\\8\\Registration", L"Serial" },
    { L"Passware Kit",                                      HKEY_CURRENT_USER,  L"Passware\\Passware Kit\\Registration", L"Serial" },
    { L"Photomatix Pro (Name)",                             HKEY_CURRENT_USER,  L"MultimediaPhoto\\Photomatix\\Registration", L"UserID" },
    { L"Photomatix Pro (Key)",                              HKEY_CURRENT_USER,  L"MultimediaPhoto\\Photomatix\\Registration", L"Key" },
    { L"PopCap Games: Peggle Nights (Name)",                HKEY_CURRENT_USER,  L"PopCap\\PeggleNights", L"RegName" },
    { L"PopCap Games: Peggle Nights (Key)",                 HKEY_CURRENT_USER,  L"PopCap\\PeggleNights", L"RegCode" },
    { L"Pro Evolution Soccer 6",                            HKEY_LOCAL_MACHINE, L"KONAMIPES6\\PES6", L"code" },
    { L"R-Studio Data Recovery (Name)",                     HKEY_CURRENT_USER,  L"R-TT\\R-Studio\\00000000", L"name" },
    { L"R-Studio Data Recovery (Company)",                  HKEY_CURRENT_USER,  L"R-TT\\R-Studio\\00000000", L"company" },
    { L"R-Studio Data Recovery (Key)",                      HKEY_CURRENT_USER,  L"R-TT\\R-Studio\\00000000", L"serial" },
    { L"RIP3 The Last Hero",                                HKEY_LOCAL_MACHINE, L"White Elephant\\RIP3 The Last Hero", L"Key" },
    { L"Ravenshield",                                       HKEY_LOCAL_MACHINE, L"Red Storm Entertainment\\RAVENSHIELD", L"CDKey" },
    { L"Registy Mechanic (Name)",                           HKEY_LOCAL_MACHINE, L"PCTools\\Registry Mechanic\\Settings Owner", L"Name" },
    { L"Registy Mechanic (Company)",                        HKEY_LOCAL_MACHINE, L"PCTools\\Registry Mechanic\\Settings LicenseKey", L"serial" },
    { L"Restorator 2005 (Name)",                            HKEY_CURRENT_USER,  L"Bomers\\Restorator 2005\\Registration", L"Name" },
    { L"Restorator 2005 (Key)",                             HKEY_CURRENT_USER,  L"Bomers\\Restorator 2005\\Registration", L"Password" },
    { L"Roxio 2009",                                        HKEY_LOCAL_MACHINE, L"Sonic\\CinePlayer\\5.0\\Preference", L"CDKey" },
    { L"Roxio My DVD 8 Premier",                            HKEY_LOCAL_MACHINE, L"roxio\\Registration\\RoxioCentral", L"RoxioCentral_SN" },
    { L"Roxio My DVD 9.0",                                  HKEY_LOCAL_MACHINE, L"Roxio\\Registration\\RoxioCentral33", L"RoxioCentral_SN" },
    { L"Scansoft Omnipage Professional 16",                 HKEY_LOCAL_MACHINE, L"ScanSoft\\OmniPage 16", L"SerialNumber" },
    { L"Screensaver Remover (Name)",                        HKEY_CURRENT_USER,  L"Softany\\Screensaver Remover", L"RegName" },
    { L"Screensaver Remover (Key)",                         HKEY_CURRENT_USER,  L"Softany\\Screensaver Remover", L"RegCodeSecurDataStor v6 CD-Media" },
    { L"SecurDataStor v6 (Machine)",                        HKEY_LOCAL_MACHINE, L"encryptX\\SecurDataStor v6\\Registration", L"License" },
    { L"SecurDataStor v6 (User)",                           HKEY_CURRENT_USER,  L"encryptX\\SecurDataStor v6\\Registration", L"License" },
    { L"Slysoft AnyDVD",                                    HKEY_LOCAL_MACHINE, L"SlySoft\\AnyDVD\\Key", L"Key" },
    { L"Slysoft CloneCD (Name)",                            HKEY_LOCAL_MACHINE, L"SlySoft\\CloneCD\\CDKey", L"Name" },
    { L"Slysoft CloneCD (Company)",                         HKEY_LOCAL_MACHINE, L"SlySoft\\CloneCD\\CDKey", L"Company" },
    { L"Slysoft CloneCD (Key)",                             HKEY_LOCAL_MACHINE, L"SlySoft\\CloneCD\\CDKey", L"Key" },
    { L"Slysoft CloneDVD mobile",                           HKEY_LOCAL_MACHINE, L"SlySoft\\CloneDVDmobile\\Key", L"Key" },
    { L"Slysoft CloneDVD",                                  HKEY_LOCAL_MACHINE, L"SlySoft\\CloneDVD\\Key", L"Key" },
    { L"Slysoft GameJackal Pro",                            HKEY_LOCAL_MACHINE, L"SlySoft\\GameJackal\\Key", L"Key" },
    { L"Soldiers Of Anarchy",                               HKEY_CURRENT_USER,  L"Silver Style Entertainment\\Soldiers Of Anarchy\\Settings", L"CDKey" },
    { L"Sonic Foundry ACID 4",                              HKEY_LOCAL_MACHINE, L"Sonic Foundry\\ACID\\4.0\\License", L"CurrentKey" },
    { L"Sonic RecordNow",                                   HKEY_LOCAL_MACHINE, L"Sonic\\Registration\\RecordNow", L"RecordNow_SN" },
    { L"Sony ACID XMC",                                     HKEY_LOCAL_MACHINE, L"Sony Media Software\\ACID XMC\\6.0", L"ProductCode" },
    { L"Sony Sound Forge 7",                                HKEY_LOCAL_MACHINE, L"Sony Media Software\\Sound Forge\\7.0\\License", L"CurrentKey" },
    { L"Sony Sound Forge 9",                                HKEY_LOCAL_MACHINE, L"Sony Media Software\\Sound Forge\\9.0\\License", L"CurrentKey" },
    { L"Stardock Apogee Icon Suite (Email)",                HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ap", L"Email" },
    { L"Stardock Apogee Icon Suite (Key)",                  HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ap", L"Serial No" },
    { L"Stardock Aquarium Desktop (Email)",                 HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ad", L"Email" },
    { L"Stardock Aquarium Desktop (Key)",                   HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ad", L"Serial No" },
    { L"Stardock Aquarium Desktop 2006 (Email)",            HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ad2", L"Email" },
    { L"Stardock Aquarium Desktop 2006 (Key)",              HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ad2", L"Serial No" },
    { L"Stardock Aquarium Desktop 2006 (Email)",            HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\adu", L"Email" },
    { L"Stardock Aquarium Desktop 2006 (Key)",              HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\adu", L"Serial No" },
    { L"Stardock Aquarium Desktop 2007 (Email)",            HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ad3", L"Email" },
    { L"Stardock Aquarium Desktop 2007 (Key)",              HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ad3", L"Serial No" },
    { L"Stardock Blog Navigator Pro (Email)",               HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\bn", L"Email" },
    { L"Stardock Blog Navigator Pro (Key)",                 HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\bn", L"Serial No" },
    { L"Stardock CursorXP Plus (Email)",                    HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\cxp", L"Email" },
    { L"Stardock CursorXP Plus (Key)",                      HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\cxp", L"Serial No" },
    { L"Stardock Desktop Pet (Email)",                      HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\pt1", L"Email" },
    { L"Stardock Desktop Pet (Key)",                        HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\pt1", L"Serial No" },
    { L"Stardock DesktopX (Name)",                          HKEY_LOCAL_MACHINE, L"Stardock\\DesktopX", L"Name" },
    { L"Stardock DesktopX (Key)",                           HKEY_LOCAL_MACHINE, L"Stardock\\DesktopX", L"Serial" },
    { L"Stardock Natural Desktop (Email)",                  HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\nd", L"Email" },
    { L"Stardock Natural Desktop (Key)",                    HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\nd", L"Serial No" },
    { L"Stardock ObjectDesktop (First Name)",               HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\ObjectDesktop", L"First Name" },
    { L"Stardock ObjectDesktop (Last Name)",                HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\ObjectDesktop", L"Last Name" },
    { L"Stardock ObjectDesktop (Email)",                    HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\ObjectDesktop", L"Email" },
    { L"Stardock ObjectDesktop (Key)",                      HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\ObjectDesktop", L"Serial No" },
    { L"Stardock Orion Icon Suite (Email)",                 HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\or", L"Email" },
    { L"Stardock Orion Icon Suite (Key)",                   HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\or", L"Serial No" },
    { L"Stardock ThinkDesk (First Name)",                   HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\ThinkDesk", L"First Name" },
    { L"Stardock ThinkDesk (Last Name)",                    HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\ThinkDesk", L"Last Name" },
    { L"Stardock ThinkDesk (Email)",                        HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\ThinkDesk", L"Email" },
    { L"Stardock ThinkDesk (Key)",                          HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\ThinkDesk", L"Serial No" },
    { L"Stardock Unorthodox Suite (Email)",                 HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\un", L"Email" },
    { L"Stardock Unorthodox Suite (Key)",                   HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\un", L"Serial No" },
    { L"Stardock Utopia Icon Suite (Email)",                HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ut", L"Email" },
    { L"Stardock Utopia Icon Suite (Key)",                  HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\Stardock\\ut", L"Serial No" },
    { L"Stardock WinCustomize (First Name)",                HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\WinCustomize", L"First Name" },
    { L"Stardock WinCustomize (Last Name)",                 HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\WinCustomize", L"Last name" },
    { L"Stardock WinCustomize (Email)",                     HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\WinCustomize", L"Email" },
    { L"Stardock WinCustomize (Key)",                       HKEY_LOCAL_MACHINE, L"Stardock\\ComponentManager\\WinCustomize", L"Serial No" },
    { L"TMPGEnc Plus 2.5 (Name)",                           HKEY_LOCAL_MACHINE, L"Pegasys Inc.\\TMPGEnc Plus\\2.5", L"UserName" },
    { L"TMPGEnc Plus 2.5 (Company)",                        HKEY_LOCAL_MACHINE, L"Pegasys Inc.\\TMPGEnc Plus\\2.5", L"CompanyName" },
    { L"TMPGEnc Plus 2.5 (Key)",                            HKEY_LOCAL_MACHINE, L"Pegasys Inc.\\TMPGEnc Plus\\2.5", L"SerialID" },
    { L"Teleport Pro (Name)",                               HKEY_LOCAL_MACHINE, L"Tennyson Maxwell\\Teleport Pro", L"Name" },
    { L"Teleport Pro (Company)",                            HKEY_LOCAL_MACHINE, L"Tennyson Maxwell\\Teleport Pro", L"Company" },
    { L"The Gladiators",                                    HKEY_CURRENT_USER,  L"Eugen Systems\\The Gladiators", L"RegNumber" },
    { L"The Gladiators",                                    HKEY_LOCAL_MACHINE, L"Eugen Systems\\The Gladiators", L"RegNumber" },
    { L"Tobit ClipIncPlayer",                               HKEY_LOCAL_MACHINE, L"Tobit\\Tobit ClipInc\\Server\\Setup", L"LicenseNo" },
    { L"TrackMania United",                                 HKEY_LOCAL_MACHINE, L"Nadeo\\TmUnited\\Keys\\License0", L"Key" },
    { L"Tracks Eraser Pro (Name)",                          HKEY_LOCAL_MACHINE, L"Acesoft\\tracks eraser pro", L"User" },
    { L"Tracks Eraser Pro (Key)",                           HKEY_LOCAL_MACHINE, L"Acesoft\\tracks eraser pro", L"Code" },
    { L"VanDyke SecureFX (Key)",                            HKEY_LOCAL_MACHINE, L"VanDyke\\SecureFX\\License", L"key" },
    { L"VanDyke SecureFX (Serial)",                         HKEY_LOCAL_MACHINE, L"VanDyke\\SecureFX\\License", L"Serial Number" },
    { L"WinASO Registry Optimizer 3",                       HKEY_LOCAL_MACHINE, L"WinASO\\Registry Optimizer", L"RegOptKey3.0" },
    { L"WinASO Registry Optimizer 4",                       HKEY_LOCAL_MACHINE, L"WinASO\\Registry Optimizer", L"RegOptKey4" },
    { L"WinImage (Name)",                                   HKEY_CURRENT_USER,  L"WinImage", L"NameRegistered" },
    { L"WinImage (Key)",                                    HKEY_CURRENT_USER,  L"WinImage", L"CodeRegistered" },
    { L"WinZIP (Name)",                                     HKEY_LOCAL_MACHINE, L"Nico Mak Computing\\WinZip\\Winini", L"Name1" },
    { L"WinZIP (Key)",                                      HKEY_LOCAL_MACHINE, L"Nico Mak Computing\\WinZip\\Winini", L"SN1" },
    { L"WinZIP (Name)",                                     HKEY_CURRENT_USER,  L"Nico Mak Computing\\WinZip\\Winini", L"Name1" },
    { L"WinZIP (Key)",                                      HKEY_CURRENT_USER,  L"Nico Mak Computing\\WinZip\\Winini", L"SN1" },
    { L"X1 Desktop Search",                                 HKEY_LOCAL_MACHINE, L"X1 Desktop Search", L"Registration" },
    { L"XThink Calculator (Name)",                          HKEY_LOCAL_MACHINE, L"xThink\\Calculator\\UserInfo", L"UserName" },
    { L"XThink Calculator (Email)",                         HKEY_LOCAL_MACHINE, L"xThink\\Calculator\\UserInfo", L"Email" },
    { L"XThink Calculator (Key)",                           HKEY_LOCAL_MACHINE, L"xThink\\Calculator\\UserInfo", L"KeyCode" },
    { L"ZoneAlarm (Name)",                                  HKEY_LOCAL_MACHINE, L"Zone Labs\\ZoneAlarm\\Registration", L"RegisteredOwner" },
    { L"ZoneAlarm (Company)",                               HKEY_LOCAL_MACHINE, L"Zone Labs\\ZoneAlarm\\Registration", L"RegisteredOrganization" },
    { L"ZoneAlarm (Key)",                                   HKEY_LOCAL_MACHINE, L"Zone Labs\\ZoneAlarm\\Registration", L"SerialNum" },
    { L"eHelp RoboHelp Office X3",                          HKEY_LOCAL_MACHINE, L"eHelp\\RoboHelp\\11.00\\Serial Number", L"" },
    { L"gotoMaxx pdfMAILER",                                HKEY_LOCAL_MACHINE, L"gotomaxx\\maxx PDFMAILER\\Professional\\Registration", L"SerialCode" },
    { L"onOne Focalpoint v1.0",                             HKEY_LOCAL_MACHINE, L"onOne Software\\FocalPoint 1.0\\Registration", L"SerialNumber" },
    { L"onOne Mask Pro 4.0 (Name)",                         HKEY_LOCAL_MACHINE, L"onOne Software\\Mask Pro 4.0\\Registration", L"Name" },
    { L"onOne Mask Pro 4.0 (Company)",                      HKEY_LOCAL_MACHINE, L"onOne Software\\Mask Pro 4.0\\Registration", L"Company" },
    { L"onOne Mask Pro 4.0 (Key)",                          HKEY_LOCAL_MACHINE, L"onOne Software\\Mask Pro 4.0\\Registration", L"SerialNumber" },
    { L"onOne Phototools Professional 1.0 (Name)",          HKEY_LOCAL_MACHINE, L"onOne Software\\PhotoTools 1.0 Professional Edition\\Registration", L"Name" },
    { L"onOne Phototools Professional 1.0 (Company)",       HKEY_LOCAL_MACHINE, L"onOne Software\\PhotoTools 1.0 Professional Edition\\Registration", L"Company" },
    { L"onOne Phototools Professional 1.0 (Key)",           HKEY_LOCAL_MACHINE, L"onOne Software\\PhotoTools 1.0 Professional Edition\\Registration", L"SerialNumber" },
    { L"Sony Vegas 6",                                      HKEY_LOCAL_MACHINE, L"Sony Media Software\\Vegas\\6.0\\License", L"CurrentKey" },
    { L"Sony Vegas 8",                                      HKEY_LOCAL_MACHINE, L"Sony Media Software\\Vegas Movie Studio Platinum\\6.0\\License", L"CurrentKey" },
    { L"Star Wars: Empire at War",                          HKEY_LOCAL_MACHINE, L"LucasArts\\Star Wars Empire at War\\1.0", L"CD Key" },
    { L"Submarine TITANS",                                  HKEY_LOCAL_MACHINE, L"Ellipse Studios\\Submarine Titans\\Version", L"Item" },
    { L"Swiftdog GameHike (Name)",                          HKEY_LOCAL_MACHINE, L"SWIFTDOG\\GameHike", L"Name" },
    { L"Swiftdog GameHike (Key)",                           HKEY_LOCAL_MACHINE, L"SWIFTDOG\\GameHike", L"Serial" },
    { L"Swiftdog GameThrust (Name)",                        HKEY_LOCAL_MACHINE, L"SWIFTDOG\\GameThrust", L"Name" },
    { L"Swiftdog GameThrust (Key)",                         HKEY_LOCAL_MACHINE, L"SWIFTDOG\\GameThrust", L"Serial" },

    { 0 }
};

VOID
GetOtherLicensiesInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szPath[MAX_PATH];
    INT i = 0;

    do
    {
        szText[0] = 0;

        StringCbPrintf(szPath, sizeof(szPath), L"SOFTWARE\\%s",
                       LicensiesInfo[i].lpKeyPath);

        if (GetStringFromRegistry(TRUE,
                                  LicensiesInfo[i].hRootKey,
                                  szPath,
                                  LicensiesInfo[i].lpKeyName,
                                  szText,
                                  MAX_STR_LEN))
        {
            if (szText[0] != 0)
            {
                IoAddItem(0, 0, LicensiesInfo[i].lpProductName);
                IoSetItemText(szText);
            }
        }
    }
    while (LicensiesInfo[++i].hRootKey != 0);

    if (!IsWin64System()) return;

    i = 0;

    do
    {
        szText[0] = 0;

        StringCbPrintf(szPath, sizeof(szPath),
                       L"SOFTWARE\\Wow6432Node\\%s",
                       LicensiesInfo[i].lpKeyPath);

        if (GetStringFromRegistry(TRUE,
                                  LicensiesInfo[i].hRootKey,
                                  szPath,
                                  LicensiesInfo[i].lpKeyName,
                                  szText,
                                  MAX_STR_LEN))
        {
            if (szText[0] != 0)
            {
                IoAddItem(0, 0, LicensiesInfo[i].lpProductName);
                IoSetItemText(szText);
            }
        }
    }
    while (LicensiesInfo[++i].hRootKey != 0);
}

typedef struct
{
    LPWSTR lpProductName;
    LPSTR lpKeyPath;
} MS_LICENSIES_INFO;

MS_LICENSIES_INFO MsLicensies[] =
{
    {L"Microsoft Windows",     "SOFTWARE\\MICROSOFT\\Windows NT\\CurrentVersion"},

    {L"Microsoft SQL 7.0", "SOFTWARE\\Microsoft\\Microsoft SQL Server\\70\\Registration"},
    {L"Microsoft SQL Server 2000", "SOFTWARE\\Microsoft\\Microsoft SQL Server\\80\\Registration"},
    {L"Microsoft SQL Server 2005", "SOFTWARE\\Microsoft\\Microsoft SQL Server\\90\\Registration"},
    {L"Microsoft SQL Server 2008", "SOFTWARE\\Microsoft\\MSDN\\9.0\\Registration"},

    {L"Microsoft Office 2010", "SOFTWARE\\Microsoft\\Office\\14.0\\Registration\\{90140000-0057-0000-0000-0000000FF1CE}"},

    {L"Microsoft Office 2007", "SOFTWARE\\Microsoft\\Office\\12.0\\Registration\\{90120000-0030-0000-0000-0000000FF1CE}"},
    {L"Microsoft Office 2007", "SOFTWARE\\Microsoft\\Office\\12.0\\Registration\\{90120000-0011-0000-0000-0000000FF1CE}"},

    {L"Microsoft Office 2003", "SOFTWARE\\Microsoft\\Office\\11.0\\Registration\\{90170409-6000-11D3-8CFE-0150048383C9}"},
    {L"Microsoft Office 2003", "SOFTWARE\\Microsoft\\Office\\11.0\\Registration\\{90110419-6000-11D3-8CFE-0150048383C9}"},

    {L"Microsoft Office XP", "SOFTWARE\\Microsoft\\Office\\10.0\\Registration\\{90280409-6000-11D3-8CFE-0050048383C9}"},

    {L"Office Web Developer 2007", "SOFTWARE\\Microsoft\\Office\\12.0\\Registration\\{90120000-0021-0000-0000-0000000FF1CE}"},

    {L"Windows Mobile Device Center 6.1", "SOFTWARE\\Microsoft\\Windows Mobile Device Center\\6.1\\Registration"},

    {L"Internet Explorer", "SOFTWARE\\Microsoft\\Internet Explorer\\Registration"},

    {0}
};

VOID
InsertKeySep(LPWSTR szIn, LPWSTR szOut)
{
    SIZE_T i = 0, j, c = 0;
    INT len;

    for (j = 0, len = SafeStrLen(szIn); i < (SIZE_T)len; j++)
    {
        if (c == 5)
        {
            szOut[j] = L'-';
            c = 0;
        }
        else
        {
            szOut[j] = szIn[i];
            ++i, ++c;
        }
    }

    szOut[5 * 5 + 4] = 0;
}

/* http://social.msdn.microsoft.com/Forums/en-US/vclanguage/thread/348a99cb-758e-4cff-858c-10d09710f784/ */
BOOL
GetMSProductKey(BOOL is64, LPSTR lpszKeyPath, LPWSTR lpszKey, INT iSize)
{
    REGSAM samDesired = KEY_QUERY_VALUE;
    const CHAR *KeyChars[] = {"B","C","D","F","G","H","J","K","M",
                              "P","Q","R","T","V","W","X","Y","2",
                              "3","4","6","7","8","9",NULL};
    CHAR *wsResult, *pch, sCDKey[30];
    BYTE *DigitalProductID;
    SIZE_T DataLength;
    BYTE ProductKeyExtract[15];
    INT ByteCounter, k, nCur;
    HKEY hKey;

    DebugTrace(L"is64 = %d, lpszKeyPath = %S, lpszKey = %s, iSize = %d",
               is64, lpszKeyPath, lpszKey, iSize);

    DigitalProductID = 0;

    /* Check Win2000 (KEY_WOW64_64KEY - not supported) */
    if (!IsWindows2000() && is64)
    {
        samDesired |= KEY_WOW64_64KEY;
    }

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, lpszKeyPath,
                      REG_OPTION_NON_VOLATILE,
                      samDesired,
                      &hKey) == ERROR_SUCCESS)
    {
        DataLength = 164;
        DigitalProductID = (BYTE*)Alloc(DataLength);
        SecureZeroMemory(DigitalProductID, DataLength);

        if (RegQueryValueExA(hKey,
                             "DigitalProductId",
                             NULL, NULL,
                             DigitalProductID,
                             &DataLength) == ERROR_SUCCESS)
        {
            CopyMemory(ProductKeyExtract, DigitalProductID + 52, 15);
        }

        if (DigitalProductID) Free(DigitalProductID);
        RegCloseKey(hKey);
    }
    else return FALSE;

    SecureZeroMemory(sCDKey, sizeof(sCDKey));

    for (ByteCounter = 24; ByteCounter >= 0; --ByteCounter)
    {
        nCur = 0;
        for (k = 14; k >= 0; --k)
        {
            nCur = (nCur * 256) ^ ProductKeyExtract[k];
            ProductKeyExtract[k] = (BYTE)(nCur / 24);
            nCur %= 24;
        }
        StringCbCatA((STRSAFE_LPSTR)sCDKey,
                      sizeof(sCDKey),
                      KeyChars[nCur]);
        if (!(ByteCounter % 5) && ByteCounter)
            StringCbCatA(sCDKey, sizeof(sCDKey), "-");
    }

    _strrev(sCDKey);
    wsResult = sCDKey;

    pch = strstr(wsResult, "-");

    if (pch)
    {
        if (lpszKey)
        {
            MultiByteToWideChar(CP_ACP, 0, wsResult, -1, lpszKey, iSize);
        }
    }
    else
    {
         GetMSProductKey(TRUE, lpszKeyPath, NULL, 0);
    }

    return TRUE;
}

VOID
GetMsVsLicensies(VOID)
{
    WCHAR szText[MAX_STR_LEN], szTemp[MAX_STR_LEN],
          szPath[MAX_PATH];
    INT i = 0;

    typedef struct
    {
        LPWSTR lpProductName;
        LPWSTR lpKeyPath;
    } MS_VS_LICENSIES_INFO;

    MS_VS_LICENSIES_INFO MsVsLicensies[] =
    {
        { L"Microsoft Visual Studio 2010",                 L"Microsoft\\VisualStudio\\10.0\\Registration" },
        { L"Microsoft Visual Studio 2008",                 L"Microsoft\\VisualStudio\\9.0\\Registration" },
        { L"Microsoft Visual Studio 2005",                 L"Microsoft\\VisualStudio\\8.0\\Registration" },
        { L"Microsoft Visual Studio for Applications 7.1", L"Microsoft\\VSA\\7.1\\Registration" },
        { L"Microsoft Visual Studio for Applications 8",   L"Microsoft\\VSA\\8.0\\Registration" },
        { 0 }
    };

    do
    {
        StringCbPrintf(szPath, sizeof(szPath),
                       L"SOFTWARE\\%s",
                       MsVsLicensies[i].lpKeyPath);

        if (GetStringFromRegistry(TRUE,
                                  HKEY_LOCAL_MACHINE,
                                  szPath,
                                  L"PIDKEY",
                                  szTemp,
                                  sizeof(szTemp)/sizeof(WCHAR)))
        {
            InsertKeySep(szTemp, szText);
            IoAddItem(0, 0, MsVsLicensies[i].lpProductName);
            IoSetItemText(szText);
        }
    }
    while (MsVsLicensies[++i].lpProductName != 0);

    if (!IsWin64System()) return;

    i = 0;
    do
    {
        StringCbPrintf(szPath, sizeof(szPath),
                       L"SOFTWARE\\Wow6432Node\\%s",
                       MsVsLicensies[i].lpKeyPath);

        if (GetStringFromRegistry(TRUE,
                                  HKEY_LOCAL_MACHINE,
                                  szPath,
                                  L"PIDKEY",
                                  szTemp,
                                  sizeof(szTemp)/sizeof(WCHAR)))
        {
            if (szTemp[0] == 0)
            {
                DWORD dwType, dwSize = MAX_PATH;
                WCHAR szKeyName[MAX_PATH];
                INT ItemIndex = 0;
                HKEY hKey, hSubKey;

                if (RegOpenKey(HKEY_LOCAL_MACHINE,
                               szPath,
                               &hKey) == ERROR_SUCCESS)
                {
                    while (RegEnumKeyEx(hKey, ItemIndex, szKeyName, &dwSize,
                           NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
                    {
                        if (RegOpenKey(hKey, szKeyName, &hSubKey) == ERROR_SUCCESS)
                        {
                            dwType = REG_SZ;
                            dwSize = MAX_STR_LEN;

                            if (RegQueryValueEx(hSubKey,
                                                L"PIDKEY",
                                                NULL,
                                                &dwType,
                                                (LPBYTE)szTemp,
                                                &dwSize) == ERROR_SUCCESS &&
                                szTemp[0] != 0)
                            {
                                RegCloseKey(hSubKey);
                                break;
                            }

                            RegCloseKey(hSubKey);
                        }

                        dwSize = MAX_PATH;
                        ++ItemIndex;
                    }

                    RegCloseKey(hKey);
                }
            }

            if (szTemp[0] != 0)
            {
                InsertKeySep(szTemp, szText);
                IoAddItem(0, 0, MsVsLicensies[i].lpProductName);
                IoSetItemText(szText);
            }
        }
    }
    while (MsVsLicensies[++i].lpProductName != 0);
}

VOID
SOFTWARE_LicensesInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    INT i = 0;

    DebugStartReceiving();

    IoAddIcon(IDI_CONTACT);

    do
    {
        szText[0] = 0;
        if (!GetMSProductKey(FALSE,
                             MsLicensies[i].lpKeyPath,
                             szText, MAX_STR_LEN))
        {
            GetMSProductKey(TRUE,
                            MsLicensies[i].lpKeyPath,
                            szText, MAX_STR_LEN);
        }
        if (szText[0] != 0)
        {
            IoAddItem(0, 0, MsLicensies[i].lpProductName);
            IoSetItemText(szText);
        }
    }
    while (MsLicensies[++i].lpProductName != 0);

    /* MS Visual Studio */
    GetMsVsLicensies();

    /* VMWare Workstation */
    GetVMWareWorkstationLicenses();

    /* VMWare Server */
    GetVMWareServerLicenses();

    /* Ahead Nero */
    GetAheadNeroLicensies();

    /* 3Planesoft Screensavers Keys */
    Get3PlanesoftKeys();

    GetOtherLicensiesInfo();

    DebugEndReceiving();
}
