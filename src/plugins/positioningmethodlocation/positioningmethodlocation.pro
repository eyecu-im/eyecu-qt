TARGET = positioningmethodlocation
os2:TARGET_SHORT = pmlocatn
include(positioningmethodlocation.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += location geo
else {
CONFIG += mobility geo
MOBILITY = location
}

