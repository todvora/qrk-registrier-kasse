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

#include "depexportdialog.h"
#include "ui_depexportdialog.h"

DepExportDialog::DepExportDialog(QWidget *parent, QProgressBar *progressbar) :
  QDialog(parent),
  ui(new Ui::DepExportDialog)
{
  ui->setupUi(this);

  this->progressbar = progressbar;
  this->fromDT = new QDateTime;
  this->toDT = new QDateTime;

  QString sDate = QString("%1-01-01").arg(QDate::currentDate().year());
  QDate date = QDate::fromString(sDate,"yyyy-MM-dd");

  ui->dateEditTo->setDate(QDate::currentDate());
  ui->dateEditFrom->setDate(date);

  fromDT->setDate(date);
  toDT->setDate(QDate::currentDate());
  toDT->setTime(QTime::fromString("23:59:59"));

  connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(onCancelButton_clicked()));
  connect(ui->exportButton, SIGNAL(clicked()), this, SLOT(onExportButton_clicked()));

  connect(ui->dateEditFrom, SIGNAL(dateChanged(QDate)), this, SLOT(onDateEditFrom_dateChanged(QDate)));
  connect(ui->dateEditTo, SIGNAL(dateChanged(QDate)), this, SLOT(onDateEditTo_dateChanged(QDate)));

}

DepExportDialog::~DepExportDialog()
{
  progressbar->reset();
  delete ui;
}

void DepExportDialog::onCancelButton_clicked()
{
  this->close();
}

void DepExportDialog::onDateEditTo_dateChanged(const QDate &date)
{
  toDT->setDate(date);
  toDT->setTime(QTime::fromString("23:59:59"));
}

void DepExportDialog::onDateEditFrom_dateChanged(const QDate &date)
{
  fromDT->setDate(date);
}

void DepExportDialog::onExportButton_clicked()
{
  QString filename = QFileDialog::getSaveFileName( this, "Save file", "", ".csv");

  /* Try and open a file for output */
  QFile outputFile(filename);
  outputFile.open(QIODevice::WriteOnly);

  /* Check it opened OK */
  if(!outputFile.isOpen()){
    // qDebug() << "- Error, unable to open" << filename << "for output";
  }

  /* Point a QTextStream object at the file */
  QTextStream outStream(&outputFile);

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  q.prepare(QString("SELECT text FROM dep WHERE id < 4"));
  q.exec();
  while (q.next())
  {
    /* Write the line to the file */
    outStream << q.value(0).toString() + "\n";
  }

  QString query = QString("SELECT version, cashregisterid, text FROM dep WHERE datetime BETWEEN '%1' AND '%2' AND id > 3").arg(fromDT->toString(Qt::ISODate)).arg(toDT->toString(Qt::ISODate));

  // qDebug() << "DepExportDialog::onExportButton_clicked() query:" << query;
  q.prepare(query);
  q.exec();

  int i = 0;
  int count = q.record().count();

  progressbar->setMaximum(count);
  while (q.next())
  {
    i++;
    progressbar->setValue(i);
    QString s = QString("%1\t%2\t%3\t%4\n").arg(i).arg(q.value(0).toString()).arg(q.value(1).toString()).arg(q.value(2).toString());
    outStream << s;
  }

  /* Close the file */
  outputFile.close();

  this->close();
}
