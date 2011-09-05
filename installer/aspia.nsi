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
;Interface Settings

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "aspia.bmp" ; optional
  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "..\aspia\license-en.txt"
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
;General

  ;Name and file
  Name "Aspia"
  OutFile "aspia.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES32\Aspia"

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\Aspia" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin


;--------------------------------
;Language Selection Dialog Settings

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKLM" 
  !define MUI_LANGDLL_REGISTRY_KEY "Software\Aspia" 
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------
;Reserve Files

  ;If you are using solid compression, files that are required before
  ;the actual installation should be stored first in the data block,
  ;because this will make your installer start faster.

  !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;Installer Sections

  LangString DESC_FULL_INSTALL ${LANG_ENGLISH} "Full"
  LangString DESC_FULL_INSTALL ${LANG_RUSSIAN} "Полная установка"
  LangString DESC_FULL_INSTALL ${LANG_UKRAINIAN} "Full"

  LangString DESC_MIN_INSTALL ${LANG_ENGLISH} "Minimal"
  LangString DESC_MIN_INSTALL ${LANG_RUSSIAN} "Минимальная установка"
  LangString DESC_MIN_INSTALL ${LANG_UKRAINIAN} "Minimal"

  InstType "$(DESC_FULL_INSTALL)"
  InstType "$(DESC_MIN_INSTALL)"

  LangString DESC_MAIN_PROG ${LANG_ENGLISH} "Main Program"
  LangString DESC_MAIN_PROG ${LANG_RUSSIAN} "Основная программа"
  LangString DESC_MAIN_PROG ${LANG_UKRAINIAN} "Main Program"
  
  LangString DESC_CURRENT_LANG ${LANG_ENGLISH} ""
  LangString DESC_CURRENT_LANG ${LANG_RUSSIAN} "ru-RU.dll"
  LangString DESC_CURRENT_LANG ${LANG_UKRAINIAN} "uk-UA.dll"

Section "!$(DESC_MAIN_PROG)" SecMainProgram

  SetOutPath "$INSTDIR"

  SectionIn 1 2

  File /oname=aspia.exe ..\Release\Win32\aspia.exe
  File /oname=aspia_x32.sys ..\Release\Win32\aspia_x32.sys
  File /oname=aspia_x64.sys ..\Release\Win32\aspia_x64.sys
  File /oname=license-en.txt ..\Release\Win32\license-en.txt

  ;Store installation folder
  WriteRegStr HKLM "Software\Aspia" "" $INSTDIR

  WriteINIStr "$PROGRAMFILES32\Aspia\aspia.ini" "general" "language" "$(DESC_CURRENT_LANG)"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Create shortcuts
  CreateShortCut "$STARTMENU\Aspia.lnk" "$INSTDIR\aspia.exe"

SectionEnd

  LangString DESC_LANG_FILES ${LANG_ENGLISH} "Language Files"
  LangString DESC_LANG_FILES ${LANG_RUSSIAN} "Языковые файлы"
  LangString DESC_LANG_FILES ${LANG_UKRAINIAN} "Language Files"

SectionGroup "$(DESC_LANG_FILES)"

Section /o "Russian" SecRuRuFile
  SetOutPath "$INSTDIR\languages"
  SectionIn 1
  File /oname=ru-RU.dll ..\Release\Win32\languages\ru-RU.dll
SectionEnd

Section /o "Ukrainian" SecUkUaFile
  SetOutPath "$INSTDIR\languages"
  SectionIn 1
  File /oname=uk-UA.dll ..\Release\Win32\languages\uk-UA.dll
SectionEnd

SectionGroupEnd

  LangString DESC_ICONS_THEM ${LANG_ENGLISH} "Icons Themes"
  LangString DESC_ICONS_THEM ${LANG_RUSSIAN} "Темы иконок"
  LangString DESC_ICONS_THEM ${LANG_UKRAINIAN} "Icons Themes"

SectionGroup "$(DESC_ICONS_THEM)"

  LangString DESC_XP_ICONS ${LANG_ENGLISH} "XP Icons"
  LangString DESC_XP_ICONS ${LANG_RUSSIAN} "Иконки XP"
  LangString DESC_XP_ICONS ${LANG_UKRAINIAN} "XP Icons"

Section "$(DESC_XP_ICONS)" SecXpIconFile
  SetOutPath "$INSTDIR\icons"
  SectionIn 1
  File /oname=xp_icons.dll ..\Release\Win32\icons\xp_icons.dll
SectionEnd

  LangString DESC_VISTA_ICONS ${LANG_ENGLISH} "Vista Icons"
  LangString DESC_VISTA_ICONS ${LANG_RUSSIAN} "Иконки Vista"
  LangString DESC_VISTA_ICONS ${LANG_UKRAINIAN} "Vista Icons"

Section "$(DESC_VISTA_ICONS)" SecVistaIconFile
  SetOutPath "$INSTDIR\icons"
  SectionIn 1
  File /oname=vista_icons.dll ..\Release\Win32\icons\vista_icons.dll
SectionEnd

SectionGroupEnd

  LangString DESC_DEV_DB ${LANG_ENGLISH} "Devices Database"
  LangString DESC_DEV_DB ${LANG_RUSSIAN} "База данных устройств"
  LangString DESC_DEV_DB ${LANG_UKRAINIAN} "Devices Database"

