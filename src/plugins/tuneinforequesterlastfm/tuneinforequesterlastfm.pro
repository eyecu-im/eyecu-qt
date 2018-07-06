TARGET = tuneinforequesterlastfm 
os2:TARGET_SHORT = tirlstfm
include(tuneinforequesterlastfm.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qputil
else: CONFIG += qputil
