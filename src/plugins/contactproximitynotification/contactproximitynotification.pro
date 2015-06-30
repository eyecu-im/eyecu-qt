TARGET = contactproximitynotification 
os2:TARGET_SHORT = conproxn
include(contactproximitynotification.pri) 
include(../plugins.inc) 
greaterThan(QT_MAJOR_VERSION, 4): QT += geo
else: CONFIG += geo
