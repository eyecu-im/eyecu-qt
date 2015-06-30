/*
+QT Base
*/
Section "-Un.$(SecUnQtBase)" UnQtBase
SectionIn 1
  ;----------${COMPILE}  "mingw" --------------------------
; MinGW redistribute files
	Delete "$INSTDIR\libgcc_s_dw2-1.dll"
	Delete "$INSTDIR\libwinpthread-1.dll"
	Delete "$INSTDIR\libstdc++-6.dll" 
; Visual Studio redistribute files
	;Delete "$INSTDIR\msvcr90.dll"
	;Delete "$INSTDIR\msvcp90.dll"
	;Delete "$INSTDIR\Microsoft.VC90.CRT.manifest"
; Qt5.3 modules
	;Delete "$INSTDIR\icudt52.dll"
	;Delete "$INSTDIR\icuin52.dll"
	;Delete "$INSTDIR\icuuc52.dll"
; Qt Total ###### TEMP  ######
	Delete "$INSTDIR\Qt5Util.dll"
	Delete "$INSTDIR\Qt5Geo.dll"
; Qt5.4 modules
	Delete "$INSTDIR\icudt53.dll"
	Delete "$INSTDIR\icuin53.dll"
	Delete "$INSTDIR\icuuc53.dll"
; Qt total modules
	Delete "$INSTDIR\Qt5Core.dll"
	Delete "$INSTDIR\Qt5Gui.dll"
	Delete "$INSTDIR\Qt5Multimedia.dll"
	Delete "$INSTDIR\Qt5MultimediaWidgets.dll"
	Delete "$INSTDIR\Qt5Network.dll"
	Delete "$INSTDIR\Qt5OpenGL.dll"
	Delete "$INSTDIR\Qt5Positioning.dll"
	Delete "$INSTDIR\Qt5PrintSupport.dll"
	Delete "$INSTDIR\Qt5Qml.dll"
	Delete "$INSTDIR\Qt5Quick.dll"
	Delete "$INSTDIR\Qt5Script.dll"
	Delete "$INSTDIR\Qt5Sensors.dll"
	Delete "$INSTDIR\Qt5SerialPort.dll"
	Delete "$INSTDIR\Qt5Sql.dll"
	Delete "$INSTDIR\Qt5Svg.dll"
	Delete "$INSTDIR\Qt5WebChannel.dll"
	Delete "$INSTDIR\Qt5WebKit.dll"
	Delete "$INSTDIR\Qt5WebKitWidgets.dll"
	Delete "$INSTDIR\Qt5Widgets.dll"
	Delete "$INSTDIR\Qt5Xml.dll"
	;------------------------------------
	RMDir /r "$INSTDIR\platforms" 
	;------------------------------------
	; Qt imageformats
	RMDir /r "$INSTDIR\imageformats"
	;------------------------------------
SectionEnd
