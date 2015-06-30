TARGET = mapsource2gis
os2:TARGET_SHORT = ms2gis
include(mapsource2gis.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
