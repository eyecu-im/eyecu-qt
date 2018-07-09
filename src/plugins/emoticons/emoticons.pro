TARGET = emoticons
os2:TARGET_SHORT = emoticon
include(emoticons.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += qputil widgets
else: CONFIG += qputil
