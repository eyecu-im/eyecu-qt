TARGET = mapsearch 
os2:TARGET_SHORT = mpsearch
include(mapsearch.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else: CONFIG += geo util
