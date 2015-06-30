TARGET = poi 
include(poi.pri) 
include(../plugins.inc) 
QT += svg
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
