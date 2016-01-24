TARGET = emoji
include(emoji.pri)
include(../plugins.inc)
QT += script # TODO: Use JSON parser instead of Qt Script in Qt5
greaterThan(QT_MAJOR_VERSION, 4): QT += util widgets
else: CONFIG += util
