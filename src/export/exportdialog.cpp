/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015-2017 Christian Kvasny <chris@ckvsoft.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Button Design, and Idea for the Layout are lean out from LillePOS, Copyright 2010, Martin Koller, kollix@aon.at
 *
*/

#include "exportdialog.h"
#include "preferences/qrksettings.h"
#include <QFileDialog>

#include "ui_exportdialog.h"

ExportDialog::ExportDialog( bool save2dir, QWidget *parent)
    : QDialog(parent), ui(new Ui::ExportDialog), m_saveTodir(save2dir)
{
    ui->setupUi(this);

    m_fromDT = new QDateTime;
    m_toDT = new QDateTime;
    m_filename = "";

    QString sDate = QString("%1-01-01").arg(QDate::currentDate().year());
    QDate date = QDate::fromString(sDate,"yyyy-MM-dd");

    ui->dateEditTo->setDate(QDate::currentDate());
    ui->dateEditFrom->setDate(date);

    m_fromDT->setDate(date);
    m_toDT->setDate(QDate::currentDate());
    m_toDT->setTime(QTime::fromString("23:59:59"));

    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->exportButton, SIGNAL(clicked()), this, SLOT(onExportButton_clicked()));

    connect(ui->dateEditFrom, SIGNAL(dateChanged(QDate)), this, SLOT(onDateEditFrom_dateChanged(QDate)));
    connect(ui->dateEditTo, SIGNAL(dateChanged(QDate)), this, SLOT(onDateEditTo_dateChanged(QDate)));

}

ExportDialog::~ExportDialog()
{
    delete ui;
    delete m_fromDT;
    delete m_toDT;
}

QString ExportDialog::getFilename()
{
    return m_filename;
}

QString ExportDialog::getFrom()
{
    return m_fromDT->toString(Qt::ISODate);
}

QString ExportDialog::getTo()
{
    return m_toDT->toString(Qt::ISODate);
}

void ExportDialog::onDateEditTo_dateChanged(const QDate &date)
{
    m_toDT->setDate(date);
    m_toDT->setTime(QTime::fromString("23:59:59"));
}

void ExportDialog::onDateEditFrom_dateChanged(const QDate &date)
{
    m_fromDT->setDate(date);
}

void ExportDialog::onExportButton_clicked()
{
    QrkSettings settings;
    QString lastUsedDirectory = settings.value("lastUsedDirectory", QDir::currentPath()).toString();

    if (m_saveTodir) {
        m_filename = QFileDialog::getExistingDirectory(0, tr("Speichern nach ..."),lastUsedDirectory, QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly | QFileDialog::DontConfirmOverwrite);
    } else {
        m_filename = QFileDialog::getSaveFileName(0, tr("Datei speichern"), lastUsedDirectory, "Journal (*.csv)", 0, QFileDialog::DontUseNativeDialog);
    }

    if (m_filename.isNull())
        return;

    settings.save2Settings("lastUsedDirectory", m_filename);

    QDialog::accept();

}
