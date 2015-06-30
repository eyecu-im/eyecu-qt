	; Program Version
	!define PROGRAM_VERSION     "1.3.0.1355M_beta"
	; Install Folder
	; now - C:\Program Files\Road Works Software\eyeCU
	!define PROGRAM_FOLDER      "Road Works Software\eyeCU"
	; Install Start Menu Folder
	!define PROGRAM_SM_FOLDER   "eyeCU"
	; Registry Key
	!define PROGRAM_REG_KEY     "eyeCU"
	; The name of the installer
	Name "eyeCU"
	!define Name1 "eyeCU"
	##########  FOR CHANGE ##############
	;!define QT_PLATFORM     	"4.8", "5.4"
	!define DISK				"e:"
#VER  QT-4 ###############################
;	!define QT     				"4"
;	!define VersionQT     		"8"
;	!define QTFF   				""
#VER  QT-5 ###############################
	!define QT     				"5"
	!define VersionQT     		"4"
	!define QTFF   				"5"		
	#####################################
	; Program binaries, example: "e:\eyecu4.8" OR "e:\eyecu5.4" .....
;	!define PROGRAM_BIN_FOLDER  "${DISK}\${Name1}${QT_PLATFORM}\"
	!define PROGRAM_BIN_FOLDER  "${DISK}\${Name1}${QT}.${VersionQT}"
	;-------------------------------------
	;Caption "Install eyeCU"
	Caption "Install eyeCU, QT Platform-${QT}.${VersionQT}, Version - ${PROGRAM_VERSION} "
	;Icon
	Icon "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"
	UninstallIcon 	"${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"
	; The name of the Uninstaller
	!define UnIstName "UninstalleyeCU"
;--------------------------------
	; The file to write, example: "eyeCU_QT5.4_V-1.3.0.1268_beta.exe"
	OutFile "${Name1}_QT${QT}.${VersionQT}_V-${PROGRAM_VERSION}.exe"
	; The default installation directory
	InstallDir "$PROGRAMFILES\${PROGRAM_FOLDER}"
	; Registry key to check for directory (so if you install again, it will overwrite the old one automatically)
	InstallDirRegKey HKLM "Software\${PROGRAM_REG_KEY}" "Install_Dir"
	; Request application privileges for Windows Vista
	RequestExecutionLevel admin
	;RequestExecutionLevel user
;----------------------------------
/* this for next ...
	;Product code {3DE9AF7F-2CD3-4134-BD32-B856E55DEDE0}
	;Upgrade code {CCB05E14-8A42-4A20-8FE6-DD66BB20251A}
	;Package code {C4BDC854-627D-47A1-9456-F5D4870610FA}
	;Uninstall [SystemFolder]msiexec.exe /x {3DE9AF7F-2CD3-4134-BD32-B856E55DEDE0}
*/