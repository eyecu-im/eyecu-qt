TARGET = mapsourcekosmosnimki
os2:TARGET_SHORT = mskosmos
include(mapsourcekosmosnimki.pri)
include(../plugins.inc)
greaterThan(QT_MAJOR_VERSION, 4): QT += qpgeo
else: CONFIG += qpgeo
