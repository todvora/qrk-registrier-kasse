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

#include <QtWidgets>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QPrinterInfo>

#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QSettings &s, QWidget *parent)
  : QDialog(parent), settings(s)
{

  general = new GeneralTab(settings);
  master = new MasterDataTab(settings);
  printer = new PrinterTab(settings);
  extra = new ExtraTab(settings);

  tabWidget = new QTabWidget;
  tabWidget->addTab(master, tr("Stammdaten"));
  tabWidget->addTab(printer, tr("Drucker"));
  tabWidget->addTab(general, tr("Allgemein"));
  tabWidget->addTab(extra, tr("Extra"));

  QPushButton *pushButton = new QPushButton;
  pushButton->setMinimumHeight(60);
  pushButton->setMinimumWidth(0);

  QIcon icon = QIcon(":icons/ok.png");
  QSize size = QSize(32,32);
  pushButton->setIcon(icon);
  pushButton->setIconSize(size);
  pushButton->setText(tr("OK"));
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding );
  buttonLayout->addItem(spacer);
  buttonLayout->addWidget(pushButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabWidget);
  mainLayout->addLayout(buttonLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Einstellungen"));

  connect(pushButton, SIGNAL(clicked()), this, SLOT(accept()));

}

void SettingsDialog::accept()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='printHeader'").arg(general->getHeader()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='printFooter'").arg(general->getFooter()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='backupTarget'").arg(general->getBackupPath()));

  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopName'").arg(master->getShopName()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopOwner'").arg(master->getShopOwner()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopAddress'").arg(master->getShopAddress()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopUid'").arg(master->getShopUid()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopCashRegisterId'").arg(master->getShopCashRegisterId()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='currency'").arg(master->getShopCurrency()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='taxlocation'").arg(master->getShopTaxes()));

  settings.setValue("useInputNetPrice", extra->getInputNetPrice());

  settings.setValue("paperFormat", printer->getPaperFormat());
  settings.setValue("useReportPrinter", printer->getUseReportPrinter());
  settings.setValue("logoRight", printer->getIsLogoRight());
  settings.setValue("numberCopies", printer->getNumberCopies());
  settings.setValue("reportPrinter", printer->getReportPrinter());
  settings.setValue("receiptPrinter", printer->getReceiptPrinter());
  settings.setValue("paperWidth", printer->getpaperWidth());
  settings.setValue("paperHeight", printer->getpaperHeight());
  settings.setValue("marginLeft", printer->getmarginLeft());
  settings.setValue("marginTop", printer->getmarginTop());
  settings.setValue("marginRight", printer->getmarginRight());
  settings.setValue("marginBottom", printer->getmarginBottom());

  QDialog::accept();

}

ExtraTab::ExtraTab(QSettings &settings, QWidget *parent)
  : QWidget(parent)
{

  useInputNetPriceCheck = new QCheckBox;
  useInputNetPriceCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);;

  QGroupBox *registerGroup = new QGroupBox();
  registerGroup->setTitle(tr("Kasse"));
  QFormLayout *extraLayout = new QFormLayout;
  extraLayout->setAlignment(Qt::AlignLeft);
  extraLayout->addRow(tr("Netto Eingabe ermöglichen:"),useInputNetPriceCheck);
  registerGroup->setLayout(extraLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(registerGroup);

  mainLayout->addStretch(1);
  setLayout(mainLayout);

  useInputNetPriceCheck->setChecked(settings.value("useInputNetPrice", false).toBool());

}

bool ExtraTab::getInputNetPrice()
{
  return useInputNetPriceCheck->isChecked();
}

