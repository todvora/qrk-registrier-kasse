/*currentCardReader
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015-2017 Christian Kvasny <chris@ckvsoft.at>
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
#include <QStackedWidget>

#include "font/fontselector.h"
#include "documentprinter.h"
#include "settingsdialog.h"
#include "qrkregister.h"
#include "utils/demomode.h"
#include "RK/rk_smartcardinfo.h"
#include "RK/rk_signaturemodulefactory.h"
#include "database.h"
#include "reports.h"
#include "preferences/qrksettings.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    m_journal = new Journal(this);
    m_general = new GeneralTab(this);
    m_master = new MasterDataTab(this);
    m_printer = new PrinterTab(this);
    m_receiptprinter = new ReceiptPrinterTab(this);
    m_receipt = new ReceiptTab(this);
    m_extra = new ExtraTab(this);
    m_scardreader = new SCardReaderTab(this);

    m_general->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_scardreader->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    m_tabWidget = new QTabWidget;
    m_tabWidget->addTab(m_master, tr("Stammdaten"));
    m_tabWidget->addTab(m_printer, tr("Drucker"));
    m_tabWidget->addTab(m_receiptprinter, tr("BON Drucker"));
    m_tabWidget->addTab(m_receipt, tr("Kassa BON"));
    m_tabWidget->addTab(m_general, tr("Verzeichnispfade"));
    m_tabWidget->addTab(m_extra, tr("Extra"));
    if (m_master->getShopTaxes() == "AT" && !Database::isCashRegisterInAktive())
        m_tabWidget->addTab(m_scardreader, tr("SignaturErstellungsEinheit"));

    m_tabWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QPushButton *pushButton = new QPushButton;
    pushButton->setMinimumHeight(60);
    pushButton->setMinimumWidth(0);

    QIcon icon = QIcon(":icons/ok.png");
    QSize size = QSize(24,24);
    pushButton->setIcon(icon);
    pushButton->setIconSize(size);
    pushButton->setText(tr("OK"));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding );
    buttonLayout->addItem(spacer);
    buttonLayout->addWidget(pushButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_tabWidget);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    setLayout(mainLayout);

    setWindowTitle(tr("Einstellungen"));
    setMinimumWidth(700);

    if ( QApplication::desktop()->height() < 650 )
    {
        pushButton->setMinimumHeight(0);
        setFixedHeight(550);
    }

    pushButton->setMinimumHeight(0);
    setFixedHeight(550);

    connect(pushButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_master, SIGNAL(taxChanged(QString)),this, SLOT(masterTaxChanged(QString)));
    connect(m_master, SIGNAL(taxChanged(QString)),m_general, SLOT(masterTaxChanged(QString)));
}

void SettingsDialog::masterTaxChanged(QString tax)
{
    if (tax == "AT" && !Database::isCashRegisterInAktive()) {
        m_tabWidget->addTab(m_scardreader, tr("SignaturErstellungsEinheit"));
    } else {
        int idx = m_tabWidget->indexOf(m_scardreader);
        m_tabWidget->removeTab(idx);
    }
}

void SettingsDialog::accept()
{

    QrkSettings *settings = new QrkSettings(this);

    settings->save2Database("printAdvertisingText", m_receipt->getAdvertisingText());
    settings->save2Database("printHeader", m_receipt->getHeader());
    settings->save2Database("printFooter", m_receipt->getFooter());

    settings->save2Database("shopName", m_master->getShopName());
    settings->save2Database("shopOwner", m_master->getShopOwner());
    settings->save2Database("shopAddress", m_master->getShopAddress());
    settings->save2Database("shopUid", m_master->getShopUid());
    settings->save2Database("shopCashRegisterId", m_master->getShopCashRegisterId());
    settings->save2Database("currency", m_master->getShopCurrency());
    settings->save2Database("taxlocation", m_master->getShopTaxes());
    settings->save2Database("defaulttax", m_extra->getDefaultTax());

    settings->save2Settings("useLogo", m_receipt->getUseLogo());
    settings->save2Settings("logo", m_receipt->getLogo());

    settings->save2Settings("useAdvertising", m_receipt->getUseAdvertising());
    settings->save2Settings("advertising", m_receipt->getAdvertising());

    QString oldDataDir = settings->value("sqliteDataDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data").toString();
    settings->save2Settings("sqliteDataDirectory", m_general->getDataDirectory());

    if (oldDataDir != settings->value("sqliteDataDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data").toString()) {
        Database::reopen();
    }

    settings->save2Settings("importDirectory", m_general->getImportDirectory());
    settings->save2Settings("backupDirectory", m_general->getBackupDirectory());
    settings->save2Settings("pdfDirectory", m_general->getPdfDirectory());
    settings->save2Settings("externalDepDirectory", m_general->getExternalDepDirectory());

    if (m_extra->isFontsGroup()) {
        settings->save2Settings("systemfont", m_extra->getSystemFont());
        settings->save2Settings("printerfont", m_extra->getPrinterFont());
        settings->save2Settings("receiptprinterfont", m_extra->getReceiptPrinterFont());
    } else {
        settings->removeSettings("systemfont");
        settings->removeSettings("printerfont");
        settings->removeSettings("receiptprinterfont");
    }

    settings->save2Settings("useInputNetPrice", m_extra->getInputNetPrice());
    settings->save2Settings("useMaximumItemSold", m_extra->getMaximumItemSold());
    settings->save2Settings("useDecimalQuantity", m_extra->getDecimalQuantity());
    settings->save2Settings("useGivenDialog", m_extra->getGivenDialog());
    settings->save2Settings("useReceiptPrintedDialog", m_extra->getReceiptPrintedDialog());
    settings->save2Settings("barcodeReaderPrefix", m_extra->getBarcodePrefix());

    settings->save2Settings("reportPrinterPDF", m_printer->getReportPrinterPDF());
    settings->save2Settings("reportPrinter", m_printer->getReportPrinter());
    settings->save2Settings("paperFormat", m_printer->getPaperFormat());

    settings->save2Settings("invoiceCompanyPrinter", m_printer->getInvoiceCompanyPrinter());
    settings->save2Settings("invoiceCompanyPaperFormat", m_printer->getInvoiceCompanyPaperFormat());
    settings->save2Settings("invoiceCompanyMarginLeft", m_printer->getInvoiceCompanyMarginLeft());
    settings->save2Settings("invoiceCompanyMarginTop", m_printer->getInvoiceCompanyMarginTop());
    settings->save2Settings("invoiceCompanyMarginRight", m_printer->getInvoiceCompanyMarginRight());
    settings->save2Settings("invoiceCompanyMarginBottom", m_printer->getInvoiceCompanyMarginBottom());

    settings->save2Settings("receiptPrinterHeading", m_receipt->getReceiptPrinterHeading());
    settings->save2Settings("receiptPrinter", m_receiptprinter->getReceiptPrinter());
    settings->save2Settings("printCollectionReceipt", m_receipt->getPrintCollectionReceipt());
    settings->save2Settings("collectionReceiptText", m_receipt->getCollectionReceiptText());
    settings->save2Settings("printCompanyNameBold", m_receipt->getPrintCompanyNameBold());
    settings->save2Settings("useReportPrinter", m_receiptprinter->getUseReportPrinter());
    settings->save2Settings("logoRight", m_receipt->getIsLogoRight());
    settings->save2Settings("qrcode", m_receipt->getPrintQRCode());
    settings->save2Settings("qrcodeleft", m_receipt->getPrintQRCodeLeft());
    settings->save2Settings("numberCopies", m_receiptprinter->getNumberCopies());
    settings->save2Settings("paperWidth", m_receiptprinter->getpaperWidth());
    settings->save2Settings("paperHeight", m_receiptprinter->getpaperHeight());
    settings->save2Settings("marginLeft", m_receiptprinter->getmarginLeft());
    settings->save2Settings("marginTop", m_receiptprinter->getmarginTop());
    settings->save2Settings("marginRight", m_receiptprinter->getmarginRight());
    settings->save2Settings("marginBottom", m_receiptprinter->getmarginBottom());

    settings->save2Settings("feedProdukt", m_receiptprinter->getfeedProdukt());
    settings->save2Settings("feedCompanyHeader", m_receiptprinter->getfeedCompanyHeader());
    settings->save2Settings("feedCompanyAddress", m_receiptprinter->getfeedCompanyAddress());
    settings->save2Settings("feedCashRegisterid", m_receiptprinter->getfeedCashRegisterid());
    settings->save2Settings("feedTimestamp()", m_receiptprinter->getfeedTimestamp());
    settings->save2Settings("feedTax", m_receiptprinter->getfeedTax());
    settings->save2Settings("feedPrintHeader", m_receiptprinter->getfeedPrintHeader());
    settings->save2Settings("feedHeaderText", m_receiptprinter->getfeedHeaderText());

    m_scardreader->saveSettings();

    /* remove unused settings*/
    settings->removeSettings("dataDirectory");

    QDialog::accept();

}

