TARGET = abbreviations 
os2: TARGET_SHORT = abbrvtns
include(abbreviations.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += util
else: CONFIG += util
