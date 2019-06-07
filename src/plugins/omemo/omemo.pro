TARGET = omemo
include(omemo.pri)
include(../plugins.inc)

QT          += sql
LIBS        += -L../../libs -lsignal -lgcrypt
INCLUDEPATH += ../../thirdparty/libsignal-protocol-c \
               ../../thirdparty/libgcrypt/SMP \
               ../../thirdparty/libgpg-error/SMP/src
