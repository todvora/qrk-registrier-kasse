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
    utils/utils.cpp

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
    utils/utils.h

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
    manager/productswidget.ui

RESOURCES += \
    qrk.qrc

TRANSLATIONS += tr/QRK_en.ts \
    tr/QRK_de.ts

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
