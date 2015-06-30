TARGET = notifications 
os2:TARGET_SHORT = notifics
greaterThan(QT_MAJOR_VERSION, 4): QT    += multimedia
include(notifications.pri)
include(../plugins.inc)
