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

#include "csvimportwizardpage1.h"
#include "preferences/qrksettings.h"
#include "qrkprogress.h"

#include <QStandardItemModel>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDialog>
#include <QTimer>
#include <QProgressDialog>
#include <QThread>
#include <QTextCodec>

CsvImportWizardPage1::CsvImportWizardPage1(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::CsvImportWizardPage1)
{
    ui->setupUi(this);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(loadCsvFile()));

    ui->buttonGroup->setId(ui->semicolonCheckBox, SEMIKOLON);
    ui->buttonGroup->setId(ui->commaCheckBox, COMMA);
    ui->buttonGroup->setId(ui->tabulatorCheckBox, TABULATOR);
    ui->buttonGroup->setId(ui->spaceCheckBox, SPACE);
    ui->buttonGroup->setId(ui->otherCheckBox, OTHER);

    connect(ui->filePathEdit, SIGNAL(textChanged(QString)), this, SLOT(csvPathTextChanged(QString)));
    connect(ui->fileLoadButton, SIGNAL(clicked(bool)), this, SLOT(fileLoadClicked(bool)));
    connect(ui->otherEdit, SIGNAL(returnPressed()), this, SLOT(otherDelimiterChanged()));
    connect(ui->firstRowIsHeaderCheckBox, SIGNAL(toggled(bool)), this, SLOT(firstRowCheckBoxToogled(bool)));
    connect(ui->textSeperatorBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(textDelimiterChanged(QString)));
    connect(ui->checkBox, SIGNAL(toggled(bool)), this, SLOT(setLoadAll(bool)));

    connect(ui->fromLineSpinBox, SIGNAL(valueChanged(int)), this, SLOT(fromValueChanged(int)));
    connect(ui->toLineSpinBox, SIGNAL(valueChanged(int)), this, SLOT(toValueChanged(int)));

    connect(ui->buttonGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(checkBoxToogled(int, bool)));
    connect(ui->codecComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(codecChanged(QString)));

    registerField("fromLine", ui->fromLineSpinBox);
    registerField("toLine", ui->toLineSpinBox);

}

CsvImportWizardPage1::~CsvImportWizardPage1()
{
    delete ui;
}

void CsvImportWizardPage1::loadSettings()
{
    QrkSettings settings;
    settings.beginGroup("CsvImportWizard");

    m_isFileLoaded = false;

    emit checkBoxToogled(settings.value("delimiter", 0).toInt(), true);
    ui->textSeperatorBox->setCurrentIndex(settings.value("textdelimiter", 0).toInt());
    ui->checkBox->setChecked(settings.value("check", true).toBool());
    if (!ui->checkBox->isChecked()) {
        ui->fromLineSpinBox->setValue(settings.value("fromline", 1).toInt());
        ui->toLineSpinBox->setValue(settings.value("toline", ui->toLineSpinBox->value()).toInt());
    }
    ui->codecComboBox->setCurrentIndex(settings.value("codec", 0).toInt());
    ui->otherEdit->setText(settings.value("other", "").toString());
    ui->rememberFileNameCheck->setChecked(settings.value("remember", false).toBool());
    if (ui->rememberFileNameCheck->isChecked())
        ui->filePathEdit->setText(settings.value("lastimportfile", "").toString());

    ui->firstRowIsHeaderCheckBox->setChecked(settings.value("firstrowisheader", true).toBool());

    settings.endGroup();
}

void CsvImportWizardPage1::saveSettings() const
{
    QrkSettings settings;
    settings.beginGroup("CsvImportWizard");

    settings.save2Settings("delimiter", ui->buttonGroup->checkedId(), false);
    settings.save2Settings("textdelimiter", ui->textSeperatorBox->currentIndex(), false);
    settings.save2Settings("check", ui->checkBox->isChecked(), false);
    if (ui->checkBox->isChecked()) {
        settings.removeSettings("fromline", false);
        settings.removeSettings("toline", false);
    } else {
        settings.save2Settings("fromline", ui->fromLineSpinBox->value(), false);
        settings.save2Settings("toline", ui->toLineSpinBox->value(), false);
    }
    settings.save2Settings("codec", ui->codecComboBox->currentIndex(), false);
    settings.save2Settings("other", ui->otherEdit->text(), false);
    settings.save2Settings("lastimportfile", ui->filePathEdit->text(), false);
    settings.save2Settings("remember", ui->rememberFileNameCheck->isChecked(), false);
    settings.save2Settings("firstrowisheader", ui->firstRowIsHeaderCheckBox->isChecked(), true);

    settings.endGroup();
}

bool CsvImportWizardPage1::isComplete() const
{
    if (m_isFileLoaded)
        saveSettings();

    return m_isFileLoaded;
}

