TARGET = mapsearchprovidernavitel 
os2:TARGET_SHORT = mspnavtl
include(mapsearchprovidernavitel.pri) 
include(../plugins.inc) 
QT += script
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else: CONFIG += geo util
