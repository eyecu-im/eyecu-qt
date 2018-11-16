include(../../make/config.inc)

TARGET     = gcrypt
TEMPLATE   = lib
CONFIG    -= qt
CONFIG    += staticlib warn_off

message("App Binary architecture: $$QT_ARCH")

DESTDIR    = ../../libs

LIBGPG_ERROR_DIR = $$PWD/../libgpg-error/SMP/src

LIBS += -L$$LIBGPG_ERROR_DIR  -lgpg-error

INCLUDEPATH += SMP src SMP/mpi mpi SMP/cipher $$LIBGPG_ERROR_DIR
DEFINES += HAVE_CONFIG_H 

include(libgcrypt.pri)

#
# Copies the given files to the destination directory
# http://stackoverflow.com/questions/3984104/qmake-how-to-copy-a-file-to-the-output
#
defineTest(copyToDestdir) {
    files = $$1

    for(FILE, files) {
        DDIR = $$DESTDIR

        # Replace slashes in paths with backslashes for Windows
        win32:FILE ~= s,/,\\,g
        win32:DDIR ~= s,/,\\,g

        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)
    }

    export(QMAKE_POST_LINK)
}

copyToDestdir($$PWD/SMP/gcrypt.h)
