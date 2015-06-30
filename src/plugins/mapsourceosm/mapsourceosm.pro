TARGET = mapsourceosm
os2:TARGET_SHORT = msosm
include(mapsourceosm.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
