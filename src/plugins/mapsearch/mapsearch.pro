TARGET = mapsearch 
os2:TARGET_SHORT = mpsearch
include(mapsearch.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo qputil
else: CONFIG += qpgeo qputil
