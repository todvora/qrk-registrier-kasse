#-------------------------------------------------
#
# Project created by QtCreator 2015-11-27T11:30:17
#
#-------------------------------------------------

include(../defaults.pri)

QT += sql
QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ../bin/QRK
# TEMPLATE = app

VERSION = 0.16.0130

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
    utils.cpp \
    r2bdialog.cpp \
    qrkhome.cpp \
    qrkdocument.cpp \
    qrkregister.cpp \
    aboutdlg.cpp \
    utils/base32decode.cpp \
    utils/base32encode.cpp \
    utils/aesutil.cpp

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
    utils.h \
    r2bdialog.h \
    qrkdocument.h \
    qrkhome.h \
    qrkregister.h \
    defines.h \
    aboutdlg.h \
    utils/base32decode.h \
    utils/base32encode.h \
    utils/aesutil.h

FORMS += \
    ui/qrk.ui \
    ui/databasedefinition.ui \
    ui/depexportdialog.ui \
    ui/r2bdialog.ui \
    ui/qrkdocument.ui \
    ui/qrkhome.ui \
    ui/qrkregister.ui \
    ui/aboutdlg.ui

RESOURCES += \
    qrk.qrc

TRANSLATIONS += tr/QRK_en.ts \
    tr/QRK_de.ts

DISTFILES +=

# quazip
!include("3rdparty/quazip/quazip.pri") {
<------>error("Unable to include quazip.")
}

# Fervor autoupdater
# (set TARGET and VERSION of your app before including Fervor.pri)
!include("3rdparty/fervor-autoupdate/Fervor.pri") {
<------>error("Unable to include Fervor autoupdater.")
}

# LIBS += -lqrencode
# LIBS += -lpthread
LIBS += -lcryptopp
LIBS += -lz
