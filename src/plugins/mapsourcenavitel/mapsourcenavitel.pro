TARGET = mapsourcenavitel
os2:TARGET_SHORT = msnavitl
include(mapsourcenavitel.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
