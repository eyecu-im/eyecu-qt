TARGET = mapsourcemegafon
os2:TARGET_SHORT = msmegafn
include(mapsourcemegafon.pri)
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
