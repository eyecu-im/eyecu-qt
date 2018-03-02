TARGET = adiummessagestyle 
os2: TARGET_SHORT	= adiummsg
QT += webkit
greaterThan(QT_MAJOR_VERSION, 4): QT += webkitwidgets
QT -= phonon xmlpatterns
QT += webkitwidgets webkit
include(adiummessagestyle.pri) 
include(../plugins.inc) 
