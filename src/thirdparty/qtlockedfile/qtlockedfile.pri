HEADERS = qtlockedfile.h
SOURCES = qtlockedfile.cpp
unix:SOURCES += $$PWD/qtlockedfile_unix.cpp
win32:SOURCES += $$PWD/qtlockedfile_win.cpp
os2:SOURCES += $$PWD/qtlockedfile_os2.cpp