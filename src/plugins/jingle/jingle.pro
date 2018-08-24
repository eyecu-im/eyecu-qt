TARGET = jingle 
include(jingle.pri) 
include(../plugins.inc) 

greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
else: win32-g++:QMAKE_CXXFLAGS += -std=c++11
