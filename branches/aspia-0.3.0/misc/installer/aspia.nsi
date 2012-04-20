;
; PROJECT:      Aspia
; FILE:         NSIS Installer Script
; LICENSE:      LGPL (GNU Lesser General Public License)
; PROGRAMMERS:  Dmitry Chapyshev
;

;--------------------------------
;Include Section

  !include "MUI2.nsh"
  !include "LogicLib.nsh"
  !include "nsDialogs.nsh"

;--------------------------------
;Enable Aero Effects
!define MUI_CUSTOMFUNCTION_GUIINIT onGUIInit

Function onGUIInit
  Aero::Apply
FunctionEnd
  
;--------------------------------
;Interface Settings

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "aspia.bmp"
  !define MUI_ABORTWARNING

;--------------------------------
;Pages

    !insertmacro MUI_PAGE_LICENSE "license.txt"
    Page custom ChooseInstallMethod ChooseInstallMethodLeave
    !insertmacro MUI_PAGE_COMPONENTS
	Page custom InstallSettings InstallSettingsLeave
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
;Code for choosing install method (standard / portable)
LangString TEXT_STD_INSTALLATION ${LANG_ENGLISH} "&Standard installation"
LangString TEXT_STD_INSTALLATION ${LANG_RUSSIAN} "&Стандартная установка"
LangString TEXT_STD_INSTALLATION ${LANG_UKRAINIAN} "&Стандартне встановлення"

LangString TEXT_STD_INST_LABEL ${LANG_ENGLISH} "Install Aspia for all users of this machine."
LangString TEXT_STD_INST_LABEL ${LANG_RUSSIAN} "Aspia будет установлена для всех пользователей этого компьютера."
LangString TEXT_STD_INST_LABEL ${LANG_UKRAINIAN} "Aspia буде встановлена для всіх користувачів цього комп'ютера."

LangString TEXT_PORTABLE_INST ${LANG_ENGLISH} "&Portable installation"
LangString TEXT_PORTABLE_INST ${LANG_RUSSIAN} "Портативная установка"
LangString TEXT_PORTABLE_INST ${LANG_UKRAINIAN} "Портативне встановлення"

LangString TEXT_PORTABLE_INST_LABEL ${LANG_ENGLISH} "In portable mode all configuration data is stored in the application folder and no information is written to the registry."
LangString TEXT_PORTABLE_INST_LABEL ${LANG_RUSSIAN} "В портативном режиме все конфигурационных данные будут сохраненяться в папке с программой и никаких данных в реестр записано не будет."
LangString TEXT_PORTABLE_INST_LABEL ${LANG_UKRAINIAN} "У портативному режимі всі конфігураційні дані будуть зберігатись в теці з програмою і ніяких даних до реєстру записано не буде."

LangString TEXT_INST_METHOD_TITLE ${LANG_ENGLISH} "Installation Method"
LangString TEXT_INST_METHOD_TITLE ${LANG_RUSSIAN} "Способ установки"
LangString TEXT_INST_METHOD_TITLE ${LANG_UKRAINIAN} "Спосіб встановлення"

LangString TEXT_INST_METHOD_SUBTITLE ${LANG_ENGLISH} "Choose installation method."
LangString TEXT_INST_METHOD_SUBTITLE ${LANG_RUSSIAN} "Выбирите способ установки."
LangString TEXT_INST_METHOD_SUBTITLE ${LANG_UKRAINIAN} "Оберіть спосіб встановлення."

Var InstallMethodDialog
Var StandardRadioButton
Var StandardLabel
Var PortableRadioButton
Var PortableLabel
Var PortableChecked

