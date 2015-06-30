TEMPLATE    = subdirs
SUBDIRS     = zlib minizip idn qtlockedfile hunspell
CONFIG(debug, debug|release)!symbian: SUBDIRS += live
!symbian: SUBDIRS += qxtglobalshortcut
!symbian: SUBDIRS += idle
