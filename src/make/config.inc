#Loader and library names
EYECU_LOADER_NAME      = eyecu
EYECU_UTILS_NAME       = eyecuutils
os2:EYECU_UTILS_SNAME  = eyecutls
QTLIVE_NAME            = qtlive

#Library ABI version
unix {
QTLIVE_ABI      = 1
EYECU_UTILS_ABI = 36
}

#Build config
CONFIG(debug, debug|release) {
  DEFINES               += DEBUG_MODE
}

#Configurable values
isEmpty(INSTALL_PREFIX) {
  win32:INSTALL_PREFIX   = "c:"
  os2:	INSTALL_PREFIX   = "c:"
  symbian: INSTALL_PREFIX  = "c:"
  unix:!symbian: INSTALL_PREFIX   = /usr/local
  macx: INSTALL_PREFIX   = /Applications
  haiku:INSTALL_PREFIX   = /boot/apps
}

isEmpty(INSTALL_APP_DIR) {
  os2:		INSTALL_APP_DIR  = eyecu
  win32:	INSTALL_APP_DIR  = eyecu  
  unix:		INSTALL_APP_DIR  = eyecu
  macx:		INSTALL_APP_DIR  = eyecu.app
  symbian:	INSTALL_APP_DIR  = /private/0xe3837d66
  haiku:	INSTALL_APP_DIR  = eyeCU
}

isEmpty(INSTALL_LIB_DIR) {
  os2:		INSTALL_LIB_DIR  =
  win32:	INSTALL_LIB_DIR  = .
  unix:		INSTALL_LIB_DIR  = lib
  macx:		INSTALL_LIB_DIR  = Frameworks
  symbian:	INSTALL_LIB_DIR  =
  haiku:	INSTALL_LIB_DIR  = .
}

isEmpty(INSTALL_RES_DIR) {
  os2:		INSTALL_RES_DIR  =
  win32:	INSTALL_RES_DIR  = .
  unix:		INSTALL_RES_DIR  = share
  macx:		INSTALL_RES_DIR  = Resources
  symbian:	INSTALL_RES_DIR  = resource/eyeCU
  haiku:	INSTALL_RES_DIR  = .
}

isEmpty(INSTALL_SDK_DIR) {
  os2:  INSTALL_SDK_DIR  = sdk
  win32:INSTALL_SDK_DIR  = sdk
  unix: INSTALL_SDK_DIR  = include
  macx: INSTALL_SDK_DIR  = include
  haiku:INSTALL_SDK_DIR  = sdk
}


#Resources directories
!macx:contains(DEFINES,DEBUG_MODE) {
  DEFINES               += PLUGINS_DIR='\\\"./plugins\\\"'
  DEFINES               += RESOURCES_DIR='\\\"$${PWD}/../../resources\\\"'
  DEFINES               += TRANSLATIONS_DIR='\\\"./src/translations\\\"'
} else:win32 {
  DEFINES               += PLUGINS_DIR='\\\"./$${INSTALL_LIB_DIR}/plugins\\\"'
  DEFINES               += RESOURCES_DIR='\\\"./$${INSTALL_RES_DIR}/resources\\\"'
  DEFINES               += TRANSLATIONS_DIR='\\\"./$${INSTALL_RES_DIR}/translations\\\"'
} else:os2 {
  DEFINES               += PLUGINS_DIR='\\\"$${INSTALL_LIB_DIR}plugins\\\"'
  DEFINES               += RESOURCES_DIR='\\\"$${INSTALL_RES_DIR}resources\\\"'
  DEFINES               += TRANSLATIONS_DIR='\\\"$${INSTALL_RES_DIR}translations\\\"'
} else:haiku {
  DEFINES               += PLUGINS_DIR='\\\"./$${INSTALL_LIB_DIR}/plugins\\\"'
  DEFINES               += RESOURCES_DIR='\\\"./$${INSTALL_RES_DIR}/resources\\\"'
  DEFINES               += TRANSLATIONS_DIR='\\\"./$${INSTALL_RES_DIR}/translations\\\"'
} else:macx {
  DEFINES               += PLUGINS_DIR='\\\"../PlugIns\\\"'
  DEFINES               += RESOURCES_DIR='\\\"../$${INSTALL_RES_DIR}\\\"'
  DEFINES               += TRANSLATIONS_DIR='\\\"../$${INSTALL_RES_DIR}/translations\\\"'
} else:symbian {
  DEFINES               += PLUGINS_DIR='\\\"$${INSTALL_LIB_DIR}plugins\\\"'
  DEFINES               += RESOURCES_DIR='\\\"$${INSTALL_PREFIX}/$${INSTALL_RES_DIR}/resources\\\"'
  DEFINES               += TRANSLATIONS_DIR='\\\"$${INSTALL_PREFIX}/$${INSTALL_RES_DIR}/translations\\\"'
} else:unix {
  DEFINES               += PLUGINS_DIR='\\\"../$${INSTALL_LIB_DIR}/$${INSTALL_APP_DIR}/plugins\\\"'
  DEFINES               += RESOURCES_DIR='\\\"../$${INSTALL_RES_DIR}/$${INSTALL_APP_DIR}/resources\\\"'
  DEFINES               += TRANSLATIONS_DIR='\\\"../$${INSTALL_RES_DIR}/$${INSTALL_APP_DIR}/translations\\\"'
}


