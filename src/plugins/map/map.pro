TARGET = map
include(map.pri)
include(../plugins.inc)
QT += network
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
