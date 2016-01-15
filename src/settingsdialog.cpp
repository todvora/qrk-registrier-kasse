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
  receiptprinter = new ReceiptPrinterTab(settings);
  extra = new ExtraTab(settings);

  tabWidget = new QTabWidget;
  tabWidget->addTab(master, tr("Stammdaten"));
  tabWidget->addTab(printer, tr("Drucker"));
  tabWidget->addTab(receiptprinter, tr("BON Drucker"));
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

  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopName'").arg(master->getShopName()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopOwner'").arg(master->getShopOwner()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopAddress'").arg(master->getShopAddress()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopUid'").arg(master->getShopUid()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='shopCashRegisterId'").arg(master->getShopCashRegisterId()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='currency'").arg(master->getShopCurrency()));
  query.exec(QString("UPDATE globals SET strValue='%1' WHERE name='taxlocation'").arg(master->getShopTaxes()));

  settings.setValue("logo", general->getLogo());
  settings.setValue("dataDirectory", general->getDataDirectory());
  settings.setValue("backupDirectory", general->getBackupDirectory());
  settings.setValue("useInputNetPrice", extra->getInputNetPrice());

  settings.setValue("reportPrinter", printer->getReportPrinter());
  settings.setValue("paperFormat", printer->getPaperFormat());

  settings.setValue("invoiceCompanyPrinter", printer->getInvoiceCompanyPrinter());
  settings.setValue("invoiceCompanyPaperFormat", printer->getInvoiceCompanyPaperFormat());
  settings.setValue("invoiceCompanyMarginLeft", printer->getInvoiceCompanyMarginLeft());
  settings.setValue("invoiceCompanyMarginTop", printer->getInvoiceCompanyMarginTop());
  settings.setValue("invoiceCompanyMarginRight", printer->getInvoiceCompanyMarginRight());
  settings.setValue("invoiceCompanyMarginBottom", printer->getInvoiceCompanyMarginBottom());

  settings.setValue("receiptPrinter", receiptprinter->getReceiptPrinter());
  settings.setValue("useReportPrinter", receiptprinter->getUseReportPrinter());
  settings.setValue("logoRight", receiptprinter->getIsLogoRight());
  settings.setValue("numberCopies", receiptprinter->getNumberCopies());
  settings.setValue("paperWidth", receiptprinter->getpaperWidth());
  settings.setValue("paperHeight", receiptprinter->getpaperHeight());
  settings.setValue("marginLeft", receiptprinter->getmarginLeft());
  settings.setValue("marginTop", receiptprinter->getmarginTop());
  settings.setValue("marginRight", receiptprinter->getmarginRight());
  settings.setValue("marginBottom", receiptprinter->getmarginBottom());

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

GeneralTab::GeneralTab(QSettings &settings, QWidget *parent)
  : QWidget(parent)
{
  QLabel *printHeaderLabel = new QLabel(tr("BON Kopfzeilen:"));
  printHeaderEdit = new QTextEdit();

  QLabel *printFooterLabel = new QLabel(tr("BON Fußzeilen:"));
  printFooterEdit = new QTextEdit();

  QLabel *logoLabel = new QLabel(tr("Logo:"));
  QLabel *dataDirectoryLabel = new QLabel(tr("Daten Verzeichnis:"));
  QLabel *backupDirectoryLabel = new QLabel(tr("Backup Verzeichnis:"));

  logoEdit = new QLineEdit();
  logoEdit->setEnabled(false);
  backupDirectoryEdit = new QLineEdit();
  backupDirectoryEdit->setEnabled(false);
  dataDirectoryEdit = new QLineEdit();
  dataDirectoryEdit->setEnabled(false);

  QPushButton *logoButton = new QPushButton;
  QPushButton *backupDirectoryButton = new QPushButton;
  QPushButton *dataDirectoryButton = new QPushButton;

  QHBoxLayout *logoLayout = new QHBoxLayout;
  QHBoxLayout *backupDirectoryLayout = new QHBoxLayout;
  QHBoxLayout *dataDirectoryLayout = new QHBoxLayout;

  logoLayout->addWidget(logoEdit);
  logoLayout->addWidget(logoButton);
  dataDirectoryLayout->addWidget(dataDirectoryEdit);
  dataDirectoryLayout->addWidget(dataDirectoryButton);
  backupDirectoryLayout->addWidget(backupDirectoryEdit);
  backupDirectoryLayout->addWidget(backupDirectoryButton);

  QIcon icon = QIcon(":icons/save.png");
  QSize size = QSize(32,32);
  logoButton->setIcon(icon);
  logoButton->setIconSize(size);
  logoButton->setText(tr("Auswahl"));

  dataDirectoryButton->setIcon(icon);
  dataDirectoryButton->setIconSize(size);
  dataDirectoryButton->setText(tr("Auswahl"));

  backupDirectoryButton->setIcon(icon);
  backupDirectoryButton->setIconSize(size);
  backupDirectoryButton->setText(tr("Auswahl"));

  connect(logoButton, SIGNAL(clicked(bool)), this, SLOT(logoButton_clicked()));
  connect(backupDirectoryButton, SIGNAL(clicked(bool)), this, SLOT(backupDirectoryButton_clicked()));
  connect(dataDirectoryButton, SIGNAL(clicked(bool)), this, SLOT(dataDirectoryButton_clicked()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(printHeaderLabel);
  mainLayout->addWidget(printHeaderEdit);
  mainLayout->addWidget(printFooterLabel);
  mainLayout->addWidget(printFooterEdit);
  mainLayout->addWidget(logoLabel);
  mainLayout->addLayout(logoLayout);
  mainLayout->addWidget(dataDirectoryLabel);
  mainLayout->addLayout(dataDirectoryLayout);
  mainLayout->addWidget(backupDirectoryLabel);
  mainLayout->addLayout(backupDirectoryLayout);

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


  logoEdit->setText(settings.value("logo", "./logo.png").toString());
  dataDirectoryEdit->setText(settings.value("dataDirectory", "./").toString());
  backupDirectoryEdit->setText(settings.value("backupDirectory", "./").toString());

}

void GeneralTab::logoButton_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this,
      tr("Logo Auswahl"), getLogo(), tr("Image Files (*.png *.jpg *.bmp)"));

  if (!fileName.isEmpty())
    logoEdit->setText(fileName);

}

void GeneralTab::backupDirectoryButton_clicked()
{

  QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis Auswahl"),
                                                   getBackupDirectory(),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);

  if (!path.isEmpty())
    backupDirectoryEdit->setText(path);

}

