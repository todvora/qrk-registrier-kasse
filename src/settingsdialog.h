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

#ifndef SETTINGSDIALOG
#define SETTINGSDIALOG

#include "database.h"
#include <QDialog>
#include <QSettings>
#include <QSqlQuery>
#include <QPrinterInfo>

#include "ui_settingsdialog.h"


class SettingsDialog : public QDialog
{
    Q_OBJECT

  public:
    SettingsDialog(QWidget *parent, QSettings &s);
    ~SettingsDialog();

  public slots:
    virtual void accept();

  private:
    Ui::SettingsDialog *ui;
    QSettings &settings;

};

#endif // SETTINGSDIALOG