void CsvImportWizardPage1::initializePage()
{
    ui->tableView->setModel(m_model);
    loadSettings();

}

void CsvImportWizardPage1::setModel(QStandardItemModel *model)
{
    m_model =  model;

}

void CsvImportWizardPage1::codecChanged(QString codec)
{
    m_codec = codec;
    if (m_isFileLoaded) {
        m_isFileLoaded = false;
        loadCsvFile(ui->filePathEdit->text());
    }
}

void CsvImportWizardPage1::fromValueChanged(int from)
{
    if (m_isFileLoaded && m_fromLine != from) {
        m_fromLine = from;
        m_timer->start(500);
    }
}

void CsvImportWizardPage1::setLoadAll(bool setall)

{
    if (setall) {
        m_toLine = 1;
        ui->toLineSpinBox->setValue(1);
    } else {
        m_toLine = ui->toLineSpinBox->value();
    }

    ui->fromLineSpinBox->setEnabled(!setall);
    ui->toLineSpinBox->setEnabled(!setall);
}

void CsvImportWizardPage1::toValueChanged(int to)
{
    if (m_isFileLoaded && m_toLine != to) {
        m_toLine = to;
        m_timer->start(500);
    }
}

void CsvImportWizardPage1::otherDelimiterChanged()
{
    if (ui->otherCheckBox->isChecked() && !ui->otherEdit->text().isEmpty()) {
        m_isFileLoaded = false;
        m_delimiter = ui->otherEdit->text();
        loadCsvFile(ui->filePathEdit->text());
    } else {
        ui->otherCheckBox->setChecked(true);
    }
}

void CsvImportWizardPage1::firstRowCheckBoxToogled(bool)
{
    loadCsvFile(ui->filePathEdit->text());
}

void CsvImportWizardPage1::checkBoxToogled(int id, bool checked)
{
    if (!checked)
        return;

    ui->buttonGroup->button(id)->setChecked(true);

    switch (id) {
    case SEMIKOLON:
        m_delimiter = ";";
        break;
    case COMMA:
        m_delimiter = ",";
        break;
    case TABULATOR:
        m_delimiter = "\t";
        break;
    case SPACE:
        m_delimiter = " ";
        break;
    case OTHER:
        if (ui->otherEdit->text().isEmpty())
            return;

        m_delimiter = ui->otherEdit->text();
        break;
    }

    if (m_isFileLoaded) {
        m_isFileLoaded = false;
        csvPathTextChanged(ui->filePathEdit->text());
    }
}

void CsvImportWizardPage1::fileLoadClicked(bool)
{
    QString file;
    QStringList ls;
    QFileDialog dlg(this, tr("Datei öffnen"));

    QrkSettings settings;
    QString path = settings.value("csvimportpath", QDir::homePath()).toString();

    dlg.setViewMode(QFileDialog::Detail);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setOption(QFileDialog::DontUseNativeDialog);
    dlg.setDirectory(path);

    if (dlg.exec()) {
        ls = dlg.selectedFiles();
        settings.save2Settings("csvimportpath", dlg.directory().absolutePath());
        if (ls.count() > 0) {
            file = ls[0]; //get selected file
            ui->filePathEdit->setText(file);
            ui->fromLineSpinBox->setValue(1);
            m_model->clear();
        }
        ls.clear();
    }
}

void CsvImportWizardPage1::csvPathTextChanged(QString text)
{
    m_isFileLoaded = false;
    loadCsvFile(text);
}

void CsvImportWizardPage1::textDelimiterChanged(QString textDelemiter)
{
    m_textDelimiter = textDelemiter;
    m_isFileLoaded = false;
    loadCsvFile(ui->filePathEdit->text());
}

void CsvImportWizardPage1::loadCsvFile()
{
    if (m_isFileLoaded)
        loadCsvFile(ui->filePathEdit->text());
}

void CsvImportWizardPage1::loadCsvFile(QString text)
{
    if (QFile::exists(text)) {
        m_isFileLoaded = false;
        emit completeChanged();
        m_model->clear();
        m_load = new LoadCsvFile(ui->filePathEdit->text(), m_delimiter, ui->textSeperatorBox->currentText(), ui->firstRowIsHeaderCheckBox->isChecked(), ui->fromLineSpinBox->value(), ui->toLineSpinBox->value(), m_codec);
        m_thread = new QThread(this);
        m_load->moveToThread(m_thread);

        connect(m_thread, SIGNAL(started()), m_load, SLOT(run()));

        connect(m_load, SIGNAL(percentChanged(int)), ui->progressBar, SLOT(setValue(int)));
        connect(m_load, SIGNAL(addHeader(int, int, QString)), this, SLOT(addHeader(int, int, QString)));
        connect(m_load, SIGNAL(addItem(int, int, QString)), this, SLOT(addItem(int, int, QString)));
        connect(m_load, SIGNAL(finished()), this, SLOT(fileLoadFinished()));
        connect(m_load, SIGNAL(finished()), this, SIGNAL(completeChanged()));

        m_thread->start();
    }
}

