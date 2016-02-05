TARGET = emoji
include(emoji.pri)
include(../plugins.inc)
CONFIG += util
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
else: QT += script # TODO: Use JSON parser instead of Qt Script in Qt5
