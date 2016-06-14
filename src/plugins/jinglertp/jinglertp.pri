HEADERS = \   
    jinglertp.h \
    audio.h \
    renderarea.h \
    mediathread.h \
    audioinfo.h \
    rtppayloadtypeselector.h \
    payloadtypeeditdialog.h \
	audiooptions.h \
    $$PWD/payloadtypeoptions.h

SOURCES = \   
    jinglertp.cpp \
    renderarea.cpp \
    audio.cpp \
    mediathread.cpp \
    audioinfo.cpp \
    rtppayloadtypeselector.cpp \
    payloadtypeeditdialog.cpp \
	audiooptions.cpp \
    $$PWD/payloadtypeoptions.cpp

FORMS += \
    audio.ui \
    payloadtypeeditdialog.ui \
	audiooptions.ui \
    $$PWD/payloadtypeoptions.ui
