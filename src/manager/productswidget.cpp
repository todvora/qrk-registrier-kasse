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

#include "productswidget.h"
#include "productedit.h"
#include "qrkdelegate.h"

#include <QSqlRelationalTableModel>
#include <QSqlRelation>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QHeaderView>

//--------------------------------------------------------------------------------

ProductsWidget::ProductsWidget(QWidget *parent)
  : QWidget(parent), ui(new Ui::ProductsWidget), m_newProductDialog(0)
{
  ui->setupUi(this);

  connect(ui->plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui->minus, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui->edit, SIGNAL(clicked()), this, SLOT(editSlot()));
  connect(ui->productFilter, SIGNAL(textChanged(const QString &)), this, SLOT(filterProduct(const QString &)));

  QSqlDatabase dbc = QSqlDatabase::database("CN");

  m_model = new QSqlRelationalTableModel(this, dbc);
  m_model->setTable("products");
  m_model->setRelation(m_model->fieldIndex("group"), QSqlRelation("groups", "id", "name"));
  // model->setFilter("\"group\" > 1");
  m_model->setFilter("`group` > 1");

  m_model->setEditStrategy(QSqlTableModel::OnFieldChange);
//  model->setEditStrategy(QSqlTableModel::OnRowChange);
  m_model->select();
  m_model->fetchMore();  // else the list is not filled with all possible rows

  m_model->setHeaderData(m_model->fieldIndex("itemnum"), Qt::Horizontal, tr("Artikel #"), Qt::DisplayRole);
  m_model->setHeaderData(m_model->fieldIndex("name"), Qt::Horizontal, tr("Artikelname"), Qt::DisplayRole);
  m_model->setHeaderData(m_model->fieldIndex("gross"), Qt::Horizontal, tr("Preis"), Qt::DisplayRole);
  m_model->setHeaderData(7, Qt::Horizontal, tr("Gruppe"), Qt::DisplayRole);
  m_model->setHeaderData(m_model->fieldIndex("visible"), Qt::Horizontal, tr("sichtbar"), Qt::DisplayRole);
  m_model->setHeaderData(m_model->fieldIndex("tax"), Qt::Horizontal, tr("MwSt"), Qt::DisplayRole);

  m_proxyModel = new QSortFilterProxyModel(this);
  m_proxyModel->setSourceModel(m_model);
  m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_proxyModel->setFilterKeyColumn(m_model->fieldIndex("name"));

  ui->tableView->setModel(m_proxyModel);
  ui->tableView->setSortingEnabled(true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("id"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("barcode"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("sold"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("net"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("completer"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("color"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("button"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("image"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("tax"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("gross"), true);
  ui->tableView->setColumnHidden(m_model->fieldIndex("coupon"), true);

  ui->tableView->setAlternatingRowColors(true);
  ui->tableView->resizeColumnsToContents();
  ui->tableView->horizontalHeader()->setStretchLastSection(true);

}

//--------------------------------------------------------------------------------

void ProductsWidget::filterProduct(const QString &filter)
{
  // show only matching items

  m_proxyModel->setFilterWildcard("*" + filter + "*");

  m_model->fetchMore();  // else the list is not filled with all possible rows
                       // e.g. when using mouse wheel it would fetch more items
                       // but on the WeTab we have no mouse
}

//--------------------------------------------------------------------------------

void ProductsWidget::plusSlot()
{
  // reuse the "new" dialog so that the next call has already the previous
  // settings defined; makes input of a lot of products of a given group simpler
  // if ( !newProductDialog )
    m_newProductDialog = new ProductEdit(this);

  m_newProductDialog->exec();

  m_model->select();
  m_model->fetchMore();  // else the list is not filled with all possible rows
}

//--------------------------------------------------------------------------------

void ProductsWidget::minusSlot()
{
  int row = m_proxyModel->mapToSource(ui->tableView->currentIndex()).row();
  if ( row == -1 )
    return;

  QMessageBox msgBox;
  msgBox.setIcon(QMessageBox::Question);
  msgBox.setWindowTitle(tr("Artikel löschen"));
  msgBox.setText(tr("Möchten sie den Artikel '%1' wirklich löschen ?").arg(m_model->data(m_model->index(row, 3)).toString()));
  msgBox.setStandardButtons(QMessageBox::Yes);
  msgBox.addButton(QMessageBox::No);
  msgBox.setButtonText(QMessageBox::Yes, tr("Löschen"));
  msgBox.setButtonText(QMessageBox::No, tr("Abbrechen"));
  msgBox.setDefaultButton(QMessageBox::No);

  if(msgBox.exec() == QMessageBox::No)
      return;

  m_model->removeRow(row);

  /* Workaround, removeRow always return false*/
  if ( m_model->data(m_model->index(row, 0)).toInt() != 0)
  {
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Information);
      msgBox.setWindowTitle(tr("Löschen nicht möglich"));
      msgBox.setText(tr("Artikel '%1' kann nicht gelöscht werden, da er schon in Verwendung ist.").arg(m_model->data(m_model->index(row, 3)).toString()));
      msgBox.setStandardButtons(QMessageBox::Yes);
      msgBox.setButtonText(QMessageBox::Yes, tr("Ok"));
      msgBox.exec();
  }

  m_model->select();
  m_model->fetchMore();  // else the list is not filled with all possible rows

}

//--------------------------------------------------------------------------------

void ProductsWidget::editSlot()
{
  QModelIndex current(m_proxyModel->mapToSource(ui->tableView->currentIndex()));
  int row = current.row();
  if ( row == -1 )
    return;

  ProductEdit dialog(this, m_model->data(m_model->index(row, m_model->fieldIndex("id"))).toInt());
  if ( dialog.exec() == QDialog::Accepted)
  {
    m_model->select();
    m_model->fetchMore();  // else the list is not filled with all possible rows
    ui->tableView->resizeRowsToContents();
    ui->tableView->setCurrentIndex(m_proxyModel->mapFromSource(current));
  }
}

//--------------------------------------------------------------------------------
