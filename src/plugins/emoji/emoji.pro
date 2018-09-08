TARGET = emoji
include(emoji.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets qputil
else {
    QT += script
    CONFIG += qputil
}
