;--------------------------------
;Header Files
	!include "MUI2.nsh"
	!include "Logiclib.nsh"
	!include "Sections.nsh"
	!include "WinMessages.nsh"
;--------------------------------
	Unicode true
	!ifdef NOCOMPRESS
		SetCompress auto
	!endif
;	SetCompressor /SOLID lzma
;--------------------------------
	;Definitions
	!define SHCNE_ASSOCCHANGED 0x8000000
	!define SHCNF_IDLIST 0
;--------------------------------
	;Style window
	SetDateSave on
	SetDatablockOptimize on
	SetOverwrite on
	CRCCheck on
	;SilentInstall normal
	BGGradient 000000 800000 FFFFFF
	InstallColors FF8080 000030
	XPStyle on
	CheckBitmap "${NSISDIR}\Contrib\Graphics\Checks\classic-cross.bmp"
	
#####################################
	;Configuration
	!include eyecu-config.nsi	
;--------------------------------
	;Interface Configuration
	!define MUI_HEADERIMAGE
	!define MUI_HEADERIMAGE_BITMAP 	 "${NSISDIR}\Contrib\Graphics\Header\orange.bmp" ; nsis.bmp
	!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-uninstall.bmp"
	
	!define MUI_WELCOMEPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
	!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
	!define MUI_UNWELCOMEHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"
	!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"
	
	!define MUI_ABORTWARNING
	;Show all languages, despite user's codepage
	!define MUI_LANGDLL_ALLLANGUAGES
	!define MUI_LANGDLL_WINDOWTITLE $(LangMenuWindowTitle)
	!define MUI_LANGDLL_INFO $(LangMenuInfo)
	!define MUI_FINISHPAGE_BUTTON $(FinishButton)	; rename button "Ready"
	!define MUI_FINISHPAGE_RUN 	"${Name1}.exe"		; checkBox start -> "Setup.exe"
;	!define MUI_UNFINISHPAGE_BUTTON $(FinishButton)	; No Effect
	!define MUI_FINISHPAGE_RUN_NOTCHECKED
;------------------------------
	!define MUI_COMPONENTSPAGE_SMALLDESC
;--------------------------------
	Var DelAll
	Var UnDelAll
	Var SecIsEmpty
	Var SecIsFull
;--------------------------------
;Pages
	!insertmacro MUI_PAGE_WELCOME
	!insertmacro MUI_PAGE_LICENSE "${PROGRAM_BIN_FOLDER}\Copying"	; --License.txt--
	!insertmacro MUI_PAGE_DIRECTORY	
	!define MUI_PAGE_CUSTOMFUNCTION_PRE "ReadIniFile"
	!insertmacro MUI_PAGE_COMPONENTS
	!define MUI_PAGE_CUSTOMFUNCTION_LEAVE "ComponentsSelected"
	!insertmacro MUI_PAGE_INSTFILES
	!insertmacro MUI_PAGE_FINISH 
;---Uninstaller pages------------
	!insertmacro MUI_UNPAGE_CONFIRM
	!define MUI_PAGE_CUSTOMFUNCTION_PRE "un.ReadIniFile"
	!insertmacro MUI_UNPAGE_COMPONENTS 
	!define MUI_PAGE_CUSTOMFUNCTION_LEAVE "un.ComponentsSelected"	
	!insertmacro MUI_UNPAGE_INSTFILES
	!insertmacro MUI_UNPAGE_FINISH 
;--------------------------------
	!define SF_MASK   			3
	!define SF_DESELECTED  		62  ; 0x3E	; 62
	!define SF_EXPAND_INVERS 	0xDF
	!define SF_RO_INVERS 		0xEF
	!define SF_PSELECTED_INV 	0x3F
	!define SF_CLEAR_MB	 		0x7F
;--------------------------------
	;Languages Interface
	!insertmacro MUI_LANGUAGE "English" ;first language is the default language
	!insertmacro MUI_LANGUAGE "Russian"
	;!insertmacro MUI_LANGUAGE "German"
	;!insertmacro MUI_LANGUAGE "Spanish"
	;!insertmacro MUI_LANGUAGE "Japanese"
	;!insertmacro MUI_LANGUAGE "Dutch"
	;!insertmacro MUI_LANGUAGE "Polish"
	;!insertmacro MUI_LANGUAGE "Ukrainian"
;--------------------------------  
	InstType $(FullSetup)
	InstType $(BaseSetup)
	;InstType /NOCUSTOM
	;InstType /COMPONENTSONLYONCUSTOM
	InstType "Un.$(DelAll)"
	InstType "Un.$(DelToBase)"
;-------------------------------- 
	AutoCloseWindow false
	ShowInstDetails show  
 
#####################################
;Installer Sections
#####################################

Section "-Install"  AbInstall
SectionIn 1 2 3 
	; Write the installation path into the registry
	WriteRegStr   HKLM "SOFTWARE\${PROGRAM_REG_KEY}" "Install_Dir" "$INSTDIR"
	; Write the uninstall keys for Windows
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_REG_KEY}" "DisplayName" "eyeCU Instant Messenger"
	WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_REG_KEY}" "UninstallString" '"$INSTDIR\${UnIstName}.exe"'
;	WriteRegStr   HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\MyProgram "UninstallString" "$OUTDIR\uninst.exe"

	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_REG_KEY}" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PROGRAM_REG_KEY}" "NoRepair" 1
	;--------------
	WriteUninstaller "${UnIstName}.exe"
  
SectionEnd
;--------------------------------
;Installer Base Sections
	!include "eyecu-qt${QT}.nsi"
	!include "eyecu-base.nsi"
;--------------------------------	
;Installer Sections
	!include "eyecu-sections.nsi"
;------------------------------

Section -$(SecMenuShort) MenuShort
SectionIn 1 2 3 RO
	CreateDirectory "$SMPROGRAMS\${PROGRAM_SM_FOLDER}"
	CreateShortCut "$SMPROGRAMS\${PROGRAM_SM_FOLDER}\${Name1}.lnk" "$INSTDIR\${Name1}.exe" "" "$INSTDIR\${Name1}.exe" 0
	CreateShortCut "$SMPROGRAMS\${PROGRAM_SM_FOLDER}\${UnIstName}.lnk" "$INSTDIR\${UnIstName}.exe" "" "$INSTDIR\${UnIstName}.exe" 0

SectionEnd

Section -$(SecMenuDesktop) MenuDesktop
SectionIn 1 2 3 RO
	CreateShortCut "$DESKTOP\${Name1}.lnk" "$INSTDIR\${Name1}.exe" "" "$INSTDIR\${Name1}.exe" 0
  
SectionEnd

#####################################
; Uninstaller
#####################################

Section "-Uninstall" AbUninstall
SectionIn 1  

SectionEnd
;--------------------------------
; Uninstaller Base Sections
	!include "eyecu-un-qt${QT}.nsi"
	!include "eyecu-un-base.nsi"
;--------------------------------	
; Uninstaller Sections
	!include "eyecu-un-sections.nsi"
;--------------------------------
;Translate Sections
	!include "eyecu-translate.nsi"
;--------------------------------
; functions
	!include "eyecu-functions.nsi"
;--------------------------------	
