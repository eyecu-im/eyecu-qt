include(../../../make/config.inc)
include(groupsock.pri)

INCLUDEPATH   += ../UsageEnvironment/include include

TEMPLATE = lib
TARGET   = groupsock

QMAKE_CXXFLAGS += -fpermissive
CONFIG  += staticlib warn_off
os2: DEFINES += USE_GETHOSTBYNAME
SUBDIRS += groupsock
DESTDIR = ../../../libs
