/*
 * PROJECT:         Aspia
 * FILE:            include/version.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
 */

#pragma once

#include <wchar.h>

#define VER_FILEVERSION             0,3,0,0
#define VER_FILEVERSION_STR         L"0.3.0\0"

#define VER_PRODUCTVERSION          VER_FILEVERSION
#define VER_PRODUCTVERSION_STR      VER_FILEVERSION_STR

#define VER_COMPANYNAME_STR         "Aspia Software\0"
#define VER_LEGALCOPYRIGHT_STR      "Aspia Software\0"
#define VER_INTERNALNAME_STR        "Aspia\0"
#define VER_LEGALTRADEMARKS1_STR    "\0"
#define VER_LEGALTRADEMARKS2_STR    "\0"
#define VER_PRODUCTNAME_STR         "Aspia\0"

/* aspia.exe */
#define VER_FILEDESCRIPTION_STR_ASPIA_EXE  "Aspia\0"
#define VER_ORIGINALFILENAME_STR_ASPIA_EXE "aspia.exe\0"

/* diskbench.exe */
#define VER_FILEDESCRIPTION_STR_DISKBENCH_EXE  "Aspia Disc Benchmark\0"
#define VER_ORIGINALFILENAME_STR_DISKBENCH_EXE "diskbench.exe\0"

/* aspia.dll */
#define VER_FILEDESCRIPTION_STR_ASPIA_DLL  "Aspia Info Helper DLL\0"
#define VER_ORIGINALFILENAME_STR_ASPIA_DLL "aspia.dll\0"

/* helper.dll */
#define VER_FILEDESCRIPTION_STR_HELPER_DLL  "Aspia Helper DLL\0"
#define VER_ORIGINALFILENAME_STR_HELPER_DLL "helper.dll\0"

/* aspia_x86.sys */
#define VER_FILEDESCRIPTION_STR_ASPIA_X86_SYS  "Aspia x86 Driver\0"
#define VER_ORIGINALFILENAME_STR_ASPIA_X86_SYS "aspia_x86.sys\0"

/* aspia_x64.sys */
#define VER_FILEDESCRIPTION_STR_ASPIA_X64_SYS  "Aspia x64 Driver\0"
#define VER_ORIGINALFILENAME_STR_ASPIA_X64_SYS "aspia_x64.sys\0"

/* For aspia_x86.sys and aspia_x64.sys */
#ifdef _M_IX86
    #define VER_FILEDESCRIPTION_STR_ASPIA_SYS VER_FILEDESCRIPTION_STR_ASPIA_X86_SYS
    #define VER_ORIGINALFILENAME_STR_ASPIA_SYS VER_ORIGINALFILENAME_STR_ASPIA_X86_SYS
#else
    #define VER_FILEDESCRIPTION_STR_ASPIA_SYS VER_FILEDESCRIPTION_STR_ASPIA_X64_SYS
    #define VER_ORIGINALFILENAME_STR_ASPIA_SYS VER_ORIGINALFILENAME_STR_ASPIA_X64_SYS
#endif
