#
# This file is part of QRK - Qt Registrier Kasse
#
# Copyright (C) 2015-2016 Christian Kvasny <chris@ckvsoft.at>
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

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ../bin/qrk
# TEMPLATE = app

SOURCES += main.cpp\
    databasedefinition.cpp \
    settingsdialog.cpp \
    database.cpp \
    receiptitemmodel.cpp \
    qsortfiltersqlquerymodel.cpp \
    documentprinter.cpp \
    qrk.cpp \
    depexportdialog.cpp \
    qrkdelegate.cpp \
    dep.cpp \
    reports.cpp \
    r2bdialog.cpp \
    qrkhome.cpp \
    qrkdocument.cpp \
    qrkregister.cpp \
    aboutdlg.cpp \
    utils/base32decode.cpp \
    utils/base32encode.cpp \
    utils/aesutil.cpp \
    backup.cpp \
    givendialog.cpp \
    manager/groupedit.cpp \
    manager/groupswidget.cpp \
    manager/managerdialog.cpp \
    manager/productedit.cpp \
    manager/productswidget.cpp \
    utils/utils.cpp \
    import/filewatcher.cpp \
    import/import.cpp \
    utils/demomode.cpp \
    qrkdialog.cpp \
    utils/qrcode.cpp \
    font/fontselector.cpp

HEADERS  += \
    databasedefinition.h \
    settingsdialog.h \
    database.h \
    receiptitemmodel.h \
    qsortfiltersqlquerymodel.h \
    documentprinter.h \
    qrk.h \
    depexportdialog.h \
    qrkdelegate.h \
    dep.h \
    reports.h \
    r2bdialog.h \
    qrkdocument.h \
    qrkhome.h \
    qrkregister.h \
    defines.h \
    aboutdlg.h \
    utils/base32decode.h \
    utils/base32encode.h \
    utils/aesutil.h \
    backup.h \
    givendialog.h \
    manager/groupedit.h \
    manager/groupswidget.h \
    manager/managerdialog.h \
    manager/productedit.h \
    manager/productswidget.h \
    utils/utils.h \
    import/filewatcher.h \
    import/import.h \
    utils/demomode.h \
    qrkdialog.h \
    utils/qrcode.h \
    font/fontselector.h

FORMS += \
    ui/qrk.ui \
    ui/databasedefinition.ui \
    ui/depexportdialog.ui \
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
    font/fontselector.ui

RESOURCES += \
    qrk.qrc

TRANSLATIONS += tr/QRK_en.ts \
    tr/QRK_de.ts

# quazip
!include("3rdparty/quazip/quazip.pri") {
<------>error("Unable to include quazip.")
}

win32 {
# Fervor autoupdater
# (set TARGET and VERSION of your app before including Fervor.pri)
!include("3rdparty/fervor-autoupdate/Fervor.pri") {
<------>error("Unable to include Fervor autoupdater.")
}
}

macx {
 INCLUDEPATH += /usr/local/include
 LIBS += -L/usr/local/lib -lqrencode
} else {
 LIBS += -lqrencode
}

LIBS += -lcryptopp
LIBS += -lz

CONFIG += link_pkgconfig
unix:PKGCONFIG += libpcsclite
macx:PKGCONFIG += libpcsclite
win32:LIBS += libwinscard
