DEFINES += QHOTKEYS
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/qhotkey.h \
    $$PWD/helper.h

win32 {
    SOURCES += $$PWD/qhotkey_win.cpp
    LIBS += -luser32
} else:linux {
    SOURCES += $$PWD/qhotkey_linux.cpp
} else:macx {
    SOURCES += $$PWD/qhotkey_osx.cpp
} else {
    SOURCES += $$PWD/qhotkey.cpp
}

win32:contains(TEMPLATE, lib):contains(CONFIG, shared) {
    DEFINES += QT_QTSINGLEAPPLICATION_EXPORT=__declspec(dllexport)
}