Function ChooseInstallMethod
	!insertmacro MUI_HEADER_TEXT "$(TEXT_INST_METHOD_TITLE)" "$(TEXT_INST_METHOD_SUBTITLE)"

	nsDialogs::Create 1018
	Pop $InstallMethodDialog

	${If} $InstallMethodDialog == error
		Abort
	${EndIf}

	CreateFont $1 "Tahoma" "9" "800"

	${NSD_CreateRadioButton} 0 15u 100% 12u "$(TEXT_STD_INSTALLATION)"
	Pop $StandardRadioButton
	SendMessage $StandardRadioButton ${WM_SETFONT} $1 1

	${NSD_CreateLabel} 15u 30u 100% 12u "$(TEXT_STD_INST_LABEL)"
	Pop $StandardLabel

	${NSD_CreateRadioButton} 0 60u 100% 12u "$(TEXT_PORTABLE_INST)"
	Pop $PortableRadioButton
	SendMessage $PortableRadioButton ${WM_SETFONT} $1 1

	${NSD_CreateLabel} 15u 75u 90% 25u "$(TEXT_PORTABLE_INST_LABEL)"
	Pop $PortableLabel

	${If} $PortableChecked == ${BST_CHECKED}
		${NSD_SetState} $StandardRadioButton ${BST_UNCHECKED}
		${NSD_SetState} $PortableRadioButton ${BST_CHECKED}
	${Else}
		${NSD_SetState} $StandardRadioButton ${BST_CHECKED}
		${NSD_SetState} $PortableRadioButton ${BST_UNCHECKED}
	${EndIf}

	nsDialogs::Show

FunctionEnd

Function ChooseInstallMethodLeave
	${NSD_GetState} $PortableRadioButton $PortableChecked
	
	${If} $PortableChecked == ${BST_CHECKED}
		StrCpy $INSTDIR "$DESKTOP\Aspia"
	${Else}
		StrCpy $INSTDIR "$PROGRAMFILES32\Aspia"
	${EndIf}
FunctionEnd

;--------------------------------
LangString TEXT_ICONS_LABEL ${LANG_ENGLISH} "Select where you want to create shortcuts:"
LangString TEXT_ICONS_LABEL ${LANG_RUSSIAN} "Выберите, где нужно создать ярлыки:"
LangString TEXT_ICONS_LABEL ${LANG_UKRAINIAN} "Виберіть, де потрібно створити ярлики:"

LangString TEXT_STARTMENU_ICONS ${LANG_ENGLISH} "Create &Start menu shortcuts"
LangString TEXT_STARTMENU_ICONS ${LANG_RUSSIAN} "Создать ярлыки в меню Пуск"
LangString TEXT_STARTMENU_ICONS ${LANG_UKRAINIAN} "Створити ярлики в меню Пуск"

LangString TEXT_DESKTOP_ICONS ${LANG_ENGLISH} "Create &desktop shortcuts"
LangString TEXT_DESKTOP_ICONS ${LANG_RUSSIAN} "Создать ярлыки на рабочем столе"
LangString TEXT_DESKTOP_ICONS ${LANG_UKRAINIAN} "Створити ярлики на робочому столі"

LangString TEXT_USERS_LABEL ${LANG_ENGLISH} "Aspia will be installed for:"
LangString TEXT_USERS_LABEL ${LANG_RUSSIAN} "Aspia будет установлена для:"
LangString TEXT_USERS_LABEL ${LANG_UKRAINIAN} "Aspia буде встановлена для:"

LangString TEXT_ALL_USERS ${LANG_ENGLISH} "&All Users"
LangString TEXT_ALL_USERS ${LANG_RUSSIAN} "Всех пользователей"
LangString TEXT_ALL_USERS ${LANG_UKRAINIAN} "Усіх користувачів"

LangString TEXT_CURRENT_USER ${LANG_ENGLISH} "&Current User"
LangString TEXT_CURRENT_USER ${LANG_RUSSIAN} "Текущего пользователя"
LangString TEXT_CURRENT_USER ${LANG_UKRAINIAN} "Поточного користувача"

LangString TEXT_INST_SETTINGS_TITLE ${LANG_ENGLISH} "Installation Settings"
LangString TEXT_INST_SETTINGS_TITLE ${LANG_RUSSIAN} "Параметры установки"
LangString TEXT_INST_SETTINGS_TITLE ${LANG_UKRAINIAN} "Параметри встановлення"

LangString TEXT_INST_SETTINGS_SUBTITLE ${LANG_ENGLISH} "Choose installation settings."
LangString TEXT_INST_SETTINGS_SUBTITLE ${LANG_RUSSIAN} "Выбирите параметры установки."
LangString TEXT_INST_SETTINGS_SUBTITLE ${LANG_UKRAINIAN} "Оберіть параметри встановлення."

