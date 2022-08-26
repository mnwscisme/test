INCLUDEPATH	+= $$PWD
DEPENDPATH      += $$PWD
HEADERS		+= $$PWD/QtSingleApplication.h $$PWD/QtLocalPeer.h
SOURCES		+= $$PWD/QtSingleApplication.cpp $$PWD/QtLocalPeer.cpp

QT *= network widgets

gotqtlockedfile = $$find(HEADERS, .*QtLockedFile.h)
isEmpty(gotqtlockedfile):include(../QtLockedFile/QtLockedFile.pri)


win32:contains(TEMPLATE, lib):contains(CONFIG, shared) {
    DEFINES += QT_QTSINGLEAPPLICATION_EXPORT=__declspec(dllexport)
}
