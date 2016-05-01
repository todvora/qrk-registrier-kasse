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

#include "groupedit.h"
#include "groupswidget.h"

#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSortFilterProxyModel>
#include <QSqlError>
#include <QMessageBox>
#include <QHeaderView>

//--------------------------------------------------------------------------------

GroupsWidget::GroupsWidget(QWidget *parent)
  : QWidget(parent), ui(new Ui::GroupsWidget)

{
  ui->setupUi(this);

  QSqlDatabase dbc = QSqlDatabase::database("CN");

  connect(ui->plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui->minus, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui->edit, SIGNAL(clicked()), this, SLOT(editSlot()));
  connect(ui->groupFilter, SIGNAL(textChanged(const QString &)), this, SLOT(filterGroup(const QString &)));

  model = new QSqlTableModel(this, dbc);
  model->setTable("groups");
  model->setFilter("id > 1");
  model->setEditStrategy(QSqlTableModel::OnFieldChange);
  model->select();

  model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Gruppe"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("visible"), Qt::Horizontal, tr("sichtbar"), Qt::DisplayRole);

  proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(model);
  proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  proxyModel->setFilterKeyColumn(model->fieldIndex("name"));

//  ui->tableView->setModel(model);
  ui->tableView->setModel(proxyModel);
  ui->tableView->setSortingEnabled(true);
  ui->tableView->setColumnHidden(model->fieldIndex("id"), true);
  ui->tableView->setColumnWidth(model->fieldIndex("name"), 250);
  ui->tableView->setColumnHidden(model->fieldIndex("color"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("button"), true);
  ui->tableView->setColumnHidden(model->fieldIndex("image"), true);

  ui->tableView->setAlternatingRowColors(true);
  ui->tableView->resizeColumnsToContents();
//  ui->tableView->horizontalHeader()->setSectionResizeMode(model->fieldIndex("name"), QHeaderView::Stretch);
  ui->tableView->horizontalHeader()->setStretchLastSection(true);

}

//--------------------------------------------------------------------------------

void GroupsWidget::filterGroup(const QString &filter)
{
  // show only matching items

  proxyModel->setFilterWildcard("*" + filter + "*");

  model->fetchMore();  // else the list is not filled with all possible rows
                       // e.g. when using mouse wheel it would fetch more items
                       // but on the WeTab we have no mouse
}

//--------------------------------------------------------------------------------

void GroupsWidget::plusSlot()
{
  GroupEdit dialog(this);
  dialog.exec();

  model->select();
}

//--------------------------------------------------------------------------------

void GroupsWidget::minusSlot()
{
  int row = proxyModel->mapToSource(ui->tableView->currentIndex()).row();
  if ( row == -1 )
    return;

  if ( QMessageBox::question(this, tr("Gruppe löschen"),
         tr("Möchten sie die Gruppe '%1' wirklich löschen ?")
            .arg(model->data(model->index(row, 1)).toString()),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  if ( !model->removeRow(row) )
  {
    QMessageBox::information(this, tr("Löschen nicht möglich"),
        tr("Gruppe '%1' kann nicht gelöscht werden, da sie noch in Verwendung ist")
           .arg(model->data(model->index(row, 1)).toString()));
  }
  model->select();
}

//--------------------------------------------------------------------------------

void GroupsWidget::editSlot()
{

  QModelIndex current(proxyModel->mapToSource(ui->tableView->currentIndex()));
  int row = current.row();
  if ( row == -1 )
    return;

  GroupEdit dialog(this, model->data(model->index(row, model->fieldIndex("id"))).toInt());
  if ( dialog.exec() == QDialog::Accepted )
  {
    model->select();
    ui->tableView->resizeRowsToContents();
    ui->tableView->setCurrentIndex(current);
  }
}

//--------------------------------------------------------------------------------
