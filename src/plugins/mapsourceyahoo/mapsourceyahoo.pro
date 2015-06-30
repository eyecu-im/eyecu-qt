TARGET = mapsourceyahoo
os2:TARGET_SHORT = msyahoo
include(mapsourceyahoo.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
