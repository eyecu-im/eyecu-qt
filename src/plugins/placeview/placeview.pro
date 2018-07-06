TARGET = placeview 
os2:TARGET_SHORT = plceview
include(placeview.pri) 
include(../plugins.inc) 
QT += webkit
greaterThan(QT_MAJOR_VERSION, 4): QT += webkitwidgets qpgeo qputil
else: CONFIG += qpgeo qputil

