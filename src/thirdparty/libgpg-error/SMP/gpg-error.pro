TEMPLATE = lib
TARGET = gpg-error
CONFIG += warn_off

DESTDIR = $$PWD/../../bin

INCLUDEPATH += . ./src
DEFINES += HAVE_CONFIG_H

win32 {
    DEF_FILE = libgpg-error.def
    CONFIG += shared dll
} else {
    CONFIG += staticlib
}

HEADERS = \
    ./src/gpg-error.h \
    ../src/estream-printf.h \
    ../src/estream.h \
    ../src/gettext.h \
    ../src/gpgrt-int.h \
    ../src/init.h \
    ../src/lock.h \
    ../src/thread.h \
    ../src/visibility.h \
    compat.h \
    config.h \
    version.h 

SOURCES = \
    ../src/code-from-errno.c \
    ../src/code-to-errno.c \
    ../src/estream-printf.c \
    ../src/estream.c \
    ../src/gpg-error.c \
    ../src/init.c \
    ../src/strerror-sym.c \
    ../src/strerror.c \
    ../src/strsource-sym.c \
    ../src/strsource.c \
    ../src/version.c \
    ../src/visibility.c


win32 {
    HEADERS += \
        ../src/w32-add.h \
        ../src/w32-lock-obj.h

    SOURCES += \
        ../src/w32-gettext.c \
        ../src/w32-iconv.c \
        ../src/w32-lock.c \
        ../src/w32-thread.c
} else {
    HEADERS += \
        ../src/posix-lock-obj.h

    SOURCES += \
        ../src/posix-lock.c \
        ../src/posix-thread.c
}

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

copyToDestdir($$PWD/src/gpg-error.h)
