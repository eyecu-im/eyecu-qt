TARGET = mapsearchprovidernavitel 
os2:TARGET_SHORT = mspnavtl
include(mapsearchprovidernavitel.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else{
QT += script
CONFIG += geo util
}
