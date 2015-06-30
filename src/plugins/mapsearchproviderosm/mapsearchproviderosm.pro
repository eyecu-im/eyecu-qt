TARGET = mapsearchproviderosm 
os2:TARGET_SHORT = msposm
include(mapsearchproviderosm.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else: CONFIG += geo util
