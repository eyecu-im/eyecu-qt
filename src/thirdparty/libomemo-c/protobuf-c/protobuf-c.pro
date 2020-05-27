include(../../../make/config.inc)

TARGET     = protobuf-c
TEMPLATE   = lib
CONFIG    -= qt
CONFIG    += staticlib warn_off
DESTDIR    = ../../../libs
include(protobuf-c.pri)
