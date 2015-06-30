TARGET = mapsourcerosreestr 
os2:TARGET_SHORT = msrosrsr
include(mapsourcerosreestr.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
