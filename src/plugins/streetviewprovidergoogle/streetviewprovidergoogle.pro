TARGET = streetviewprovidergoogle 
os2:TARGET_SHORT = svpgoogl
include(streetviewprovidergoogle.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo qputil
else: CONFIG += qpgeo qputil
