QT += core widgets network
# QT += webkitwidgets
QT += gui
DEFINES += FV_GUI

win32:INCLUDEPATH += $$[QT_INSTALL_PREFIX]/include/QtZlib

DEFINES += FV_APP_NAME=\\\"$$TARGET\\\"
DEFINES += FV_APP_VERSION=\\\"$$VERSION\\\"

# DEFINES +=QUAZIP_BUILD QUAZIP_STATIC

DEPENDPATH += "$$PWD"
INCLUDEPATH += "$$PWD"
SOURCES += $$PWD/fvupdater.cpp \
        $$PWD/fvversioncomparator.cpp \
        $$PWD/fvplatform.cpp \
        $$PWD/fvignoredversions.cpp \
        $$PWD/fvavailableupdate.cpp

HEADERS += $$PWD/fvupdater.h \
        $$PWD/fvversioncomparator.h \
        $$PWD/fvplatform.h \
        $$PWD/fvignoredversions.h \
        $$PWD/fvavailableupdate.h

    SOURCES += $$PWD/fvupdatewindow.cpp \
            $$PWD/fvupdatedownloadprogress.cpp

    HEADERS += $$PWD/fvupdatewindow.h \
            $$PWD/fvupdatedownloadprogress.h

    FORMS += $$PWD/fvupdatewindow.ui \
            $$PWD/fvupdatedownloadprogress.ui


TRANSLATIONS += fervor_lt.ts
CODECFORTR = UTF-8
