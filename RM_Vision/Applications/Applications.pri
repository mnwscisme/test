!isEmpty(APPLICATIONS_PRI_INCLUDED):error("Applications.pri already included")
APPLICATIONS_PRI_INCLUDED = 1

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
HEADERS += \
    $$PWD/CameraProcessor.h \
    $$PWD/CommProcessor.h \
    $$PWD/VisionProcessor.h \
    $$PWD/Protocol.h
SOURCES += \
    $$PWD/CameraProcessor.cpp \
    $$PWD/CommProcessor.cpp \
    $$PWD/VisionProcessor.cpp


# Include App files
#APPLICATIONS_FILES += \
#

#for(l, APPLICATIONS_FILES) {
#    include(./$$l/$${l}.pri)
#}
