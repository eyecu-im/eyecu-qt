TARGET = positioningmethodmanual
os2:TARGET_SHORT = pmmanual
include(positioningmethodmanual.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo qputil
else: CONFIG += qpgeo qputil