void GeneralTab::dataDirectoryButton_clicked()
{

  QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis Auswahl"),
                                                   getDataDirectory(),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);

  if (!path.isEmpty())
    dataDirectoryEdit->setText(path);

}

QString GeneralTab::getHeader()
{
  return printHeaderEdit->toPlainText();
}

QString GeneralTab::getFooter()
{
  return printFooterEdit->toPlainText();
}

QString GeneralTab::getLogo()
{
  return logoEdit->text();
}

QString GeneralTab::getBackupDirectory()
{
  return backupDirectoryEdit->text();
}

QString GeneralTab::getDataDirectory()
{
  return dataDirectoryEdit->text();
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
  reportPrinterCombo = new QComboBox();
  paperFormatCombo = new QComboBox();

  invoiceCompanyPrinterCombo = new QComboBox();
  invoiceCompanyPaperFormatCombo = new QComboBox();

  invoiceCompanyMarginLeftSpin = new QSpinBox();
  invoiceCompanyMarginLeftSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  invoiceCompanyMarginRightSpin = new QSpinBox();
  invoiceCompanyMarginRightSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  invoiceCompanyMarginTopSpin = new QSpinBox();
  invoiceCompanyMarginTopSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  invoiceCompanyMarginBottomSpin = new QSpinBox();
  invoiceCompanyMarginBottomSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  QGroupBox *reportPrinterGroup = new QGroupBox();
  QFormLayout *reportPrinterLayout = new QFormLayout();
  reportPrinterLayout->addRow(tr("Bericht Drucker:"), reportPrinterCombo);
  reportPrinterLayout->addRow(tr("Papierformat:"), paperFormatCombo);
  reportPrinterGroup->setLayout(reportPrinterLayout);

  QGroupBox *invoiceCompanyPrinterGroup = new QGroupBox();
  QFormLayout *invoiceCompanyPrinterLayout = new QFormLayout;
  invoiceCompanyPrinterLayout->setAlignment(Qt::AlignLeft);
  invoiceCompanyPrinterLayout->addRow(tr("Firmenrechnung Drucker:"),invoiceCompanyPrinterCombo);
  invoiceCompanyPrinterLayout->addRow(tr("Firmenrechnung Papierformat:"), invoiceCompanyPaperFormatCombo);

  invoiceCompanyPrinterLayout->addRow(tr("Rand Links [mm]"), invoiceCompanyMarginLeftSpin);
  invoiceCompanyPrinterLayout->addRow(tr("Rand Rechts [mm]"), invoiceCompanyMarginRightSpin);
  invoiceCompanyPrinterLayout->addRow(tr("Rand Oben [mm]"), invoiceCompanyMarginTopSpin);
  invoiceCompanyPrinterLayout->addRow(tr("Rand Unten [mm]"), invoiceCompanyMarginBottomSpin);
  invoiceCompanyPrinterGroup->setLayout(invoiceCompanyPrinterLayout);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(reportPrinterGroup);
  mainLayout->addWidget(invoiceCompanyPrinterGroup);

  mainLayout->addStretch(1);
  setLayout(mainLayout);

  QString reportPrinter = settings.value("reportPrinter").toString();
  QString invoiceCompanyPrinter = settings.value("invoiceCompanyPrinter").toString();

  QList<QPrinterInfo> availablePrinters = QPrinterInfo::availablePrinters();
  for (int i = 0; i < availablePrinters.count(); i++)
  {
    reportPrinterCombo->addItem(availablePrinters[i].printerName());
    invoiceCompanyPrinterCombo->addItem(availablePrinters[i].printerName());

    if ( reportPrinter == availablePrinters[i].printerName() )
      reportPrinterCombo->setCurrentIndex(i);
    if ( invoiceCompanyPrinter == availablePrinters[i].printerName() )
      invoiceCompanyPrinterCombo->setCurrentIndex(i);

  }
  QString x = settings.value("paperFormat", "A4").toString();
  paperFormatCombo->addItem("A4");
  paperFormatCombo->addItem("A5");
  paperFormatCombo->setCurrentText(settings.value("paperFormat", "A4").toString());

  invoiceCompanyPaperFormatCombo->addItem("A4");
  invoiceCompanyPaperFormatCombo->addItem("A5");
  invoiceCompanyPaperFormatCombo->setCurrentText(settings.value("invoiceCompanyPaperFormat", "A4").toString());

  invoiceCompanyMarginLeftSpin->setValue(settings.value("invoiceCompanyMarginLeft", 90).toInt());
  invoiceCompanyMarginTopSpin->setValue(settings.value("invoiceCompanyMarginTop", 50).toInt());
  invoiceCompanyMarginRightSpin->setValue(settings.value("invoiceCompanyMarginRight", 5).toInt());
  invoiceCompanyMarginBottomSpin->setValue(settings.value("invoiceCompanyMarginBottom", 0).toInt());

}

