TARGET = positioningmethodipprovideripstack
os2: TARGET_SHORT = pmipstck
include(positioningmethodipprovideripstack.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo qputil
else: CONFIG += qpgeo qputil
