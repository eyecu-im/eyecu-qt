/*
+QT Base
*/
Section -$(SecQtBase) QtBase
SectionIn 1 2 3 
	SetOutPath $INSTDIR
  ;------------------------------------${COMPILE}  "mingw"
; MinGW redistribute files
	File "${PROGRAM_BIN_FOLDER}\libgcc_s_dw2-1.dll"
	File "${PROGRAM_BIN_FOLDER}\mingwm10.dll"
	File "${PROGRAM_BIN_FOLDER}\libstdc++-6.dll" 
; Visual Studio redistribute files
	;File "${PROGRAM_BIN_FOLDER}\msvcr90.dll"
	;File "${PROGRAM_BIN_FOLDER}\msvcp90.dll"
	;File "${PROGRAM_BIN_FOLDER}\Microsoft.VC90.CRT.manifest"
; Qt Total ###### TEMP  ######
	File "${PROGRAM_BIN_FOLDER}\QtUtil.dll"
	File "${PROGRAM_BIN_FOLDER}\QtGeo.dll"
	;File "${PROGRAM_BIN_FOLDER}\QtSerialPort.dll"
	File "${PROGRAM_BIN_FOLDER}\QtFFMpeg.dll"
; Qt4 modules
	File "${PROGRAM_BIN_FOLDER}\QtCore4.dll"
	File "${PROGRAM_BIN_FOLDER}\QtGui4.dll"
	File "${PROGRAM_BIN_FOLDER}\QtMultimedia4.dll"
	File "${PROGRAM_BIN_FOLDER}\QtNetwork4.dll"
	File "${PROGRAM_BIN_FOLDER}\QtScript4.dll"
	File "${PROGRAM_BIN_FOLDER}\QtSql4.dll"
	File "${PROGRAM_BIN_FOLDER}\QtSvg4.dll"
	File "${PROGRAM_BIN_FOLDER}\QtWebkit4.dll"
	File "${PROGRAM_BIN_FOLDER}\QtXml4.dll"
	;------------------------------------
	; Qt imageformats
	SetOutPath $INSTDIR\imageformats
	File "${PROGRAM_BIN_FOLDER}\imageformats\*.*"
	;------------------------------------

SectionEnd
