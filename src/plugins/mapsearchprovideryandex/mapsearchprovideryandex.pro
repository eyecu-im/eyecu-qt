TARGET = mapsearchprovideryandex 
os2:TARGET_SHORT = mspyandx
include(mapsearchprovideryandex.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else: CONFIG += geo util
