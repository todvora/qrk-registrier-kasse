/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015 Christian Kvasny <chris@ckvsoft.at>
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
 */

#include <databasedefinition.h>

//--------------------------------------------------------------------------------

DatabaseDefinition::DatabaseDefinition(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::DatabaseDefinition)
{
  ui->setupUi(this);
}

DatabaseDefinition::~DatabaseDefinition()
{
  delete ui;
}

//--------------------------------------------------------------------------------

QString DatabaseDefinition::getDbType() const
{
  if ( ui->sqlite->isChecked() )
    return QLatin1String("QSQLITE");
  else
    return QLatin1String("QMYSQL");
}

//--------------------------------------------------------------------------------

QString DatabaseDefinition::getUserName() const
{
  return ui->username->text();
}

//--------------------------------------------------------------------------------

QString DatabaseDefinition::getPassword() const
{
  return ui->password->text();
}

//--------------------------------------------------------------------------------

QString DatabaseDefinition::getHostName() const
{
  return ui->hostname->text();
}

//--------------------------------------------------------------------------------
