TARGET = mapsearchproviderosm 
os2:TARGET_SHORT = msposm
include(mapsearchproviderosm.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo qputil
else: CONFIG += qpgeo qputil
