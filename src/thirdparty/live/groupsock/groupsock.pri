INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD


HEADERS += include/NetInterface.hh\
           include/NetCommon.h\
           include/NetAddress.hh\
           include/IOHandlers.hh\
           include/GroupsockHelper.hh\
           include/groupsock_version.hh\
           include/Groupsock.hh\
           include/GroupEId.hh\
           include/TunnelEncaps.hh

SOURCES += NetInterface.cpp\
    NetAddress.cpp\
    IOHandlers.cpp\
    inet.c\
    GroupsockHelper.cpp\
    Groupsock.cpp\
    GroupEId.cpp

