TARGET = omemo
include(omemo.pri)
include(../plugins.inc)

QT          += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += qputil
else: CONFIG += qputil

LIBS        += -L../../libs -lsignal -lgcrypt
INCLUDEPATH += ../../thirdparty/libsignal-protocol-c \
               ../../thirdparty/libgcrypt/SMP \
               ../../thirdparty/libgpg-error/SMP/src
