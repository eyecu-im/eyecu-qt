TARGET = placeviewprovidergoogle 
os2:TARGET_SHORT = pvpgoogl
include(placeviewprovidergoogle.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += util geo
else: CONFIG += util geo

