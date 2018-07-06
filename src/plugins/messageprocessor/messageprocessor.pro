TARGET = messageprocessor
os2:TARGET_SHORT = msgprcsr
include(messageprocessor.pri) 
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += qputil
else: CONFIG += qputil
