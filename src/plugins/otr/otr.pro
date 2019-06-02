TARGET = otr
LIBS += -lotr -lgcrypt
INCLUDEPATH += ../../thirdparty/libotr/src \
               ../../thirdparty/libgcrypt/SMP \
               ../../thirdparty/libgpg-error/SMP/src
greaterThan(QT_MAJOR_VERSION,4): QT += concurrent
include(otr.pri)
include(../plugins.inc)
