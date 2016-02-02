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
 */

#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include "ui_aboutdlg.h"

#include <QDialog>

class AboutDlg : public QDialog
{
    Q_OBJECT

public:
    AboutDlg( QWidget* parent = 0);
    ~AboutDlg();

private:
    Ui::AboutDlg *ui;

};

#endif // ABOUTDLG_H