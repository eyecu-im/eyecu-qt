include(../../../make/config.inc)
include(QtUsageEnvironment.pri)

INCLUDEPATH  += ../UsageEnvironment/include ../groupsock/include include
TEMPLATE = lib
TARGET   = QtUsageEnvironment

CONFIG  += staticlib warn_off

SUBDIRS += QtUsageEnvironment
DESTDIR = ../../../libs
