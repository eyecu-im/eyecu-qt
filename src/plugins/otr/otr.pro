TARGET = otr
include(../plugins.inc)
LIBS += -lotr -lgcrypt -lgpg-error
INCLUDEPATH += ../../thirdparty/otr/src \
			   ../../thirdparty/gcrypt/SMP \
			   ../../thirdparty/gpg-error/SMP/src
greaterThan(QT_MAJOR_VERSION,4): QT += concurrent
include(otr.pri)