GeneralTab::GeneralTab(QSettings &, QWidget *parent)
  : QWidget(parent)
{
  QLabel *printHeaderLabel = new QLabel(tr("BON Kopfzeilen:"));
  printHeaderEdit = new QTextEdit();

  QLabel *printFooterLabel = new QLabel(tr("BON Fußzeilen:"));
  printFooterEdit = new QTextEdit();

  QLabel *backupDirectoryLabel = new QLabel(tr("Backup Ziel:"));
  backupDirectoryEdit = new QLineEdit();
  backupDirectoryEdit->setEnabled(false);
  QPushButton *backupButton = new QPushButton;
  QHBoxLayout *backupLayout = new QHBoxLayout;
  backupLayout->addWidget(backupDirectoryEdit);
  backupLayout->addWidget(backupButton);
  QIcon icon = QIcon(":icons/save.png");
  QSize size = QSize(32,32);
  backupButton->setIcon(icon);
  backupButton->setIconSize(size);
  backupButton->setText(tr("Ziel"));

  connect(backupButton, SIGNAL(clicked(bool)), this, SLOT(backupButton_clicked()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(printHeaderLabel);
  mainLayout->addWidget(printHeaderEdit);
  mainLayout->addWidget(printFooterLabel);
  mainLayout->addWidget(printFooterEdit);
  mainLayout->addWidget(backupDirectoryLabel);
  mainLayout->addLayout(backupLayout);

  mainLayout->addStretch(1);
  setLayout(mainLayout);

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  query.prepare("SELECT strValue FROM globals WHERE name='printHeader'");
  query.exec();
  if (query.next())
    printHeaderEdit->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('printHeader', '')");

  query.prepare("SELECT strValue FROM globals WHERE name='printFooter'");
  query.exec();
  if ( query.next() )
    printFooterEdit->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('printFooter', '')");

  query.exec("SELECT strValue FROM globals WHERE name='backupTarget'");
  if ( query.next() )
    backupDirectoryEdit->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('backupTarget', '')");

}
void GeneralTab::backupButton_clicked()
{

  QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis öffnen"),
                                                   getBackupPath(),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);

  if (!path.isEmpty())
    backupDirectoryEdit->setText(path);

}

QString GeneralTab::getHeader()
{
  return printHeaderEdit->toPlainText();
}

QString GeneralTab::getFooter()
{
  return printFooterEdit->toPlainText();
}

QString GeneralTab::getBackupPath()
{
  return backupDirectoryEdit->text();
}

MasterDataTab::MasterDataTab(QSettings &, QWidget *parent)
  : QWidget(parent)
{

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  shopName = new QLineEdit;
  shopOwner = new QLineEdit;
  shopAddress = new QTextEdit;
  shopUid = new QLineEdit;
  shopCashRegisterId = new QLineEdit;
  taxlocation = new QComboBox;
  currency = new QComboBox;

  query.prepare("SELECT strValue FROM globals WHERE name='shopName'");
  query.exec();
  if (query.next())
    shopName->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('shopName', '')");

  query.prepare("SELECT strValue FROM globals WHERE name='shopOwner'");
  query.exec();
  if (query.next())
    shopOwner->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('shopOwner', '')");

  query.prepare("SELECT strValue FROM globals WHERE name='shopAddress'");
  query.exec();
  if (query.next())
    shopAddress->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('shopAddress', '')");

  query.prepare("SELECT strValue FROM globals WHERE name='shopUid'");
  query.exec();
  if (query.next())
    shopUid->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('shopUid', '')");

  currency->addItem("EUR");
  currency->addItem("CHF");

  taxlocation->addItem("AT");
  taxlocation->addItem("DE");
  taxlocation->addItem("CH");

  query.prepare("SELECT strValue FROM globals WHERE name='shopCashRegisterId'");
  query.exec();
  if (query.next())
    shopCashRegisterId->setText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('shopCashRegisterId', '')");

  query.prepare("SELECT strValue FROM globals WHERE name='currency'");
  query.exec();
  if (query.next())
    currency->setCurrentText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('currency', '')");

  query.prepare("SELECT strValue FROM globals WHERE name='taxlocation'");
  query.exec();
  if (query.next())
    taxlocation->setCurrentText(query.value(0).toString());
  else
    query.exec("INSERT INTO globals (name, strValue) VALUES('taxlocation', '')");

  QFormLayout *shopLayout = new QFormLayout;
  shopLayout->setAlignment(Qt::AlignLeft);
  shopLayout->addRow(tr("Firmenname:"),shopName);
  shopLayout->addRow(tr("Eigentümer:"),shopOwner);
  shopLayout->addRow(tr("Adresse:"),shopAddress);
  shopLayout->addRow(tr("UID:"),shopUid);
  shopLayout->addRow(tr("Kassenidentifikationsnummer:"),shopCashRegisterId);

  QGroupBox *currencyGroup = new QGroupBox();
  QFormLayout *currencyLayout = new QFormLayout;
  currency->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);;
  taxlocation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);;
  currencyLayout->addRow(tr("Währung:"),currency);
  currencyLayout->addRow(tr("Steuersatz:"),taxlocation);
  currencyGroup->setLayout(currencyLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(shopLayout);
  mainLayout->addWidget(currencyGroup);

  mainLayout->addStretch(1);
  setLayout(mainLayout);

}

QString MasterDataTab::getShopName()
{
  return shopName->text();
}

QString MasterDataTab::getShopOwner()
{
  return shopOwner->text();
}

QString MasterDataTab::getShopAddress()
{
  return shopAddress->toPlainText();
}

QString MasterDataTab::getShopUid()
{
  return shopUid->text();
}

QString MasterDataTab::getShopTaxes()
{
  return taxlocation->currentText();
}

QString MasterDataTab::getShopCurrency()
{
  return currency->currentText();
}

QString MasterDataTab::getShopCashRegisterId()
{
  return shopCashRegisterId->text();
}

