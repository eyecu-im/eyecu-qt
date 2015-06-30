TARGET = mapsourcenavteq 
os2:TARGET_SHORT = msnavteq
include(mapsourcenavteq.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
