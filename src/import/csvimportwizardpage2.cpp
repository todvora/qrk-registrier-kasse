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

#include "csvimportwizardpage2.h"
#include "comboboxdelegate.h"
#include "preferences/qrksettings.h"

#include <QStandardItemModel>

CsvImportWizardPage2::CsvImportWizardPage2(QWidget *parent) :
  QWizardPage(parent), ui(new Ui::CsvImportWizardPage2)
{
  ui->setupUi(this);

  registerField("autogroup", ui->autoGroupCheckBox);
  registerField("guessgroup", ui->guessGroupCheckBox);
  registerField("ignoreexisting", ui->ignoreExistingProductCheckBox);
  registerField("updateexisting", ui->updateExistingProductCheckBox);
  registerField("importtype", ui->importTypeComboBox);
  registerField("visiblegroup", ui->visibleGroupCheckBox);
  registerField("visibleproduct", ui->visibleProductCheckBox);

  m_loadmap = new QMap<QString, QVariant>;

}

CsvImportWizardPage2::~CsvImportWizardPage2()
{
  delete ui;
}

void CsvImportWizardPage2::loadSettings()
{
    QrkSettings settings;
    settings.beginGroup("CsvImportWizard");
    QByteArray readData = settings.value("map").toByteArray();
    QDataStream readStream(&readData, QIODevice::ReadOnly);
    readStream >> *m_loadmap;
    ui->autoGroupCheckBox->setChecked( settings.value("autogroup", ui->autoGroupCheckBox->isChecked()).toBool());
    ui->guessGroupCheckBox->setChecked( settings.value("guessgroup", ui->guessGroupCheckBox->isChecked()).toBool());
    ui->ignoreExistingProductCheckBox->setChecked( settings.value("ignoreexisting", ui->ignoreExistingProductCheckBox->isChecked()).toBool());
    ui->updateExistingProductCheckBox->setChecked( settings.value("updateexisting", ui->updateExistingProductCheckBox->isChecked()).toBool());
    ui->importTypeComboBox->setCurrentIndex(settings.value("importtype", ui->importTypeComboBox->currentIndex()).toInt());
    ui->visibleGroupCheckBox->setChecked( settings.value("visiblegroup", ui->visibleGroupCheckBox->isChecked()).toBool());
    ui->visibleProductCheckBox->setChecked( settings.value("visibleproduct", ui->visibleProductCheckBox->isChecked()).toBool());
    settings.endGroup();
}

void CsvImportWizardPage2::saveSettings() const
{
    QrkSettings settings;
    settings.beginGroup("CsvImportWizard");
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << *m_loadmap;
    settings.save2Settings("map", data, false);
    settings.save2Settings("autogroup", ui->autoGroupCheckBox->isChecked(), false);
    settings.save2Settings("guessgroup", ui->guessGroupCheckBox->isChecked(), false);
    settings.save2Settings("ignoreexisting", ui->ignoreExistingProductCheckBox->isChecked(), false);
    settings.save2Settings("updateexisting", ui->updateExistingProductCheckBox->isChecked(), false);
    settings.save2Settings("importtype", ui->importTypeComboBox->currentIndex(), false);
    settings.save2Settings("visiblegroup", ui->visibleGroupCheckBox->isChecked(), false);
    settings.save2Settings("visibleproduct", ui->visibleProductCheckBox->isChecked(), false);
    settings.endGroup();
}

bool CsvImportWizardPage2::isComplete() const
{
  if (m_map->contains(tr("Artikelname"))) {
    saveSettings();
    bool ret = (m_map->value(tr("Artikelname")) > 0)? true: false;
    return ret;
  }
  return false;
}

