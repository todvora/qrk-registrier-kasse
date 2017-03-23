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

TEMPLATE = lib
CONFIG += plugin
CONFIG += c++11

SRC_DIR = $$PWD
BUILD_DIR = $$OUT_PWD

DESTDIR = $$BUILD_DIR/../bin/

win32:CONFIG(release, debug|release): LIBS += -L$$BUILD_DIR/../../qrkcore/release -lQrkCore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$BUILD_DIR/../../qrkcore/debug -lQrkCore
else:unix: LIBS += -L$$BUILD_DIR/../../qrkcore -lQrkCore

INCLUDEPATH += $$SRC_DIR/../qrkcore
DEPENDPATH += $$SRC_DIR/../qrkcore

macx {
 QMAKE_LFLAGS += -Wl,-rpath,@executable_path/
 QMAKE_POST_LINK += install_name_tool -change libQrkCore.1.dylib @executable_path/libQrkCore.1.dylib ../bin/lib$${TARGET}.dylib
}
