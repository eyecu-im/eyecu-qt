include(../../../make/config.inc)
include(liveMedia.pri)

INCLUDEPATH += ../UsageEnvironment/include ../groupsock/include include
TEMPLATE = lib
TARGET   = liveMedia

CONFIG  += staticlib warn_off
DEFINES += USE_OUR_BZERO=1 \
           SOCKLEN_T=int \
           LOCALE_NOT_USED
QMAKE_CXXFLAGS += -fpermissive

DESTDIR = ../../../libs
