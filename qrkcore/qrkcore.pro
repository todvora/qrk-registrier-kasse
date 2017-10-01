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

QT += sql
QT += printsupport
QT += widgets
QT += network

TEMPLATE = lib

DEFINES += QRK_BUILD QUAZIP_BUILD
INCLUDEPATH += 3rdparty/quazip

TARGET = QrkCore
# DESTDIR = ../lib

# Input
SOURCES = database.cpp \
    databasedefinition.cpp \
    utils/demomode.cpp \
    preferences/qrksettings.cpp \
    journal.cpp \
    utils/qrcode.cpp \
    utils/utils.cpp \
    singleton/spreadsignal.cpp \
    RK/a_signacos_04.cpp \
    RK/a_signcardos_53.cpp \
    RK/a_signonline.cpp \
    RK/a_signsmardcard.cpp \
    RK/base32decode.cpp \
    RK/base32encode.cpp \
    RK/rk_signaturemodule.cpp \
    RK/rk_signaturemodulefactory.cpp \
    RK/rk_signatureonline.cpp \
    RK/rk_signaturesmartcard.cpp \
    RK/rk_smartcardinfo.cpp \
    documentprinter.cpp \
    receiptitemmodel.cpp \
    reports.cpp \
    backup.cpp \
    pluginmanager/pluginmanager.cpp \
    pluginmanager/treeitem.cpp \
    pluginmanager/treemodel.cpp \
    pluginmanager/pluginview.cpp

HEADERS = database.h \
    databasedefinition.h \
    utils/demomode.h \
    preferences/qrksettings.h \
    journal.h \
    defines.h \
    utils/qrcode.h \
    utils/utils.h \
    singleton/Singleton.h \
    singleton/spreadsignal.h \
    RK/a_signacos_04.h \
    RK/a_signcardos_53.h \
    RK/a_signonline.h \
    RK/a_signsmardcard.h \
    RK/base32decode.h \
    RK/base32encode.h \
    RK/rk_signaturemodule.h \
    RK/rk_signaturemodulefactory.h \
    RK/rk_signatureonline.h \
    RK/rk_signaturesmartcard.h \
    RK/rk_smartcardinfo.h \
    documentprinter.h \
    receiptitemmodel.h \
    reports.h \
    backup.h \
    qrkcore_global.h \
    pluginmanager/pluginmanager.h \
    pluginmanager/Interfaces/barcodesinterface.h \
    pluginmanager/Interfaces/plugininterface.h \
    pluginmanager/treeitem.h \
    pluginmanager/treemodel.h \
    pluginmanager/pluginview.h

FORMS = databasedefinition.ui \
    pluginmanager/pluginview.ui

# quazip
!include("3rdparty/quazip/quazip.pri") {
    error("Unable to include quazip.")
}

#win32|macx {
#    # Fervor autoupdater
#    # (set TARGET and VERSION of your app before including Fervor.pri)
#    !include("3rdparty/fervor-autoupdate/Fervor.pri") {
#	error("Unable to include Fervor autoupdater.")
#    }
#}

# qrencode
!include("3rdparty/qrencode/qrencode.pri") {
    error("Unable to include qrencode.")
}

# qbcmath
!include("3rdparty/qbcmath/qbcmath.pri") {
    error("Unable to include qbcmath.")
}

unix:!macx {
 INCLUDEPATH += /usr/include/PCSC
 LIBS += -lpcsclite
}

macx {
 INCLUDEPATH += /usr/local/include

 LIBS += -L/usr/local/lib
 LIBS += -framework PCSC
 LIBS += -framework CoreFoundation
}

win32 {
 LIBS += libwinscard
 LIBS += -pthread
}

LIBS += -lcryptopp
LIBS += -lz