Var InstallSettingsDialog
Var IconsLabel
Var StartMenuIconsCheckBox
Var DesktopIconsCheckBox
Var UsersLabel
Var AllUsersRadioButton
Var CurrentUserRadioButton
Var AllUsersChecked
Var StartmenuChecked
Var DesktopChecked

Function InstallSettings

	${If} $PortableChecked == ${BST_CHECKED}
		Return
	${EndIf}

	!insertmacro MUI_HEADER_TEXT "$(TEXT_INST_SETTINGS_TITLE)" "$(TEXT_INST_SETTINGS_SUBTITLE)"

	nsDialogs::Create 1018
	Pop $InstallSettingsDialog

	${If} $InstallSettingsDialog == error
		Abort
	${EndIf}

	${NSD_CreateLabel} 15u 10u 90% 12u "$(TEXT_ICONS_LABEL)"
	Pop $IconsLabel
	${NSD_CreateCheckBox} 25u 25u 100% 12u "$(TEXT_STARTMENU_ICONS)"
	Pop $StartMenuIconsCheckBox
	${NSD_CreateCheckBox} 25u 40u 100% 12u "$(TEXT_DESKTOP_ICONS)"
	Pop $DesktopIconsCheckBox

	${NSD_CreateLabel} 15u 65u 90% 12u "$(TEXT_USERS_LABEL)"
	Pop $UsersLabel
	${NSD_CreateRadioButton} 25u 80u 100% 12u "$(TEXT_ALL_USERS)"
	Pop $AllUsersRadioButton
	${NSD_CreateRadioButton} 25u 95u 100% 12u "$(TEXT_CURRENT_USER)"
	Pop $CurrentUserRadioButton

	${If} $AllUsersChecked == ${BST_CHECKED}
		${NSD_SetState} $CurrentUserRadioButton ${BST_UNCHECKED}
		${NSD_SetState} $AllUsersRadioButton ${BST_CHECKED}
	${Else}
		${NSD_SetState} $CurrentUserRadioButton ${BST_CHECKED}
		${NSD_SetState} $AllUsersRadioButton ${BST_UNCHECKED}
	${EndIf}

	${NSD_SetState} $StartMenuIconsCheckBox ${BST_CHECKED}

	nsDialogs::Show

FunctionEnd

Function InstallSettingsLeave
	${If} $PortableChecked == ${BST_CHECKED}
		Return
	${EndIf}

	${NSD_GetState} $AllUsersRadioButton $AllUsersChecked
	${NSD_GetState} $StartMenuIconsCheckBox $StartMenuChecked
	${NSD_GetState} $DesktopIconsCheckBox $DesktopChecked
	
	${If} $AllUsersChecked == ${BST_CHECKED}
		SetShellVarContext all
	${Else}
		SetShellVarContext current
	${EndIf}
FunctionEnd

;--------------------------------
;General

  ;Name and file
  Name "Aspia"
  OutFile "aspia-setup.exe"

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
  LangString DESC_FULL_INSTALL ${LANG_UKRAINIAN} "Повне"

  LangString DESC_MIN_INSTALL ${LANG_ENGLISH} "Minimal"
  LangString DESC_MIN_INSTALL ${LANG_RUSSIAN} "Минимальная установка"
  LangString DESC_MIN_INSTALL ${LANG_UKRAINIAN} "Мінімальне"

  InstType "$(DESC_FULL_INSTALL)"
  InstType "$(DESC_MIN_INSTALL)"

  LangString DESC_MAIN_PROG_X86 ${LANG_ENGLISH} "Main Program (x86)"
  LangString DESC_MAIN_PROG_X86 ${LANG_RUSSIAN} "Основная программа (x86)"
  LangString DESC_MAIN_PROG_X86 ${LANG_UKRAINIAN} "Основна програма (x86)"

  LangString DESC_MAIN_PROG_X64 ${LANG_ENGLISH} "Main Program (x64)"
  LangString DESC_MAIN_PROG_X64 ${LANG_RUSSIAN} "Основная программа (x64)"
  LangString DESC_MAIN_PROG_X64 ${LANG_UKRAINIAN} "Основна програма (x64)"
  
  LangString DESC_CURRENT_LANG ${LANG_ENGLISH} "en-US"
  LangString DESC_CURRENT_LANG ${LANG_RUSSIAN} "ru-RU"
  LangString DESC_CURRENT_LANG ${LANG_UKRAINIAN} "uk-UA"

