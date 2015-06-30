include(../../../make/config.inc)
include(UsageEnvironment.pri)

INCLUDEPATH   += ../groupsock/include include

TEMPLATE = lib
TARGET   = UsageEnvironment

CONFIG  += staticlib warn_off

SUBDIRS += UsageEnvironment
DESTDIR = ../../../libs