ExtraTab::ExtraTab(QWidget *parent)
    : QWidget(parent)
{
    QrkSettings settings;

    m_useInputNetPriceCheck = new QCheckBox;
    m_useInputNetPriceCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_useMaximumItemSoldCheck = new QCheckBox;
    m_useMaximumItemSoldCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_useDecimalQuantityCheck = new QCheckBox;
    m_useDecimalQuantityCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_useGivenDialogCheck = new QCheckBox;
    m_useGivenDialogCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_useReceiptPrintedDialogCheck = new QCheckBox;
    m_useReceiptPrintedDialogCheck->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    m_useReceiptPrintedDialogCheck->setEnabled(!m_useMaximumItemSoldCheck->isChecked());
    m_useReceiptPrintedDialogCheck->setToolTip(tr("Diese Option \"Beleg wurde gedruckt\" kann nur verwendet werden wenn\ndie Option \"Meistverkauften Artikel als Standard Artikel verwenden\" nicht aktiviert ist."));

    /* BarcodePrefixes */

    m_barcodePrefixesComboBox = new QComboBox();
    m_barcodePrefixesComboBox->addItem("F1", Qt::Key_F1);
    m_barcodePrefixesComboBox->addItem("F2", Qt::Key_F2);
    m_barcodePrefixesComboBox->addItem("F3", Qt::Key_F3);
    m_barcodePrefixesComboBox->addItem("F4", Qt::Key_F4);
    m_barcodePrefixesComboBox->addItem("F5", Qt::Key_F5);
    m_barcodePrefixesComboBox->addItem("F6", Qt::Key_F6);
    m_barcodePrefixesComboBox->addItem("F7", Qt::Key_F7);
    m_barcodePrefixesComboBox->addItem("F8", Qt::Key_F8);
    m_barcodePrefixesComboBox->addItem("F9", Qt::Key_F9);
    m_barcodePrefixesComboBox->addItem("F10", Qt::Key_F10);
    m_barcodePrefixesComboBox->addItem("F11", Qt::Key_F11);
    m_barcodePrefixesComboBox->addItem("F12", Qt::Key_F12);

    /* Default Taxes*/
    m_defaultTaxComboBox = new QComboBox();
    QSqlDatabase dbc= QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare(QString("SELECT tax FROM taxTypes WHERE taxlocation=:taxlocation ORDER BY id"));
    query.bindValue(":taxlocation", Database::getTaxLocation());
    if(!query.exec()){
      qWarning() << "Function Name: " << Q_FUNC_INFO << " Query:" << Database::getLastExecutedQuery(query);
    }
    while(query.next()){
      m_defaultTaxComboBox->addItem(query.value(0).toString());
    }
    m_defaultTaxComboBox->setCurrentText(Database::getDefaultTax());

    /* Fonts */

    m_systemFont = new QFont(QApplication::font());

    QList<QString> printerFontList = settings.value("printerfont", "Courier-New,10,100").toString().split(",");
    QList<QString> receiptPrinterFontList = settings.value("receiptprinterfont", "Courier-New,8,100").toString().split(",");

    m_printerFont = new QFont(printerFontList.at(0));
    m_printerFont->setPointSize(printerFontList.at(1).toInt());
    m_printerFont->setStretch(printerFontList.at(2).toInt());

    m_receiptPrinterFont = new QFont(receiptPrinterFontList.at(0));
    m_receiptPrinterFont->setPointSize(receiptPrinterFontList.at(1).toInt());
    m_receiptPrinterFont->setStretch(receiptPrinterFontList.at(2).toInt());

    QGroupBox *registerGroup = new QGroupBox();
    registerGroup->setTitle(tr("Kasse"));
    QFormLayout *extraLayout = new QFormLayout;
    extraLayout->setAlignment(Qt::AlignLeft);
    extraLayout->addRow(tr("Netto Eingabe ermöglichen:"),m_useInputNetPriceCheck);
    extraLayout->addRow(tr("Meistverkauften Artikel als Standard Artikel verwenden:"),m_useMaximumItemSoldCheck);
    extraLayout->addRow(tr("Dezimale Eingabe bei Anzahl Artikel:"),m_useDecimalQuantityCheck);
    extraLayout->addRow(tr("Standard Steuersatz:"),m_defaultTaxComboBox);
    extraLayout->addRow(tr("Barcodereader prefix:"),m_barcodePrefixesComboBox);

    registerGroup->setLayout(extraLayout);

    QGroupBox *dialogGroup = new QGroupBox();
    dialogGroup->setTitle(tr("Dialoge"));
    QFormLayout *dialogLayout = new QFormLayout;
    dialogLayout->setAlignment(Qt::AlignLeft);
    dialogLayout->addRow(tr("Betrag gegeben:"),m_useGivenDialogCheck);
    dialogLayout->addRow(tr("Beleg wurde gedruckt:"), m_useReceiptPrintedDialogCheck);

    dialogGroup->setLayout(dialogLayout);

    m_fontsGroup = new QGroupBox();
    m_fontsGroup->setCheckable(true);
    m_fontsGroup->setTitle(tr("Schriftarten"));
    if (settings.value("systemfont").isNull())
        m_fontsGroup->setChecked(false);

    QGridLayout *fontsLayout = new QGridLayout;
    fontsLayout->setAlignment(Qt::AlignLeft);

    fontsLayout->addWidget( new QLabel(tr("Systemschrift:")), 1,1,1,1);
    fontsLayout->addWidget( new QLabel(tr("Druckerschrift:")), 2,1,1,1);
    fontsLayout->addWidget( new QLabel(tr("BON - Druckerschrift:")), 3,1,1,1);

    m_systemFontButton = new QPushButton(m_systemFont->family());
    m_systemFontButton->setFont(*m_systemFont);
    m_systemFontSizeLabel = new QLabel(QString::number(m_systemFont->pointSize()));
    m_systemFontStretchLabel = new QLabel(QString::number(m_systemFont->stretch()));

    QFontInfo printerFontInfo(*m_printerFont);
    QString sPrinterFontInfo = printerFontInfo.family();

    m_printerFont->setFamily(sPrinterFontInfo);
    m_printerFontButton = new QPushButton(sPrinterFontInfo);
    m_printerFontButton->setFont(*m_printerFont);
    m_printerFontSizeLabel = new QLabel(QString::number(m_printerFont->pointSize()));
    m_printerFontStretchLabel = new QLabel(QString::number(m_printerFont->stretch()));

    QFontInfo receiptPrinterFontInfo(*m_receiptPrinterFont);
    QString sReceiptPrinterFontInfo = receiptPrinterFontInfo.family();

    m_receiptPrinterFont->setFamily(sReceiptPrinterFontInfo);
    m_receiptPrinterFontButton = new QPushButton(sReceiptPrinterFontInfo);
    m_receiptPrinterFontButton->setFont(*m_receiptPrinterFont);
    m_receiptPrinterFontSizeLabel = new QLabel(QString::number(m_receiptPrinterFont->pointSize()));
    m_receiptPrinterFontStretchLabel = new QLabel(QString::number(m_receiptPrinterFont->stretch()));

    QPushButton *printerTestButton = new QPushButton(tr("Drucktest"));
    QPushButton *receiptPrinterTestButton = new QPushButton(tr("Drucktest"));

    fontsLayout->addWidget( m_systemFontButton, 1,2,1,1);
    fontsLayout->addWidget( m_printerFontButton, 2,2,1,1);
    fontsLayout->addWidget( m_receiptPrinterFontButton, 3,2,1,1);

    fontsLayout->addWidget( new QLabel("Größe:"), 1,3,1,1);
    fontsLayout->addWidget( new QLabel("Größe:"), 2,3,1,1);
    fontsLayout->addWidget( new QLabel("Größe:"), 3,3,1,1);

    fontsLayout->addWidget( m_systemFontSizeLabel, 1,4,1,1);
    fontsLayout->addWidget( m_printerFontSizeLabel, 2,4,1,1);
    fontsLayout->addWidget( m_receiptPrinterFontSizeLabel, 3,4,1,1);

    fontsLayout->addWidget( new QLabel("Stretch:"), 1,5,1,1);
    fontsLayout->addWidget( new QLabel("Stretch:"), 2,5,1,1);
    fontsLayout->addWidget( new QLabel("Stretch:"), 3,5,1,1);

    fontsLayout->addWidget( m_systemFontStretchLabel, 1,6,1,1);
    fontsLayout->addWidget( m_printerFontStretchLabel, 2,6,1,1);
    fontsLayout->addWidget( m_receiptPrinterFontStretchLabel, 3,6,1,1);

    fontsLayout->addWidget( printerTestButton, 2,7,1,1);
    fontsLayout->addWidget( receiptPrinterTestButton, 3,7,1,1);

    m_fontsGroup->setLayout(fontsLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(registerGroup);
    mainLayout->addWidget(dialogGroup);
    mainLayout->addWidget(m_fontsGroup);

    mainLayout->addStretch(1);
    setLayout(mainLayout);

    m_useInputNetPriceCheck->setChecked(settings.value("useInputNetPrice", false).toBool());
    m_useMaximumItemSoldCheck->setChecked(settings.value("useMaximumItemSold", false).toBool());
    m_useDecimalQuantityCheck->setChecked(settings.value("useDecimalQuantity", false).toBool());
    m_useGivenDialogCheck->setChecked(settings.value("useGivenDialog", false).toBool());
    if (m_useReceiptPrintedDialogCheck->isEnabled())
        m_useReceiptPrintedDialogCheck->setChecked(settings.value("useReceiptPrintedDialog", true).toBool());
    else
        m_useReceiptPrintedDialogCheck->setChecked(false);

    int idx = m_barcodePrefixesComboBox->findData(settings.value("barcodeReaderPrefix", Qt::Key_F11).toInt());
    if ( idx != -1 ) {
        m_barcodePrefixesComboBox->setCurrentIndex(idx);
    }

    connect(m_systemFontButton, SIGNAL(clicked(bool)), this, SLOT(systemFontButton_clicked(bool)));
    connect(m_printerFontButton, SIGNAL(clicked(bool)), this, SLOT(printerFontButton_clicked(bool)));
    connect(m_receiptPrinterFontButton, SIGNAL(clicked(bool)), this, SLOT(receiptPrinterFontButton_clicked(bool)));
    connect(m_useMaximumItemSoldCheck, SIGNAL(clicked(bool)), this, SLOT(maximumSoldItemChanged(bool)));

    connect(printerTestButton, SIGNAL(clicked(bool)), this, SLOT(printerTestButton_clicked(bool)));
    connect(receiptPrinterTestButton, SIGNAL(clicked(bool)), this, SLOT(receiptPrinterTestButton_clicked(bool)));

    connect(m_fontsGroup, SIGNAL(toggled(bool)), this, SLOT(fontsGroup_toggled(bool)));
}

void ExtraTab::maximumSoldItemChanged(bool enabled)
{
    m_useReceiptPrintedDialogCheck->setEnabled(!enabled);
    if (enabled)
        m_useReceiptPrintedDialogCheck->setChecked(!enabled);
}

QString ExtraTab::getDefaultTax()
{
    int idx  = m_defaultTaxComboBox->currentIndex();
    return m_defaultTaxComboBox->itemText(idx);
}

int ExtraTab::getBarcodePrefix()
{
    int idx  = m_barcodePrefixesComboBox->currentIndex();
    return m_barcodePrefixesComboBox->itemData(idx).toInt();
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
    return m_fontsGroup->isChecked();
}

QString ExtraTab::getSystemFont()
{
    return QString("%1,%2,%3").arg(m_systemFont->family()).arg(m_systemFont->pointSize()).arg(m_systemFont->stretch());
}

QString ExtraTab::getPrinterFont()
{
    return QString("%1,%2,%3").arg(m_printerFont->family()).arg(m_printerFont->pointSize()).arg(m_printerFont->stretch());
}

QString ExtraTab::getReceiptPrinterFont()
{
    return QString("%1,%2,%3").arg(m_receiptPrinterFont->family()).arg(m_receiptPrinterFont->pointSize()).arg(m_receiptPrinterFont->stretch());
}

void ExtraTab::printerTestButton_clicked(bool)
{
    DocumentPrinter *p = new DocumentPrinter();
    p->printTestDocument(*m_printerFont);
}

void ExtraTab::receiptPrinterTestButton_clicked(bool)
{
    ReceiptItemModel *reg = new ReceiptItemModel(this);

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
    FontSelector *fontSelect = new FontSelector(*m_systemFont);
    if ( fontSelect->exec() == FontSelector::Accepted ) {
        m_systemFont = new QFont(fontSelect->getFont());
        m_systemFontButton->setText(m_systemFont->family());
        m_systemFontSizeLabel->setText(QString::number(m_systemFont->pointSize()));
        m_systemFontStretchLabel->setText(QString::number(m_systemFont->stretch()));
        QApplication::setFont(*m_systemFont);
    }
}

void ExtraTab::printerFontButton_clicked(bool)
{
    FontSelector *fontSelect = new FontSelector(*m_printerFont);
    if ( fontSelect->exec() == FontSelector::Accepted ) {
        m_printerFont = new QFont(fontSelect->getFont());
        m_printerFontButton->setText(m_printerFont->family());
        m_printerFontButton->setFont(*m_printerFont);
        m_printerFontSizeLabel->setText(QString::number(m_printerFont->pointSize()));
        m_printerFontStretchLabel->setText(QString::number(m_printerFont->stretch()));
    }
}

void ExtraTab::receiptPrinterFontButton_clicked(bool)
{
    FontSelector *fontSelect = new FontSelector(*m_receiptPrinterFont);
    if ( fontSelect->exec() == FontSelector::Accepted ) {
        m_receiptPrinterFont = new QFont(fontSelect->getFont());
        m_receiptPrinterFontButton->setText(m_receiptPrinterFont->family());
        m_receiptPrinterFontButton->setFont(*m_receiptPrinterFont);
        m_receiptPrinterFontSizeLabel->setText(QString::number(m_receiptPrinterFont->pointSize()));
        m_receiptPrinterFontStretchLabel->setText(QString::number(m_receiptPrinterFont->stretch()));
    }
}

bool ExtraTab::getInputNetPrice()
{
    return m_useInputNetPriceCheck->isChecked();
}

bool ExtraTab::getMaximumItemSold()
{
    return m_useMaximumItemSoldCheck->isChecked();
}

bool ExtraTab::getDecimalQuantity()
{
    return m_useDecimalQuantityCheck->isChecked();
}

bool ExtraTab::getGivenDialog()
{
    return m_useGivenDialogCheck->isChecked();
}

bool ExtraTab::getReceiptPrintedDialog()
{
    return m_useReceiptPrintedDialogCheck->isChecked();
}

GeneralTab::GeneralTab(QWidget *parent)
    : QWidget(parent)
{
    m_dataDirectoryEdit = new QLineEdit();
    m_dataDirectoryEdit->setEnabled(false);
    m_backupDirectoryEdit = new QLineEdit();
    m_backupDirectoryEdit->setEnabled(false);
    m_pdfDirectoryEdit = new QLineEdit();
    m_pdfDirectoryEdit->setEnabled(false);
    m_importDirectoryEdit = new QLineEdit();
    m_importDirectoryEdit->setEnabled(false);
    m_externalDepDirectoryEdit = new QLineEdit();
    m_externalDepDirectoryEdit->setEnabled(false);

    QPushButton *dataDirectoryButton = new QPushButton;
    QPushButton *backupDirectoryButton = new QPushButton;
    QPushButton *pdfDirectoryButton = new QPushButton;
    QPushButton *importDirectoryButton = new QPushButton;
    QPushButton *externalDepDirectoryButton = new QPushButton;

    QIcon icon = QIcon(":icons/save.png");
    QSize size = QSize(24,24);

    dataDirectoryButton->setIcon(icon);
    dataDirectoryButton->setIconSize(size);
    dataDirectoryButton->setText(tr("Auswahl"));

    importDirectoryButton->setIcon(icon);
    importDirectoryButton->setIconSize(size);
    importDirectoryButton->setText(tr("Auswahl"));

    backupDirectoryButton->setIcon(icon);
    backupDirectoryButton->setIconSize(size);
    backupDirectoryButton->setText(tr("Auswahl"));

    pdfDirectoryButton->setIcon(icon);
    pdfDirectoryButton->setIconSize(size);
    pdfDirectoryButton->setText(tr("Auswahl"));

    externalDepDirectoryButton->setIcon(icon);
    externalDepDirectoryButton->setIconSize(size);
    externalDepDirectoryButton->setText(tr("Auswahl"));

    QLabel *externalDepInfoLabel = new QLabel();
    externalDepInfoLabel->setWordWrap(true);
    externalDepInfoLabel->setAlignment(Qt::AlignTop);
    externalDepInfoLabel->setText(tr("Das vollständige DEP ist zumindest vierteljährlich auf einem elektronischen, externen "
                                     "Medium unveränderbar zu sichern. Als geeignete Medien gelten beispielsweise "
                                     "schreibgeschützte (abgeschlossene) externe Festplatten, USB-Sticks und Speicher externer "
                                     "Server, die vor unberechtigten Datenzugriffen geschützt sind. Die Unveränderbarkeit des "
                                     "Inhaltes der Daten ist durch die Signatur bzw. das Siegel und insbesondere durch die "
                                     "signierten Monatsbelege gegeben. Jede Sicherung ist gemäß § 132 BAO aufzubewahren."
                                     ));

    QLabel *externalDepDescriptionLabel = new QLabel(tr("DEP Information:"));
    externalDepDescriptionLabel->setAlignment(Qt::AlignTop);
    QGroupBox *pathGroup = new QGroupBox;
    QGridLayout *pathLayout = new QGridLayout;
    pathLayout->addWidget(new QLabel(tr("Daten Verzeichnis:")), 1,1);
    pathLayout->addWidget(new QLabel(tr("Server Mode Import Verzeichnis:")), 2,1);
    pathLayout->addWidget(new QLabel(tr("Backup Verzeichnis:")), 3,1);
    pathLayout->addWidget(new QLabel(tr("Pdf Verzeichnis:")), 4,1);

    pathLayout->addWidget(m_dataDirectoryEdit, 1,2);
    pathLayout->addWidget(m_importDirectoryEdit, 2,2);
    pathLayout->addWidget(m_backupDirectoryEdit, 3,2);
    pathLayout->addWidget(m_pdfDirectoryEdit, 4,2);

    pathLayout->addWidget(dataDirectoryButton, 1,3);
    pathLayout->addWidget(importDirectoryButton, 2,3);
    pathLayout->addWidget(backupDirectoryButton, 3,3);
    pathLayout->addWidget(pdfDirectoryButton, 4,3);

    pathGroup->setLayout(pathLayout);

    m_externalDepGroup = new QGroupBox;
    QGridLayout *externalDepLayout = new QGridLayout;
    externalDepLayout->addWidget(externalDepDescriptionLabel, 1,1);
    externalDepLayout->addWidget(externalDepInfoLabel, 1,2,1,2);
    externalDepLayout->addWidget(new QLabel(tr("Externes DEP Backup Verzeichnis:")), 2,1);
    externalDepLayout->addWidget(m_externalDepDirectoryEdit, 2,2);
    externalDepLayout->addWidget(externalDepDirectoryButton, 2,3);
    m_externalDepGroup->setLayout(externalDepLayout);

    connect(backupDirectoryButton, SIGNAL(clicked(bool)), this, SLOT(backupDirectoryButton_clicked()));
    connect(pdfDirectoryButton, SIGNAL(clicked(bool)), this, SLOT(pdfDirectoryButton_clicked()));
    connect(importDirectoryButton, SIGNAL(clicked(bool)), this, SLOT(importDirectoryButton_clicked()));
    connect(dataDirectoryButton, SIGNAL(clicked(bool)), this, SLOT(dataDirectoryButton_clicked()));
    connect(externalDepDirectoryButton, SIGNAL(clicked(bool)), this, SLOT(externalDepDirectoryButton_clicked()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(pathGroup);
    mainLayout->addWidget(m_externalDepGroup);

    mainLayout->addStretch(1);
    setLayout(mainLayout);

    QrkSettings settings;
    m_dataDirectoryEdit->setText(settings.value("sqliteDataDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data").toString());
    m_importDirectoryEdit->setText(settings.value("importDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());
    m_backupDirectoryEdit->setText(settings.value("backupDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());
    m_pdfDirectoryEdit->setText(settings.value("pdfDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+ "/pdf").toString());
    m_externalDepDirectoryEdit->setText(settings.value("externalDepDirectory", "").toString());

    masterTaxChanged(Database::getTaxLocation());
}

void GeneralTab::masterTaxChanged(QString tax)
{
    if (tax == "AT") {
        m_externalDepGroup->setVisible(true);
    } else {
        m_externalDepGroup->setVisible(false);
    }
    QrkSettings settings;
    settings.save2Database("taxlocation", tax);
}

void GeneralTab::backupDirectoryButton_clicked()
{

    QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis Auswahl"),
                                                     getBackupDirectory(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty())
        m_backupDirectoryEdit->setText(path);

}

void GeneralTab::pdfDirectoryButton_clicked()
{

    QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis Auswahl"),
                                                     getPdfDirectory(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty())
        m_pdfDirectoryEdit->setText(path);

}

void GeneralTab::dataDirectoryButton_clicked()
{

    QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis Auswahl"),
                                                     getDataDirectory(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty()) {
        if ( !moveDataFiles( getDataDirectory(), path)) {
            QMessageBox::critical(this,"Änderung des Daten Verzeichnis!",
                                  "Achtung!\n"
                                  "Das Verschieben der Daten ist Fehlgeschlagen.\n"
                                  "Sie müssen die Daten manuell verschieben und danach QRK neu starten."
                                  , "Ok");

        }

        m_dataDirectoryEdit->setText(path);
    }

}

bool GeneralTab::moveDataFiles( QString fromDir, QString toDir)
{

    QDir from(fromDir);
    QDir to(toDir);

    QStringList filter;
    filter << "*.db";
    QStringList fileList = from.entryList(filter, QDir::Files);
    foreach(QString fileName, fileList)
    {
        QFileInfo fi(from, fileName);
        if (fi.isFile())  {
            if (QFile::copy(fi.absoluteFilePath(), to.absoluteFilePath(fi.fileName()))) {
                if (!QFile::remove(fi.absoluteFilePath ())) {
                    return false;
                }
            } else {
                return false;
            }
        }
    }
    return true;
}

void GeneralTab::importDirectoryButton_clicked()
{

    QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis Auswahl"),
                                                     getImportDirectory(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty())
        m_importDirectoryEdit->setText(path);
}

void GeneralTab::externalDepDirectoryButton_clicked()
{

    QString path = QFileDialog::getExistingDirectory(this, tr("Verzeichnis Auswahl"),
                                                     getExternalDepDirectory(),
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);

    if (!path.isEmpty())
        m_externalDepDirectoryEdit->setText(path);
}

QString GeneralTab::getBackupDirectory()
{
    return m_backupDirectoryEdit->text();
}

QString GeneralTab::getPdfDirectory()
{
    return m_pdfDirectoryEdit->text();
}

QString GeneralTab::getDataDirectory()
{
    return m_dataDirectoryEdit->text();
}

QString GeneralTab::getImportDirectory()
{
    return m_importDirectoryEdit->text();
}

QString GeneralTab::getExternalDepDirectory()
{
    return m_externalDepDirectoryEdit->text();
}

MasterDataTab::MasterDataTab(QWidget *parent)
    : QWidget(parent)
{

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    m_shopName = new QLineEdit;
    m_shopOwner = new QLineEdit;
    m_shopAddress = new QTextEdit;
    m_shopUid = new QLineEdit;
    m_shopCashRegisterId = new QLineEdit;
    m_taxlocation = new QComboBox;
    m_currency = new QComboBox;

    connect(m_shopCashRegisterId, SIGNAL(editingFinished()), this, SLOT(cashRegisterIdChanged()));

    QRegExp re("^[^_]+$");
    QRegExpValidator *v = new QRegExpValidator(re);
    m_shopCashRegisterId->setValidator(v);
    m_shopCashRegisterId->setPlaceholderText(tr("z.B. Firmenname-1, QRK1, oder FN-1 ..."));

    query.prepare("SELECT strValue FROM globals WHERE name='shopName'");
    query.exec();
    if (query.next())
        m_shopName->setText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('shopName', '')");

    query.prepare("SELECT strValue FROM globals WHERE name='shopOwner'");
    query.exec();
    if (query.next())
        m_shopOwner->setText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('shopOwner', '')");

    query.prepare("SELECT strValue FROM globals WHERE name='shopAddress'");
    query.exec();
    if (query.next())
        m_shopAddress->setText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('shopAddress', '')");

    query.prepare("SELECT strValue FROM globals WHERE name='shopUid'");
    query.exec();
    if (query.next())
        m_shopUid->setText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('shopUid', '')");

    m_currency->addItem("EUR");
    m_currency->addItem("CHF");

    m_taxlocation->addItem("AT");
    m_taxlocation->addItem("DE");
    m_taxlocation->addItem("CH");

    query.prepare("SELECT strValue FROM globals WHERE name='shopCashRegisterId'");
    query.exec();
    if (query.next())
        m_shopCashRegisterId->setText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('shopCashRegisterId', '')");

    query.prepare("SELECT strValue FROM globals WHERE name='currency'");
    query.exec();
    if (query.next())
        m_currency->setCurrentText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('currency', '')");

    query.prepare("SELECT strValue FROM globals WHERE name='taxlocation'");
    query.exec();
    if (query.next())
        m_taxlocation->setCurrentText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('taxlocation', '')");

    QFormLayout *shopLayout = new QFormLayout;
    shopLayout->setAlignment(Qt::AlignLeft);
    shopLayout->addWidget(new QLabel());
    shopLayout->addRow(tr("Firmenname:"),m_shopName);
    shopLayout->addRow(tr("Eigentümer:"),m_shopOwner);
    shopLayout->addRow(tr("Adresse:"),m_shopAddress);
    shopLayout->addRow(tr("UID:"),m_shopUid);
    shopLayout->addRow(tr("Kassenidentifikationsnummer:"),m_shopCashRegisterId);

    QGroupBox *currencyGroup = new QGroupBox();
    QFormLayout *currencyLayout = new QFormLayout;
    m_currency->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);;
    m_taxlocation->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);;
    currencyLayout->addRow(tr("Währung:"), m_currency);
    currencyLayout->addRow(tr("Steuersatz:"), m_taxlocation);
    currencyGroup->setLayout(currencyLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(shopLayout);
    mainLayout->addWidget(currencyGroup);

    mainLayout->addStretch(1);
    setLayout(mainLayout);

    if (RKSignatureModule::isDEPactive()) {
        m_shopCashRegisterId->setEnabled(false);
        currencyGroup->setEnabled(false);
    }

    connect(m_taxlocation, SIGNAL(currentIndexChanged(QString)),this, SIGNAL(taxChanged(QString)));

}

void MasterDataTab::cashRegisterIdChanged()
{
    QrkSettings settings;
    settings.save2Database("shopCashRegisterId", getShopCashRegisterId());
}

QString MasterDataTab::getShopName()
{
    return m_shopName->text();
}

QString MasterDataTab::getShopOwner()
{
    return m_shopOwner->text();
}

QString MasterDataTab::getShopAddress()
{
    return m_shopAddress->toPlainText();
}

QString MasterDataTab::getShopUid()
{
    return m_shopUid->text();
}

QString MasterDataTab::getShopTaxes()
{
    return m_taxlocation->currentText();
}

QString MasterDataTab::getShopCurrency()
{
    return m_currency->currentText();
}

QString MasterDataTab::getShopCashRegisterId()
{
    return m_shopCashRegisterId->text();

}

PrinterTab::PrinterTab(QWidget *parent)
    : QWidget(parent)
{
    m_reportPrinterCheck = new QCheckBox();
    m_reportPrinterCombo = new QComboBox();
    m_paperFormatCombo = new QComboBox();

    m_invoiceCompanyPrinterCombo = new QComboBox();
    m_invoiceCompanyPaperFormatCombo = new QComboBox();

    m_invoiceCompanyMarginLeftSpin = new QSpinBox();
    m_invoiceCompanyMarginLeftSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_invoiceCompanyMarginRightSpin = new QSpinBox();
    m_invoiceCompanyMarginRightSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_invoiceCompanyMarginTopSpin = new QSpinBox();
    m_invoiceCompanyMarginTopSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_invoiceCompanyMarginBottomSpin = new QSpinBox();
    m_invoiceCompanyMarginBottomSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    QGroupBox *reportPrinterGroup = new QGroupBox();
    QFormLayout *reportPrinterLayout = new QFormLayout();
    reportPrinterLayout->addRow(tr("PDF erstellen:"), m_reportPrinterCheck);
    reportPrinterLayout->addRow(tr("Bericht Drucker:"), m_reportPrinterCombo);
    reportPrinterLayout->addRow(tr("Papierformat:"), m_paperFormatCombo);
    reportPrinterGroup->setLayout(reportPrinterLayout);

    QGroupBox *invoiceCompanyPrinterGroup = new QGroupBox();
    QFormLayout *invoiceCompanyPrinterLayout = new QFormLayout;
    invoiceCompanyPrinterLayout->setAlignment(Qt::AlignLeft);
    invoiceCompanyPrinterLayout->addRow(tr("Firmenrechnung Drucker:"), m_invoiceCompanyPrinterCombo);
    invoiceCompanyPrinterLayout->addRow(tr("Firmenrechnung Papierformat:"), m_invoiceCompanyPaperFormatCombo);

    invoiceCompanyPrinterLayout->addRow(tr("Rand Links [mm]"), m_invoiceCompanyMarginLeftSpin);
    invoiceCompanyPrinterLayout->addRow(tr("Rand Rechts [mm]"), m_invoiceCompanyMarginRightSpin);
    invoiceCompanyPrinterLayout->addRow(tr("Rand Oben [mm]"), m_invoiceCompanyMarginTopSpin);
    invoiceCompanyPrinterLayout->addRow(tr("Rand Unten [mm]"), m_invoiceCompanyMarginBottomSpin);
    invoiceCompanyPrinterGroup->setLayout(invoiceCompanyPrinterLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(reportPrinterGroup);
    mainLayout->addWidget(invoiceCompanyPrinterGroup);

    mainLayout->addStretch(1);
    setLayout(mainLayout);

    QrkSettings settings;
    QString reportPrinter = settings.value("reportPrinter").toString();
    QString invoiceCompanyPrinter = settings.value("invoiceCompanyPrinter").toString();

    QList<QPrinterInfo> availablePrinters = QPrinterInfo::availablePrinters();
    for (int i = 0; i < availablePrinters.count(); i++)
    {
        m_reportPrinterCombo->addItem(availablePrinters[i].printerName());
        m_invoiceCompanyPrinterCombo->addItem(availablePrinters[i].printerName());

        if ( reportPrinter == availablePrinters[i].printerName() )
            m_reportPrinterCombo->setCurrentIndex(i);
        if ( invoiceCompanyPrinter == availablePrinters[i].printerName() )
            m_invoiceCompanyPrinterCombo->setCurrentIndex(i);

    }

    m_reportPrinterCheck->setChecked(settings.value("reportPrinterPDF", false).toBool());
    connect(m_reportPrinterCheck, SIGNAL(toggled(bool)),this, SLOT(reportPrinterCheck_toggled(bool)));

    m_reportPrinterCombo->setEnabled(!m_reportPrinterCheck->isChecked());

    m_paperFormatCombo->addItem("A4");
    m_paperFormatCombo->addItem("A5");
    // paperFormatCombo->addItem("POS");
    m_paperFormatCombo->setCurrentText(settings.value("paperFormat", "A4").toString());

    m_invoiceCompanyPaperFormatCombo->addItem("A4");
    m_invoiceCompanyPaperFormatCombo->addItem("A5");
    m_invoiceCompanyPaperFormatCombo->setCurrentText(settings.value("invoiceCompanyPaperFormat", "A4").toString());

    m_invoiceCompanyMarginLeftSpin->setValue(settings.value("invoiceCompanyMarginLeft", 90).toInt());
    m_invoiceCompanyMarginTopSpin->setValue(settings.value("invoiceCompanyMarginTop", 50).toInt());
    m_invoiceCompanyMarginRightSpin->setValue(settings.value("invoiceCompanyMarginRight", 5).toInt());
    m_invoiceCompanyMarginBottomSpin->setValue(settings.value("invoiceCompanyMarginBottom", 0).toInt());

}

void PrinterTab::reportPrinterCheck_toggled(bool checkState)
{
    m_reportPrinterCombo->setEnabled(!checkState);
}

bool PrinterTab::getReportPrinterPDF()
{
    return m_reportPrinterCheck->isChecked();
}

QString PrinterTab::getReportPrinter()
{
    return m_reportPrinterCombo->currentText();
}

QString PrinterTab::getPaperFormat()
{
    return m_paperFormatCombo->currentText();
}

QString PrinterTab::getInvoiceCompanyPrinter()
{
    return m_invoiceCompanyPrinterCombo->currentText();
}

QString PrinterTab::getInvoiceCompanyPaperFormat()
{
    return m_invoiceCompanyPaperFormatCombo->currentText();
}

int PrinterTab::getInvoiceCompanyMarginLeft()
{
    return m_invoiceCompanyMarginLeftSpin->value();
}

int PrinterTab::getInvoiceCompanyMarginRight()
{
    return m_invoiceCompanyMarginRightSpin->value();
}

int PrinterTab::getInvoiceCompanyMarginTop()
{
    return m_invoiceCompanyMarginTopSpin->value();
}

int PrinterTab::getInvoiceCompanyMarginBottom()
{
    return m_invoiceCompanyMarginBottomSpin->value();
}


ReceiptPrinterTab::ReceiptPrinterTab(QWidget *parent)
    : QWidget(parent)
{
    m_receiptPrinterCombo = new QComboBox();
    m_useReportPrinterCheck = new QCheckBox();

    m_numberCopiesSpin = new QSpinBox();
    m_numberCopiesSpin->setMinimum(1);
    m_numberCopiesSpin->setMaximum(2);
    m_numberCopiesSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_paperWidthSpin = new QSpinBox();
    m_paperWidthSpin->setMaximum(210);
    m_paperWidthSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_paperHeightSpin = new QSpinBox();
    m_paperHeightSpin->setMaximum(4000);
    m_paperHeightSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_marginLeftSpin = new QSpinBox();
    m_marginLeftSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_marginRightSpin = new QSpinBox();
    m_marginRightSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_marginTopSpin = new QSpinBox();
    m_marginTopSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_marginBottomSpin = new QSpinBox();
    m_marginBottomSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_feedProduktSpin = new QSpinBox();
    m_feedProduktSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_feedCompanyHeaderSpin = new QSpinBox();
    m_feedCompanyHeaderSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_feedCompanyAddressSpin = new QSpinBox();
    m_feedCompanyAddressSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_feedCashRegisteridSpin = new QSpinBox();
    m_feedCashRegisteridSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_feedTimestampSpin = new QSpinBox();
    m_feedTimestampSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_feedTaxSpin = new QSpinBox();
    m_feedTaxSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_feedPrintHeaderSpin = new QSpinBox();
    m_feedPrintHeaderSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    m_feedHeaderTextSpin = new QSpinBox();
    m_feedHeaderTextSpin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    QGroupBox *receiptPrinterGroup = new QGroupBox(tr("BON Drucker"));


    QGridLayout *receiptPrinterLayout = new QGridLayout;
    receiptPrinterLayout->setAlignment(Qt::AlignLeft);

    receiptPrinterLayout->addWidget( new QLabel(tr("Drucker:")), 1,1,1,2);
    receiptPrinterLayout->addWidget( m_receiptPrinterCombo, 1,3,1,2);

    receiptPrinterLayout->addWidget( new QLabel(tr("Berichtdrucker für den zweiten Ausdruck verwenden:")), 2,1,1,2);
    receiptPrinterLayout->addWidget( m_useReportPrinterCheck, 2,3,1,2);

    receiptPrinterLayout->addWidget( new QLabel(tr("Anzahl Kopien")), 3,1,1,1);
    receiptPrinterLayout->addWidget( m_numberCopiesSpin, 3,2,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Papier Breite [mm]")), 4,1,1,1);
    receiptPrinterLayout->addWidget( m_paperWidthSpin, 4,2,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Papier Höhe [mm]")), 5,1,1,1);
    receiptPrinterLayout->addWidget( m_paperHeightSpin, 5,2,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Rand Links [mm]")), 6,1,1,1);
    receiptPrinterLayout->addWidget( m_marginLeftSpin, 6,2,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Rand Rechts [mm]")), 7,1,1,1);
    receiptPrinterLayout->addWidget( m_marginRightSpin, 7,2,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Rand Oben [mm]")), 8,1,1,1);
    receiptPrinterLayout->addWidget( m_marginTopSpin, 8,2,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Rand Unten [mm]")), 9,1,1,1);
    receiptPrinterLayout->addWidget( m_marginBottomSpin, 9,2,1,1);

    QLabel *infoTextLabel = new QLabel(tr("Zeilenabstand in Pixel nach ..."));
    infoTextLabel->setStyleSheet("font-weight: bold");
    receiptPrinterLayout->addWidget( infoTextLabel, 3,3,1,2);

    receiptPrinterLayout->addWidget( new QLabel(tr("Firmenname [px]")), 4,3,1,1);
    receiptPrinterLayout->addWidget( m_feedCompanyHeaderSpin, 4,4,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Adresszeile [px]")), 5,3,1,1);
    receiptPrinterLayout->addWidget( m_feedCompanyAddressSpin, 5,4,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Kassenidzeile [px]")), 6,3,1,1);
    receiptPrinterLayout->addWidget( m_feedCashRegisteridSpin, 6,4,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Zeitzeile [px]")), 7,3,1,1);
    receiptPrinterLayout->addWidget( m_feedTimestampSpin, 7,4,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Artikelzeile [px]")), 8,3,1,1);
    receiptPrinterLayout->addWidget( m_feedProduktSpin, 8,4,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Mwst. Zeile [px]")), 9,3,1,1);
    receiptPrinterLayout->addWidget( m_feedTaxSpin, 9,4,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("BON Kopfzeile [px]")), 10,3,1,1);
    receiptPrinterLayout->addWidget( m_feedPrintHeaderSpin, 10,4,1,1);

    receiptPrinterLayout->addWidget( new QLabel(tr("Kunden Zusatztext [px]")), 11,3,1,1);
    receiptPrinterLayout->addWidget( m_feedHeaderTextSpin, 11,4,1,1);

    receiptPrinterGroup->setLayout(receiptPrinterLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(receiptPrinterGroup);

    mainLayout->addStretch(1);
    setLayout(mainLayout);

    QrkSettings settings;
    QString receiptPrinter = settings.value("receiptPrinter").toString();
    QList<QPrinterInfo> availablePrinters = QPrinterInfo::availablePrinters();
    for (int i = 0; i < availablePrinters.count(); i++)
    {
        m_receiptPrinterCombo->addItem(availablePrinters[i].printerName());
        if ( receiptPrinter == availablePrinters[i].printerName() )
            m_receiptPrinterCombo->setCurrentIndex(i);
    }


    m_useReportPrinterCheck->setChecked(settings.value("useReportPrinter", true).toBool());
    m_numberCopiesSpin->setValue(settings.value("numberCopies", 1).toInt());
    m_paperWidthSpin->setValue(settings.value("paperWidth", 80).toInt());
    m_paperHeightSpin->setValue(settings.value("paperHeight", 3000).toInt());

    m_marginLeftSpin->setValue(settings.value("marginLeft", 0).toInt());
    m_marginTopSpin->setValue(settings.value("marginTop", 17).toInt());
    m_marginRightSpin->setValue(settings.value("marginRight", 5).toInt());
    m_marginBottomSpin->setValue(settings.value("marginBottom", 0).toInt());

    m_feedProduktSpin->setValue(settings.value("feedProdukt", 5).toInt());
    m_feedCompanyHeaderSpin->setValue(settings.value("feedCompanyHeader", 5).toInt());
    m_feedCompanyAddressSpin->setValue(settings.value("feedCompanyAddress", 5).toInt());
    m_feedCashRegisteridSpin->setValue(settings.value("feedCashRegisterid", 5).toInt());
    m_feedTimestampSpin->setValue(settings.value("feedTimestamp", 5).toInt());
    m_feedTaxSpin->setValue(settings.value("feedTaxSpin", 5).toInt());
    m_feedPrintHeaderSpin->setValue(settings.value("feedPrintHeader", 5).toInt());
    m_feedHeaderTextSpin->setValue(settings.value("feedHeaderText", 5).toInt());

}

QString ReceiptPrinterTab::getReceiptPrinter()
{
    return m_receiptPrinterCombo->currentText();
}

bool ReceiptPrinterTab::getUseReportPrinter()
{
    return m_useReportPrinterCheck->isChecked();
}

int ReceiptPrinterTab::getNumberCopies()
{
    return m_numberCopiesSpin->value();
}

int ReceiptPrinterTab::getpaperWidth()
{
    return m_paperWidthSpin->value();
}

int ReceiptPrinterTab::getpaperHeight()
{
    return m_paperHeightSpin->value();
}

int ReceiptPrinterTab::getmarginLeft()
{
    return m_marginLeftSpin->value();
}

int ReceiptPrinterTab::getmarginRight()
{
    return m_marginRightSpin->value();
}

int ReceiptPrinterTab::getmarginTop()
{
    return m_marginTopSpin->value();
}

int ReceiptPrinterTab::getmarginBottom()
{
    return m_marginBottomSpin->value();
}

int ReceiptPrinterTab::getfeedProdukt()
{
    return m_feedProduktSpin->value();
}

int ReceiptPrinterTab::getfeedCompanyHeader()
{
    return m_feedCompanyHeaderSpin->value();
}

int ReceiptPrinterTab::getfeedCompanyAddress()
{
    return m_feedCompanyAddressSpin->value();
}

int ReceiptPrinterTab::getfeedCashRegisterid()
{
    return m_feedCashRegisteridSpin->value();
}

int ReceiptPrinterTab::getfeedTimestamp()
{
    return m_feedTimestampSpin->value();
}

int ReceiptPrinterTab::getfeedTax()
{
    return m_feedTaxSpin->value();
}

int ReceiptPrinterTab::getfeedPrintHeader()
{
    return m_feedPrintHeaderSpin->value();
}

int ReceiptPrinterTab::getfeedHeaderText()
{
    return m_feedHeaderTextSpin->value();
}



ReceiptTab::ReceiptTab(QWidget *parent)
    : QWidget(parent)
{

    m_printAdvertisingEdit = new QTextEdit();
    m_printHeaderEdit = new QTextEdit();
    m_printFooterEdit = new QTextEdit();

    m_receiptPrinterHeading = new QComboBox();
    m_printCollectionReceiptCheck = new QCheckBox();
    m_collectionReceiptTextEdit = new QLineEdit();
    m_collectionReceiptTextEdit->setPlaceholderText(tr("Abholbon für"));

    m_printCompanyNameBoldCheck = new QCheckBox();
    m_printQRCodeCheck = new QCheckBox();
    m_printQRCodeLeftCheck = new QCheckBox();

    m_useLogo = new QCheckBox();
    m_useLogoRightCheck = new QCheckBox();
    m_logoEdit = new QLineEdit();
    m_logoEdit->setEnabled(false);
    m_logoButton = new QPushButton;

    QIcon icon = QIcon(":icons/save.png");
    QSize size = QSize(24,24);
    m_logoButton->setIcon(icon);
    m_logoButton->setIconSize(size);
    m_logoButton->setText(tr("Auswahl"));

    m_useAdvertising = new QCheckBox();
    m_advertisingEdit = new QLineEdit();
    m_advertisingEdit->setEnabled(false);
    m_advertisingButton = new QPushButton;

    m_advertisingButton->setIcon(icon);
    m_advertisingButton->setIconSize(size);
    m_advertisingButton->setText(tr("Auswahl"));

    QHBoxLayout *logoLayout = new QHBoxLayout;
    logoLayout->addWidget(m_useLogo);
    logoLayout->addWidget(m_logoEdit);
    logoLayout->addWidget(m_logoButton);

    QHBoxLayout *advertisingLayout = new QHBoxLayout;
    advertisingLayout->addWidget(m_useAdvertising);
    advertisingLayout->addWidget(m_advertisingEdit);
    advertisingLayout->addWidget(m_advertisingButton);

    QHBoxLayout *collectionBonLayout = new QHBoxLayout;
    collectionBonLayout->addWidget(m_printCollectionReceiptCheck);
    collectionBonLayout->addWidget(new QLabel(tr("Extrabon Text:")));
    collectionBonLayout->addWidget(m_collectionReceiptTextEdit);

    QGroupBox *receiptGroup = new QGroupBox(tr("BON"));
    QFormLayout *receiptLayout = new QFormLayout;
    receiptLayout->setAlignment(Qt::AlignLeft);
    receiptLayout->addRow(tr("Überschrift:"), m_receiptPrinterHeading);
    receiptLayout->addRow(tr("Extrabon pro Artikel drucken:"), collectionBonLayout);
    receiptLayout->addRow(tr("Firmenname Fett drucken:"), m_printCompanyNameBoldCheck);
    receiptLayout->addRow(tr("QRCode drucken:"), m_printQRCodeCheck);
    receiptLayout->addRow(tr("QRCode links neben der Gesamtsumme drucken:"), m_printQRCodeLeftCheck);
    receiptLayout->addRow(tr("Logo verwenden:"), logoLayout);
    receiptLayout->addRow(tr("Logo auf der rechten Seite drucken:"), m_useLogoRightCheck);
    receiptLayout->addRow(tr("KassaBon Werbung Grafik:"), advertisingLayout);

    receiptLayout->addRow(tr("KassaBon Werbung Text:"), m_printAdvertisingEdit);
    receiptLayout->addRow(tr("BON Kopfzeile:"), m_printHeaderEdit);
    receiptLayout->addRow(tr("BON Fußzeile:"), m_printFooterEdit);

    receiptGroup->setLayout(receiptLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(receiptGroup);

    mainLayout->addStretch(1);
    setLayout(mainLayout);

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare("SELECT strValue FROM globals WHERE name='printAdvertisingText'");
    query.exec();
    if (query.next())
        m_printAdvertisingEdit->setText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('printAdvertisingText', '')");

    query.prepare("SELECT strValue FROM globals WHERE name='printHeader'");
    query.exec();
    if (query.next())
        m_printHeaderEdit->setText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('printHeader', '')");

    query.prepare("SELECT strValue FROM globals WHERE name='printFooter'");
    query.exec();
    if ( query.next() )
        m_printFooterEdit->setText(query.value(0).toString());
    else
        query.exec("INSERT INTO globals (name, strValue) VALUES('printFooter', '')");

    QrkSettings settings;
    m_receiptPrinterHeading->addItem("KASSABON");
    m_receiptPrinterHeading->addItem("KASSENBON");
    m_receiptPrinterHeading->addItem("Zahlungsbestätigung");
    m_receiptPrinterHeading->addItem("");

    m_receiptPrinterHeading->setCurrentText(settings.value("receiptPrinterHeading", "KASSABON").toString());

    m_printCompanyNameBoldCheck->setChecked(settings.value("printCompanyNameBold", true).toBool());

    m_useLogo->setChecked(settings.value("useLogo", false).toBool());
    m_logoEdit->setText(settings.value("logo", "./logo.png").toString());
    m_useLogoRightCheck->setChecked(settings.value("logoRight", false).toBool());

    m_useAdvertising->setChecked(settings.value("useAdvertising", false).toBool());
    m_advertisingEdit->setText(settings.value("advertising", "./advertising.png").toString());

    m_printCollectionReceiptCheck->setChecked(settings.value("printCollectionReceipt", false).toBool());
    QString collectionReceiptText = settings.value("collectionReceiptText", tr("Abholbon für")).toString();
    m_collectionReceiptTextEdit->setText(collectionReceiptText);

    m_printQRCodeCheck->setChecked(settings.value("qrcode", true).toBool());
    m_printQRCodeLeftCheck->setChecked(settings.value("qrcodeleft", false).toBool());

    connect(m_logoButton, SIGNAL(clicked(bool)), this, SLOT(logoButton_clicked()));
    connect(m_useLogo, SIGNAL(toggled(bool)) , this, SLOT(useLogoCheck_toggled(bool)));
    connect(m_advertisingButton, SIGNAL(clicked(bool)), this, SLOT(advertisingButton_clicked()));
    connect(m_useAdvertising, SIGNAL(toggled(bool)) , this, SLOT(useAdvertisingCheck_toggled(bool)));
    connect(m_printQRCodeCheck, SIGNAL(toggled(bool)) , m_printQRCodeLeftCheck, SLOT(setEnabled(bool)));

}

QString ReceiptTab::getAdvertisingText()
{
    return m_printAdvertisingEdit->toPlainText();
}

QString ReceiptTab::getHeader()
{
    return m_printHeaderEdit->toPlainText();
}

QString ReceiptTab::getFooter()
{
    return m_printFooterEdit->toPlainText();
}

QString ReceiptTab::getLogo()
{
    return m_logoEdit->text();
}

void ReceiptTab::useLogoCheck_toggled(bool toggled)
{
    m_logoButton->setEnabled(toggled);
}

bool ReceiptTab::getUseLogo()
{
    return m_useLogo->isChecked();
}

void ReceiptTab::logoButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Logo Auswahl"), getLogo(), tr("Image Files (*.png *.jpg *.bmp)"));

    if (!fileName.isEmpty())
        m_logoEdit->setText(fileName);

}

QString ReceiptTab::getAdvertising()
{
    return m_advertisingEdit->text();
}

void ReceiptTab::useAdvertisingCheck_toggled(bool toggled)
{
    m_advertisingButton->setEnabled(toggled);
}

bool ReceiptTab::getUseAdvertising()
{
    return m_useAdvertising->isChecked();
}

void ReceiptTab::advertisingButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Werbung Auswahl"), getAdvertising(), tr("Image Files (*.png *.jpg *.bmp)"));

    if (!fileName.isEmpty())
        m_advertisingEdit->setText(fileName);

}

bool ReceiptTab::getPrintCompanyNameBold()
{
    return m_printCompanyNameBoldCheck->isChecked();
}

QString ReceiptTab::getReceiptPrinterHeading()
{
    return m_receiptPrinterHeading->currentText();
}

bool ReceiptTab::getPrintCollectionReceipt()
{
    return m_printCollectionReceiptCheck->isChecked();
}

QString ReceiptTab::getCollectionReceiptText()
{
    return m_collectionReceiptTextEdit->text();
}

bool ReceiptTab::getIsLogoRight()
{
    return m_useLogoRightCheck->isChecked();
}

bool ReceiptTab::getPrintQRCode()
{
    return m_printQRCodeCheck->isChecked();
}

bool ReceiptTab::getPrintQRCodeLeft()
{
    return m_printQRCodeLeftCheck->isChecked();
}

SCardReaderTab::SCardReaderTab(QWidget *parent)
    : QWidget(parent)
{

    QrkSettings settings;
    m_scardReaderComboBox = new QComboBox();
    m_providerComboBox = new QComboBox();
    m_providerLoginEdit = new QLineEdit();
    m_providerPasswordEdit = new QLineEdit();

    QStringList *scardReaders = new QStringList();
    RKSmartCardInfo::getReaders(scardReaders);
    int readerCount = scardReaders->count();

    QString currentCardReader = settings.value("currentCardReader").toString();
    for (int i = 0; i < readerCount; i++) {
        m_scardReaderComboBox->addItem(scardReaders->at(i));
        if (currentCardReader == scardReaders->at(i))
            m_scardReaderComboBox->setCurrentText(currentCardReader);
    }

    for (int i = 0; i < readerCount; i++) {
        m_scardReaderComboBox->addItem(scardReaders->at(i));
        if (currentCardReader == scardReaders->at(i))
            m_scardReaderComboBox->setCurrentText(currentCardReader);
    }

    m_providerComboBox->addItem("A-Trust");
    QString currentOnlineReader = settings.value("atrust_connection").toString();
    bool onlineReader = false;
    if (currentOnlineReader.split("@").size() == 3) {
        m_providerLoginEdit->setText(currentOnlineReader.split("@")[0]);
        m_providerPasswordEdit->setText(currentOnlineReader.split("@")[1]);
        onlineReader = currentCardReader.isEmpty();
    }

    /*----------------------------------------------------------------------*/
    m_scardGroup = new QGroupBox(tr("Kartenleser"));
    m_scardGroup->setCheckable(true);

    QFormLayout *scardLayout = new QFormLayout;
    scardLayout->setAlignment(Qt::AlignLeft);
    scardLayout->addRow(tr("Kartenleser:"), m_scardReaderComboBox);
    m_scardGroup->setLayout(scardLayout);

    /*----------------------------------------------------------------------*/

    m_onlineGroup = new QGroupBox(tr("Onlinedienst"));
    m_onlineGroup->setCheckable(true);
    m_onlineGroup->setChecked(onlineReader && currentCardReader.isEmpty());
    m_scardGroup->setChecked(!currentCardReader.isEmpty());

    QFormLayout *onlineLayout = new QFormLayout;
    QHBoxLayout *loginLayout = new QHBoxLayout;
    loginLayout->addWidget(m_providerLoginEdit);
    loginLayout->addWidget(new QLabel((tr("Passwort:"))));
    loginLayout->addWidget(m_providerPasswordEdit);

    onlineLayout->setAlignment(Qt::AlignLeft);
    onlineLayout->addRow(tr("Anbieter:"), m_providerComboBox);
    onlineLayout->addRow(tr("Login:"), loginLayout);
    m_onlineGroup->setLayout(onlineLayout);

    /*----------------------------------------------------------------------*/

    QLabel *infoLabel = new QLabel();
    infoLabel->setWordWrap(true);

    QPushButton *scardTestButton = new QPushButton(tr("Test"));
    m_scardActivateButton = new QPushButton(tr("DEP Aktivieren"));
    if (RKSignatureModule::isDEPactive()) {
        m_scardActivateButton->setText(tr("Kasse\naußer Betrieb nehmen"));
        //        m_scardActivateButton->setVisible(false);
        infoLabel->setText(tr("DEP (Daten Erfassungs Protokoll) ist aktiv. Um den Kartenleser, die Karte oder die Online Daten zu ändern führen Sie den Test durch. Es wird auf eine gültige Signaturkarte bzw. auf die richtigen Onlinedaten überprüft.\nACHTUNG beim verlassen werden die neuen Daten gespeichert. Führen Sie hier nur Änderungen durch wenn Sie genau wissen was Sie tun. Bei Problemen hilft Ihnen die Community im Forum weiter."));
    } else {
        m_scardActivateButton->setEnabled(false);
        infoLabel->setText(tr("Um das DEP (Daten Erfassungs Protokoll) zu aktivieren führen Sie zuerst den Test durch. Es wird auf eine gültige Signaturkarte bzw. auf die richtigen Onlinedaten überprüft."));
    }

    scardTestButton->setFixedWidth(150);
    scardTestButton->setFixedHeight(60);

    m_scardActivateButton->setFixedWidth(150);
    m_scardActivateButton->setFixedHeight(60);

    m_infoWidget = new QListWidget();

//    QGroupBox *testGroup = new QGroupBox();
    QHBoxLayout *testHLayout = new QHBoxLayout();
    QVBoxLayout *testLayout = new QVBoxLayout();

    testHLayout->setAlignment(Qt::AlignTop);
    testHLayout->addWidget(scardTestButton);
    testHLayout->addWidget(m_scardActivateButton);

    testLayout->setAlignment(Qt::AlignTop);
    testLayout->addWidget(infoLabel);
    testLayout->addLayout(testHLayout);
    testLayout->addWidget(m_infoWidget);

//    testGroup->setLayout(testLayout);

    /*----------------------------------------------------------------------*/

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_scardGroup);
    mainLayout->addWidget(m_onlineGroup);
    mainLayout->addLayout(testLayout);

    mainLayout->addStretch(1);
    setLayout(mainLayout);

    /*----------------------------------------------------------------------*/
    connect(m_scardGroup, SIGNAL(clicked(bool)), this, SLOT(scardGroup_clicked(bool)));
    connect(m_onlineGroup, SIGNAL(clicked(bool)), this, SLOT(onlineGroup_clicked(bool)));
    connect(scardTestButton, SIGNAL(clicked(bool)), this, SLOT(testButton_clicked(bool)));
    connect(m_scardActivateButton, SIGNAL(clicked(bool)), this, SLOT(activateButton_clicked(bool)));
    if (readerCount == 0) {
        m_scardReaderComboBox->addItem(tr("Kein Kartenleser vorhanden"));
        m_scardReaderComboBox->setEnabled(false);
        emit scardGroup_clicked(false);
    }
}

bool SCardReaderTab::saveSettings()
{
    QrkSettings *settings = new QrkSettings(this);
    bool ret = false;
    QString currentCardReader = getCurrentCardReader();
    if (currentCardReader.isEmpty()) {
        settings->removeSettings("currentCardReader");
    } else {
        if (!m_onlineGroup->isChecked()) {
            settings->save2Settings("currentCardReader", currentCardReader);
            ret = true;
        }
    }

    QString connectionString = getCurrentOnlineConnetionString();
    if (connectionString.split("@").size() == 3) {
        settings->save2Settings("atrust_connection", connectionString);
        ret = true;
    } else {
        settings->removeSettings("atrust_connection");
    }

    return ret;
}

QString SCardReaderTab::getCurrentCardReader()
{
    if (m_scardReaderComboBox->isEnabled())
        return m_scardReaderComboBox->currentText();

    return QString();
}

QString SCardReaderTab::getCurrentOnlineConnetionString()
{
    QString connectionstring;
    if (!m_providerLoginEdit->text().isEmpty()) {
        connectionstring.append(m_providerLoginEdit->text());
        connectionstring.append("@");
    }
    if (!m_providerPasswordEdit->text().isEmpty()) {
        connectionstring.append(m_providerPasswordEdit->text());
        connectionstring.append("@");
    }
    connectionstring.append("https://www.a-trust.at/asignrkonline/v2");
    if (connectionstring.split("@").size() == 3)
        return connectionstring;

    return "";
}

void SCardReaderTab::scardGroup_clicked(bool b)
{
    m_scardGroup->setChecked(b);
    m_onlineGroup->setChecked(!b);
}

void SCardReaderTab::onlineGroup_clicked(bool b)
{
    m_onlineGroup->setChecked(b);
    m_scardGroup->setChecked(!b);
}

void SCardReaderTab::testButton_clicked(bool)
{

    m_infoWidget->clear();
    if (m_onlineGroup->isChecked())
        m_rkSignature = RKSignatureModuleFactory::createInstance(getCurrentOnlineConnetionString(), DemoMode::isDemoMode());
    else
        m_rkSignature = RKSignatureModuleFactory::createInstance(m_scardReaderComboBox->currentText(), DemoMode::isDemoMode());

    if (m_rkSignature->selectApplication()) {
        m_scardActivateButton->setEnabled(true);
        m_infoWidget->addItem(tr("Signatureinheit: %1").arg(m_rkSignature->getCardType()));
        m_infoWidget->addItem(tr("Zertifikat Seriennummer: hex %1, dec %2").arg(m_rkSignature->getCertificateSerial(true)).arg(m_rkSignature->getCertificateSerial(false)));
    } else {
        m_scardActivateButton->setEnabled(false);
        m_infoWidget->addItem(tr("Kein Signatur Module gefunden."));
    }

    delete m_rkSignature;
}

void SCardReaderTab::activateButton_clicked(bool)
{
    if (!saveSettings() && !DemoMode::isDemoMode()) {
        QMessageBox messageBox(QMessageBox::Information,
                               QObject::tr("DEP aktivieren"),
                               QObject::tr("Keine gültige Signaturerstellungseinheit gefunden."),
                               QMessageBox::Yes | QMessageBox::No,
                               0);
        messageBox.setButtonText(QMessageBox::Yes, QObject::tr("Ja"));
        messageBox.setButtonText(QMessageBox::No, QObject::tr("Nein"));
        messageBox.exec();
    } else {
        if (!RKSignatureModule::isDEPactive()) {
            QString cashRegisterId = Database::getCashRegisterId();
            if (cashRegisterId.isEmpty()) {
                QMessageBox messageBox(QMessageBox::Information,
                                       QObject::tr("Kassenidentifikationsnummer"),
                                       QObject::tr("Keine gültige Kassenidentifikationsnummer."),
                                       QMessageBox::Yes,
                                       0);
                messageBox.setButtonText(QMessageBox::Yes, QObject::tr("Ok"));
                messageBox.exec();
                return;
            }
            QMessageBox messageBox(QMessageBox::Question,
                                   QObject::tr("DEP aktivieren"),
                                   QObject::tr("Achtung!\nWenn das DEP (Daten Erfassungs Prodokoll) aktiviert wird beachten sie Bitte folgende Punkte.\n1. Es werden alle Rechnungen elektronisch signiert und im DEP gespeichert.\n2. Eine De-aktivierung ist nur möglich wenn die Kasse außer Betrieb genommen wird.\n3. Sie müssen den erstellten Startbeleg sicher aufbewahren.\n4. Die Inbetriebnahme muß dem zuständigen Finanzamt gemeldet werden.\n5. Die Kassenidentifikationsnummer kann nicht mehr geändert werden"),
                                   QMessageBox::Yes | QMessageBox::No,
                                   0);
            messageBox.setButtonText(QMessageBox::Yes, QObject::tr("Ja"));
            messageBox.setButtonText(QMessageBox::No, QObject::tr("Nein"));
            if (messageBox.exec() == QMessageBox::Yes ) {
                Reports *rep = new Reports(0, true);
                bool check = rep->checkEOAnyServerMode();
                delete rep;
                if (check) {
                    ReceiptItemModel *rec = new ReceiptItemModel();
                    if (rec->createStartReceipt()) {
                        m_scardActivateButton->setVisible(false);
                        QMessageBox::information(this,tr("DEP wurde aktiviert!"), tr("DEP wurde aktiviert. Die Kasse ist RKSV-konform.\n Der STARTBELEG wurde erstellt."), "Ok");
                    } else {
                        QMessageBox::information(this,tr("Fehler!"), tr("DEP konnte nicht aktiviert werden. Siehe Log."), "Ok");
                    }
                    delete rec;
                } else {
                    QMessageBox::information(this,tr("Fehler!"), tr("DEP konnte nicht aktiviert werden. Tages/Monatsabschluss wurde schon erstellt."), "Ok");
                }
            }
        } else {
            QMessageBox messageBox(QMessageBox::Question,
                                   QObject::tr("Kasse außer Betrieb nehmen"),
                                   QObject::tr("Achtung!\nDie Kasse wird außer Betrieb genommen. Das DEP (Daten Erfassungs Prodokoll) wird de-aktiviert. Melden Sie die außer Betriebnahme dem zuständigen Finanzamt."),
                                   QMessageBox::Yes | QMessageBox::No,
                                   0);
            messageBox.setButtonText(QMessageBox::Yes, QObject::tr("Ja"));
            messageBox.setButtonText(QMessageBox::No, QObject::tr("Nein"));
            if (messageBox.exec() == QMessageBox::Yes ) {
                RKSignatureModule::setDEPactive(false);
                m_scardActivateButton->setText(tr("Kasse außer Betrieb."));
                m_scardActivateButton->setEnabled(false);

                Reports *rep = new Reports(0, true);
                bool check = rep->checkEOAnyServerMode();
                delete rep;
                if (check) {
                    ReceiptItemModel *rec = new ReceiptItemModel();
                    if (rec->createNullReceipt(CONCLUSION_RECEIPT)) {
                        m_scardActivateButton->setVisible(false);
                        Database::setCashRegisterInAktive();
                        QMessageBox::information(this,tr("Außer Betrieb!"), tr("Die Kasse wurde außer Betrieb genommen.\n Der SCHLUSSBELEG wurde erstellt."), "Ok");
                    } else {
                        QMessageBox::information(this,tr("Fehler!"), tr("DEP konnte nicht aktiviert werden. Siehe Log."), "Ok");
                    }
                    delete rec;
                } else {
                    QMessageBox::information(this,tr("Fehler!"), tr("DEP konnte nicht aktiviert werden. Tages/Monatsabschluss wurde schon erstellt."), "Ok");
                }
            }
        }
    }
}
