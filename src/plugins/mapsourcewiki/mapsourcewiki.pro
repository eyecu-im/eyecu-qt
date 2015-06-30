TARGET = mapsourcewiki
os2:TARGET_SHORT = mswiki
include(mapsourcewiki.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
