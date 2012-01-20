/*
 * PROJECT:         Aspia
 * FILE:            aspia/info/licenses.c
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#include "../main.h"
#include "driver.h"


VOID
GetVMWareWorkstationLicenses(VOID)
{
    WCHAR szKeyName[MAX_PATH], szSerial[MAX_PATH];
    DWORD dwType, dwSize = MAX_PATH;
    INT Index, ItemIndex = 0;
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
                    Index = IoAddItem(0, 0, L"VMWare Workstation");
                    IoSetItemText(Index, 1, szSerial);
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
    INT Index, ItemIndex = 0;
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
                    Index = IoAddItem(0, 0, L"VMWare Server");
                    IoSetItemText(Index, 1, szSerial);
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
    WCHAR szKeyName[MAX_PATH], szValue[MAX_PATH],
          szProductName[MAX_PATH];
    DWORD dwSize = MAX_PATH, dwType;
    INT Index, KeyIndex = 0;
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
            StringCbPrintf(szProductName, sizeof(szProductName), L"%s (Name)", szKeyName);
            Index = IoAddItem(0, 0, szProductName);
            IoSetItemText(Index, 1, szValue);
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
            StringCbPrintf(szProductName, sizeof(szProductName), L"%s (Key)", szKeyName);
            Index = IoAddItem(0, 0, szProductName);
            IoSetItemText(Index, 1, szValue);
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
    INT Index, KeyIndex = 0, ValIndex;
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
                    Index = IoAddItem(0, 0, L"Ahead Nero");
                    IoSetItemText(Index, 1, szValue);
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
    { L"3D Mark 2001 (Name)",                  HKEY_LOCAL_MACHINE, L"SOFTWARE\\MadOnion.com\\Registration2001",                L"3DMarkRegName" },
    { L"3D Mark 2001 (Key)",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\MadOnion.com\\Registration2001",                L"3DMarkRegKey"  },
    { L"Adobe Acrobat 7",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Adobe Acrobat\\7.0\\Registration",       L"SERIAL"        },
    { L"Adobe Acrobat 8",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Adobe Acrobat\\8.0\\Registration",       L"SERIAL"        },
    { L"Adobe Photoshop 7",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Photoshop\\7.0\\Registration",           L"SERIAL"        },
    { L"Advanced Direct Remailer",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Tweak Marketing\\Advanced Direct Remailer\\Registration", L"code"},
    { L"Advanced Direct Remailer",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Direct Remailer\\Registration", L"code"          },
    { L"After Effects 7 (Name)",               HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\After Effects\\7.0\\Registration",       L"NAME"          },
    { L"After Effects 7 (Company)",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\After Effects\\7.0\\Registration",       L"COMPAN"        },
    { L"After Effects 7 (Key)",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\After Effects\\7.0\\Registration",       L"Serial"        },
    { L"Alcohol 120% (Name)",                  HKEY_CURRENT_USER,  L"Software\\Alcohol Soft\\Alcohol 120%\\Info",              L"UserName"      },
    { L"Alcohol 120% (Key)",                   HKEY_CURRENT_USER,  L"Software\\Alcohol Soft\\Alcohol 120%\\Info",              L"SerialNo"     },
    { L"Alcohol 120% (Key)",                   HKEY_CURRENT_USER,  L"Software\\Alcohol Soft\\Alcohol 120%\\Info",              L"ServerKey"     },
    { L"Axailis IconWorkshop 6.0",             HKEY_CURRENT_USER,  L"Software\\Axialis\\IconWorkshop\\registration",           L"ProductKey"    },
    { L"Beyond TV 4",                          HKEY_LOCAL_MACHINE, L"SOFTWARE\\SnapStream Media\\Beyond TV",                   L"ProductKey"    },
    { L"Beyond TV 4 Link",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\SnapStream Media\\Beyond TV",                   L"NetworkLicense"},
    { L"Beyond Media",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\SnapStream Media\\Beyond Media",                L"ProductKey"    },
    { L"Borland Delphi 6 (Serial)",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Borland\\Delphi\\6.0",                          L"LMLIC"         },
    { L"Borland Delphi 6 (Key)",               HKEY_LOCAL_MACHINE, L"SOFTWARE\\Borland\\Delphi\\6.0",                          L"LMKEY"         },
    { L"Borland Delphi 7 (Serial)",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Borland\\Delphi\\7.0",                          L"LMLIC"         },
    { L"Borland Delphi 7 (Key)",               HKEY_LOCAL_MACHINE, L"SOFTWARE\\Borland\\Delphi\\7.0",                          L"LMKEY"         },
    { L"Call of Duty 2",                       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Activision\\Call of Duty 2",                    L"codkey"        },
    { L"Company of Heroes (Key)",              HKEY_LOCAL_MACHINE, L"SOFTWARE\\THQ\\Company of Heroes",                        L"ProductKey"    },
    { L"Cyberlink PowerDVD (Key)",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Cyberlink\\PowerDVD",                           L"CDKey"         },
    { L"Dell Service Tag",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Dell Computer Corporation\\SysInfo",            L"SerialNumber"  },
    { L"DVD Profiler (First Name)",            HKEY_CURRENT_USER,  L"Software\\InterVocative Software\\DVD Profiler",          L"RegFName"      },
    { L"DVD Profiler (Last Name)",             HKEY_CURRENT_USER,  L"SoftWare\\InterVocative Software\\DVD Profiler",          L"RegLName"      },
    { L"DVD Profiler (Key)",                   HKEY_CURRENT_USER,  L"Software\\InterVocative Software\\DVD Profiler",          L"RegKey"        },
    { L"3DMark 2003",                          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Futuremark\\3DMark03",                          L"KeyCode"       },
    { L"3DMark 2005",                          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Futuremark\\3DMark05",                          L"KeyCode"       },
    { L"3DMark 2006",                          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Futuremark\\3DMark06",                          L"KeyCode"       },
    { L"PCMark 2005",                          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Futuremark\\PCMark05",                          L"KeyCode"       },
    { L"HDD State Inspector",                  HKEY_LOCAL_MACHINE, L"SOFTWARE\\AltrixSoft\\HDD State Inspector",               L"Key"           },
    { L"James Bond 007 Nightfire",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\James Bond 007 Nightfire\\ergc", L""  },
    { L"mIRC (User Name)",                     HKEY_CURRENT_USER,  L"Software\\mIRC\\UserName",                                L""              },
    { L"mIRC (Key)",                           HKEY_CURRENT_USER,  L"Software\\mIRC\\License",                                 L""              },
    { L"Naturally Speaking 8",                 HKEY_LOCAL_MACHINE, L"SOFTWARE\\ScanSoft\\NaturallySpeaking8\\Activation",      L"SerialNumber"  },
    { L"Nero Burning Rom 6",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Ahead\\Nero - Burning Rom\\Info",               L"serial6"       },
    { L"Nero 7 (Name)",                        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Ahead\\Installation\\Families\\Nero 7\\Info",   L"User" },
    { L"Nero 7 (Company)",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Ahead\\Installation\\Families\\Nero 7\\Info",   L"Company" },
    { L"Nero 7 (Key)",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Ahead\\Installation\\Families\\Nero 7\\Info",   L"Serial7_*" },
    { L"Nero 8 (Name)",                        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nero\\Installation\\Families\\Nero 8\\Info",    L"User" },
    { L"Nero 8 (Company)",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nero\\Installation\\Families\\Nero 8\\Info",    L"Company" },
    { L"Nero 8 (Key)",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nero\\Installation\\Families\\Nero 8\\Info",    L"Serial8_*" },
    { L"Nero BackItUp",                        HKEY_LOCAL_MACHINE, L"Software\\Ahead\\Nero BackItUp\\Preferences\\General",    L"Serial" },
    { L"NewsBin Pro 5 (First Name)",           HKEY_CURRENT_USER,  L"Software\\DJI Interprises\\Newsbin50\\RegNew",            L"FirstName"     },
    { L"NewsBin Pro 5 (Last Name)",            HKEY_CURRENT_USER,  L"Software\\DJI Interprises\\Newsbin50\\RegNew",            L"LastName"      },
    { L"NewsBin Pro 5 (Key)",                  HKEY_CURRENT_USER,  L"Software\\DJI Interprises\\Newsbin50\\RegNew",            L"Code1"         },
    { L"Norton PartitionMagic 8 (Name)",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Symantec\\Norton PartitionMagic\\8.0\\UserInfo",L"Name"          },
    { L"Norton PartitionMagic 8 (Company)",    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Symantec\\Norton PartitionMagic\\8.0\\UserInfo",L"Company"       },
    { L"Norton PartitionMagic 8 (Key)",        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Symantec\\Norton PartitionMagic\\8.0\\UserInfo",L"SerialNumber"  },
    { L"Symantec ACT! 6",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Symantec\\ACT!\\install", L"SerialNumber"                        },
    { L"Symantec Norton Internet Security 2007",  HKEY_LOCAL_MACHINE, L"SOFTWARE\\Symantec\\CCPD-LC\\KStore\\00000082\\00000049\\000000b9", L"Key" },
    { L"Symantec Norton SystemWorks 2008 11.0.1", HKEY_LOCAL_MACHINE, L"SOFTWARE\\Symantec\\CCPD-LC\\KStore\\00000082\\000000d2\\0000025f", L"Key" },
    { L"Norton Antivirus 2006",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Symantec\\CCPD-LC\\KStore\\00000082\\0000001e\\0000004a",    L"Key" },
    { L"O&O CleverCache 6 (Name)",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\O&O\\O&O CleverCache\\6.0",                     L"User"          },
    { L"O&O CleverCache 6 (Company)",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\O&O\\O&O CleverCache\\6.0",                     L"Company"       },
    { L"O&O CleverCache 6 (Key)",              HKEY_LOCAL_MACHINE, L"SOFTWARE\\O&O\\O&O CleverCache\\6.0",                     L"SerialNo"      },
    { L"O&O Defrag 8 (Name)",                  HKEY_LOCAL_MACHINE, L"SOFTWARE\\O&O\\O&O Defrag\\8.0\\Pro\\licenses",           L"User"          },
    { L"O&O Defrag 8 (Company)",               HKEY_LOCAL_MACHINE, L"SOFTWARE\\O&O\\O&O Defrag\\8.0\\Pro\\licenses",           L"Company"       },
    { L"O&O Defrag 8 (Key)",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\O&O\\O&O Defrag\\8.0\\Pro\\licenses",           L"SerialNo"      },
    { L"O&O DriveLED 2 (Name)",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\O&O\\O&O DriveLED\\2.0",                        L"User"          },
    { L"O&O DriveLED 2 (Company)",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\O&O\\O&O DriveLED\\2.0",                        L"Company"       },
    { L"O&O DriveLED 2 (Key)",                 HKEY_LOCAL_MACHINE, L"SOFTWARE\\O&O\\O&O DriveLED\\2.0",                        L"SerialNo"      },
    { L"Quake 4",                              HKEY_LOCAL_MACHINE, L"SOFTWARE\\id\\Quake 4",                                   L"CDKey"         },
    { L"PC Icon Editor (Name)",                HKEY_CURRENT_USER,  L"Software\\Program4Pc\\PC Icon Editor",                    L"Name"          },
    { L"PC Icon Editor (Key)",                 HKEY_CURRENT_USER,  L"Software\\Program4Pc\\PC Icon Editor",                    L"Key"           },
    { L"ReplayConverter",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\ReplayConverter",                               L"RegCode"       },
    { L"SmartVersion",                         HKEY_CURRENT_USER,  L"Software\\SmartVersion",                                  L"RegistrationCode" },
    { L"Splinter Cell - Chaos Theory",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Ubisoft\\Splinter Cell Chaos Theory\\Keys",     L"DiscKey_SCCT"  },
    { L"Stardock",                             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Stardock\\ComponentManager\\Stardock\\odp",     L"Serial No"     },
    { L"SuperCleaner (Name)",                  HKEY_CURRENT_USER,  L"Software\\SuperCleaner\\Registration",                    L"Name"          },
    { L"SuperCleaner (Key)",                   HKEY_CURRENT_USER,  L"Software\\SuperCleaner\\Registration",                    L"Code"          },
    { L"Tag&Rename (Name)",                    HKEY_CURRENT_USER,  L"Software\\Softpointer\\Tag&Rename\\Config",               L"Name"          },
    { L"Tag&Rename (Key)",                     HKEY_CURRENT_USER,  L"Software\\Softpointer\\Tag&Rename\\Config",               L"cbVQFFtoTagReplaseUnde" },
    { L"Techsmith Camtasia Studio 3.0 (Name)", HKEY_LOCAL_MACHINE, L"SOFTWARE\\TechSmith\\Camtasia Studio\\3.0",               L"RegisteredTo"    },
    { L"Techsmith Camtasia Studio 3.0 (Key)",  HKEY_LOCAL_MACHINE, L"SOFTWARE\\TechSmith\\Camtasia Studio\\3.0",               L"RegistrationKey" },
    { L"Techsmith Camtasia Studio 4.0",        HKEY_LOCAL_MACHINE, L"SOFTWARE\\TechSmith\\Camtasia Studio\\4.0",               L"RegistrationKey" },
    { L"Techsmith SnagIt 8.0 (Name)",          HKEY_CURRENT_USER,  L"Software\\TechSmith\\SnagIt\\8",                          L"RegisteredTo"    },
    { L"Techsmith SnagIt 8.0 (Key)",           HKEY_CURRENT_USER,  L"Software\\TechSmith\\SnagIt\\8",                          L"RegistrationKey" },
    { L"Techsmith SnagIt 8.1 (Name)",          HKEY_LOCAL_MACHINE, L"Software\\TechSmith\\SnagIt\\8",                          L"RegisteredTo"    },
    { L"Techsmith SnagIt 8.1 (Key)",           HKEY_LOCAL_MACHINE, L"Software\\TechSmith\\SnagIt\\8",                          L"RegistrationKey" },
    { L"TGTSoft StyleXP",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\TGT Soft\\StyleXP",                             L"RegKey"          },
    { L"Trend Micro PC-cillin Antivirus 11",   HKEY_LOCAL_MACHINE, L"SOFTWARE\\TrendMicro\\PC-cillin",                         L"register no."    },
    { L"Trend Micro PC-cillin Antivirus 2007", HKEY_LOCAL_MACHINE, L"SOFTWARE\\TrendMicro\\AntiVirus\\15",                     L"SerialNo"        },
    { L"TuneUP 2006 (Name)",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\5.0",                        L"UserName"        },
    { L"TuneUP 2006 (Company)",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\5.0",                        L"Company"         },
    { L"TuneUP 2006 (Key)",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\5.0",                        L"RegCode"         },
    { L"TuneUP 2007 (Name)",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\6.0",                        L"UserName"        },
    { L"TuneUP 2007 (Company)",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\6.0",                        L"Company"         },
    { L"TuneUP 2007 (Key)",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\6.0",                        L"RegCode"         },
    { L"TuneUp 2009 (Name)",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\8.0",                        L"UserName" },
    { L"TuneUp 2009 (Company)",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\8.0",                        L"Company" },
    { L"TuneUp 2009 (Key)",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\8.0",                        L"ProductKey" },
    { L"TuneUp 2010 (Name)",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\9.0",                        L"UserName" },
    { L"TuneUp 2010 (Company)",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\9.0",                        L"Company" },
    { L"TuneUp 2010 (Key)",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\TuneUp\\Utilities\\9.0",                        L"ProductKey" },
    { L"Unreal Tournament 2003",               HKEY_LOCAL_MACHINE, L"SOFTWARE\\Unreal Technology\\Installed Apps\\UT2003",     L"CDKey"           },
    { L"Unreal Tournament 2004",               HKEY_LOCAL_MACHINE, L"SOFTWARE\\Unreal Technology\\Installed Apps\\UT2004",     L"CDKey"           },
    { L"VSO ConvertX to DVD",                  HKEY_LOCAL_MACHINE, L"SOFTWARE\\VSO\\ConvertXtoDVD",                            L"LicenseKey"      },
    { L"VSO ConvertXtoDVD",                    HKEY_CURRENT_USER,  L"Software\\VSO\\ConvertXToDVD",                            L"LicenseKey"      },
    { L"Westwood Alarmstufe Rot 2",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Westwood\\Red Alert 2",                         L"Serial"          },
    { L"Westwood Alarmstufe Rot 2 Yuri's Revenge", HKEY_LOCAL_MACHINE, L"SOFTWARE\\Westwood\\Yuri's Revenge",                  L"Serial"          },
    { L"Westwood Tiberian Sun",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Westwood\\Tiberian Sun",                        L"Serial"          },
    { L"Winamp (Name)",                        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nullsoft\\Winamp",                              L"regname"         },
    { L"Winamp (Key)",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Nullsoft\\Winamp",                              L"regkey"          },
    { L"WinPatrol",                            HKEY_LOCAL_MACHINE, L"SOFTWARE\\BillP Studios\\WinPatrol",                      L"RegNumber"       },
    { L"WS FTP",                               HKEY_LOCAL_MACHINE, L"SOFTWARE\\Ipswitch\\WS_FTP",                              L"SerialNumber"    },
    { L"AutoCAD 2000, 2002",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R15.0\\ACAD-1:409",          L"SerialNumber" },
    { L"AutoCAD 2004",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R16.0\\ACAD-201:409",        L"SerialNumber" },
    { L"AutoCAD 2005",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R16.1\\ACAD-301:409",        L"SerialNumber" },
    { L"AutoCAD 2006",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R16.2\\ACAD-4001:409",       L"SerialNumber" },
    { L"AutoCAD 2007",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.0\\ACAD-5001:409",       L"SerialNumber" },
    { L"AutoCAD 2008",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.1\\ACAD-6001:409",       L"SerialNumber" },
    { L"AutoCAD 2009",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.2\\ACAD-7001:409",       L"SerialNumber" },
    { L"AutoCAD 2010",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R18.0\\ACAD-8001:409",       L"SerialNumber" },
    { L"AutoCAD Architecture 2008",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.1\\ACAD-6004:419",       L"SerialNumber" },
    { L"AutoCAD Architecture 2009",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.2\\ACAD-7004:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2006",        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R16.2\\ACAD-4006:409",       L"SerialNumber" },
    { L"AutoCAD Building Systems 2007",        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.0\\ACAD-5006:409",       L"SerialNumber" },
    { L"AutoCAD Electrical 2005",              HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R16.1\\ACAD-307:409",        L"SerialNumber" },
    { L"AutoCAD Electrical 2006",              HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R16.2\\ACAD-4007:409",       L"SerialNumber" },
    { L"AutoCAD Electrical 2007",              HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.0\\ACAD-5007:409",       L"SerialNumber" },
    { L"AutoCAD LT 2000",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R2000\\ACLT-1:409",       L"SerialNumber" },
    { L"AutoCAD LT 2002i",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R7\\ACLT-1:409",          L"SerialNumber" },
    { L"AutoCAD LT 2002",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R8\\ACLT-1:409",          L"SerialNumber" },
    { L"AutoCAD LT 2004",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R9\\ACLT-201:409",        L"SerialNumber" },
    { L"AutoCAD LT 2005",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R10\\ACLT-301:409",       L"SerialNumber" },
    { L"AutoCAD LT 2006",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R11\\ACLT-4001:409",      L"SerialNumber" },
    { L"AutoCAD LT 2007",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R12\\ACADLT-5001:409",    L"SerialNumber" },
    { L"AutoCAD LT 2008",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R13\\ACADLT-6001:409",    L"SerialNumber" },
    { L"AutoCAD LT 2009",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R14\\ACADLT-7001:409",    L"SerialNumber" },
    { L"AutoCAD LT 2010",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R15\\ACADLT-8001:409",    L"SerialNumber" },
    { L"AutoCAD LT 98",                        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD LT\\R5.0\\ACLT-2452551:43869540", L"SerialNumber" },
    { L"AutoCAD MEP 2008",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.1\\ACAD-6006:409",       L"SerialNumber" },
    { L"AutoCAD MEP 2009",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.2\\ACAD-7006:409",       L"SerialNumber" },
    { L"AutoCAD Mechanical 2000i",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R15.0\\ACAD-7:409",          L"SerialNumber" },
    { L"Autodesk 3ds Max 8",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\3dsmax\\8.0",                         L"SerialNumber" },
    { L"Autodesk Inventor",                    HKEY_LOCAL_MACHINE, L"Inventor\\System\\License",                               L"SerialNumber" },
    { L"Autodesk Raster Design 2007",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\AutoCAD\\R17.0\\ACAD-5xxx:409\\AddOns\\RasterDesign", L"SerialNumber" },
    { L"Autodesk Volo View 2002-627",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Autodesk\\Volo View\\2002-627",                 L"SerialNumber" },
    { L"Adobe Acrobat 5.0",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Adobe Acrobat 5.0", L"ProductID" },
    { L"Adobe Acrobat 5.x",                    HKEY_CURRENT_USER,  L"Software\\Adobe\\Acrobat\\5.0\\Registration",             L"SERIAL" },
    { L"Adobe Acrobat 6.0",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Adobe Acrobat\\6.0\\Registration",       L"SERIAL" },
    { L"Adobe Acrobat 7.x",                    HKEY_CURRENT_USER,  L"Software\\Adobe\\Acrobat\\7.0\\Registration",             L"SERIAL" },
    { L"Adobe Acrobat 9.x",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Adobe Acrobat\\9.0\\Registration",       L"SERIAL" },
    { L"Adobe Acrobat Distiller 6.0",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Acrobat Distiller\\6.0\\Registration",   L"SERIAL" },
    { L"Adobe Acrobat Distiller 7.0",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Acrobat Distiller\\7.0\\Registration",   L"SERIAL" },
    { L"Adobe Acrobat Distiller 8.0",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Acrobat Distiller\\8.0\\Registration",   L"SERIAL" },
    { L"Adobe Creative Suite 2",               HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{0134A1A1-C283-4A47-91A1-92F19F960372}", L"EPIC_SERIAL" },
    { L"Adobe Illustrator CS2 (Name)",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{B2F5D08C-7E79-4FCD-AAF4-57AD35FF0601}", L"EPIC_NAME" },
    { L"Adobe Illustrator CS2 (Key)",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{B2F5D08C-7E79-4FCD-AAF4-57AD35FF0601}", L"EPIC_SERIAL" },
    { L"Adobe InDesign CS2 (Name)",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F4C8163-F259-49A0-A018-2857A90578BC}", L"EPIC_NAME" },
    { L"Adobe InDesign CS2 (Key)",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{7F4C8163-F259-49A0-A018-2857A90578BC}", L"EPIC_SERIAL" },
    { L"Adobe Lightroom 1.0",                  HKEY_LOCAL_MACHINE, L"SOFTWARE\\Adobe\\Lightroom\\1.0\\Registration",           L"serial_number" },
    { L"Adobe Photoshop CS2 (Name)",           HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{236BB7C4-4419-42FD-0409-1E257A25E34D}", L"EPIC_NAME" },
    { L"Adobe Photoshop CS2 (Key)",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{236BB7C4-4419-42FD-0409-1E257A25E34D}", L"EPIC_SERIAL" },
    { L"Adobe Photoshop CS (Name)",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{EFB21DE7-8C19-4A88-BB28-A766E16493BC}", L"RegOwner" },
    { L"Adobe Photoshop CS (Key)",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{EFB21DE7-8C19-4A88-BB28-A766E16493BC}", L"Serial" },
    { L"Adobe Premiere Pro (Name)",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{A14F7508-B784-40B8-B11A-E0E2EEB7229F}", L"NAME" },
    { L"Adobe Premiere Pro (Key)",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{A14F7508-B784-40B8-B11A-E0E2EEB7229F}", L"SERIAL" },
    { L"The Sims 2 Apartment Life",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Apartment Life\\ergc", L"" },
    { L"The Sims 2 Bon Voyage",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Bon Voyage\\ergc", L"" },
    { L"The Sims 2 Celebration Stuff",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Celebration Stuff\\ergc", L"" },
    { L"The Sims 2 Deluxe",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Deluxe\\ergc", L"" },
    { L"The Sims 2 Double Deluxe",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Double Deluxe\\ergc", L"" },
    { L"The Sims 2 Family Fun Stuff",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Family Fun Stuff\\ergc", L"" },
    { L"The Sims 2 Free Time",                 HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 FreeTime\\ergc", L"" },
    { L"The Sims 2 Glamour Life Stuff",        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Glamour Life Stuff\\ergc", L"" },
    { L"The Sims 2 H M Fashion Stuff",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 H M Fashion Stuff\\ergc", L"" },
    { L"The Sims 2 IKEA Home Stuff",           HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 IKEA Home Stuff\\ergc", L"" },
    { L"The Sims 2 Kitchen & Bath Interior Design Stuff", HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Kitchen & Bath Interior Design Stuff\\ergc", L"" },
    { L"The Sims 2 Nightlife",                 HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Nightlife\\ergc", L"" },
    { L"The Sims 2 Open for Business",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Open for Business\\ergc", L"" },
    { L"The Sims 2 Pets",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Pets\\ergc", L"" },
    { L"The Sims 2 Seasons",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Seasons\\ergc", L"" },
    { L"The Sims 2 Teen Style Stuff",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 Teen Style Stuff\\ergc", L"" },
    { L"The Sims 2 University",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2 University\\ergc", L"" },
    { L"The Sims 2",                           HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Sims 2\\ergc", L"" },
    { L"The Sims Castaway Stories",            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\The Sims Castaway Stories\\ergc", L"" },
    { L"The Sims Deluxe",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\The Sims Deluxe\\ergc", L"" },
    { L"The Sims Hot Date",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\The Sims Hot Date\\ergc", L"" },
    { L"The Sims House Party",                 HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\The Sims House Party\\ergc", L"" },
    { L"The Sims Life Stories",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\The Sims Life Stories\\ergc", L"" },
    { L"The Sims Livin’ Large",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\The Sims Livin' Large\\ergc", L"" },
    { L"The Sims Makin’ Magic",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\The Sims Makin' Magic\\ergc", L"" },
    { L"The Sims Pet Stories",                 HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\The Sims Pet Stories\\ergc", L"" },
    { L"The Sims Superstar",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\The Sims Superstar\\ergc", L"" },
    { L"The Sims Unleashed",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\The Sims Unleashed\\ergc", L"" },
    { L"The Sims Vacation",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\The Sims Vacation\\ergc", L"" },
    { L"The Sims",                             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\The Sims\\ergc", L"" },
    { L"FIFA 2002",                            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Sports\\FIFA 2002\\ergc", L"" },
    { L"FIFA 2003",                            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Sports\\FIFA 2003\\ergc", L"" },
    { L"FIFA 2007",                            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Sports\\FIFA 07\\ergc", L"" },
    { L"Freedom Force",                        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Distribution\\Freedom Force\\ergc", L"" },
    { L"Global Operations",                    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Global Operations\\ergc", L"" },
    { L"Medal of Honor Airborne",              HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Electronic Arts\\Medal of Honor Airborne\\ergc", L"" },
    { L"Medal of Honor: Allied Assault: Breakthrough", HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Medal of Honor Allied Assault Breakthrough\\ergc", L"" },
    { L"Medal of Honor: Allied Assault: Spearhead",    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Medal of Honor Allied Assault Spearhead\\ergc", L"" },
    { L"Medal of Honor: Allied Assault",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Medal of Honor Allied Assault\\egrc", L"" },
    { L"Medal of Honor: Pacific Assault",      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\Medal of Honor Pacific Assault\\ergc", L"" },
    { L"Medal of Honor",                       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Electronic Arts\\Medal of Honor\\ergc", L"" },
    { L"NHL 2002",                             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Sports\\NHL 2002\\ergc", L"" },
    { L"NHL 2003",                             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Sports\\NHL 2003\\ergc", L"" },
    { L"Nascar Racing 2002",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Sports\\Nascar Racing 2002\\ergc", L"" },
    { L"Nascar Racing 2003",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Sports\\Nascar Racing 2003\\ergc", L"" },
    { L"Need For Speed Hot Pursuit",           HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Need For Speed Hot Pursuit\\egrc", L"" },
    { L"Need for Speed Carbon",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Electronic Arts\\Need for Speed Carbon\\ergc", L"" },
    { L"Need for Speed Most Wanted",           HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\Need for Speed Most Wanted\\ergc", L"" },
    { L"Need for Speed Underground",           HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Electronic Arts\\Need for Speed Underground\\ergc", L"" },
    { L"SPORE: Creepy and Cute Parts Pack",    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\SPORE Creepy and Cute Parts Pack\\ergc", L"" },
    { L"SPORE",                                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\SPORE(TM)\\ergc", L"" },
    { L"Shogun: Total War: Warlord Edition",   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Shogun Total War - WarlordEdition\\ergc", L"" },
    { L"SimCity 4 Deluxe",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\SimCity 4 Deluxe\\ergc", L"" },
    { L"SimCity 4 Rush Hour",                  HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\SimCity 4 Rush Hour\\ergc", L"" },
    { L"SimCity 4",                            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\SimCity 4\\ergc", L"" },
    { L"Battle for Middle Earth",              HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\The Battle for Middle-earth\\ergc", L"" },
    { L"Battlefield 1942: Road To Rome",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Battlefield 1942 The Road to Rome\\egrc", L"" },
    { L"Battlefield 1942: Secret Weapons of WWII", HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Battlefield 1942 Secret Weapons of WWII\\ergc", L"" },
    { L"Battlefield 1942",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Battlefield 1942\\ergc", L"" },
    { L"Battlefield 2 Demo",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Battlefield 2 Demo\\ergc", L"" },
    { L"Battlefield 2 Special Forces",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\Battlefield 2 Special Forces\\ergc", L"" },
    { L"Battlefield 2142",                     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Battlefield 2142\\ergc", L"" },
    { L"Battlefield 2",                        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\Battlefield 2\\ergc", L"" },
    { L"Battlefield Vietnam",                  HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Battlefield Vietnam\\ergc", L"" },
    { L"Black and White",                      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA GAMES\\Black and White\\ergc", L"" },
    { L"Command & Conquer 3: Kane’s Wrath",    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Electronic Arts\\Command and Conquer 3 Kanes Wrath\\ergc", L"" },
    { L"Command & Conquer 3: Tiberium Wars",   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Electronic Arts\\Command and Conquer 3\\ergc", L"" },
    { L"Command & Conquer: Generals Zero Hour",HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour\\ergc", L"" },
    { L"Command & Conquer: Generals",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\EA Games\\Generals\\ergc", L"" },
    { L"Command & Conquer: Red Alert 3",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Electronic Arts\\RedAlert 3\\ergc", L"" },
    { L"Crysis",                               HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Electronic Arts\\Crysis\\ergc", L"" },
    { L"SimCity 3000 Unlimited",               HKEY_LOCAL_MACHINE, L"SOFTWARE\\Electronic Arts\\Maxis\\SimCity 3000 Unlimited", L"Serial Number" },
    { L"Command & Conquer: Red Alert 2 Yuri's Revenge", HKEY_LOCAL_MACHINE, L"SOFTWARE\\Westwood\\Yuri's Revenge", L"Serial" },
    { L"Command & Conquer: Red Alert 2",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Westwood\\Red Alert 2", L"Serial" },
    { L"Command & Conquer: Red Alert",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Westwood\\Red Alert", L"Serial" },
    { L"Command & Conquer: Tiberian Sun",      HKEY_LOCAL_MACHINE, L"SOFTWARE\\Westwood\\Tiberian Sun", L"Serial" },
    { L"Counter-Strike",                       HKEY_CURRENT_USER,  L"Software\\Valve\\CounterStrike\\Settings", L"CDKey" },
    { L"Counter-Strike",                       HKEY_CURRENT_USER,  L"Software\\Valve\\CounterStrike\\Settings", L"Key" },
    { L"Gunman Chronicles",                    HKEY_CURRENT_USER,  L"Software\\Valve\\Gunman\\Settings", L"Key" },
    { L"Half-Life",                            HKEY_CURRENT_USER,  L"Software\\Valve\\Half-Life\\Settings", L"Key" },
    { L"Call of Duty 2",                       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Activision\\Call of Duty 2", L"codkey" },
    { L"Call of Duty 4",                       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Activision\\Call of Duty 4", L"codkey" },
    { L"Call of Duty",                         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Activision\\Call of Duty", L"codkey" },
    { L"Call of Duty: United Offensive",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Activision\\Call of Duty United Offensive", L"key" },
    { L"Call of Duty: World at War",           HKEY_LOCAL_MACHINE, L"SOFTWARE\\Activision\\Call of Duty WAW", L"codkey" },
    { L"Corel Paint Shop Pro photo X2",        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Corel\\Paint Shop Pro\\12\\Installer", L"SerialNumber" },
    { L"Corel VideoStudio 12 Pro",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Ulead Systems\\Corel VideoStudio\\12.0\\Installer", L"Serial Number" },
    { L"CorelDRAW Graphics Suite 12",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Corel\\CorelDRAW\\12.0", L"Serial" },
    { L"CorelDRAW Graphics Suite X3",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Corel\\CorelDRAW\\13.0", L"Serial" },
    { L"CorelDRAW Graphics Suite X4",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\Corel\\Setup\\CorelDRAW Graphics Suite 14", L"SerialNumber" },
    { L"Cyberlink CDS",                        HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\CDS", L"CDKey" },
    { L"Cyberlink LabelPrint (Name)",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\LabelPrint\\2.0", L"UserName" },
    { L"Cyberlink LabelPrint (Company)",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\LabelPrint\\2.0", L"Company" },
    { L"Cyberlink LabelPrint (Key)",           HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\LabelPrint\\2.0", L"CDKey" },
    { L"Cyberlink LabelPrint (Serial)",        HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\LabelPrint\\2.0\\BuildInfo", L"SR_No" },
    { L"Cyberlink PCM4Everio",                 HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PCM4Everio\\UserReg", L"SR_No" },
    { L"Cyberlink Power2Go",                   HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\Power2Go\\5.0\\UserReg", L"SR_No" },
    { L"Cyberlink PowerDVD DX MyMovie",        HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDVD DX\\MyMovie", L"CDkey" },
    { L"Cyberlink PowerDVD DX (User)",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDVD DX", L"UserName" },
    { L"Cyberlink PowerDVD DX (Company)",      HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDVD DX", L"Company" },
    { L"Cyberlink PowerDVD DX (Key)",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDVD DX", L"CDKey" },
    { L"Cyberlink PowerDVD DX (Serial)",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDVD DX\\UserReg", L"SR_NO" },
    { L"Cyberlink PowerDVD (Key)",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Cyberlink\\PowerDVD", L"CDKey" },
    { L"Cyberlink PowerDVD (Serial)",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDVD\\UserReg", L"SR_No" },
    { L"Cyberlink PowerDVD 10 (Name)",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Cyberlink\\PowerDVD10", L"UserName" },
    { L"Cyberlink PowerDVD 10 (Key)",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDVD10", L"CDKey" },
    { L"Cyberlink PowerDirector 6.5 Uniplayer",HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDirector\\6.5\\UniPlayer", L"CDKey" },
    { L"Cyberlink PowerDirector Express 5.0",  HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDirector Express\\5.0\\UserReg", L"SR_No" },
    { L"Cyberlink PowerDirector 6.5",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerDirector\\6.5\\UserReg", L"SR_No" },
    { L"Cyberlink PowerProducer 3.0",          HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerProducer\\3.0\\UserReg", L"SR_No" },
    { L"Cyberlink PowerStarter",               HKEY_LOCAL_MACHINE, L"SOFTWARE\\CyberLink\\PowerStarter\\UserReg", L"SR_No" },
    { L"Advanced ACT! Password Recovery",      HKEY_CURRENT_USER,  L"Software\\Elcom\\Advanced ACT! Password Recovery\\Registration", L"Code" },
    { L"Advanced ART Password Recovery",       HKEY_CURRENT_USER,  L"Software\\Elcom\\ART\\Registration", L"Code" },
    { L"Advanced Access Password Recovery",    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Access Password Recovery\\Registration", L"Code" },
    { L"Advanced Archive Password Recovery",   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Archive Password Recovery\\Registration", L"Code" },
    { L"Advanced Disk Catalog",                HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Disk Catalog\\Registration", L"Code" },
    { L"Advanced EFS Data Recovery",           HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced EFS Data Recovery\\Registration", L"Code" },
    { L"Advanced IE Password Recovery",        HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced IE Password Recovery\\Registration", L"Code" },
    { L"Advanced IM Password Recovery",        HKEY_CURRENT_USER,  L"Software\\Elcom\\Advanced IM Password Recovery\\Registration", L"Code" },
    { L"Advanced Intuit Password Recovery",    HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Intuit Password Recovery\\Registration", L"Code" },
    { L"Advanced Lotus Password Recovery",     HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Lotus Password Recovery\\Registration", L"Code" },
    { L"Advanced Mailbox Password Recovery",   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Mailbox Password Recovery\\Registration", L"Code" },
    { L"Advanced Office Key Recovery",         HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Office Key Recovery\\Registration", L"Code" },
    { L"Advanced Office Password Recovery",    HKEY_LOCAL_MACHINE, L"SOFTWARE\\ElcomSoft\\Advanced Office Password Recovery\\Registration", L"Code" },
    { L"Advanced Outlook Express Password Recovery", HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced OE Password Recovery\\Registration", L"Code" },
    { L"Advanced Outlook Password Recovery",   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Outlook Password Recovery\\Registration", L"Code" },
    { L"Advanced PDF Password Recovery",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced PDF Password Recovery\\Registration", L"Code" },
    { L"Advanced Proactive Windows Security Explorer", HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Proactive Windows Security Explorer\\Registration", L"Code" },
    { L"Advanced RAR Password Recovery",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced RAR Password Recovery\\Registration", L"Code" },
    { L"Advanced VBA Password Recovery",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\ElcomSoft\\Advanced VBA Password Recovery\\Registration", L"Code" },
    { L"Advanced Windows Password Recovery",   HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced Windows Password Recovery\\Registration", L"Code" },
    { L"Advanced Wireless Security Auditor",   HKEY_CURRENT_USER,  L"Software\\ElcomSoft\\Elcomsoft Wireless Security Auditor\\Registration", L"Code" },
    { L"Advanced ZIP Password Recovery",       HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced ZIP Password Recovery\\Registration", L"Code" },
    { L"Advanced eBook Inscriber",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced eBook Inscriber\\Registration", L"Code" },
    { L"Advanced eBook Processor",             HKEY_LOCAL_MACHINE, L"SOFTWARE\\Elcom\\Advanced eBook Processor\\Registration", L"Code" },
    { L"The Bat!",                             HKEY_CURRENT_USER,  L"Software\\RIT\\The Bat!", L"RegistrationBlock" },
    { L"S.T.A.L.K.E.R. - Shadow of Chernobyl (Name)", HKEY_LOCAL_MACHINE, L"SOFTWARE\\GSC Game World\\STALKER-SHOC", L"InstallUserName" },
    { L"S.T.A.L.K.E.R. - Shadow of Chernobyl (Key)",  HKEY_LOCAL_MACHINE, L"SOFTWARE\\GSC Game World\\STALKER-SHOC", L"InstallCDKEY" },

    { 0 }
};

VOID
GetOtherLicensiesInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN];
    INT Index, i = 0;

    do
    {
        szText[0] = 0;

        if (GetStringFromRegistry(TRUE,
                                  LicensiesInfo[i].hRootKey,
                                  LicensiesInfo[i].lpKeyPath,
                                  LicensiesInfo[i].lpKeyName,
                                  szText,
                                  MAX_STR_LEN))
        {
            if (szText[0] != 0)
            {
                Index = IoAddItem(0, 0, LicensiesInfo[i].lpProductName);
                IoSetItemText(Index, 1, szText);
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

    for (j = 0; i < (SIZE_T)SafeStrLen(szIn); j++)
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
}

VOID
SOFTWARE_LicensesInfo(VOID)
{
    WCHAR szText[MAX_STR_LEN], szTemp[MAX_STR_LEN];
    INT Index, i = 0;

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
            Index = IoAddItem(0, 0, MsLicensies[i].lpProductName);
            IoSetItemText(Index, 1, szText);
        }
    }
    while (MsLicensies[++i].lpProductName != 0);

    /* Visual Studio 2010 Key */
    if (GetStringFromRegistry(TRUE,
                              HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\VisualStudio\\10.0\\Registration",
                              L"PIDKEY",
                              szTemp,
                              sizeof(szTemp)/sizeof(WCHAR)))
    {
        InsertKeySep(szTemp, szText);
        Index = IoAddItem(0, 0, L"Microsoft Visual Studio 2010");
        IoSetItemText(Index, 1, szText);
    }

    /* Visual Studio 2008 Key */
    if (GetStringFromRegistry(TRUE,
                              HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\VisualStudio\\9.0\\Registration",
                              L"PIDKEY",
                              szTemp,
                              sizeof(szTemp)/sizeof(WCHAR)))
    {
        InsertKeySep(szTemp, szText);
        Index = IoAddItem(0, 0, L"Microsoft Visual Studio 2008");
        IoSetItemText(Index, 1, szText);
    }

    /* Visual Studio 2005 Key */
    if (GetStringFromRegistry(TRUE,
                              HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Registration",
                              L"PIDKEY",
                              szTemp,
                              sizeof(szTemp)/sizeof(WCHAR)))
    {
        InsertKeySep(szTemp, szText);
        Index = IoAddItem(0, 0, L"Microsoft Visual Studio 2005");
        IoSetItemText(Index, 1, szText);
    }

    /* Visual Studio for Applications 7.1 Key */
    if (GetStringFromRegistry(TRUE,
                              HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\VSA\\7.1\\Registration",
                              L"PIDKEY",
                              szTemp,
                              sizeof(szTemp)/sizeof(WCHAR)))
    {
        InsertKeySep(szTemp, szText);
        Index = IoAddItem(0, 0, L"Microsoft Visual Studio for Applications 7.1");
        IoSetItemText(Index, 1, szText);
    }

    /* Visual Studio for Applications 8 Key */
    if (GetStringFromRegistry(TRUE,
                              HKEY_LOCAL_MACHINE,
                              L"SOFTWARE\\Microsoft\\VSA\\8.0\\Registration",
                              L"PIDKEY",
                              szTemp,
                              sizeof(szTemp)/sizeof(WCHAR)))
    {
        InsertKeySep(szTemp, szText);
        Index = IoAddItem(0, 0, L"Microsoft Visual Studio for Applications 8");
        IoSetItemText(Index, 1, szText);
    }

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
