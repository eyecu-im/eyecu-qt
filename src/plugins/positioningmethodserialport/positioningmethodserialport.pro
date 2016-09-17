TARGET = positioningmethodserialport 
os2:TARGET_SHORT = pmserial
include(positioningmethodserialport.pri) 
include(../plugins.inc) 

greaterThan(QT_MAJOR_VERSION, 4): QT += geo serialport
else: CONFIG += serialport geo