PrinterTab::PrinterTab(QSettings &settings, QWidget *parent)
  : QWidget(parent)
{

  QLabel *reportPrinterLabel = new QLabel(tr("Bericht Drucker:"));
  reportPrinterCombo = new QComboBox();

  QLabel *paperFormatLabel = new QLabel(tr("Papierformat:"));
  paperFormatCombo = new QComboBox();

  receiptPrinterCombo = new QComboBox();

  useReportPrinterCheck = new QCheckBox();
  useLogoRightCheck = new QCheckBox();

  numberCopiesSpin = new QSpinBox();
  numberCopiesSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  paperWidthSpin = new QSpinBox();
  paperWidthSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  paperHeightSpin = new QSpinBox();
  paperHeightSpin->setMaximum(4000);
  paperHeightSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  marginLeftSpin = new QSpinBox();
  marginLeftSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  marginRightSpin = new QSpinBox();
  marginRightSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  marginTopSpin = new QSpinBox();
  marginTopSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  marginBottomSpin = new QSpinBox();
  marginBottomSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  QGroupBox *reportPrinterGroup = new QGroupBox();
  QGroupBox *receiptPrinterGroup = new QGroupBox(tr("BON Drucker"));

  QHBoxLayout *reportPrinterLayout = new QHBoxLayout();
  reportPrinterLayout->addWidget(reportPrinterLabel);
  reportPrinterLayout->addWidget(reportPrinterCombo);
  reportPrinterLayout->addWidget(paperFormatLabel);
  reportPrinterLayout->addWidget(paperFormatCombo);
  reportPrinterGroup->setLayout(reportPrinterLayout);

  QFormLayout *receiptPrinterLayout = new QFormLayout;
  receiptPrinterLayout->setAlignment(Qt::AlignLeft);
  receiptPrinterLayout->addRow(tr("Drucker:"), receiptPrinterCombo);
  receiptPrinterLayout->addRow("Berichtdrucker für den zweiten Ausdruck verwenden", useReportPrinterCheck);
  receiptPrinterLayout->addRow("Logo auf der rechten Seite Drucken", useLogoRightCheck);

  receiptPrinterLayout->addRow(tr("Anzahl Kopien"), numberCopiesSpin);
  receiptPrinterLayout->addRow(tr("Papier Breite [mm]"), paperWidthSpin);
  receiptPrinterLayout->addRow(tr("Papier Höhe [mm]"), paperHeightSpin);
  receiptPrinterLayout->addRow(tr("Rand Links [mm]"), marginLeftSpin);
  receiptPrinterLayout->addRow(tr("Rand Rechts [mm]"), marginRightSpin);
  receiptPrinterLayout->addRow(tr("Rand Oben [mm]"), marginTopSpin);
  receiptPrinterLayout->addRow(tr("Rand Unten [mm]"), marginBottomSpin);
  receiptPrinterGroup->setLayout(receiptPrinterLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(reportPrinterGroup);
  mainLayout->addWidget(receiptPrinterGroup);

  mainLayout->addStretch(1);
  setLayout(mainLayout);

  QString receiptPrinter = settings.value("receiptPrinter").toString();
  QString reportPrinter = settings.value("reportPrinter").toString();
  QList<QPrinterInfo> availablePrinters = QPrinterInfo::availablePrinters();
  for (int i = 0; i < availablePrinters.count(); i++)
  {
    receiptPrinterCombo->addItem(availablePrinters[i].printerName());
    reportPrinterCombo->addItem(availablePrinters[i].printerName());
    if ( receiptPrinter == availablePrinters[i].printerName() )
      receiptPrinterCombo->setCurrentIndex(i);
    if ( reportPrinter == availablePrinters[i].printerName() )
      reportPrinterCombo->setCurrentIndex(i);
  }

  paperFormatCombo->addItem("A4");
  paperFormatCombo->addItem("A5");
  paperFormatCombo->setEditText(settings.value("paperFormat", "A4").toString());

  useReportPrinterCheck->setChecked(settings.value("useReportPrinter", true).toBool());
  useLogoRightCheck->setChecked(settings.value("logoRight", false).toBool());
  numberCopiesSpin->setValue(settings.value("numberCopies", 1).toInt());
  paperWidthSpin->setValue(settings.value("paperWidth", 80).toInt());
  paperHeightSpin->setValue(settings.value("paperHeight", 3000).toInt());

  marginLeftSpin->setValue(settings.value("marginLeft", 0).toInt());
  marginTopSpin->setValue(settings.value("marginTop", 17).toInt());
  marginRightSpin->setValue(settings.value("marginRight", 5).toInt());
  marginBottomSpin->setValue(settings.value("marginBottom", 0).toInt());

}

QString PrinterTab::getReportPrinter()
{
  return reportPrinterCombo->currentText();
}

QString PrinterTab::getPaperFormat()
{
  return paperFormatCombo->currentText();
}

QString PrinterTab::getReceiptPrinter()
{
  return receiptPrinterCombo->currentText();
}

bool PrinterTab::getUseReportPrinter()
{
  return useReportPrinterCheck->isChecked();
}

bool PrinterTab::getIsLogoRight()
{
  return useLogoRightCheck->isChecked();
}

int PrinterTab::getNumberCopies()
{
  return numberCopiesSpin->value();
}

int PrinterTab::getpaperWidth()
{
  return paperWidthSpin->value();
}

int PrinterTab::getpaperHeight()
{
  return paperHeightSpin->value();
}

int PrinterTab::getmarginLeft()
{
  return marginLeftSpin->value();
}

int PrinterTab::getmarginRight()
{
  return marginRightSpin->value();
}

int PrinterTab::getmarginTop()
{
  return marginTopSpin->value();
}

int PrinterTab::getmarginBottom()
{
  return marginBottomSpin->value();
}
