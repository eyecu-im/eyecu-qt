TARGET = mapsearchprovidernavitel 
os2:TARGET_SHORT = mspnavtl
include(mapsearchprovidernavitel.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo qputil
else{
QT += script
CONFIG += qpgeo qputil
}
