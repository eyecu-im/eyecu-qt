TARGET = mapsourcevitel 
os2:TARGET_SHORT = msvitel
include(mapsourcevitel.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
