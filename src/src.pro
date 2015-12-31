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
    utils.cpp \
    r2bdialog.cpp

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
    r2bdialog.h

FORMS += \
    ui/qrk.ui \
    ui/databasedefinition.ui \
    ui/settingsdialog.ui \
    ui/depexportdialog.ui \
    ui/r2bdialog.ui

RESOURCES += \
    qrk.qrc

DISTFILES +=