Section "!$(DESC_MAIN_PROG_X86)" SecMainProgram_x86

	SetOutPath "$INSTDIR"

    SectionIn 1 2 3

    File /oname=aspia.exe ..\..\Release\aspia.exe
    File /oname=aspia.dll ..\..\Release\aspia.dll
    File /oname=helper.dll ..\..\Release\helper.dll
    File /oname=aspia_x32.sys ..\..\Release\aspia_x32.sys

    ;Store installation folder
    ${If} $PortableChecked == ${BST_UNCHECKED}
		WriteRegStr HKLM "Software\Aspia" "" $INSTDIR

		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspia" "DisplayName" "Aspia"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspia" "Publisher" "Aspia Software"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspia" "DisplayIcon" "$INSTDIR\aspia.exe,0"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspia" "UninstallString" "$INSTDIR\Uninstall.exe"

		WriteINIStr "$APPDATA\aspia.ini" "general" "language" "$(DESC_CURRENT_LANG)"

		;Create uninstaller
		WriteUninstaller "$INSTDIR\Uninstall.exe"

		;Create shortcuts
		${If} $StartmenuChecked == ${BST_CHECKED}
			CreateShortCut "$STARTMENU\Aspia.lnk" "$INSTDIR\aspia.exe"
		${EndIf}
		${If} $DesktopChecked == ${BST_CHECKED}
			CreateShortCut "$DESKTOP\Aspia.lnk" "$INSTDIR\aspia.exe"
		${EndIf}
	${Else}
		WriteINIStr "$INSTDIR\aspia.ini" "general" "language" "$(DESC_CURRENT_LANG)"
		FileOpen $0 $INSTDIR\portable w
		FileClose $0
    ${EndIf}

SectionEnd

Section "$(DESC_MAIN_PROG_X64)" SecMainProgram_x64

    SetOutPath "$INSTDIR"

    SectionIn 1 2 3

    File /oname=aspia64.exe ..\..\Release\aspia64.exe
    File /oname=aspia64.dll ..\..\Release\aspia64.dll
    File /oname=helper64.dll ..\..\Release\helper64.dll
    File /oname=aspia_x64.sys ..\..\Release\aspia_x64.sys

    ;Create shortcuts
    ${If} $PortableChecked == ${BST_UNCHECKED}
		${If} $StartmenuChecked == ${BST_CHECKED}
			CreateShortCut "$STARTMENU\Aspia (x64).lnk" "$INSTDIR\aspia64.exe"
		${EndIf}
		${If} $DesktopChecked == ${BST_CHECKED}
			CreateShortCut "$DESKTOP\Aspia (x64).lnk" "$INSTDIR\aspia64.exe"
		${EndIf}
	${EndIf}

SectionEnd

  LangString DESC_LANG_FILES ${LANG_ENGLISH} "Language Files"
  LangString DESC_LANG_FILES ${LANG_RUSSIAN} "Языковые файлы"
  LangString DESC_LANG_FILES ${LANG_UKRAINIAN} "Мовні файли"

SectionGroup "$(DESC_LANG_FILES)"

Section "English" SecEnUsFile
  SetOutPath "$INSTDIR\languages"
  SectionIn 1 2 3
  File /oname=en-US.dll ..\..\Release\languages\en-US.dll
SectionEnd

Section /o "Russian" SecRuRuFile
  SetOutPath "$INSTDIR\languages"
  SectionIn 1 3
  File /oname=ru-RU.dll ..\..\Release\languages\ru-RU.dll
SectionEnd

Section /o "Ukrainian" SecUkUaFile
  SetOutPath "$INSTDIR\languages"
  SectionIn 1 3
  File /oname=uk-UA.dll ..\..\Release\languages\uk-UA.dll
SectionEnd

