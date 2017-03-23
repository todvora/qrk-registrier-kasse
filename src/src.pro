#
# This file is part of QRK - Qt Registrier Kasse
#
# Copyright (C) 2015-2017 Christian Kvasny <chris@ckvsoft.at>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#
# Button Design, and Idea for the Layout are lean out from LillePOS, Copyright 2010, Martin Koller, kollix@aon.at
#
#

#-------------------------------------------------
#
# Project created by QtCreator 2015-11-27T11:30:17
#
#-------------------------------------------------

include(../defaults.pri)

QT += sql
QT += printsupport
QT += widgets
QT += network

TARGET = qrk
DESTDIR = ../bin
INCLUDEPATH += ../qrkcore
DEPENDPATH += ../qrkcore

SOURCES += main.cpp \
    qsortfiltersqlquerymodel.cpp \
    qrk.cpp \
    qrkdelegate.cpp \
    r2bdialog.cpp \
    qrkhome.cpp \
    qrkdocument.cpp \
    qrkregister.cpp \
    aboutdlg.cpp \
    givendialog.cpp \
    manager/groupedit.cpp \
    manager/groupswidget.cpp \
    manager/managerdialog.cpp \
    import/filewatcher.cpp \
    qrkdialog.cpp \
    font/fontselector.cpp \
    qrkprogress.cpp \
    import/comboboxdelegate.cpp \
    import/csvimportwizard.cpp \
    import/csvimportwizardpage1.cpp \
    import/csvimportwizardpage2.cpp \
    import/csvimportwizardpage3.cpp \
    export/exportdialog.cpp \
    export/exportdep.cpp \
    import/importworker.cpp \
    manager/productswidget.cpp \
    manager/productedit.cpp \
    export/exportjournal.cpp \
    foninfo.cpp \
    flowlayout.cpp \
    preferences/settingsdialog.cpp

HEADERS  += \
    qsortfiltersqlquerymodel.h \
    qrk.h \
    qrkdelegate.h \
    r2bdialog.h \
    qrkdocument.h \
    qrkhome.h \
    qrkregister.h \
    aboutdlg.h \
    givendialog.h \
    manager/groupedit.h \
    manager/groupswidget.h \
    manager/managerdialog.h \
    manager/productedit.h \
    manager/productswidget.h \
    import/filewatcher.h \
    qrkdialog.h \
    font/fontselector.h \
    qrkprogress.h \
    import/comboboxdelegate.h \
    import/csvimportwizard.h \
    import/csvimportwizardpage1.h \
    import/csvimportwizardpage2.h \
    import/csvimportwizardpage3.h \
    export/exportdialog.h \
    export/exportdep.h \
    import/importworker.h \
    export/exportjournal.h \
    foninfo.h \
    flowlayout.h \
    preferences/settingsdialog.h

FORMS += \
    ui/qrk.ui \
    ui/r2bdialog.ui \
    ui/qrkdocument.ui \
    ui/qrkhome.ui \
    ui/qrkregister.ui \
    ui/aboutdlg.ui \
    ui/givendialog.ui \
    manager/groupedit.ui \
    manager/groupwidget.ui \
    manager/productedit.ui \
    manager/productswidget.ui \
    font/fontselector.ui \
    ui/qrkprogress.ui \
    import/csvimportwizardpage1.ui \
    import/csvimportwizardpage2.ui \
    import/csvimportwizardpage3.ui \
    export/exportdialog.ui \
    ui/foninfo.ui

RESOURCES += \
    qrk.qrc

TRANSLATIONS += tr/QRK_en.ts \
    tr/QRK_de.ts \
    tr/QRK_en_US.ts


win32:CONFIG(release, debug|release): LIBS += -L../qrkcore/release -lQrkCore
else:win32:CONFIG(debug, debug|release): LIBS += -L../qrkcore/debug -lQrkCore
else:unix: LIBS += -L../qrkcore -lQrkCore

win32|macx {
    # Fervor autoupdater
    # (set TARGET and VERSION of your app before including Fervor.pri)
    !include("3rdparty/fervor-autoupdate/Fervor.pri") {
	error("Unable to include Fervor autoupdater.")
    }
    DEFINES += FV_GUI

    DEFINES += FV_APP_NAME=\\\"$$TARGET\\\"
    DEFINES += FV_APP_VERSION=\\\"$$VERSION\\\"
}

unix:!macx {
 INCLUDEPATH += /usr/include/PCSC
 LIBS += -lpcsclite
}

macx {
 INCLUDEPATH += /usr/local/include
 #INCLUDEPATH += /usr/local/Cellar/qrencode/3.4.4/include
 QMAKE_LFLAGS += -Wl,-rpath,@executable_path/
 LIBS += -L/usr/local/lib
 LIBS += -framework PCSC
 LIBS += -framework CoreFoundation
 ICON = ../qrk.icns
}

win32 {
 INCLUDEPATH += $$[QT_INSTALL_PREFIX]/include/QtZlib
 LIBS += libwinscard
 LIBS += -pthread
 RC_ICONS = ../qrk.ico
}

LIBS += -lcryptopp
LIBS += -lz

win32:CONFIG(release, debug|release): COREDLL = $$BUILD_DIR/../qrkcore/release/*.dll
else:win32:CONFIG(debug, debug|release): COREDLL = $$BUILD_DIR/../qrkcore/debug/*.dll
else:macx: COREDLL = $$BUILD_DIR/../qrkcore/libQrkCore.1.0.0.dylib

win32: PLUGINS = $$BUILD_DIR/../plugins/bin/*.dll
else:macx: PLUGINS = $$BUILD_DIR/../plugins/bin/*.dylib

macx {
QMAKE_POST_LINK += $$quote($(COPY_FILE) $$COREDLL $$BUILD_DIR/../bin/$${TARGET}.app/Contents/MacOS/libQrkCore.1.dylib) &
QMAKE_POST_LINK += $$quote($(MKDIR) $$BUILD_DIR/../bin/$${TARGET}.app/Contents/plugins) &
QMAKE_POST_LINK += $$quote($(COPY_DIR) $$PLUGINS $$BUILD_DIR/../bin/$${TARGET}.app/Contents/plugins/) &
QMAKE_POST_LINK += install_name_tool -change libQrkCore.1.dylib  @executable_path/libQrkCore.1.dylib $$BUILD_DIR/../bin/$${TARGET}.app/Contents/MacOS/$${TARGET}
} else:win32 {
QMAKE_POST_LINK += $$shell_path($(COPY_DIR) $$COREDLL $$BUILD_DIR/../bin/) &
QMAKE_POST_LINK += $$shell_path($(MKDIR) $$BUILD_DIR/../bin/plugins) &
QMAKE_POST_LINK += $$shell_path($(COPY_DIR) $$PLUGINS $$BUILD_DIR/../bin/plugins/)
}
