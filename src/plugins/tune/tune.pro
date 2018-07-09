TARGET = tune 
include(tune.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
