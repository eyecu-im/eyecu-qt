TARGET = tunelistenermpris 
lessThan(QT_MAJOR_VERSION, 5): QT += dbus
include(tunelistenermpris.pri) 
include(../plugins.inc) 
