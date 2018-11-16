include(../../make/config.inc)

TARGET     = gcrypt
TEMPLATE   = lib
CONFIG    -= qt
CONFIG    += staticlib warn_off

DESTDIR    = ../../libs

LIBGPG_ERROR_DIR = $$PWD/../libgpg-error/SMP/src

LIBS += -L$$LIBGPG_ERROR_DIR  -lgpg-error

INCLUDEPATH += SMP src SMP/mpi mpi SMP/cipher $$LIBGPG_ERROR_DIR
DEFINES += HAVE_CONFIG_H 

include(libgcrypt.pri)
