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

#ifndef CSVIMPORTWIZARDPAGE1_H
#define CSVIMPORTWIZARDPAGE1_H

#include <QStandardItemModel>
#include <QWizardPage>
#include "ui_csvimportwizardpage1.h"

class QStandardItem;
class LoadCsvFile;

enum {
  SEMIKOLON = 0,
  COMMA,
  TABULATOR,
  SPACE,
  OTHER,
};

class CsvImportWizardPage1 : public QWizardPage
{
    Q_OBJECT

  public:
    explicit CsvImportWizardPage1(QWidget *parent = 0);
    ~CsvImportWizardPage1();
    bool isComplete() const;
    void setModel(QStandardItemModel *model);

  private slots:
    void csvPathTextChanged(QString text);
    void textDelimiterChanged(QString text);
    void otherDelimiterChanged();
    void fileLoadClicked(bool);
    void fileLoadFinished();
    void addItem(int, int, QString);
    void addHeader(int count, int section, QString value);
    void checkBoxToogled(int, bool);
    void firstRowCheckBoxToogled(bool);
    void fromValueChanged(int);
    void toValueChanged(int);
    void loadCsvFile();
    void setLoadAll(bool);
    void codecChanged(QString code);

  private:
    Ui::CsvImportWizardPage1 *ui;
    void loadSettings();
    void saveSettings() const;
    void loadCsvFile(QString text);
    void updateAssignmentTable();
    void initializePage();

    QList<QString> importType(int importType);

    QTimer *m_timer;
    QStandardItemModel *m_model;
    QString m_delimiter;
    QString m_textDelimiter;
    QString m_codec;
    bool m_isFileLoaded;
    LoadCsvFile *m_load;
    QThread *m_thread;
    int m_fromLine;
    int m_toLine;

};

class LoadCsvFile : public QObject
{
    Q_OBJECT

  public:
    LoadCsvFile(QString filename, QString delimiter, QString textDelimiter, bool firstRowIsHeader, int fromLine, int toLine, const QString codec);
    ~LoadCsvFile();

  private slots:
    void run();

  signals:
    void percentChanged(int percent);
    void addHeader(int, int, QString);
    void addItem(int, int, QString);
    void finished();

  private:
    QStringList parseCSV(const QString &string);

    QString m_filename;
    QString m_delimiter;
    QString m_textDelimiter;

    bool m_firstRowIsHeader;
    int m_fromLine;
    int m_toLine;
    QString m_codec;

};

#endif // CSVIMPORTWIZARDPAGE1_H
