INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD
#INCLUDES = -I../groupsock

HEADERS += $$PWD/include/UsageEnvironment.hh\
    $$PWD/include/strDup.hh\
    $$PWD/include/HashTable.hh\
    $$PWD/include/Boolean.hh\
    $$PWD/include/UsageEnvironment_version.hh

SOURCES += $$PWD/UsageEnvironment.cpp\
    $$PWD/strDup.cpp\
    $$PWD/HashTable.cpp
