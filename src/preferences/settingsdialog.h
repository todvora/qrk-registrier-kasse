/*
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

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "RK/rk_signaturemodule.h"

class QDialogButtonBox;
class QTabWidget;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QLineEdit;
class QRadioButton;
class QLabel;
class QPushButton;
class QGroupBox;
class QListWidget;
class ASignSmartCard;
class Journal;
class QStackedWidget;

class ExtraTab : public QWidget
{
    Q_OBJECT

public:
    explicit ExtraTab(QWidget *parent = 0);
    bool getInputNetPrice();
    bool getMaximumItemSold();
    bool getDecimalQuantity();
    bool getGivenDialog();
    bool getReceiptPrintedDialog();
    bool isFontsGroup();
    QString getSystemFont();
    QString getPrinterFont();
    QString getReceiptPrinterFont();
    QString getDefaultTax();
    int getBarcodePrefix();

signals:

private slots:
    void systemFontButton_clicked(bool);
    void printerFontButton_clicked(bool);
    void receiptPrinterFontButton_clicked(bool);
    void printerTestButton_clicked(bool);
    void receiptPrinterTestButton_clicked(bool);
    void fontsGroup_toggled(bool toggled);
    void maximumSoldItemChanged(bool enabled);

private:
    QCheckBox *m_useInputNetPriceCheck;
    QCheckBox *m_useMaximumItemSoldCheck;
    QCheckBox *m_useDecimalQuantityCheck;
    QCheckBox *m_useGivenDialogCheck;
    QCheckBox *m_useReceiptPrintedDialogCheck;

    QComboBox *m_barcodePrefixesComboBox;
    QComboBox *m_defaultTaxComboBox;

    QGroupBox *m_fontsGroup;

    QPushButton *m_systemFontButton;
    QPushButton *m_printerFontButton;
    QPushButton *m_receiptPrinterFontButton;

    QLabel *m_systemFontSizeLabel;
    QLabel *m_printerFontSizeLabel;
    QLabel *m_receiptPrinterFontSizeLabel;

    QLabel *m_systemFontStretchLabel;
    QLabel *m_printerFontStretchLabel;
    QLabel *m_receiptPrinterFontStretchLabel;


    QFont *m_systemFont;
    QFont *m_printerFont;
    QFont *m_receiptPrinterFont;

};

class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralTab(QWidget *parent = 0);
    QString getBackupDirectory();
    QString getPdfDirectory();
    QString getImportDirectory();
    QString getDataDirectory();
    QString getExternalDepDirectory();

public slots:
    void masterTaxChanged(QString tax);

private slots:
    void backupDirectoryButton_clicked();
    void pdfDirectoryButton_clicked();
    void importDirectoryButton_clicked();
    void dataDirectoryButton_clicked();
    void externalDepDirectoryButton_clicked();

private:
    bool moveDataFiles( QString fromDir, QString toDir);

    QLineEdit *m_backupDirectoryEdit;
    QLineEdit *m_pdfDirectoryEdit;
    QLineEdit *m_importDirectoryEdit;
    QLineEdit *m_dataDirectoryEdit;
    QLineEdit *m_externalDepDirectoryEdit;
    QGroupBox *m_externalDepGroup;

};

class PrinterTab : public QWidget
{
    Q_OBJECT

public:
    explicit PrinterTab(QWidget *parent = 0);

    bool getReportPrinterPDF();
    QString getReportPrinter();
    QString getPaperFormat();
    QString getInvoiceCompanyPrinter();
    QString getInvoiceCompanyPaperFormat();
    int getInvoiceCompanyMarginLeft();
    int getInvoiceCompanyMarginRight();
    int getInvoiceCompanyMarginTop();
    int getInvoiceCompanyMarginBottom();

private slots:
    void reportPrinterCheck_toggled(bool);

private:

    QCheckBox *m_reportPrinterCheck;
    QComboBox *m_reportPrinterCombo;
    QComboBox *m_paperFormatCombo;
    QComboBox *m_invoiceCompanyPrinterCombo;
    QComboBox *m_invoiceCompanyPaperFormatCombo;
    QSpinBox *m_invoiceCompanyMarginLeftSpin;
    QSpinBox *m_invoiceCompanyMarginRightSpin;
    QSpinBox *m_invoiceCompanyMarginTopSpin;
    QSpinBox *m_invoiceCompanyMarginBottomSpin;
};

class ReceiptPrinterTab : public QWidget
{
    Q_OBJECT

public:
    explicit ReceiptPrinterTab(QWidget *parent = 0);

    QString getReceiptPrinter();
    bool getUseReportPrinter();
    int getNumberCopies();
    int getpaperWidth();
    int getpaperHeight();
    int getmarginLeft();
    int getmarginRight();
    int getmarginTop();
    int getmarginBottom();

    int getfeedProdukt();
    int getfeedCompanyHeader();
    int getfeedCompanyAddress();
    int getfeedCashRegisterid();
    int getfeedTimestamp();
    int getfeedTax();
    int getfeedPrintHeader();
    int getfeedHeaderText();

private:
//    QComboBox *m_paperFormatCombo;
    QComboBox *m_receiptPrinterCombo;
    QCheckBox *m_useReportPrinterCheck;
    QSpinBox *m_fontSizeSpin;
    QSpinBox *m_grossFontSpin;
    QSpinBox *m_numberCopiesSpin;
    QSpinBox *m_paperWidthSpin;
    QSpinBox *m_paperHeightSpin;
    QSpinBox *m_marginLeftSpin;
    QSpinBox *m_marginRightSpin;
    QSpinBox *m_marginTopSpin;
    QSpinBox *m_marginBottomSpin;

    QSpinBox *m_feedProduktSpin;
    QSpinBox *m_feedCompanyHeaderSpin;
    QSpinBox *m_feedCompanyAddressSpin;
    QSpinBox *m_feedCashRegisteridSpin;
    QSpinBox *m_feedTimestampSpin;
    QSpinBox *m_feedTaxSpin;
    QSpinBox *m_feedPrintHeaderSpin;
    QSpinBox *m_feedHeaderTextSpin;

};

class ReceiptTab : public QWidget
{
    Q_OBJECT

public:
    explicit ReceiptTab(QWidget *parent = 0);

    QString getReceiptPrinterHeading();
    bool getPrintCompanyNameBold();
    bool getPrintCollectionReceipt();
    QString getCollectionReceiptText();
    bool getIsLogoRight();
    bool getPrintQRCode();
    bool getPrintQRCodeLeft();
    QString getLogo();
    bool getUseLogo();
    QString getAdvertising();
    bool getUseAdvertising();

    QString getAdvertisingText();
    QString getHeader();
    QString getFooter();

private slots:
    void logoButton_clicked();
    void useLogoCheck_toggled(bool);
    void advertisingButton_clicked();
    void useAdvertisingCheck_toggled(bool);

private:
    QTextEdit *m_printAdvertisingEdit;
    QTextEdit *m_printHeaderEdit;
    QTextEdit *m_printFooterEdit;

    QLineEdit *m_logoEdit;
    QCheckBox *m_useLogo;
    QPushButton *m_logoButton;

    QLineEdit *m_advertisingEdit;
    QCheckBox *m_useAdvertising;
    QPushButton *m_advertisingButton;

    QComboBox *m_receiptPrinterHeading;
    QCheckBox *m_printCompanyNameBoldCheck;
    QCheckBox *m_printCollectionReceiptCheck;
    QLineEdit *m_collectionReceiptTextEdit;
    QCheckBox *m_useLogoRightCheck;
    QCheckBox *m_printQRCodeCheck;
    QCheckBox *m_printQRCodeLeftCheck;
};

class MasterDataTab : public QWidget
{
    Q_OBJECT

public:
    explicit MasterDataTab(QWidget *parent = 0);

    QString getShopName();
    QString getShopOwner();
    QString getShopAddress();
    QString getShopUid();
    QString getShopCashRegisterId();
    QString getShopTaxes();
    QString getShopCurrency();

signals:
    void taxChanged(QString);

private slots:
    void cashRegisterIdChanged();

private:
    QLineEdit *m_shopName;
    QLineEdit *m_shopOwner;
    QTextEdit *m_shopAddress;
    QLineEdit *m_shopUid;
    QLineEdit *m_shopCashRegisterId;
    QComboBox *m_currency;
    QComboBox *m_taxlocation;

};

class SCardReaderTab : public QWidget
{
    Q_OBJECT

public:
    explicit SCardReaderTab(QWidget *parent = 0);
    QString getCurrentCardReader();
    QString getCurrentOnlineConnetionString();
    bool saveSettings();



private slots:
    void scardGroup_clicked(bool);
    void onlineGroup_clicked(bool);
    void testButton_clicked(bool);
    void activateButton_clicked(bool);

private:

    QComboBox *m_scardReaderComboBox;
    QComboBox *m_providerComboBox;
    QLineEdit *m_providerLoginEdit;
    QLineEdit *m_providerPasswordEdit;

    QGroupBox *m_scardGroup;
    QGroupBox *m_onlineGroup;
    QListWidget *m_infoWidget;

    QPushButton *m_scardActivateButton;

    RKSignatureModule *m_rkSignature;
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);

private slots:
    void masterTaxChanged(QString);
    void accept();

private:
    QTabWidget *m_tabWidget;
    QDialogButtonBox *m_buttonBox;

    GeneralTab *m_general;
    MasterDataTab *m_master;
    PrinterTab *m_printer;
    ReceiptPrinterTab *m_receiptprinter;
    ReceiptTab *m_receipt;
    ExtraTab *m_extra;
    SCardReaderTab *m_scardreader;
    Journal *m_journal;

};

#endif