QString PrinterTab::getReportPrinter()
{
  return reportPrinterCombo->currentText();
}

QString PrinterTab::getPaperFormat()
{
  return paperFormatCombo->currentText();
}

QString PrinterTab::getInvoiceCompanyPrinter()
{
  return invoiceCompanyPrinterCombo->currentText();
}

QString PrinterTab::getInvoiceCompanyPaperFormat()
{
  return invoiceCompanyPaperFormatCombo->currentText();
}

int PrinterTab::getInvoiceCompanyMarginLeft()
{
  return invoiceCompanyMarginLeftSpin->value();
}

int PrinterTab::getInvoiceCompanyMarginRight()
{
  return invoiceCompanyMarginRightSpin->value();
}

int PrinterTab::getInvoiceCompanyMarginTop()
{
  return invoiceCompanyMarginTopSpin->value();
}

int PrinterTab::getInvoiceCompanyMarginBottom()
{
  return invoiceCompanyMarginBottomSpin->value();
}


ReceiptPrinterTab::ReceiptPrinterTab(QSettings &settings, QWidget *parent)
  : QWidget(parent)
{


  receiptPrinterCombo = new QComboBox();

  useReportPrinterCheck = new QCheckBox();
  useLogoRightCheck = new QCheckBox();

  numberCopiesSpin = new QSpinBox();
  numberCopiesSpin->setMinimum(1);
  numberCopiesSpin->setMaximum(2);
  numberCopiesSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  paperWidthSpin = new QSpinBox();
  paperWidthSpin->setMaximum(210);
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

  QGroupBox *receiptPrinterGroup = new QGroupBox(tr("BON Drucker"));

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
  mainLayout->addWidget(receiptPrinterGroup);

  mainLayout->addStretch(1);
  setLayout(mainLayout);

  QString receiptPrinter = settings.value("receiptPrinter").toString();
  QList<QPrinterInfo> availablePrinters = QPrinterInfo::availablePrinters();
  for (int i = 0; i < availablePrinters.count(); i++)
  {
    receiptPrinterCombo->addItem(availablePrinters[i].printerName());
    if ( receiptPrinter == availablePrinters[i].printerName() )
      receiptPrinterCombo->setCurrentIndex(i);
  }


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

QString ReceiptPrinterTab::getReceiptPrinter()
{
  return receiptPrinterCombo->currentText();
}

bool ReceiptPrinterTab::getUseReportPrinter()
{
  return useReportPrinterCheck->isChecked();
}

bool ReceiptPrinterTab::getIsLogoRight()
{
  return useLogoRightCheck->isChecked();
}

int ReceiptPrinterTab::getNumberCopies()
{
  return numberCopiesSpin->value();
}

int ReceiptPrinterTab::getpaperWidth()
{
  return paperWidthSpin->value();
}

int ReceiptPrinterTab::getpaperHeight()
{
  return paperHeightSpin->value();
}

int ReceiptPrinterTab::getmarginLeft()
{
  return marginLeftSpin->value();
}

int ReceiptPrinterTab::getmarginRight()
{
  return marginRightSpin->value();
}

int ReceiptPrinterTab::getmarginTop()
{
  return marginTopSpin->value();
}

int ReceiptPrinterTab::getmarginBottom()
{
  return marginBottomSpin->value();
}
