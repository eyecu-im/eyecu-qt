TARGET = emoji
include(emoji.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += util widgets
else: CONFIG += util
