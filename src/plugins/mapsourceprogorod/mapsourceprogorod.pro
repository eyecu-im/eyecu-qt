TARGET = mapsourceprogorod 
os2:TARGET_SHORT = msprogrd
include(mapsourceprogorod.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
