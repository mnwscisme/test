#大恒SDK
DAHENG_SDK_PATH = $$PWD/GalaxySDK/linux
SYSTEM_NAME = armv8

LIBS += -L$$DAHENG_SDK_PATH/lib/$$SYSTEM_NAME/ -lgxiapi

INCLUDEPATH += \
    $$PWD \
    $$DAHENG_SDK_PATH/lib/$$SYSTEM_NAME \
    $$DAHENG_SDK_PATH/inc
DEPENDPATH += \
    $$PWD \
    $$DAHENG_SDK_PATH/lib/$$SYSTEM_NAME \
    $$DAHENG_SDK_PATH/inc

HEADERS += \
    $$DAHENG_SDK_PATH/inc/DxImageProc.h \
    $$DAHENG_SDK_PATH/inc/GxIAPI.h \
    $$PWD/Camera.h \
    $$PWD/fps.h

SOURCES += \
    $$PWD/Camera.cpp \
    $$PWD/fps.cpp

