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

#include "givendialog.h"
#include "database.h"
#include "ui_givendialog.h"

GivenDialog::GivenDialog(double &sum, QWidget *parent) :
    QDialog(parent), ui(new Ui::GivenDialog)
{
    ui->setupUi(this);
    ui->lcdNumber->setDigitCount(10);
    QDoubleValidator *doubleVal = new QDoubleValidator(0.0, 9999999.99, 2, this);
    doubleVal->setNotation(QDoubleValidator::StandardNotation);
    ui->givenEdit->setValidator(doubleVal);
    this->sum = sum;
    ui->toPayLabel->setText(tr("Zu bezahlen: %1 %2").arg(QString::number(sum,'f',2)).arg(Database::getCurrency()));

    connect (ui->givenEdit, SIGNAL(textChanged(QString)), SLOT(textChanged(QString)));
    connect (ui->pushButton, SIGNAL(clicked(bool)), SLOT(accept(bool)));

}

GivenDialog::~GivenDialog()
{
    delete ui;
}

void GivenDialog::accept(bool)
{

  QDialog::accept();
}

void GivenDialog::textChanged(QString given)
{

    given.replace(",",".");
    double retourMoney = given.toDouble() - sum;

    ui->lcdNumber->display(QString::number(retourMoney,'f',2));

}
