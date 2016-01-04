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

class GeneralTab : public QWidget
{
    Q_OBJECT

  public:
    explicit GeneralTab(QSettings &s, QWidget *parent = 0);
    QString getHeader();
    QString getFooter();
    QString getBackupPath();

  private slots:
    void backupButton_clicked();

  private:
    QTextEdit *printHeaderEdit;
    QTextEdit *printFooterEdit;
    QLineEdit *backupDirectoryEdit;
};

class PrinterTab : public QWidget
{
    Q_OBJECT

  public:
    explicit PrinterTab(QSettings &s, QWidget *parent = 0);

    QString getReportPrinter();
    QString getPaperFormat();
    QString getReceiptPrinter();
    bool getUseReportPrinter();
    bool getIsLogoRight();
    int getNumberCopies();
    int getpaperWidth();
    int getpaperHeight();
    int getmarginLeft();
    int getmarginRight();
    int getmarginTop();
    int getmarginBottom();

  private:

    QComboBox *reportPrinterCombo;
    QComboBox *paperFormatCombo;
    QComboBox *receiptPrinterCombo;
    QCheckBox *useReportPrinterCheck;
    QCheckBox *useLogoRightCheck;
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
    QString getShopLocation();
    QString getShopCashRegisterId();

  private:
    QLineEdit *shopName;
    QLineEdit *shopOwner;
    QTextEdit *shopAddress;
    QLineEdit *shopUid;
    QComboBox *shopLocation;
    QLineEdit *shopCashRegisterId;
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

};

#endif
