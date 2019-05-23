HEADERS += \
    SMP/src/gpg-error.h \
    src/estream-printf.h \
    src/estream.h \
    src/gettext.h \
    src/gpgrt-int.h \
    src/init.h \
    src/lock.h \
    src/thread.h \
    src/visibility.h \
    SMP/compat.h \
    SMP/config.h \
    SMP/version.h 

SOURCES += \
    src/argparse.c \
    src/b64dec.c \
    src/b64enc.c \
    src/logging.c \
    src/syscall-clamp.c \
    src/sysutils.c \
    src/w32-estream.c \
    src/w32-reg.c \
    src/code-from-errno.c \
    src/code-to-errno.c \
    src/estream-printf.c \
    src/estream.c \
    src/gpg-error.c \
    src/init.c \
    src/strerror-sym.c \
    src/strerror.c \
    src/strsource-sym.c \
    src/strsource.c \
    src/version.c \
    src/visibility.c


win32 {
    HEADERS += \
        src/w32-add.h \
        src/w32-lock-obj.h

    SOURCES += \
        src/w32-gettext.c \
        src/w32-iconv.c \
        src/w32-lock.c \
        src/w32-thread.c \
        src/spawn-w32.c
} else {
    HEADERS += \
        src/posix-lock-obj.h

    SOURCES += \
        src/posix-lock.c \
        src/posix-thread.c \
        src/spawn-posix.c
}

DISTFILES += \
    src/gpg-error-config-test.sh \
    src/gpg-error.pc.in \
    src/gpgrt-config \
    src/gpgrt.m4
