TARGET = positioningmethodip 
os2: TARGET_SHORT = pmip
include(positioningmethodip.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else: CONFIG += geo util
