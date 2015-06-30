TARGET = wizardtransport 
os2:TARGET_SHORT = wtrnsprt
include(wizardtransport.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += util
else: CONFIG += util
