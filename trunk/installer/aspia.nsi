;
; PROJECT:      Aspia
; FILE:         NSIS Installer Script
; LICENSE:      LGPL (GNU Lesser General Public License)
; PROGRAMMERS:  Dmitry Chapyshev
;

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Aspia"
  OutFile "aspia.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES32\Aspia"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Aspia" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Language Selection Dialog Settings

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
  !define MUI_LANGDLL_REGISTRY_KEY "Software\Aspia" 
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "..\aspia\license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English" ;first language is the default language
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Ukrainian"

;--------------------------------
;Reserve Files
  
  ;If you are using solid compression, files that are required before
  ;the actual installation should be stored first in the data block,
  ;because this will make your installer start faster.
  
  !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;Installer Sections

InstType "Full"
InstType "Minimal"


Section "!Main Program" SecMainProgram

  SetOutPath "$INSTDIR"
  
  SectionIn 1 2
  
  File /oname=aspia.exe ..\Win32\Release\aspia.exe
  File /oname=aspia_x32.sys ..\Win32\Release\aspia_x32.sys
  File /oname=aspia_x64.sys ..\Win32\Release\aspia_x64.sys
  
  ;Store installation folder
  WriteRegStr HKCU "Software\Aspia" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
 
  ;Create shortcuts
  CreateShortCut "$STARTMENU\Aspia.lnk" "$INSTDIR\aspia.exe"

SectionEnd

SectionGroup "Language Files"

Section /o "Russian"
  SetOutPath "$INSTDIR\languages"
  SectionIn 1
  File /oname=ru-RU.dll ..\Win32\Release\languages\ru-RU.dll
SectionEnd

Section /o "Ukrainian"
  SetOutPath "$INSTDIR\languages"
  SectionIn 1
  File /oname=uk-UA.dll ..\Win32\Release\languages\uk-UA.dll
SectionEnd

SectionGroupEnd

SectionGroup "Icons Themes"

Section "XP Icons"
  SetOutPath "$INSTDIR\icons"
  SectionIn 1
  File /oname=xp_icons.dll ..\Win32\Release\icons\xp_icons.dll
SectionEnd

Section "Vista Icons"
  SetOutPath "$INSTDIR\icons"
  SectionIn 1
  File /oname=vista_icons.dll ..\Win32\Release\icons\vista_icons.dll
SectionEnd

SectionGroupEnd

SectionGroup "Devices Database"

Section "PCI Devices"
  SetOutPath "$INSTDIR"
  SectionIn 1
  File /oname=pci_dev.ini ..\Win32\Release\pci_dev.ini
SectionEnd

Section "USB Devices"
  SetOutPath "$INSTDIR"
  SectionIn 1
  File /oname=usb_dev.ini ..\Win32\Release\usb_dev.ini
SectionEnd

SectionGroupEnd


;--------------------------------
;Installer Functions

Function .onInit
  IntOp $0 ${SF_SELECTED} | ${SF_RO}
  SectionSetFlags ${SecMainProgram} $0
  
  SetCurInstType 0
 
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

;--------------------------------
;Descriptions

  ;USE A LANGUAGE STRING IF YOU WANT YOUR DESCRIPTIONS TO BE LANGAUGE SPECIFIC

  ;Assign descriptions to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    ;!insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} "Main Program Files."
	;!insertmacro MUI_DESCRIPTION_TEXT ${SecFullInstallation} "Full"
	;!insertmacro MUI_DESCRIPTION_TEXT ${SecMinimalInstallation} "Minimal"
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$STARTMENU\Aspia.lnk"

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\aspia.exe"

  Delete "$INSTDIR\languages\ru-RU.dll"
  Delete "$INSTDIR\languages\uk-UA.dll"
  
  Delete "$INSTDIR\icons\xp_icons.dll"
  Delete "$INSTDIR\icons\vista_icons.dll"
  
  Delete "$INSTDIR\aspia_x32.sys"
  Delete "$INSTDIR\aspia_x64.sys"
  
  Delete "$INSTDIR\pci_dev.ini"
  Delete "$INSTDIR\usb_dev.ini"
  
  Delete "$INSTDIR\debug.log"
  
  Delete "$INSTDIR\aspia.ini"

  RMDir "$INSTDIR\languages"
  RMDir "$INSTDIR\icons"
  RMDir "$INSTDIR"

  DeleteRegKey HKCU "Software\Aspia"

SectionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd