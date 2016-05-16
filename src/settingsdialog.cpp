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

#include <QtWidgets>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QPrinterInfo>

#include "font/fontselector.h"
#include "documentprinter.h"
#include "settingsdialog.h"
#include "qrkregister.h"

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
  setMinimumWidth(600);

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

  settings.setValue("useLogo", general->getUseLogo());
  settings.setValue("logo", general->getLogo());
  settings.setValue("importDirectory", general->getImportDirectory());
  settings.setValue("backupDirectory", general->getBackupDirectory());

  if (extra->isFontsGroup()) {
    settings.setValue("systemfont", extra->getSystemFont());
    settings.setValue("printerfont", extra->getPrinterFont());
    settings.setValue("receiptprinterfont", extra->getReceiptPrinterFont());
  } else {
    settings.remove("systemfont");
    settings.remove("printerfont");
    settings.remove("receiptprinterfont");
  }

  settings.setValue("useInputNetPrice", extra->getInputNetPrice());
  settings.setValue("useMaximumItemSold", extra->getMaximumItemSold());

  settings.setValue("useDecimalQuantity", extra->getDecimalQuantity());
  settings.setValue("useGivenDialog", extra->getGivenDialog());

  settings.setValue("reportPrinterPDF", printer->getReportPrinterPDF());
  settings.setValue("reportPrinter", printer->getReportPrinter());
  settings.setValue("paperFormat", printer->getPaperFormat());

  settings.setValue("invoiceCompanyPrinter", printer->getInvoiceCompanyPrinter());
  settings.setValue("invoiceCompanyPaperFormat", printer->getInvoiceCompanyPaperFormat());
  settings.setValue("invoiceCompanyMarginLeft", printer->getInvoiceCompanyMarginLeft());
  settings.setValue("invoiceCompanyMarginTop", printer->getInvoiceCompanyMarginTop());
  settings.setValue("invoiceCompanyMarginRight", printer->getInvoiceCompanyMarginRight());
  settings.setValue("invoiceCompanyMarginBottom", printer->getInvoiceCompanyMarginBottom());

  settings.setValue("receiptPrinterHeading", receiptprinter->getReceiptPrinterHeading());
  settings.setValue("receiptPrinter", receiptprinter->getReceiptPrinter());
  settings.setValue("printCompanyNameBold", receiptprinter->getPrintCompanyNameBold());
  settings.setValue("useReportPrinter", receiptprinter->getUseReportPrinter());
  settings.setValue("logoRight", receiptprinter->getIsLogoRight());
  settings.setValue("qrcode", receiptprinter->getPrintQRCode());
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
  useInputNetPriceCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  useMaximumItemSoldCheck = new QCheckBox;
  useMaximumItemSoldCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  useDecimalQuantityCheck = new QCheckBox;
  useDecimalQuantityCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

  useGivenDialogCheck = new QCheckBox;
  useGivenDialogCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);;

  /* Fonts */

  systemFont = new QFont(QApplication::font());

  QList<QString> printerFontList = settings.value("printerfont", "Courier-New,10,100").toString().split(",");
  QList<QString> receiptPrinterFontList = settings.value("receiptprinterfont", "Courier-New,8,100").toString().split(",");

  printerFont = new QFont(printerFontList.at(0));
  printerFont->setPointSize(printerFontList.at(1).toInt());
  printerFont->setStretch(printerFontList.at(2).toInt());

  receiptPrinterFont = new QFont(receiptPrinterFontList.at(0));
  receiptPrinterFont->setPointSize(receiptPrinterFontList.at(1).toInt());
  receiptPrinterFont->setStretch(receiptPrinterFontList.at(2).toInt());

  QGroupBox *registerGroup = new QGroupBox();
  registerGroup->setTitle(tr("Kasse"));
  QFormLayout *extraLayout = new QFormLayout;
  extraLayout->setAlignment(Qt::AlignLeft);
  extraLayout->addRow(tr("Netto Eingabe ermöglichen:"),useInputNetPriceCheck);
  extraLayout->addRow(tr("Meistverkauften Artikel als Standard Artikel verwenden:"),useMaximumItemSoldCheck);
  extraLayout->addRow(tr("Dezimale Eingabe bei Anzahl Artikel:"),useDecimalQuantityCheck);
  extraLayout->addRow(tr("Betrag gegeben Dialog:"),useGivenDialogCheck);
  registerGroup->setLayout(extraLayout);

  fontsGroup = new QGroupBox();
  fontsGroup->setCheckable(true);
  fontsGroup->setTitle(tr("Schriftarten"));
  if (settings.value("systemfont").isNull())
    fontsGroup->setChecked(false);

  QGridLayout *fontsLayout = new QGridLayout;
  fontsLayout->setAlignment(Qt::AlignLeft);

  fontsLayout->addWidget( new QLabel(tr("Systemschrift:")), 1,1,1,1);
  fontsLayout->addWidget( new QLabel(tr("Druckerschrift:")), 2,1,1,1);
  fontsLayout->addWidget( new QLabel(tr("BON - Druckerschrift:")), 3,1,1,1);

  systemFontButton = new QPushButton(systemFont->family());
  systemFontButton->setFont(*systemFont);
  systemFontSizeLabel = new QLabel(QString::number(systemFont->pointSize()));
  systemFontStretchLabel = new QLabel(QString::number(systemFont->stretch()));

  printerFontButton = new QPushButton(printerFont->family());
  printerFontButton->setFont(*printerFont);
  printerFontSizeLabel = new QLabel(QString::number(printerFont->pointSize()));
  printerFontStretchLabel = new QLabel(QString::number(printerFont->stretch()));

  receiptPrinterFontButton = new QPushButton(receiptPrinterFont->family());
  receiptPrinterFontButton->setFont(*receiptPrinterFont);
  receiptPrinterFontSizeLabel = new QLabel(QString::number(receiptPrinterFont->pointSize()));
  receiptPrinterFontStretchLabel = new QLabel(QString::number(receiptPrinterFont->stretch()));

  QPushButton *printerTestButton = new QPushButton(tr("Drucktest"));
  QPushButton *receiptPrinterTestButton = new QPushButton(tr("Drucktest"));

  fontsLayout->addWidget( systemFontButton, 1,2,1,1);
  fontsLayout->addWidget( printerFontButton, 2,2,1,1);
  fontsLayout->addWidget( receiptPrinterFontButton, 3,2,1,1);

  fontsLayout->addWidget( new QLabel("Größe:"), 1,3,1,1);
  fontsLayout->addWidget( new QLabel("Größe:"), 2,3,1,1);
  fontsLayout->addWidget( new QLabel("Größe:"), 3,3,1,1);

  fontsLayout->addWidget( systemFontSizeLabel, 1,4,1,1);
  fontsLayout->addWidget( printerFontSizeLabel, 2,4,1,1);
  fontsLayout->addWidget( receiptPrinterFontSizeLabel, 3,4,1,1);

  fontsLayout->addWidget( new QLabel("Stretch:"), 1,5,1,1);
  fontsLayout->addWidget( new QLabel("Stretch:"), 2,5,1,1);
  fontsLayout->addWidget( new QLabel("Stretch:"), 3,5,1,1);

  fontsLayout->addWidget( systemFontStretchLabel, 1,6,1,1);
  fontsLayout->addWidget( printerFontStretchLabel, 2,6,1,1);
  fontsLayout->addWidget( receiptPrinterFontStretchLabel, 3,6,1,1);

  fontsLayout->addWidget( printerTestButton, 2,7,1,1);
  fontsLayout->addWidget( receiptPrinterTestButton, 3,7,1,1);

  fontsGroup->setLayout(fontsLayout);


  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(registerGroup);
  mainLayout->addWidget(fontsGroup);

  mainLayout->addStretch(1);
  setLayout(mainLayout);

  useInputNetPriceCheck->setChecked(settings.value("useInputNetPrice", false).toBool());
  useMaximumItemSoldCheck->setChecked(settings.value("useMaximumItemSold", false).toBool());
  useDecimalQuantityCheck->setChecked(settings.value("useDecimalQuantity", false).toBool());
  useGivenDialogCheck->setChecked(settings.value("useGivenDialog", false).toBool());

  connect(systemFontButton, SIGNAL(clicked(bool)), this, SLOT(systemFontButton_clicked(bool)));
  connect(printerFontButton, SIGNAL(clicked(bool)), this, SLOT(printerFontButton_clicked(bool)));
  connect(receiptPrinterFontButton, SIGNAL(clicked(bool)), this, SLOT(receiptPrinterFontButton_clicked(bool)));

  connect(printerTestButton, SIGNAL(clicked(bool)), this, SLOT(printerTestButton_clicked(bool)));
  connect(receiptPrinterTestButton, SIGNAL(clicked(bool)), this, SLOT(receiptPrinterTestButton_clicked(bool)));

  connect(fontsGroup, SIGNAL(toggled(bool)), this, SLOT(fontsGroup_toggled(bool)));

}

void ExtraTab::fontsGroup_toggled(bool toggled)
{

  if (toggled) {

  } else {
    QFont font;
    font.setFamily(font.defaultFamily());
    QApplication::setFont(font);
  }

}

bool ExtraTab::isFontsGroup()
{
  return fontsGroup->isChecked();
}

QString ExtraTab::getSystemFont()
{
  return QString("%1,%2,%3").arg(systemFont->family()).arg(systemFont->pointSize()).arg(systemFont->stretch());
}

QString ExtraTab::getPrinterFont()
{
  return QString("%1,%2,%3").arg(printerFont->family()).arg(printerFont->pointSize()).arg(printerFont->stretch());
}

QString ExtraTab::getReceiptPrinterFont()
{
  return QString("%1,%2,%3").arg(receiptPrinterFont->family()).arg(receiptPrinterFont->pointSize()).arg(receiptPrinterFont->stretch());
}

void ExtraTab::printerTestButton_clicked(bool)
{
  DocumentPrinter *p = new DocumentPrinter();
  p->printTestDocument(*printerFont);
}

void ExtraTab::receiptPrinterTestButton_clicked(bool)
{
  QRKRegister *reg = new QRKRegister(new QProgressBar());

  int id = Database::getLastReceiptNum();
  reg->setCurrentReceiptNum(id);

  QJsonObject data = reg->compileData();

  data["isTestPrint"] = true;
  data["comment"] = "DRUCKTEST BELEG";
  data["headerText"] = Database::getCustomerText(id);

  DocumentPrinter *p = new DocumentPrinter();
  p->printReceipt(data);
  delete p;

}

void ExtraTab::systemFontButton_clicked(bool)
{
  FontSelector *fontSelect = new FontSelector(*systemFont);
  if ( fontSelect->exec() == FontSelector::Accepted ) {
    systemFont = new QFont(fontSelect->getFont());
    systemFontButton->setText(systemFont->family());
    systemFontSizeLabel->setText(QString::number(systemFont->pointSize()));
    systemFontStretchLabel->setText(QString::number(systemFont->stretch()));
    QApplication::setFont(*systemFont);

  }

}

void ExtraTab::printerFontButton_clicked(bool)
{
  FontSelector *fontSelect = new FontSelector(*printerFont);
  if ( fontSelect->exec() == FontSelector::Accepted ) {
    printerFont = new QFont(fontSelect->getFont());
    printerFontButton->setText(printerFont->family());
    printerFontButton->setFont(*printerFont);
    printerFontSizeLabel->setText(QString::number(printerFont->pointSize()));
    printerFontStretchLabel->setText(QString::number(printerFont->stretch()));
  }

}

void ExtraTab::receiptPrinterFontButton_clicked(bool)
{
  FontSelector *fontSelect = new FontSelector(*receiptPrinterFont);
  if ( fontSelect->exec() == FontSelector::Accepted ) {
    receiptPrinterFont = new QFont(fontSelect->getFont());
    receiptPrinterFontButton->setText(receiptPrinterFont->family());
    receiptPrinterFontButton->setFont(*receiptPrinterFont);
    receiptPrinterFontSizeLabel->setText(QString::number(receiptPrinterFont->pointSize()));
    receiptPrinterFontStretchLabel->setText(QString::number(receiptPrinterFont->stretch()));

  }

}

bool ExtraTab::getInputNetPrice()
{
  return useInputNetPriceCheck->isChecked();
}

bool ExtraTab::getMaximumItemSold()
{
    return useMaximumItemSoldCheck->isChecked();
}

bool ExtraTab::getDecimalQuantity()
{
    return useDecimalQuantityCheck->isChecked();
}

bool ExtraTab::getGivenDialog()
{
    return useGivenDialogCheck->isChecked();
}

GeneralTab::GeneralTab(QSettings &settings, QWidget *parent)
  : QWidget(parent)
{
  QLabel *printHeaderLabel = new QLabel(tr("BON Kopfzeilen:"));
  printHeaderEdit = new QTextEdit();

  QLabel *printFooterLabel = new QLabel(tr("BON Fußzeilen:"));
  printFooterEdit = new QTextEdit();


  QLabel *logoLabel = new QLabel(tr("Logo:"));
  QLabel *importDirectoryLabel = new QLabel(tr("Server Mode\nImport Verzeichnis:"));
  QLabel *backupDirectoryLabel = new QLabel(tr("Backup Verzeichnis:"));

  useLogo = new QCheckBox(tr("Logo verwenden"));
  logoEdit = new QLineEdit();
  logoEdit->setEnabled(false);
  backupDirectoryEdit = new QLineEdit();
  backupDirectoryEdit->setEnabled(false);
  importDirectoryEdit = new QLineEdit();
  importDirectoryEdit->setEnabled(false);

  logoButton = new QPushButton;
  QPushButton *backupDirectoryButton = new QPushButton;
  QPushButton *importDirectoryButton = new QPushButton;

  QHBoxLayout *logoLayout = new QHBoxLayout;
  QHBoxLayout *backupDirectoryLayout = new QHBoxLayout;
  QHBoxLayout *importDirectoryLayout = new QHBoxLayout;

  logoLayout->addWidget(useLogo);
  logoLayout->addWidget(logoEdit);
  logoLayout->addWidget(logoButton);
  importDirectoryLayout->addWidget(importDirectoryEdit);
  importDirectoryLayout->addWidget(importDirectoryButton);
  backupDirectoryLayout->addWidget(backupDirectoryEdit);
  backupDirectoryLayout->addWidget(backupDirectoryButton);

  QIcon icon = QIcon(":icons/save.png");
  QSize size = QSize(32,32);
  logoButton->setIcon(icon);
  logoButton->setIconSize(size);
  logoButton->setText(tr("Auswahl"));

  importDirectoryButton->setIcon(icon);
  importDirectoryButton->setIconSize(size);
  importDirectoryButton->setText(tr("Auswahl"));

  backupDirectoryButton->setIcon(icon);
  backupDirectoryButton->setIconSize(size);
  backupDirectoryButton->setText(tr("Auswahl"));

  connect(logoButton, SIGNAL(clicked(bool)), this, SLOT(logoButton_clicked()));
  connect(backupDirectoryButton, SIGNAL(clicked(bool)), this, SLOT(backupDirectoryButton_clicked()));
  connect(importDirectoryButton, SIGNAL(clicked(bool)), this, SLOT(importDirectoryButton_clicked()));
  connect(useLogo, SIGNAL(toggled(bool)) , this, SLOT(useLogoCheck_toggled(bool)));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(printHeaderLabel);
  mainLayout->addWidget(printHeaderEdit);
  mainLayout->addWidget(printFooterLabel);
  mainLayout->addWidget(printFooterEdit);
  mainLayout->addWidget(logoLabel);
  mainLayout->addLayout(logoLayout);
  mainLayout->addWidget(importDirectoryLabel);
  mainLayout->addLayout(importDirectoryLayout);
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

  useLogo->setChecked(settings.value("useLogo", false).toBool());
  logoEdit->setText(settings.value("logo", "./logo.png").toString());
  importDirectoryEdit->setText(settings.value("importDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/import").toString());
  backupDirectoryEdit->setText(settings.value("backupDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());
}

void GeneralTab::useLogoCheck_toggled(bool toggled)
{
    logoButton->setEnabled(toggled);
}

bool GeneralTab::getUseLogo()
{
    return useLogo->isChecked();
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

void GeneralTab::importDirectoryButton_clicked()
{

  QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis Auswahl"),
                                                   getImportDirectory(),
                                                   QFileDialog::ShowDirsOnly
                                                   | QFileDialog::DontResolveSymlinks);

  if (!path.isEmpty())
    importDirectoryEdit->setText(path);

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

QString GeneralTab::getImportDirectory()
{
  return importDirectoryEdit->text();
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

  QRegExp re("^[^_]+$");
  QRegExpValidator *v = new QRegExpValidator(re);
  shopCashRegisterId->setValidator(v);
  shopCashRegisterId->setPlaceholderText(tr("z.B. Firmenname-1, QRK1, oder FN-1 ..."));

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
  reportPrinterCheck = new QCheckBox();
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
  reportPrinterLayout->addRow(tr("PDF erstellen:"), reportPrinterCheck);
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

  reportPrinterCheck->setChecked(settings.value("reportPrinterPDF", false).toBool());
  connect(reportPrinterCheck, SIGNAL(toggled(bool)),this, SLOT(reportPrinterCheck_toggled(bool)));

  reportPrinterCombo->setEnabled(!reportPrinterCheck->isChecked());

  QString x = settings.value("paperFormat", "A4").toString();
  paperFormatCombo->addItem("A4");
  paperFormatCombo->addItem("A5");
  // paperFormatCombo->addItem("POS");
  paperFormatCombo->setCurrentText(settings.value("paperFormat", "A4").toString());

  invoiceCompanyPaperFormatCombo->addItem("A4");
  invoiceCompanyPaperFormatCombo->addItem("A5");
  invoiceCompanyPaperFormatCombo->setCurrentText(settings.value("invoiceCompanyPaperFormat", "A4").toString());

  invoiceCompanyMarginLeftSpin->setValue(settings.value("invoiceCompanyMarginLeft", 90).toInt());
  invoiceCompanyMarginTopSpin->setValue(settings.value("invoiceCompanyMarginTop", 50).toInt());
  invoiceCompanyMarginRightSpin->setValue(settings.value("invoiceCompanyMarginRight", 5).toInt());
  invoiceCompanyMarginBottomSpin->setValue(settings.value("invoiceCompanyMarginBottom", 0).toInt());

}

void PrinterTab::reportPrinterCheck_toggled(bool checkState)
{
  reportPrinterCombo->setEnabled(!checkState);
}

bool PrinterTab::getReportPrinterPDF()
{
  return reportPrinterCheck->isChecked();
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
  receiptPrinterHeading = new QComboBox();

  printCompanyNameBoldCheck = new QCheckBox();
  useReportPrinterCheck = new QCheckBox();
  useLogoRightCheck = new QCheckBox();
  printQRCodeCheck = new QCheckBox();

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
  receiptPrinterLayout->addRow(tr("Überschrift:"), receiptPrinterHeading);
  receiptPrinterLayout->addRow(tr("Drucker:"), receiptPrinterCombo);
  receiptPrinterLayout->addRow(tr("Firmenname Fett drucken:"), printCompanyNameBoldCheck);
  receiptPrinterLayout->addRow("Berichtdrucker für den zweiten Ausdruck verwenden", useReportPrinterCheck);
  receiptPrinterLayout->addRow("Logo auf der rechten Seite drucken", useLogoRightCheck);
  receiptPrinterLayout->addRow("QRCode drucken", printQRCodeCheck);

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

  receiptPrinterHeading->addItem("KASSABON");
  receiptPrinterHeading->addItem("KASSENBON");
  receiptPrinterHeading->addItem("Zahlungsbestätigung");
  receiptPrinterHeading->setCurrentText(settings.value("receiptPrinterHeading", "KASSABON").toString());

  QString receiptPrinter = settings.value("receiptPrinter").toString();
  QList<QPrinterInfo> availablePrinters = QPrinterInfo::availablePrinters();
  for (int i = 0; i < availablePrinters.count(); i++)
  {
    receiptPrinterCombo->addItem(availablePrinters[i].printerName());
    if ( receiptPrinter == availablePrinters[i].printerName() )
      receiptPrinterCombo->setCurrentIndex(i);
  }


  printCompanyNameBoldCheck->setChecked(settings.value("printCompanyNameBold", true).toBool());
  useReportPrinterCheck->setChecked(settings.value("useReportPrinter", true).toBool());
  useLogoRightCheck->setChecked(settings.value("logoRight", false).toBool());
  printQRCodeCheck->setChecked(settings.value("qrcode", true).toBool());

  numberCopiesSpin->setValue(settings.value("numberCopies", 1).toInt());
  paperWidthSpin->setValue(settings.value("paperWidth", 80).toInt());
  paperHeightSpin->setValue(settings.value("paperHeight", 3000).toInt());

  marginLeftSpin->setValue(settings.value("marginLeft", 0).toInt());
  marginTopSpin->setValue(settings.value("marginTop", 17).toInt());
  marginRightSpin->setValue(settings.value("marginRight", 5).toInt());
  marginBottomSpin->setValue(settings.value("marginBottom", 0).toInt());

}

bool ReceiptPrinterTab::getPrintCompanyNameBold()
{
  return printCompanyNameBoldCheck->isChecked();
}

QString ReceiptPrinterTab::getReceiptPrinterHeading()
{
  return receiptPrinterHeading->currentText();
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

bool ReceiptPrinterTab::getPrintQRCode()
{
  return printQRCodeCheck->isChecked();
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
