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

#include <QSqlQuery>

//--------------------------------------------------------------------------------

GroupEdit::GroupEdit(QWidget *parent, int theId)
  : QDialog(parent), ui(new Ui::GroupEdit), id(theId)
{
  ui->setupUi(this);

  const QStringList colorNames = QColor::colorNames();
  int index = 0;
  ui->colorComboBox->addItem(tr("Standard Farbe"));
  const QModelIndex idx = ui->colorComboBox->model()->index(index++, 0);
  ui->colorComboBox->model()->setData(idx, "", Qt::BackgroundColorRole);

  foreach (const QString &colorName, colorNames) {
      const QColor color(colorName);
      ui->colorComboBox->addItem(colorName, color);
      const QModelIndex idx = ui->colorComboBox->model()->index(index++, 0);
      ui->colorComboBox->model()->setData(idx, color, Qt::BackgroundColorRole);
  }

  if ( id != -1 )
  {
    QSqlDatabase dbc = QSqlDatabase::database("CN");

    QSqlQuery query(QString("SELECT name,visible,color FROM groups WHERE id=%1").arg(id), dbc);
    query.next();

    ui->name->setText(query.value(0).toString());
    ui->visibleCheckBox->setChecked(query.value(1).toBool());

    int i;
    for (i = 0; i <= ui->colorComboBox->count(); i++) {
        QString color = ui->colorComboBox->model()->index(i, 0).data(Qt::BackgroundColorRole).toString();
        if ( query.value(2).toString() == color )
            break;
    }

    if (i > ui->colorComboBox->count())
      i = 0;

    ui->colorComboBox->setCurrentIndex(i);
    QPalette palette(ui->colorComboBox->palette());
    QColor color(query.value(2).toString());
    palette.setColor(QPalette::Active,QPalette::Button, color);
    palette.setColor(QPalette::Highlight, color);
//    palette.setColor(QPalette::ButtonText, Qt::white);
    ui->colorComboBox->setPalette(palette);

  }
}

//--------------------------------------------------------------------------------

void GroupEdit::accept()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  QString color = ui->colorComboBox->model()->index(ui->colorComboBox->currentIndex(), 0).data(Qt::BackgroundColorRole).toString();

  if ( id == -1 )  // new entry
  {
    query.exec(QString("INSERT INTO groups (name, visible, color) VALUES('%1', %2, '%3')")
                       .arg(ui->name->text())
                       .arg(ui->visibleCheckBox->isChecked())
                       .arg(color));
  }
  else
  {
    query.exec(QString("UPDATE groups SET name='%1',visible=%2, color='%3' WHERE id=%4")
                       .arg(ui->name->text())
                       .arg(ui->visibleCheckBox->isChecked())
                       .arg(color)
                       .arg(id));
  }

  QDialog::accept();
}

//--------------------------------------------------------------------------------
