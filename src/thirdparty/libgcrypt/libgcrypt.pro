include(../../make/config.inc)

TARGET     = gcrypt
TEMPLATE   = lib
CONFIG    -= qt
CONFIG    += dll warn_off

DESTDIR    = ../../libs

LIBGPG_ERROR_DIR = $$PWD/../libgpg-error/SMP/src

LIBS += -L../../libs  -lgpg-error
win32: LIBS += -lWs2_32 -lAdvapi32 -luser32

INCLUDEPATH += SMP src SMP/mpi mpi SMP/cipher $$LIBGPG_ERROR_DIR
DEFINES += HAVE_CONFIG_H 

include(libgcrypt.pri)
