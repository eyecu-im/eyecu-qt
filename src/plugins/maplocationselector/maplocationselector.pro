TARGET = maplocationselector 
os2:TARGET_SHORT = maplcsel
include(maplocationselector.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
