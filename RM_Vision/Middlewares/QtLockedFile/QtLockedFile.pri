INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
HEADERS += $$PWD/QtLockedFile.h
SOURCES += $$PWD/QtLockedFile.cpp

unix:SOURCES += $$PWD/qtlockedfile_unix.cpp
win32:SOURCES += $$PWD/qtlockedfile_win.cpp

win32:contains(TEMPLATE, lib):contains(CONFIG, shared) {
    DEFINES += QT_QTLOCKEDFILE_EXPORT=__declspec(dllexport)
}


