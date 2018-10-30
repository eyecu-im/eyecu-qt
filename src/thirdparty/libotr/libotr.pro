TEMPLATE = lib
TARGET = otr
CONFIG    -= qt
CONFIG += warn_off

DESTDIR = ../../libs

LIBS += -lgpg-error -lgcrypt

CONFIG += staticlib

INCLUDEPATH += src

include(libotr.pri)
