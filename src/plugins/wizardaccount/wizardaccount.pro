TARGET = wizardaccount
os2:TARGET_SHORT = waccount
include(wizardaccount.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += util
else: CONFIG += util
