TARGET = mapsourcebing
os2:TARGET_SHORT = msbing
include(mapsourcebing.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
