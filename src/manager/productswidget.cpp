/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015-2016 Christian Kvasny <chris@ckvsoft.at>
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
#include <QSqlQuery>
#include <QMessageBox>
#include <QHeaderView>

//--------------------------------------------------------------------------------

ProductsWidget::ProductsWidget(QWidget *parent)
  : QWidget(parent), ui(new Ui::ProductsWidget), newProductDialog(0)
{
  ui->setupUi(this);

  connect(ui->plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui->minus, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui->edit, SIGNAL(clicked()), this, SLOT(editSlot()));
  connect(ui->productFilter, SIGNAL(textChanged(const QString &)), this, SLOT(filterProduct(const QString &)));

  QSqlDatabase dbc = QSqlDatabase::database("CN");

  model = new QSqlRelationalTableModel(this, dbc);
  model->setTable("products");
  model->setRelation(model->fieldIndex("group"), QSqlRelation("groups", "id", "name"));
  // model->setFilter("\"group\" > 1");
  model->setFilter("`group` > 1");

  model->setEditStrategy(QSqlTableModel::OnFieldChange);
//  model->setEditStrategy(QSqlTableModel::OnRowChange);
  model->select();
  model->fetchMore();  // else the list is not filled with all possible rows

  model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Produkt"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("gross"), Qt::Horizontal, tr("Preis"), Qt::DisplayRole);
  model->setHeaderData(5, Qt::Horizontal, tr("Gruppe"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("visible"), Qt::Horizontal, tr("sichtbar"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("tax"), Qt::Horizontal, tr("MwSt"), Qt::DisplayRole);

  proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(model);
  proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  proxyModel->setFilterKeyColumn(model->fieldIndex("name"));

  ui->tableView->setModel(proxyModel);
  ui->tableView->setSortingEnabled(true);
  ui->tableView->setColumnHidden(model->fieldIndex("id"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("sold"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("net"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("completer"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("color"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("button"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("image"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("tax"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("gross"), true);

//  ui->tableView->setItemDelegateForColumn(model->fieldIndex("gross"), new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));
//  ui->tableView->setItemDelegateForColumn(model->fieldIndex("tax"), new QrkDelegate (QrkDelegate::COMBO_TAX, this));

  ui->tableView->setAlternatingRowColors(true);
  ui->tableView->resizeColumnsToContents();
  ui->tableView->horizontalHeader()->setStretchLastSection(true);

//  ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

//--------------------------------------------------------------------------------

void ProductsWidget::filterProduct(const QString &filter)
{
  // show only matching items

  proxyModel->setFilterWildcard("*" + filter + "*");

  model->fetchMore();  // else the list is not filled with all possible rows
                       // e.g. when using mouse wheel it would fetch more items
                       // but on the WeTab we have no mouse
}

//--------------------------------------------------------------------------------

void ProductsWidget::plusSlot()
{
  // reuse the "new" dialog so that the next call has already the previous
  // settings defined; makes input of a lot of products of a given group simpler
  // if ( !newProductDialog )
    newProductDialog = new ProductEdit(this);

  newProductDialog->exec();

  model->select();
  model->fetchMore();  // else the list is not filled with all possible rows
}

//--------------------------------------------------------------------------------

void ProductsWidget::minusSlot()
{
  int row = proxyModel->mapToSource(ui->tableView->currentIndex()).row();
  if ( row == -1 )
    return;

  if ( QMessageBox::question(this, tr("Produkt löschen"),
         tr("Möchten sie das Produkt '%1' wirklich löschen ?")
            .arg(model->data(model->index(row, 1)).toString()),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;



  model->removeRow(row);

  /* Workaround, removeRow always return false*/
  if ( model->data(model->index(row, 0)).toInt() != 0)
  {
    QMessageBox::information(this, tr("Löschen nicht möglich"),
        tr("Produkt '%1' kann nicht gelöscht werden, da es schon in Verwendung ist")
           .arg(model->data(model->index(row, 1)).toString()));
  }

  model->select();
  model->fetchMore();  // else the list is not filled with all possible rows

}

//--------------------------------------------------------------------------------

void ProductsWidget::editSlot()
{
  QModelIndex current(proxyModel->mapToSource(ui->tableView->currentIndex()));
  int row = current.row();
  if ( row == -1 )
    return;

  ProductEdit dialog(this, model->data(model->index(row, model->fieldIndex("id"))).toInt());
  if ( dialog.exec() == QDialog::Accepted )
  {
    model->select();
    model->fetchMore();  // else the list is not filled with all possible rows
    ui->tableView->resizeRowsToContents();
    ui->tableView->setCurrentIndex(proxyModel->mapFromSource(current));
  }
}

//--------------------------------------------------------------------------------
