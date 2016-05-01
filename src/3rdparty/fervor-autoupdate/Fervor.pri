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

QT += core widgets network
QT += gui
DEFINES += FV_GUI

win32:INCLUDEPATH += $$[QT_INSTALL_PREFIX]/include/QtZlib

DEFINES += FV_APP_NAME=\\\"$$TARGET\\\"
DEFINES += FV_APP_VERSION=\\\"$$VERSION\\\"

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
