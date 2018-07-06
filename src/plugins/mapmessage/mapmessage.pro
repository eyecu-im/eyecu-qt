TARGET = mapmessage 
os2:TARGET_SHORT = mapmessg
include(mapmessage.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
