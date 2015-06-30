/*
+QT Base
*/
Section "-Un.$(SecUnQtBase)" UnQtBase
SectionIn 1
  ;----------${COMPILE}  "mingw" --------------------------
; MinGW redistribute files
	Delete "$INSTDIR\libgcc_s_dw2-1.dll"
	Delete "$INSTDIR\mingwm10.dll"
;	Delete "$INSTDIR\libstdc++-6.dll" 
; Visual Studio redistribute files
	;Delete "$INSTDIR\msvcr90.dll"
	;Delete "$INSTDIR\msvcp90.dll"
	;Delete "$INSTDIR\Microsoft.VC90.CRT.manifest"
; Qt Total ###### TEMP  ######
	Delete "$INSTDIR\QtUtil.dll"
	Delete "$INSTDIR\QtGeo.dll"	
; Qt4 modules
	Delete "$INSTDIR\QtCore4.dll"
	Delete "$INSTDIR\QtGui4.dll"
	Delete "$INSTDIR\QtGui4.dll"
	Delete "$INSTDIR\QtMultimedia4.dll"
	Delete "$INSTDIR\QtNetwork4.dll"
	Delete "$INSTDIR\QtOpenGL4.dll"
	Delete "$INSTDIR\QtSerialPort.dll"
	Delete "$INSTDIR\QtScript4.dll"
	Delete "$INSTDIR\QtSql4.dll"
	Delete "$INSTDIR\QtSvg4.dll"
	Delete "$INSTDIR\QtWebkit4.dll"
	Delete "$INSTDIR\QtXml4.dll"	
	;------------------------------------
	; Qt imageformats
	RMDir /r "$INSTDIR\imageformats"
	;------------------------------------
SectionEnd
