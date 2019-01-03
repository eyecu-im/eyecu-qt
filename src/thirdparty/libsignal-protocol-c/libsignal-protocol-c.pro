include(../../make/config.inc)

include(libsignal-protocol-c.pri)

TEMPLATE = lib
TARGET   = signal

CONFIG  -= qt
CONFIG  += ordered
CONFIG  += staticlib warn_off
SUBDIRS += curve25519 protobuf-c
DESTDIR = ../../libs

INCLUDEPATH += curve25519/ed25519/nacl_includes
INCLUDEPATH += curve25519/ed25519/additions
INCLUDEPATH += curve25519/ed25519/additions/generalized
INCLUDEPATH += curve25519/ed25519/sha512
INCLUDEPATH += curve25519/ed25519
INCLUDEPATH += curve25519
