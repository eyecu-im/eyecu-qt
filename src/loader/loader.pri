FORMS   = setuppluginsdialog.ui

anonymous:  FORMS += aboutbox_anonymous.ui
!anonymous: FORMS += aboutbox.ui

HEADERS = pluginmanager.h \
          aboutbox.h \
    setuppluginsdialog.h

SOURCES = main.cpp \
          pluginmanager.cpp \
          aboutbox.cpp \
    setuppluginsdialog.cpp

