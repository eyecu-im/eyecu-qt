include(../make/config.inc)

TARGET             = $$QTLIVE_NAME
TEMPLATE           = lib
VERSION            = $$QTLIVE_ABI
CONFIG            += dll
QT                += xml network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DEFINES           += LIVE_DLL QXT_STATIC

DEPENDPATH        += ../../thirdparty/live/QtUsageEnvironment/include \
					 ../../thirdparty/live/UsageEnvironment/include \
					 ../../thirdparty/live/groupsock/include \
					 ../../thirdparty/live/liveMedia/include
INCLUDEPATH       += ../../thirdparty/live/QtUsageEnvironment/include \
					 ../../thirdparty/live/UsageEnvironment/include \
					 ../../thirdparty/live/groupsock/include \
					 ../../thirdparty/live/liveMedia/include


DESTDIR            = ../libs
win32|os2 {
  DLLDESTDIR       = ..\\..
  QMAKE_DISTCLEAN += $${DLLDESTDIR}\\$${TARGET}.dll
}

LIBS              += -L../libs
LIBS              += -lliveMedia -lgroupsock -lQtUsageEnvironment -lUsageEnvironment
win32: LIBS       += -lws2_32

macx {
  QMAKE_LFLAGS    += -framework Carbon -framework IOKit -framework Cocoa
} else:unix:!haiku {
  LIBS            += -lXss
  CONFIG          += x11
} else:win32 {
  LIBS            += -luser32 -lws2_32
}

include(qtlive.pri)
