/*
+QT Base
*/
Section -$(SecQtBase) QtBase
SectionIn 1 2 3 
	; Set output path to the installation directory.
	SetOutPath $INSTDIR
  ;------------------------------------${COMPILE}  "mingw"
; MinGW redistribute files
	File "${PROGRAM_BIN_FOLDER}\libgcc_s_dw2-1.dll"  ;libgcc_s_dw2-1.dll
	File "${PROGRAM_BIN_FOLDER}\libwinpthread-1.dll"
	File "${PROGRAM_BIN_FOLDER}\libstdc++-6.dll" 
; Visual Studio redistribute files
	;File "${PROGRAM_BIN_FOLDER}\msvcr90.dll"
	;File "${PROGRAM_BIN_FOLDER}\msvcp90.dll"
	;File "${PROGRAM_BIN_FOLDER}\Microsoft.VC90.CRT.manifest"
; Qt5.3 modules
	;File "${PROGRAM_BIN_FOLDER}\icudt52.dll"
	;File "${PROGRAM_BIN_FOLDER}\icuin52.dll"
	;File "${PROGRAM_BIN_FOLDER}\icuuc52.dll"
; Qt Total ###### TEMP  ######
	File "${PROGRAM_BIN_FOLDER}\Qt5Util.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Geo.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5FFMpeg.dll"
; Qt5.4 modules
	File "${PROGRAM_BIN_FOLDER}\icudt53.dll"
	File "${PROGRAM_BIN_FOLDER}\icuin53.dll"
	File "${PROGRAM_BIN_FOLDER}\icuuc53.dll"
; Qt total modules
	File "${PROGRAM_BIN_FOLDER}\Qt5Core.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Gui.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Multimedia.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5MultimediaWidgets.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Network.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5OpenGL.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Positioning.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5PrintSupport.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Qml.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Quick.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Script.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Sensors.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5SerialPort.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Sql.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Svg.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5WebChannel.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5WebKit.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5WebKitWidgets.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Widgets.dll"
	File "${PROGRAM_BIN_FOLDER}\Qt5Xml.dll"
	;------------------------------------
	SetOutPath $INSTDIR\platforms
	File "${PROGRAM_BIN_FOLDER}\platforms\*.*" 
	;------------------------------------
	; Qt imageformats
	SetOutPath $INSTDIR\imageformats
	File "${PROGRAM_BIN_FOLDER}\imageformats\*.*"
	;------------------------------------

SectionEnd
