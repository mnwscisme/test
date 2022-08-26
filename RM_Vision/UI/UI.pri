!isEmpty(UI_PRI_INCLUDED):error("UI.pri already included")
UI_PRI_INCLUDED = 1

QT += core

FORMS += \
    $$PWD/calibrationdialog.ui \
    $$PWD/mainwindow.ui

HEADERS += \
    $$PWD/calibrationdialog.h \
    $$PWD/mainwindow.h

SOURCES += \
    $$PWD/calibrationdialog.cpp \
    $$PWD/mainwindow.cpp

