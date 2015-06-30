TARGET = simplemessagestyle
os2:TARGET_SHORT = smpmsgst
include(simplemessagestyle.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += util
else: CONFIG += util
