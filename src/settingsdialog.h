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

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QSettings>

class QDialogButtonBox;
class QTabWidget;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QLineEdit;
class QRadioButton;
class QLabel;
class QGroupBox;

class ExtraTab : public QWidget
{
    Q_OBJECT

  public:
    explicit ExtraTab(QSettings &s, QWidget *parent = 0);
    bool getInputNetPrice();
    bool getMaximumItemSold();
    bool getDecimalQuantity();
    bool getGivenDialog();
    bool isFontsGroup();
    QString getSystemFont();
    QString getPrinterFont();
    QString getReceiptPrinterFont();

  private slots:
    void systemFontButton_clicked(bool);
    void printerFontButton_clicked(bool);
    void receiptPrinterFontButton_clicked(bool);
    void printerTestButton_clicked(bool);
    void receiptPrinterTestButton_clicked(bool);
    void fontsGroup_toggled(bool toggled);


  private:
    QCheckBox *useInputNetPriceCheck;
    QCheckBox *useMaximumItemSoldCheck;
    QCheckBox *useDecimalQuantityCheck;
    QCheckBox *useGivenDialogCheck;

    QGroupBox *fontsGroup;

    QPushButton *systemFontButton;
    QPushButton *printerFontButton;
    QPushButton *receiptPrinterFontButton;

    QLabel *systemFontSizeLabel;
    QLabel *printerFontSizeLabel;
    QLabel *receiptPrinterFontSizeLabel;

    QLabel *systemFontStretchLabel;
    QLabel *printerFontStretchLabel;
    QLabel *receiptPrinterFontStretchLabel;


    QFont *systemFont;
    QFont *printerFont;
    QFont *receiptPrinterFont;

};

class GeneralTab : public QWidget
{
    Q_OBJECT

  public:
    explicit GeneralTab(QSettings &s, QWidget *parent = 0);
    QString getHeader();
    QString getFooter();
    QString getBackupDirectory();
    QString getImportDirectory();
    QString getLogo();
    bool getUseLogo();

  private slots:
    void backupDirectoryButton_clicked();
    void importDirectoryButton_clicked();
    void logoButton_clicked();
    void useLogoCheck_toggled(bool);

  private:
    QTextEdit *printHeaderEdit;
    QTextEdit *printFooterEdit;
    QLineEdit *backupDirectoryEdit;
    QLineEdit *importDirectoryEdit;
    QLineEdit *logoEdit;
    QCheckBox *useLogo;
    QPushButton *logoButton;

};

class PrinterTab : public QWidget
{
    Q_OBJECT

  public:
    explicit PrinterTab(QSettings &s, QWidget *parent = 0);

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

    QCheckBox *reportPrinterCheck;
    QComboBox *reportPrinterCombo;
    QComboBox *paperFormatCombo;
    QComboBox *invoiceCompanyPrinterCombo;
    QComboBox *invoiceCompanyPaperFormatCombo;
    QSpinBox *invoiceCompanyMarginLeftSpin;
    QSpinBox *invoiceCompanyMarginRightSpin;
    QSpinBox *invoiceCompanyMarginTopSpin;
    QSpinBox *invoiceCompanyMarginBottomSpin;
};

class ReceiptPrinterTab : public QWidget
{
    Q_OBJECT

  public:
    explicit ReceiptPrinterTab(QSettings &s, QWidget *parent = 0);

    QString getReceiptPrinter();
    QString getReceiptPrinterHeading();
    bool getPrintCompanyNameBold();
    bool getUseReportPrinter();
    bool getIsLogoRight();
    bool getPrintQRCode();
    int getNumberCopies();
    int getpaperWidth();
    int getpaperHeight();
    int getmarginLeft();
    int getmarginRight();
    int getmarginTop();
    int getmarginBottom();

  private:
    QComboBox *paperFormatCombo;
    QComboBox *receiptPrinterHeading;
    QComboBox *receiptPrinterCombo;
    QCheckBox *printCompanyNameBoldCheck;
    QCheckBox *useReportPrinterCheck;
    QCheckBox *useLogoRightCheck;
    QCheckBox *printQRCodeCheck;
    QSpinBox *fontSizeSpin;
    QSpinBox *grossFontSpin;
    QSpinBox *numberCopiesSpin;
    QSpinBox *paperWidthSpin;
    QSpinBox *paperHeightSpin;
    QSpinBox *marginLeftSpin;
    QSpinBox *marginRightSpin;
    QSpinBox *marginTopSpin;
    QSpinBox *marginBottomSpin;
};

class MasterDataTab : public QWidget
{
    Q_OBJECT

  public:
    explicit MasterDataTab(QSettings &s, QWidget *parent = 0);

    QString getShopName();
    QString getShopOwner();
    QString getShopAddress();
    QString getShopUid();
    QString getShopCashRegisterId();
    QString getShopTaxes();
    QString getShopCurrency();

  private:
    QLineEdit *shopName;
    QLineEdit *shopOwner;
    QTextEdit *shopAddress;
    QLineEdit *shopUid;
    QLineEdit *shopCashRegisterId;
    QComboBox *currency;
    QComboBox *taxlocation;

};

class SettingsDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit SettingsDialog(QSettings &s, QWidget *parent = 0);

  private slots:
    void accept();

  private:
    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;
    QSettings &settings;

    GeneralTab *general;
    MasterDataTab *master;
    PrinterTab *printer;
    ReceiptPrinterTab *receiptprinter;
    ExtraTab *extra;

};

#endif
