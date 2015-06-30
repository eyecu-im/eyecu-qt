TARGET = mapsearchprovider2gis 
os2:TARGET_SHORT = msp2gis
include(mapsearchprovider2gis.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else: CONFIG += geo util
