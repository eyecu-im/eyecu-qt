TARGET = saslauth
include(saslauth.pri)
include(../plugins.inc)

greaterThan(QT_MAJOR_VERSION, 4): QT += qputil
else: CONFIG += qputil
