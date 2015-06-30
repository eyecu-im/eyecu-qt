TARGET = mapsearchprovidergoogle 
os2:TARGET_SHORT = mspgoogl
include(mapsearchprovidergoogle.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else: CONFIG += geo util
