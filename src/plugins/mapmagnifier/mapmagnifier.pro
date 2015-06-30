TARGET = mapmagnifier 
os2:TARGET_SHORT = mpmagnif
include(mapmagnifier.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
