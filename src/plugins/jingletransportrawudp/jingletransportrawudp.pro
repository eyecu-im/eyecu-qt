TARGET = jingletransportrawudp
OS2:TARGET_SHORT = jtrawudp
greaterThan(QT_MAJOR_VERSION, 4): QT += qputil
else: CONFIG += qputil
include(jingletransportrawudp.pri) 
include(../plugins.inc)
