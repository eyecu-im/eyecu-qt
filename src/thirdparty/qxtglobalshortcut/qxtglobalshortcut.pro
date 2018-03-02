include(../../make/config.inc)

TARGET         = qxtglobalshortcut
TEMPLATE       = lib
CONFIG        += staticlib warn_off
DESTDIR        = ../../libs
DEFINES       += QXT_STATIC
INCLUDEPATH   += ../..
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
unix:!macx:!haiku {
  QT          += x11extras
}
include(qxtglobalshortcut.pri)
