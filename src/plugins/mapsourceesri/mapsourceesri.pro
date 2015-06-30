TARGET = mapsourceesri
os2:TARGET_SHORT = msesri
include(mapsourceesri.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
