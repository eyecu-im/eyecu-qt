include(../../make/config.inc)

TARGET     = gpg-error
TEMPLATE   = lib
CONFIG    -= qt
CONFIG    += staticlib warn_off
DESTDIR    = ../../libs

INCLUDEPATH += SMP SMP/src
DEFINES += HAVE_CONFIG_H

include(libgpg-error.pri)

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

copyToDestdir($$PWD/SMP/src/gpg-error.h)
