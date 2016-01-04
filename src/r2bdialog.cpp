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

#include "r2bdialog.h"

R2BDialog::R2BDialog(QWidget *parent) :
  QDialog(parent), ui(new Ui::R2BDialog)
{

  ui->setupUi(this);

  connect (ui->pushButton, SIGNAL(clicked(bool)), SLOT(accept(bool)));
}

void R2BDialog::accept(bool)
{

  invoiceNum = ui->invoiceNum->text();
  invoiceSum = ui->invoiceSum->text();
  QDialog::accept();
}

QString R2BDialog::getInvoiceNum()
{
  if (invoiceNum.isEmpty())
    return "Zahlungsbeleg für Rechnung 1 - nicht für den Vorsteuerabzug geeignet";

  return QString("Zahlungsbeleg für Rechnung %1 - nicht für den Vorsteuerabzug geeignet" ).arg(invoiceNum);
}

QString R2BDialog::getInvoiceSum()
{
  invoiceSum.replace(',','.');
  if (invoiceSum.isEmpty())
    return "0.0";

  QString text = QString::number(invoiceSum.toDouble(),'f',2).toUpper();
  return text;
}