#Install directories
win32|os2 {
  INSTALL_BINS           = $$INSTALL_PREFIX/$$INSTALL_APP_DIR
  INSTALL_LIBS           = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_LIB_DIR
  INSTALL_INCLUDES       = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_SDK_DIR
  INSTALL_PLUGINS        = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_LIB_DIR/plugins
  INSTALL_RESOURCES      = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_RES_DIR/resources
  INSTALL_DOCUMENTS      = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_RES_DIR
  INSTALL_TRANSLATIONS   = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_RES_DIR/translations
} else:haiku {
  INSTALL_BINS           = $$INSTALL_PREFIX/$$INSTALL_APP_DIR
  INSTALL_LIBS           = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_LIB_DIR
  INSTALL_INCLUDES       = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_SDK_DIR
  INSTALL_PLUGINS        = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_LIB_DIR/plugins
  INSTALL_RESOURCES      = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_RES_DIR/resources
  INSTALL_DOCUMENTS      = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_RES_DIR/doc
  INSTALL_TRANSLATIONS   = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/$$INSTALL_RES_DIR/translations
} else:macx {
  INSTALL_BINS           = $$INSTALL_PREFIX
  INSTALL_LIBS           = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/Contents/$$INSTALL_LIB_DIR
  INSTALL_INCLUDES       = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/Contents/$$INSTALL_RES_DIR/$$INSTALL_SDK_DIR
  INSTALL_PLUGINS        = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/Contents/PlugIns
  INSTALL_RESOURCES      = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/Contents/$$INSTALL_RES_DIR
  INSTALL_DOCUMENTS      = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/Contents/$$INSTALL_RES_DIR
  INSTALL_TRANSLATIONS   = $$INSTALL_PREFIX/$$INSTALL_APP_DIR/Contents/$$INSTALL_RES_DIR/translations
} else:unix {
  INSTALL_BINS           = $$INSTALL_PREFIX/bin
  INSTALL_LIBS           = $$INSTALL_PREFIX/$$INSTALL_LIB_DIR
  INSTALL_INCLUDES       = $$INSTALL_PREFIX/$$INSTALL_SDK_DIR/$$INSTALL_APP_DIR
  INSTALL_PLUGINS        = $$INSTALL_PREFIX/$$INSTALL_LIB_DIR/$$INSTALL_APP_DIR/plugins
  INSTALL_RESOURCES      = $$INSTALL_PREFIX/$$INSTALL_RES_DIR/$$INSTALL_APP_DIR/resources
  INSTALL_DOCUMENTS      = $$INSTALL_PREFIX/$$INSTALL_RES_DIR/doc/$$INSTALL_APP_DIR
  INSTALL_TRANSLATIONS   = $$INSTALL_PREFIX/$$INSTALL_RES_DIR/$$INSTALL_APP_DIR/translations
}
