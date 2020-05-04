include(../../../make/config.inc)

TARGET     = curve25519
TEMPLATE   = lib
CONFIG    -= qt
CONFIG    += staticlib warn_off
DESTDIR    = ../../../libs

INCLUDEPATH += ed25519/nacl_includes
INCLUDEPATH += ed25519/additions
INCLUDEPATH += ed25519/additions/generalized
INCLUDEPATH += ed25519/sha512
INCLUDEPATH += ed25519/tests
INCLUDEPATH += ed25519

include(curve25519.pri)
