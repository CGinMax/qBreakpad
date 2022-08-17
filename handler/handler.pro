TEMPLATE = lib
TARGET = qBreakpad
#Application version
#VERSION = 0.4.0

CONFIG += warn_on thread exceptions rtti stl
QT -= gui
QT += core network

OBJECTS_DIR = _build/obj
MOC_DIR = _build
DESTDIR = $$PWD

### qBreakpad config
include($$PWD/../config.pri)

## google-breakpad
include($$PWD/../third_party/breakpad.pri)
DEFINES += USE_BREAKPAD

HEADERS += \
    $$PWD/singletone/call_once.h \
    $$PWD/singletone/singleton.h \
    $$PWD/QBreakpadHandler.h \
    $$PWD/QBreakpadHttpUploader.h \
    $$PWD/platformhelper.h

SOURCES += \
    $$PWD/QBreakpadHandler.cpp \
    $$PWD/QBreakpadHttpUploader.cpp \
    $$PWD/platformhelper.cpp


unix:!mac {
    SOURCES += $$PWD/platformhelper_linux.cpp
} else:win32:!win32-g++ {
    SOURCES += $$PWD/platformhelper_win_msvc.cpp
}
