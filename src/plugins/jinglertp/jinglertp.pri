HEADERS = \   
    jinglertp.h \
    jinglertpoptions.h \
    audio.h \
    renderarea.h \
    mediathread.h \
    $$PWD/audioinfo.h \
    $$PWD/rtppayloadtypeselector.h
SOURCES = \   
    jinglertp.cpp \
    jinglertpoptions.cpp \
    renderarea.cpp \
    audio.cpp \
    mediathread.cpp \
    $$PWD/audioinfo.cpp \
    $$PWD/rtppayloadtypeselector.cpp

FORMS += \
    jinglertpoptions.ui \
    audio.ui
