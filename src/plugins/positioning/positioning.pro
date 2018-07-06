TARGET = positioning 
os2:TARGET_SHORT = position
include(positioning.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
