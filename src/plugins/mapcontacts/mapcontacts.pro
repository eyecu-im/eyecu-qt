TARGET = mapcontacts
os2:TARGET_SHORT = mpcntcts
include(mapcontacts.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
