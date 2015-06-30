QT += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport
TARGET = messagearchiver
os2:TARGET_SHORT = msgarchv
include(messagearchiver.pri)
include(../plugins.inc)
