TARGET = jinglertp 
OS2:TARGET_SHORT = jinglrtp
include(jinglertp.pri) 
include(../plugins.inc) 

contains(QT_CONFIG, multimediakit): {
CONFIG += mobility
MOBILITY += multimedia
}
else: contains(QT_CONFIG, multimedia): QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += qpffmpeg
else: CONFIG += qpffmpeg