void CsvImportWizardPage2::initializePage()
{
  loadSettings();
  ui->tableView->setModel(m_model);
  ui->tableView->resizeColumnsToContents();
  ui->assignmentView->setModel(m_assignmentModel);

  for (int i = 0; i < m_model->columnCount(); i++)
    m_model->setHeaderData(i,Qt::Horizontal,QBrush(QColor("red")),Qt::BackgroundRole);

  // ui->assignmentView->horizontalHeader()->hide();
  ui->assignmentView->verticalHeader()->hide();

  updateAssignmentTable();

  m_assignmentModel->setHeaderData(0, Qt::Horizontal, "QRK Datenbank");
  m_assignmentModel->setHeaderData(1, Qt::Horizontal, "CSV Datei");

  connect(m_assignmentModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(itemChangedSlot(const QModelIndex&, const QModelIndex&)));

  QList<QString> list;
  list << "itemnum" << "barcode" << "name" << "net" << "gross" << "tax" << "group" << "color" << "coupon";
  QMap<QString, QVariant>::iterator i;
  for (i = m_loadmap->begin(); i != m_loadmap->end(); ++i) {
      list << i.value().toString();
  }

  for (int i = 0; i < m_model->columnCount(); i++) {
    if (list.contains(m_model->headerData(i,Qt::Horizontal).toString()))
      m_model->setHeaderData(i,Qt::Horizontal,QBrush(QColor("green")),Qt::BackgroundRole);
  }

}

void CsvImportWizardPage2::setModel(QStandardItemModel *model)
{
  m_model =  model;
}

void CsvImportWizardPage2::setMap(QMap<QString, QVariant> *map)
{
  m_map =  map;
}

void CsvImportWizardPage2::setAssignmentModel(QStandardItemModel *model)
{
  m_assignmentModel =  model;

}

void CsvImportWizardPage2::updateAssignmentTable()
{
  int count = m_model->columnCount();

  m_headerList.clear();
  m_assignmentModel->clear();
  m_headerList << tr("keine Zuordnung");

  for (int i = 0; i < count; i++) {
    m_headerList << m_model->headerData(i, Qt::Horizontal).toString();
  }

  importType(IMPORT_PRODUCTS);

  ComboBoxDelegate* delegate = new ComboBoxDelegate(m_headerList, this);
  ui->assignmentView->setItemDelegateForColumn(1, delegate);

  QMap<QString, QVariant>::iterator i;
  int j = 0;
  for (i = m_map->begin(); i != m_map->end(); ++i) {
    QStandardItem *item = new QStandardItem();
    if (i.key() == tr("Artikelname"))
      item->setForeground(QColor::fromRgb(255,0,0));
    item->setText(i.key());
    m_assignmentModel->setItem(j, 0, item);
    QString text =  m_headerList.at(i.value().toInt());
    m_assignmentModel->setItem(j, 1, new QStandardItem(text));
    QModelIndex index = m_assignmentModel->index(j,1,QModelIndex());
    ui->assignmentView->openPersistentEditor(index);
    j++;
  }

  ui->assignmentView->resizeColumnsToContents();
  ui->assignmentView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

}

void CsvImportWizardPage2::importType(int importType)
{

  m_map->clear();
  switch (importType) {
    case IMPORT_PRODUCTS:
      addHeaderId(tr("Artikelnummer"), "itemnum");
      addHeaderId(tr("Barcode"), "barcode");
      addHeaderId(tr("Artikelname"), "name");
      addHeaderId(tr("Netto Preis"), "net");
      addHeaderId(tr("Brutto Preis"), "gross");
      addHeaderId(tr("Gruppe"), "group");
      addHeaderId(tr("Steuersatz"), "tax");
      addHeaderId(tr("Farbe"), "color");
      addHeaderId(tr("Extrabon"), "coupon");
      break;
  }
}

void CsvImportWizardPage2::addHeaderId(QString key, QString name)
{

    QString value = m_loadmap->value(key, name).toString();
    if (value == m_headerList.value(0))
        value = name;

    for (int i = 0; i < m_model->columnCount(); i++) {
        if (m_model->headerData(i,Qt::Horizontal).toString() == value) {
            m_map->insert(key, i+1);
            return;
        }
    }
    m_map->insert(key, 0);
}

void CsvImportWizardPage2::itemChangedSlot( const QModelIndex& i, const QModelIndex&)
{

  int row = i.row();
  int col = i.column();

  QString s = m_assignmentModel->data(m_assignmentModel->index(row, col, QModelIndex())).toString();

  if (col == 1) {
    QString s2 = m_assignmentModel->data(m_assignmentModel->index(row, 0, QModelIndex())).toString();

    int value = 0;
    value = m_headerList.indexOf(s);
    if (m_map->contains(s2)) {
      m_map->insert(s2, value);
      m_loadmap->insert(s2, s);
      emit completeChanged();
    }
  }
}
