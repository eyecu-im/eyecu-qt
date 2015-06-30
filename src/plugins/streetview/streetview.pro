TARGET = streetview 
os2:TARGET_SHORT = strtview
include(streetview.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo util
else: CONFIG += geo util
