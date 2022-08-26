QT += core serialport

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(Middlewares/Middlewares.pri)
include(Applications/Applications.pri)
include(UI/UI.pri)

# Output directory
CONFIG(debug, debug|release) {
    compiled = debug
}
CONFIG(release, debug|release) {
    compiled = release
}

# Copy promotion required headers to build directory
win32 {
    COPY_DEST = $$replace(OUT_PWD, /, \\)
    system("copy Config.ini   $$COPY_DEST\\$$compiled\\Config.ini")
}
unix {
    system("cp Config.ini   $$OUT_PWD/Config.ini")
    system("cp detect.engine   $$OUT_PWD/detect.engine")
}


SOURCES += \
    main.cpp

DISTFILES += \
    Config.ini
