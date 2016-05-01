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

#ifndef R2BDIALOG_H
#define R2BDIALOG_H

#include "qrkdelegate.h"
#include "qrkdialog.h"

#include "ui_r2bdialog.h"

class R2BDialog : public QRKDialog
{
    Q_OBJECT
  public:
    explicit R2BDialog(QWidget *parent = 0);

    QString getInvoiceNum();
    QString getInvoiceSum();

  signals:

  public slots:
    virtual void accept(bool);
    void setOkButtonEnabled(bool isAccptableInput);


  private:
    Ui::R2BDialog *ui;
    QString invoiceNum;
    QString invoiceNet;
    QString invoiceSum;
};

#endif // R2BDIALOG_H
