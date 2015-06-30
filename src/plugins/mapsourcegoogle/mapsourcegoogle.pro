TARGET = mapsourcegoogle
os2:TARGET_SHORT = msgoogle
include(mapsourcegoogle.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
