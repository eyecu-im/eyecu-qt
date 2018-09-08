TARGET = jingletransporticeudp 
OS2:TARGET_SHORT = jticeudp
include(jingletransporticeudp.pri) 
include(../plugins.inc) 

greaterThan(QT_MAJOR_VERSION, 4): QT += qpice
else: CONFIG += qpice
