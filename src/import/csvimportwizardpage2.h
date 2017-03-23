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

#ifndef CSVIMPORTWIZARDPAGE2_H
#define CSVIMPORTWIZARDPAGE2_H

#include <QStandardItem>
#include <QWizardPage>
#include "ui_csvimportwizardpage2.h"

enum{
  IMPORT_PRODUCTS = 0,
  IMPORT_CUSTOMER,
};

class CsvImportWizardPage2 : public QWizardPage
{
    Q_OBJECT

  public:
    explicit CsvImportWizardPage2(QWidget *parent = 0);
    ~CsvImportWizardPage2();
    bool isComplete() const;
    void setModel(QStandardItemModel *model);
    void setAssignmentModel(QStandardItemModel *model);
    void setMap(QMap<QString, QVariant> *map);

  private slots:
    void itemChangedSlot( const QModelIndex& i, const QModelIndex&);

  private:
    Ui::CsvImportWizardPage2 *ui;
    void loadSettings();
    void saveSettings() const;
    void importType(int importType);
    void initializePage();
    void updateAssignmentTable();
    void addHeaderId(QString key, QString name);

    QStandardItemModel *m_model;
    QStandardItemModel *m_assignmentModel;
    QList<QString> m_headerList;
//    QMap<QString, int> *m_map;
    QMap<QString, QVariant> *m_map;
    QMap<QString, QVariant> *m_loadmap;

};

#endif // CSVIMPORTWIZARDPAGE2_H
