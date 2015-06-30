TARGET = mapsourcemailru
os2:TARGET_SHORT = msmailru
include(mapsourcemailru.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
