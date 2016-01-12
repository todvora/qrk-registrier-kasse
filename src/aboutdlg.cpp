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

#include "aboutdlg.h"

AboutDlg::AboutDlg( QWidget* parent)
    : QDialog( parent), ui(new Ui::AboutDlg)
{
  ui->setupUi(this);
    // connect( btClose, SIGNAL( clicked() ), this, SLOT( close() ) );
}

AboutDlg::~AboutDlg()
{
    // no need to delete child widgets, Qt does it all for us
}
