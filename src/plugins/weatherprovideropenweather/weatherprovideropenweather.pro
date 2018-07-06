TARGET = weatherprovideropenweather 
include(weatherprovideropenweather.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qputil qpgeo
else: CONFIG += qputil qpgeo