void CsvImportWizardPage1::fileLoadFinished()
{
    m_isFileLoaded = true;
    m_load = 0;
    m_thread = 0;
    m_timer->stop();
    ui->tableView->resizeColumnsToContents();
}

void CsvImportWizardPage1::addHeader(int count, int section, QString value)
{
    if (m_model->columnCount() < count)
        m_model->setColumnCount(count);

    m_model->setHeaderData(section, Qt::Horizontal, value);
    ui->tableView->resizeColumnsToContents();
}

void CsvImportWizardPage1::addItem(int lineindex, int j, QString value)
{
    QStandardItem *item = new QStandardItem(value);

    m_model->setItem(lineindex, j, item);
    ui->toLineSpinBox->setValue(lineindex + 1);
}

//---- LoadCsvFile ------------------------------------------------------------------------------------------------------

LoadCsvFile::LoadCsvFile(QString filename, QString delimiter, QString textDelimiter, bool firstRowIsHeader, int fromLine, int toLine, const QString codec)
    : m_filename(filename)
    , m_delimiter(delimiter)
    , m_textDelimiter(textDelimiter)
    , m_firstRowIsHeader(firstRowIsHeader)
    , m_fromLine(fromLine)
    , m_toLine(toLine)
    , m_codec(codec)
{
}
LoadCsvFile::~LoadCsvFile()
{
}

void LoadCsvFile::run()
{
    QFile file(m_filename);

    m_fromLine--;
    m_toLine--;
    if (m_toLine == 0)
        m_toLine = 99999;

    if (file.open(QIODevice::ReadOnly)) {
        int lineindex = 0;      // file line counter
        QTextStream in(&file);  // read to text stream
        in.setCodec(QTextCodec::codecForName(m_codec.toUtf8()));
        QFileInfo fi(file);
        qint64 currsize = 0;
        bool headerwritten = false;

        while (!in.atEnd()) {
            if (m_fromLine > lineindex) {
                lineindex++;
                in.readLine();
                continue;
            }
            if (m_toLine < lineindex)
                break;

            QString fileLine = in.readLine();

            currsize += fileLine.size() + 2;
            int percent = ((float)currsize / (float)fi.size()) * 100;
            if (percent < ((float)lineindex / (float)m_toLine) * 100)
                percent = ((float)lineindex / (float)m_toLine) * 100;

            emit percentChanged(percent);

            QStringList lineToken = parseCSV(fileLine.trimmed());

            for (int j = 0; j < lineToken.size(); j++) {
                QString value = lineToken.at(j);
                if (m_firstRowIsHeader && !headerwritten)
                    emit addHeader(lineToken.size(), j, value);
                else {
                    if (!headerwritten)
                        emit addHeader(lineToken.size(), j, QString::number(j));

                    emit addItem(lineindex - m_fromLine, j, value);
                }
            }


            if (!headerwritten && lineindex == 0) {
                headerwritten = true;
                if (!m_firstRowIsHeader)
                    lineindex++;
            } else
                lineindex++;
        }

        file.close();
        emit percentChanged(100);
        emit finished();
    }
}

QStringList LoadCsvFile::parseCSV(const QString& string)
{
    enum State { Normal,
                 Quote } state = Normal;
    QStringList fields;
    QString value;

    for (int i = 0; i < string.size(); i++) {
        QChar current = string.at(i);

        // Normal state
        if (state == Normal) {
            // delimiter
            if (current == m_delimiter && m_delimiter != m_textDelimiter) {
                // Save field
                fields.append(value);
                value.clear();
            }
            // delimitor == textDelimiter
            else if (current == m_textDelimiter) {
                state = Quote;
            }
            // Other character
            else {
                value += current;
            }
        }
        // In-quote state
        else if (state == Quote) {
            // Another textDelimiter
            if (current == m_textDelimiter) {
                if (i + 1 < string.size()) {
                    QChar next = string.at(i + 1);

                    // A double textDelimiter?
                    if (next == m_textDelimiter) {
                        value += m_textDelimiter;
                        i++;
                    } else {
                        state = Normal;
                    }
                }
            }
            // Other character
            else {
                value += current;
            }
        }
    }
    if (!value.isEmpty())
        fields.append(value);

    return fields;
}
