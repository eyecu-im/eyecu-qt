TARGET = xhtmlim
include(xhtmlim.pri) 
include(../plugins.inc) 
QT += network
greaterThan(QT_MAJOR_VERSION, 4): QT += util
else: CONFIG += util
