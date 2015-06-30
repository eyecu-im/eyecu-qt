TARGET = mapsearchproviderhere 
os2:TARGET_SHORT = msphere
include(mapsearchproviderhere.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else: CONFIG += geo util
