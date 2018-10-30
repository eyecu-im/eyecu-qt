TEMPLATE    = subdirs
SUBDIRS     = zlib minizip idn qtlockedfile hunspell
SUBDIRS    += libgpg-error/gpg-error
SUBDIRS    += libgcrypt/gcrypt
SUBDIRS    += libotr
!symbian: SUBDIRS += qxtglobalshortcut
!symbian: SUBDIRS += idle
