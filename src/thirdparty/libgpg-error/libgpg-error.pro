include(../../make/config.inc)

TARGET     = gpg-error
TEMPLATE   = lib
CONFIG    -= qt
CONFIG    += staticlib warn_off
DESTDIR    = ../../libs

INCLUDEPATH += SMP SMP/src
DEFINES += HAVE_CONFIG_H \
           PKGDATADIR=""

!unix: DEFINES += MKDIR_TAKES_ONE_ARG

include(libgpg-error.pri)
