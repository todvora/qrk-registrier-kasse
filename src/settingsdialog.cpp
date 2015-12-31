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

#include <settingsdialog.h>

//--------------------------------------------------------------------------------

SettingsDialog::SettingsDialog(QWidget *parent, QSettings &s)
  : QDialog(parent), ui(new Ui::SettingsDialog), settings(s)
{
  ui->setupUi(this);
  QSqlDatabase dbc = QSqlDatabase::database("CN");

  QSqlQuery query(dbc);
  query.prepare("SELECT strValue FROM globals WHERE name='shopName'");
  query.exec();
  query.next();
  ui->printHeader->setText(query.value(0).toString());

  QString receiptPrinter = settings.value("receiptPrinter").toString();
  QString reportPrinter = settings.value("reportPrinter").toString();
  QList<QPrinterInfo> availablePrinters = QPrinterInfo::availablePrinters();
  for (int i = 0; i < availablePrinters.count(); i++)
  {
    ui->receiptPrinter->addItem(availablePrinters[i].printerName());
    ui->reportPrinter->addItem(availablePrinters[i].printerName());
    if ( receiptPrinter == availablePrinters[i].printerName() )
      ui->receiptPrinter->setCurrentIndex(i);
    if ( reportPrinter == availablePrinters[i].printerName() )
      ui->reportPrinter->setCurrentIndex(i);
  }

  ui->paperSize->setEditText(settings.value("paperFormat", "A4").toString());
  ui->useReportPrinter->setChecked(settings.value("useReportPrinter", true).toBool());

  ui->logoRight->setChecked(settings.value("logoRight", false).toBool());
  ui->numberCopies->setValue(settings.value("numberCopies", 1).toInt());
  ui->paperWidth->setValue(settings.value("paperWidth", 80).toInt());
  ui->paperHeight->setValue(settings.value("paperHeight", 3000).toInt());

  ui->marginLeft->setValue(settings.value("marginLeft", 0).toInt());
  ui->marginTop->setValue(settings.value("marginTop", 17).toInt());
  ui->marginRight->setValue(settings.value("marginRight", 5).toInt());
  ui->marginBottom->setValue(settings.value("marginBottom", 0).toInt());

  query.exec("SELECT strValue FROM globals WHERE name='backupTarget'");
  if ( query.next() )
    ui->backupTarget->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('backupTarget', '')");
}

SettingsDialog::~SettingsDialog()
{
  delete ui;
}

//--------------------------------------------------------------------------------

void SettingsDialog::accept()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  QString s = ui->printHeader->toPlainText();
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopName'").arg(ui->printHeader->toPlainText()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='backupTarget'").arg(ui->backupTarget->text()));

  settings.setValue("paperFormat", ui->paperSize->currentText());
  settings.setValue("useReportPrinter", ui->useReportPrinter->isChecked());
  settings.setValue("logoRight", ui->logoRight->isChecked());
  settings.setValue("numberCopies", ui->numberCopies->value());
  settings.setValue("reportPrinter", ui->reportPrinter->currentText());
  settings.setValue("receiptPrinter", ui->receiptPrinter->currentText());
  settings.setValue("paperWidth", ui->paperWidth->value());
  settings.setValue("paperHeight", ui->paperHeight->value());
  settings.setValue("marginLeft", ui->marginLeft->value());
  settings.setValue("marginTop", ui->marginTop->value());
  settings.setValue("marginRight", ui->marginRight->value());
  settings.setValue("marginBottom", ui->marginBottom->value());

  QDialog::accept();
}

