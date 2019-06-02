TARGET = otr
LIBS += -lotr -lgcrypt -lgpg-error
win32: LIBS    += -lWs2_32 -lAdvapi32 -luser32
INCLUDEPATH += ../../thirdparty/libotr/src \
               ../../thirdparty/libgcrypt/SMP \
               ../../thirdparty/libgpg-error/SMP/src
greaterThan(QT_MAJOR_VERSION,4): QT += concurrent
include(otr.pri)
include(../plugins.inc)
