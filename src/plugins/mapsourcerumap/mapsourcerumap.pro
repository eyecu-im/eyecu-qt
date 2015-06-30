TARGET = mapsourcerumap 
os2: TARGET_SHORT = msrumap
include(mapsourcerumap.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
