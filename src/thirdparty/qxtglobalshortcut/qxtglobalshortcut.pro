include(../../make/config.inc)

TARGET         = qxtglobalshortcut
TEMPLATE       = lib
CONFIG        += staticlib warn_off
DESTDIR        = ../../libs
DEFINES       += QXT_STATIC
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
INCLUDEPATH   += ../..
include(qxtglobalshortcut.pri)