SectionGroup "$(DESC_DEV_DB)"

  LangString DESC_PCI_DEVS ${LANG_ENGLISH} "PCI Devices"
  LangString DESC_PCI_DEVS ${LANG_RUSSIAN} "Устройства PCI"
  LangString DESC_PCI_DEVS ${LANG_UKRAINIAN} "PCI Devices"

Section "$(DESC_PCI_DEVS)" SecPciDbFile
  SetOutPath "$INSTDIR"
  SectionIn 1
  File /oname=pci_dev.ini ..\Release\Win32\pci_dev.ini
SectionEnd

  LangString DESC_USB_DEVS ${LANG_ENGLISH} "USB Devices"
  LangString DESC_USB_DEVS ${LANG_RUSSIAN} "Устройства USB"
  LangString DESC_USB_DEVS ${LANG_UKRAINIAN} "USB Devices"

Section "$(DESC_USB_DEVS)" SecUsbDbFile
  SetOutPath "$INSTDIR"
  SectionIn 1
  File /oname=usb_dev.ini ..\Release\Win32\usb_dev.ini
SectionEnd

  LangString DESC_MON_DEVS ${LANG_ENGLISH} "Monitors"
  LangString DESC_MON_DEVS ${LANG_RUSSIAN} "Мониторы"
  LangString DESC_MON_DEVS ${LANG_UKRAINIAN} "Monitors"

Section "$(DESC_MON_DEVS)" SecMonDbFile
  SetOutPath "$INSTDIR"
  SectionIn 1
  File /oname=mon_dev.ini ..\Release\Win32\mon_dev.ini
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

;Descriptions
  LangString DESC_SEC_RU_RU_FILE ${LANG_ENGLISH} "Russian Language File."
  LangString DESC_SEC_RU_RU_FILE ${LANG_RUSSIAN} "Файл поддержки русского языка."
  LangString DESC_SEC_RU_RU_FILE ${LANG_UKRAINIAN} "Russian Language File."

  LangString DESC_SEC_UK_UA_FILE ${LANG_ENGLISH} "Ukrainian Language File."
  LangString DESC_SEC_UK_UA_FILE ${LANG_RUSSIAN} "Файл поддержки украинского языка."
  LangString DESC_SEC_UK_UA_FILE ${LANG_UKRAINIAN} "Ukrainian Language File."

  LangString DESC_SEC_XP_ICON_FILE ${LANG_ENGLISH} "XP Icon File."
  LangString DESC_SEC_XP_ICON_FILE ${LANG_RUSSIAN} "Тема иконок XP."
  LangString DESC_SEC_XP_ICON_FILE ${LANG_UKRAINIAN} "XP Icon File."

  LangString DESC_SEC_VISTA_ICON_FILE ${LANG_ENGLISH} "Vista Icon File."
  LangString DESC_SEC_VISTA_ICON_FILE ${LANG_RUSSIAN} "Тема иконок Vista."
  LangString DESC_SEC_VISTA_ICON_FILE ${LANG_UKRAINIAN} "Vista Icon File."

  LangString DESC_SEC_MON_DB_FILE ${LANG_ENGLISH} "Monitors Database File."
  LangString DESC_SEC_MON_DB_FILE ${LANG_RUSSIAN} "Файл базы данных мониторов."
  LangString DESC_SEC_MON_DB_FILE ${LANG_UKRAINIAN} "Monitors Database File."

  LangString DESC_SEC_PCI_DB_FILE ${LANG_ENGLISH} "PCI Devices Database File."
  LangString DESC_SEC_PCI_DB_FILE ${LANG_RUSSIAN} "Файл базы данных устройств PCI."
  LangString DESC_SEC_PCI_DB_FILE ${LANG_UKRAINIAN} "PCI Devices Database File."

  LangString DESC_SEC_USB_DB_FILE ${LANG_ENGLISH} "USB Devices Database File."
  LangString DESC_SEC_USB_DB_FILE ${LANG_RUSSIAN} "Файл базы данных устройств USB."
  LangString DESC_SEC_USB_DB_FILE ${LANG_UKRAINIAN} "USB Devices Database File."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecRuRuFile} $(DESC_SEC_RU_RU_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecUkUaFile} $(DESC_SEC_UK_UA_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecXpIconFile} $(DESC_SEC_XP_ICON_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecVistaIconFile} $(DESC_SEC_VISTA_ICON_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMonDbFile} $(DESC_SEC_MON_DB_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPciDbFile} $(DESC_SEC_PCI_DB_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecUsbDbFile} $(DESC_SEC_USB_DB_FILE)
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

  Delete "$INSTDIR\license-en.txt"

  Delete "$INSTDIR\pci_dev.ini"
  Delete "$INSTDIR\usb_dev.ini"
  Delete "$INSTDIR\mon_dev.ini"

  Delete "$INSTDIR\debug.log"

  Delete "$INSTDIR\aspia.ini"

  RMDir "$INSTDIR\languages"
  RMDir "$INSTDIR\icons"
  RMDir "$INSTDIR"

  DeleteRegKey HKLM "Software\Aspia"

SectionEnd

;Uninstaller Functions

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd