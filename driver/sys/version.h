/*
 * PROJECT:         Aspia
 * FILE:            version.h
 * LICENSE:         LGPL (GNU Lesser General Public License)
 * PROGRAMMERS:     Shevchuk Maksim (maksim.shevchuk@gmail.com)
 */

#pragma once

#include <wchar.h>

#define VER_FILEVERSION             1,1,0,0
#define VER_FILEVERSION_STR         L"1.1.0\0"

#define VER_PRODUCTVERSION          VER_FILEVERSION
#define VER_PRODUCTVERSION_STR      VER_FILEVERSION_STR

#define VER_COMPANYNAME_STR "Aspia Software\0"
#define VER_FILEDESCRIPTION_STR "Aspia Information Tool Kernel Driver\0"
#define VER_LEGALCOPYRIGHT_STR "Aspia Software\0"
#define VER_INTERNALNAME_STR "Aspia\0"
#define VER_LEGALTRADEMARKS1_STR "\0"
#define VER_LEGALTRADEMARKS2_STR "\0"

#define VER_PRODUCTNAME_STR "Aspia Information Suite\0"

#ifdef _AMD64_ 
    #define VER_ORIGINALFILENAME_STR "aspia_x64.sys\0"
#else
    #define VER_ORIGINALFILENAME_STR "aspia_x32.sys\0"
#endif
