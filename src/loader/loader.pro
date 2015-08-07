include(../make/config.inc)
TARGET             = $$EYECU_LOADER_NAME
TEMPLATE           = app
QT                += xml svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS              += -L../libs
LIBS              += -l$$EYECU_UTILS_NAME
DEPENDPATH        += ..
INCLUDEPATH       += ..
DESTDIR            = ../..

DEFINES           += EXTRA_TRANSLATORS=\\\"$$EYECU_LOADER_NAME;$$EYECU_UTILS_NAME;qtgeo;\\\"

include(loader.pri)
#Appication icon
win32:RC_FILE      = loader.rc
os2:  RC_FILE      = loader2.rc

symbian {
    TARGET.UID3 = 0xe3837d66
    # TARGET.CAPABILITY +=
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

#SVN Info
#isEmpty(SVN_REVISION) {
#  SVN_REVISION=$$system(svnversion -n -c ./../../)
#  SVN_REVISION_INVALID = $$find(SVN_REVISION,Unversioned) $$find(SVN_REVISION,exported)
#}

#win32|os2|symbian {
#  WIN_OUT_PWD = $$replace(OUT_PWD, /, \\)
#  exists($${WIN_OUT_PWD}\\svninfo.h):system(del $${WIN_OUT_PWD}\\svninfo.h)
#  !isEmpty(SVN_REVISION):count(SVN_REVISION_INVALID,0) {
#    system(mkdir $${WIN_OUT_PWD} & echo $${LITERAL_HASH}define SVN_REVISION \"$${SVN_REVISION}\" >> $${WIN_OUT_PWD}\\svninfo.h) {
#      DEFINES         += SVNINFO
#      QMAKE_DISTCLEAN += $${OUT_PWD}/svninfo.h
#    }
#  }
#} else {
#  exists($${OUT_PWD}/svninfo.h):system(rm -f $${OUT_PWD}/svninfo.h)
#  !isEmpty(SVN_REVISION)::count(SVN_REVISION_INVALID,0) {
#    system(mkdir -p $${OUT_PWD} && echo \\$${LITERAL_HASH}define SVN_REVISION \\\"$${SVN_REVISION}\\\" >> $${OUT_PWD}/svninfo.h) {
#      DEFINES         += SVNINFO
#      QMAKE_DISTCLEAN += $${OUT_PWD}/svninfo.h
#    }
#  }
#}

#GIT Info
GIT_HASH = $$system(git log -n 1 --format=%H)
GIT_DATE = $$system(git log -n 1 --format=%ct)
GIT_DATE = $$find(GIT_DATE,^\\d*)
!isEmpty(GIT_DATE) {
  win32|os2|symbian {
    WIN_OUT_PWD = $$replace(OUT_PWD, /, \\)
    system(mkdir $${WIN_OUT_PWD} & echo $${LITERAL_HASH}define GIT_HASH \"$${GIT_HASH}\" > $${WIN_OUT_PWD}\\gitinfo.h) {
      system(echo $${LITERAL_HASH}define GIT_DATE \"$${GIT_DATE}\" >> $${WIN_OUT_PWD}\\gitinfo.h)
      DEFINES         += GITINFO
      QMAKE_DISTCLEAN += $${OUT_PWD}/gitinfo.h
    }
  } else {
    system(mkdir -p $${OUT_PWD} && echo \\$${LITERAL_HASH}define GIT_HASH \\\"$${GIT_HASH}\\\" > $${OUT_PWD}/gitinfo.h) {
      system(echo \\$${LITERAL_HASH}define GIT_DATE \\\"$${GIT_DATE}\\\" >> $${OUT_PWD}/gitinfo.h)
      DEFINES         += GITINFO
      QMAKE_DISTCLEAN += $${OUT_PWD}/gitinfo.h
    }
  }
}

#Install
target.path        = $$INSTALL_BINS
resources.path     = $$INSTALL_RESOURCES
resources.files    = ../../resources/*
documents.path     = $$INSTALL_DOCUMENTS
documents.files    = ../../AUTHORS ../../CHANGELOG ../../README ../../COPYING ../../TRANSLATORS
INSTALLS           += target resources documents

#Translation
TRANS_BUILD_ROOT   = $${OUT_PWD}/../..
TRANS_SOURCE_ROOT  = ..
include(../translations/languages.inc)

#Linux desktop install
unix:!macx {
  icons.path       = $$INSTALL_PREFIX/$$INSTALL_RES_DIR/pixmaps
  icons.files      = ../../resources/menuicons/shared/vacuum.png
  INSTALLS        += icons

  desktop.path     = $$INSTALL_PREFIX/$$INSTALL_RES_DIR/applications
  desktop.files    = ../../src/packages/linux/*.desktop
  INSTALLS        += desktop
}

#MacOS Install
macx {
  UTILS_LIB_NAME   = lib$${EYECU_UTILS_NAME}.$${EYECU_UTILS_ABI}.dylib
  UTILS_LIB_LINK   = lib$${EYECU_UTILS_NAME}.dylib

  lib_utils.path   = $$INSTALL_LIBS
  lib_utils.extra  = cp -f ../libs/$$UTILS_LIB_NAME $(INSTALL_ROOT)$$INSTALL_LIBS/$$UTILS_LIB_NAME && \
                     ln -sf $$UTILS_LIB_NAME $(INSTALL_ROOT)$$INSTALL_LIBS/$$UTILS_LIB_LINK
  INSTALLS        += lib_utils

  name_tool.path   = $$INSTALL_BINS
  name_tool.extra  = install_name_tool -change $$UTILS_LIB_NAME @executable_path/../Frameworks/$$UTILS_LIB_NAME $(INSTALL_ROOT)$$INSTALL_BINS/$$INSTALL_APP_DIR/Contents/MacOS/$$VACUUM_LOADER_NAME
  INSTALLS        += name_tool

  sdk_utils.path   = $$INSTALL_INCLUDES/utils
  sdk_utils.files  = ../utils/*.h
  INSTALLS        += sdk_utils

  #Dirty hack to install utils translations
  TARGET           = $$EYECU_UTILS_NAME
  include(../translations/languages.inc)
  TARGET           = $$EYECU_LOADER_NAME

  ICON              = ../../eyecu.icns
  QMAKE_INFO_PLIST  = ../packages/macosx/Info.plist

  en_lproj.path     = $$INSTALL_RESOURCES/en.lproj/
  en_lproj.files    = ../packages/macosx/InfoPlist.strings
  INSTALLS         += en_lproj
}
