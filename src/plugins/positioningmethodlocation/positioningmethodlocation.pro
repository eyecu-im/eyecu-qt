TARGET = positioningmethodlocation
os2:TARGET_SHORT = pmlocatn
include(positioningmethodlocation.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += location qpgeo
else {
CONFIG += mobility qpgeo
MOBILITY = location
}

