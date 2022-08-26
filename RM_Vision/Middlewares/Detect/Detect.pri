include(../Cuda/Cuda.pri)

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
HEADERS += \
    $$PWD/DetectRobot.h \
    $$PWD/detect_common.h \
    $$PWD/logging.h \
    $$PWD/ArmorDetector.h \
    $$PWD/AngleSolver.hpp \
    $$PWD/armorbox.h

SOURCES += \
    $$PWD/DetectRobot.cpp \
    $$PWD/detect_common.cpp \
    $$PWD/ArmorDetector.cpp \
    $$PWD/AngleSolver.cpp