SectionGroupEnd

  LangString DESC_HELP_FILES ${LANG_ENGLISH} "Help Files"
  LangString DESC_HELP_FILES ${LANG_RUSSIAN} "Файлы справки"
  LangString DESC_HELP_FILES ${LANG_UKRAINIAN} "Файли довідки"

SectionGroup "$(DESC_HELP_FILES)"

Section /o "English" SecEnUsHelp
  SetOutPath "$INSTDIR\help"
  SectionIn 1 3
  File /oname=en-US.chm ..\..\Release\help\en-US.chm
SectionEnd

Section /o "Russian" SecRuRuHelp
  SetOutPath "$INSTDIR\help"
  SectionIn 1 3
  File /oname=ru-RU.chm ..\..\Release\help\ru-RU.chm
SectionEnd

Section /o "Ukrainian" SecUkUaHelp
  SetOutPath "$INSTDIR\help"
  SectionIn 1 3
  File /oname=uk-UA.chm ..\..\Release\help\uk-UA.chm
SectionEnd

SectionGroupEnd

  LangString DESC_ICONS_THEM ${LANG_ENGLISH} "Icons Themes"
  LangString DESC_ICONS_THEM ${LANG_RUSSIAN} "Темы иконок"
  LangString DESC_ICONS_THEM ${LANG_UKRAINIAN} "Теми значків"

SectionGroup "$(DESC_ICONS_THEM)"

  LangString DESC_TANGO_ICONS ${LANG_ENGLISH} "Tango Icons"
  LangString DESC_TANGO_ICONS ${LANG_RUSSIAN} "Иконки Tango"
  LangString DESC_TANGO_ICONS ${LANG_UKRAINIAN} "Значки Tango"

Section "$(DESC_TANGO_ICONS)" SecTangoIconFile
  SetOutPath "$INSTDIR\icons"
  SectionIn 1 2 3
  File /oname=tango.dll ..\..\Release\icons\tango.dll
SectionEnd

  LangString DESC_OXYGEN_ICONS ${LANG_ENGLISH} "Oxygen Icons"
  LangString DESC_OXYGEN_ICONS ${LANG_RUSSIAN} "Иконки Oxygen"
  LangString DESC_OXYGEN_ICONS ${LANG_UKRAINIAN} "Значки Oxygen"

Section "$(DESC_OXYGEN_ICONS)" SecOxygenIconFile
  SetOutPath "$INSTDIR\icons"
  SectionIn 1 3
  File /oname=oxygen.dll ..\..\Release\icons\oxygen.dll
SectionEnd

SectionGroupEnd

  LangString DESC_DEV_DB ${LANG_ENGLISH} "Devices Database"
  LangString DESC_DEV_DB ${LANG_RUSSIAN} "База данных устройств"
  LangString DESC_DEV_DB ${LANG_UKRAINIAN} "База даних пристроїв"

SectionGroup "$(DESC_DEV_DB)"

  LangString DESC_PCI_DEVS ${LANG_ENGLISH} "PCI Devices"
  LangString DESC_PCI_DEVS ${LANG_RUSSIAN} "Устройства PCI"
  LangString DESC_PCI_DEVS ${LANG_UKRAINIAN} "Пристрої PCI"

Section "$(DESC_PCI_DEVS)" SecPciDbFile
  SetOutPath "$INSTDIR"
  SectionIn 1 3
  File /oname=pci_dev.ini ..\..\db\pci_dev.ini
SectionEnd

  LangString DESC_USB_DEVS ${LANG_ENGLISH} "USB Devices"
  LangString DESC_USB_DEVS ${LANG_RUSSIAN} "Устройства USB"
  LangString DESC_USB_DEVS ${LANG_UKRAINIAN} "Пристрої USB"

Section "$(DESC_USB_DEVS)" SecUsbDbFile
  SetOutPath "$INSTDIR"
  SectionIn 1 3
  File /oname=usb_dev.ini ..\..\db\usb_dev.ini
SectionEnd

  LangString DESC_MON_DEVS ${LANG_ENGLISH} "Monitors"
  LangString DESC_MON_DEVS ${LANG_RUSSIAN} "Мониторы"
  LangString DESC_MON_DEVS ${LANG_UKRAINIAN} "Монітори"

