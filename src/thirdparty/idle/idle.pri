HEADERS += idle.h
SOURCES += idle.cpp

unix:!macx: {
	SOURCES += idle_x11.cpp
} else:win32 {
	SOURCES += idle_win.cpp
} else:os2 {
	SOURCES += idle_os2.cpp
} else:mac {
	SOURCES += idle_mac.cpp
}
