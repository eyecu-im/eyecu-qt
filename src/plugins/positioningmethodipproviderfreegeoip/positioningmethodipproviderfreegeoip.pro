TARGET = positioningmethodipproviderfreegeoip 
os2: TARGET_SHORT = pmipfree
include(positioningmethodipproviderfreegeoip.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo qputil
else: CONFIG += qpgeo qputil