Section "$(DESC_MON_DEVS)" SecMonDbFile
  SetOutPath "$INSTDIR"
  SectionIn 1 3
  File /oname=mon_dev.ini ..\..\db\mon_dev.ini
SectionEnd

SectionGroupEnd

;--------------------------------
;Installer Functions

Function .onInit
  IntOp $0 ${SF_SELECTED} | ${SF_RO}
  SectionSetFlags ${SecMainProgram_x86} $0
  SectionSetFlags ${SecEnUsFile} $0
  SectionSetFlags ${SecTangoIconFile} $0

  SetCurInstType 0

  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

;Descriptions
  LangString DESC_SEC_EN_US_FILE ${LANG_ENGLISH} "English Language File."
  LangString DESC_SEC_EN_US_FILE ${LANG_RUSSIAN} "Файл поддержки английского языка."
  LangString DESC_SEC_EN_US_FILE ${LANG_UKRAINIAN} "Файл підтримки англійської мови."

  LangString DESC_SEC_RU_RU_FILE ${LANG_ENGLISH} "Russian Language File."
  LangString DESC_SEC_RU_RU_FILE ${LANG_RUSSIAN} "Файл поддержки русского языка."
  LangString DESC_SEC_RU_RU_FILE ${LANG_UKRAINIAN} "Файл підтримки російської мови."

  LangString DESC_SEC_UK_UA_FILE ${LANG_ENGLISH} "Ukrainian Language File."
  LangString DESC_SEC_UK_UA_FILE ${LANG_RUSSIAN} "Файл поддержки украинского языка."
  LangString DESC_SEC_UK_UA_FILE ${LANG_UKRAINIAN} "Файл підтримки української мови."

  LangString DESC_SEC_EN_US_HELP ${LANG_ENGLISH} "English Help File."
  LangString DESC_SEC_EN_US_HELP ${LANG_RUSSIAN} "Справка на английском языке."
  LangString DESC_SEC_EN_US_HELP ${LANG_UKRAINIAN} "Довідка англійською мовою."

  LangString DESC_SEC_RU_RU_HELP ${LANG_ENGLISH} "Russian Help File."
  LangString DESC_SEC_RU_RU_HELP ${LANG_RUSSIAN} "Справка на русском языке."
  LangString DESC_SEC_RU_RU_HELP ${LANG_UKRAINIAN} "Довідка російською мовою."

  LangString DESC_SEC_UK_UA_HELP ${LANG_ENGLISH} "Ukrainian Help File."
  LangString DESC_SEC_UK_UA_HELP ${LANG_RUSSIAN} "Справка на украинском языке."
  LangString DESC_SEC_UK_UA_HELP ${LANG_UKRAINIAN} "Довідка українською мовою."

  LangString DESC_SEC_TANGO_ICON_FILE ${LANG_ENGLISH} "Tango Icon File."
  LangString DESC_SEC_TANGO_ICON_FILE ${LANG_RUSSIAN} "Тема иконок Tango."
  LangString DESC_SEC_TANGO_ICON_FILE ${LANG_UKRAINIAN} "Тема значків Tango."

  LangString DESC_SEC_OXYGEN_ICON_FILE ${LANG_ENGLISH} "Oxygen Icon File."
  LangString DESC_SEC_OXYGEN_ICON_FILE ${LANG_RUSSIAN} "Тема иконок Oxygen."
  LangString DESC_SEC_OXYGEN_ICON_FILE ${LANG_UKRAINIAN} "Тема значків Oxygen."

  LangString DESC_SEC_MON_DB_FILE ${LANG_ENGLISH} "Monitors Database File."
  LangString DESC_SEC_MON_DB_FILE ${LANG_RUSSIAN} "Файл базы данных мониторов."
  LangString DESC_SEC_MON_DB_FILE ${LANG_UKRAINIAN} "Файл бази даних моніторів."

  LangString DESC_SEC_PCI_DB_FILE ${LANG_ENGLISH} "PCI Devices Database File."
  LangString DESC_SEC_PCI_DB_FILE ${LANG_RUSSIAN} "Файл базы данных устройств PCI."
  LangString DESC_SEC_PCI_DB_FILE ${LANG_UKRAINIAN} "Файл бази даних пристроїв PCI."

  LangString DESC_SEC_USB_DB_FILE ${LANG_ENGLISH} "USB Devices Database File."
  LangString DESC_SEC_USB_DB_FILE ${LANG_RUSSIAN} "Файл базы данных устройств USB."
  LangString DESC_SEC_USB_DB_FILE ${LANG_UKRAINIAN} "Файл бази даних пристроїв USB."

  LangString DESC_SEC_MAIN_PROG_X86 ${LANG_ENGLISH} "Program files for the x86 architecture."
  LangString DESC_SEC_MAIN_PROG_X86 ${LANG_RUSSIAN} "Программные файлы для архитектуры x86."
  LangString DESC_SEC_MAIN_PROG_X86 ${LANG_UKRAINIAN} "Програмні файли для архітектури x86."

  LangString DESC_SEC_MAIN_PROG_X64 ${LANG_ENGLISH} "Program files for the x64 (AMD64) architecture."
  LangString DESC_SEC_MAIN_PROG_X64 ${LANG_RUSSIAN} "Программные файлы для архитектуры x64 (AMD64)."
  LangString DESC_SEC_MAIN_PROG_X64 ${LANG_UKRAINIAN} "Програмні файли для архітектури x64 (AMD64)."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMainProgram_x86} $(DESC_SEC_MAIN_PROG_X86)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecMainProgram_x64} $(DESC_SEC_MAIN_PROG_X64)

    !insertmacro MUI_DESCRIPTION_TEXT ${SecEnUsFile} $(DESC_SEC_EN_US_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecRuRuFile} $(DESC_SEC_RU_RU_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecUkUaFile} $(DESC_SEC_UK_UA_FILE)

    !insertmacro MUI_DESCRIPTION_TEXT ${SecEnUsHelp} $(DESC_SEC_EN_US_HELP)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecRuRuHelp} $(DESC_SEC_RU_RU_HELP)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecUkUaHelp} $(DESC_SEC_UK_UA_HELP)

    !insertmacro MUI_DESCRIPTION_TEXT ${SecTangoIconFile} $(DESC_SEC_TANGO_ICON_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecOxygenIconFile} $(DESC_SEC_OXYGEN_ICON_FILE)

    !insertmacro MUI_DESCRIPTION_TEXT ${SecMonDbFile} $(DESC_SEC_MON_DB_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPciDbFile} $(DESC_SEC_PCI_DB_FILE)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecUsbDbFile} $(DESC_SEC_USB_DB_FILE)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$STARTMENU\Aspia.lnk"
  Delete "$STARTMENU\Aspia (x64).lnk"

  Delete "$INSTDIR\Uninstall.exe"

  Delete "$INSTDIR\aspia.exe"
  Delete "$INSTDIR\aspia64.exe"

  Delete "$INSTDIR\aspia.dll"
  Delete "$INSTDIR\aspia64.dll"

  Delete "$INSTDIR\helper.dll"
  Delete "$INSTDIR\helper64.dll"

  Delete "$INSTDIR\languages\en-US.dll"
  Delete "$INSTDIR\languages\ru-RU.dll"
  Delete "$INSTDIR\languages\uk-UA.dll"

  Delete "$INSTDIR\help\en-US.chm"
  Delete "$INSTDIR\help\ru-RU.chm"
  Delete "$INSTDIR\help\uk-UA.chm"

  Delete "$INSTDIR\icons\tango.dll"
  Delete "$INSTDIR\icons\oxygen.dll"

  Delete "$INSTDIR\aspia_x32.sys"
  Delete "$INSTDIR\aspia_x64.sys"

  Delete "$INSTDIR\pci_dev.ini"
  Delete "$INSTDIR\usb_dev.ini"
  Delete "$INSTDIR\mon_dev.ini"

  Delete "$INSTDIR\aspia.log"

  Delete "$INSTDIR\aspia.ini"

  RMDir "$INSTDIR\languages"
  RMDir "$INSTDIR\icons"
  RMDir "$INSTDIR\help"
  RMDir "$INSTDIR"

  DeleteRegKey HKLM "Software\Aspia"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aspia"

SectionEnd

;Uninstaller Functions

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd