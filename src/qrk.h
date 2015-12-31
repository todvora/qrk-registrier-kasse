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

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "database.h"
#include "qrkdelegate.h"
#include "dep.h"
#include "reports.h"
#include "receiptitemmodel.h"
#include "qsortfiltersqlquerymodel.h"
#include "documentprinter.h"
#include "depexportdialog.h"
#include "utils.h"
#include "r2bdialog.h"

#include <QMainWindow>
#include <QWidget>
#include <QStackedWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QStackedWidget>
#include <QMessageBox>
#include <QSignalMapper>
#include <QDateTime>
#include <QSqlQueryModel>
#include <QShortcut>
#include <QLCDNumber>
#include <QTimer>
#include <QTextDocument>
#include <QDebug>

#include <ui_qrk.h>

#define MIN_HEIGHT 60
#define QRK_VERSION_MAJOR 0.15
#define QRK_VERSION_MINOR 1231


class QRK : public QMainWindow
{
    Q_OBJECT
    Q_ENUMS(REGISTER_COL)
    Q_ENUMS(PAYED_BY)


  public:
    enum REGISTER_COL
    {
      REGISTER_COL_COUNT,
      REGISTER_COL_PRODUCT,
      REGISTER_COL_TAX,
      REGISTER_COL_SINGLE,
      REGISTER_COL_TOTAL
    };

    enum DOCUMENT_COL
    {
      DOCUMENT_COL_RECEIPT,
      DOCUMENT_COL_TYPE,
      DOCUMENT_COL_TOTAL,
      DOCUMENT_COL_DATE
    };

    // values for the receipt.payedBy field
    enum PAYED_BY
    {
      PAYED_BY_CASH,
      PAYED_BY_DEBITCARD,
      PAYED_BY_CREDITCARD,
      PAYED_BY_REPORT_EOD,
      PAYED_BY_REPORT_EOM
    };

    enum DEP_ACTION
    {
      DEP_RECEIPT = 0
    };


    QRK();
    ~QRK();

    void setLastEOD(QDate lastEOD) {this->lastEOD = lastEOD;}
    void setNoPrinter() { noPrinter = true; }  // do not print to printer but only to pdf file for testing only
    void setShopName();
    int getCurrentRegisterYear(){return currentRegisterYear;}
    int getCashRegisterId(){return cashRegisterId;}


    QLabel *currentRegisterYearLabel;
    QLabel *cashRegisterIdLabel;
    QProgressBar *progressBar;

  private slots:
    void menuSlot();
    void taskSlot();
    void exitSlot();
    void settingsSlot();
    void fullScreenSlot();
    bool endOfDaySlot();
    bool endOfMonthSlot();
    void receiptToInvoiceSlot();

    void onRegisterButton_clicked();
    void onCancelRegisterButton_clicked();

    void onDocumentButton_clicked();
    void onCancelDocumentButton_clicked();

    void onButtonGroup_payNow_clicked(int);

    void totallyupSlot();
    void totallyupExitSlot();
    void plusSlot();
    void minusSlot();
    void itemChangedSlot (const QModelIndex&, const QModelIndex&);

    void onCancellationButton_clicked();
    void onPrintcopyButton_clicked();

    void actionDEP_Export();

  protected slots:
    void onDocumentSelectionChanged(const QItemSelection &, const QItemSelection &);
    virtual void timerDone();

  private:
    void init();
    void newOrder();
    void documentList();

    void updateOrderSum();

    bool finishReceipts(int, int = 0, bool = false);
    int createReceipts();
    bool createOrder(bool = false);

    QString shopname();
    QJsonObject compileData(int = 0);
    QString getEndOfDay(int = 0);


    ReceiptItemModel *orderListModel;

    QSortFilterSqlQueryModel *documentListModel;
    QSqlQueryModel *documentContentModel;
    QLCDNumber *dateLcd;

  private:
    Ui::MainWindow *ui;

    QVBoxLayout *categoriesLayout;
    QSignalMapper categoriesMapper;
    QSignalMapper textChangedMapper;

    QFrame *menu;
    QFrame *task;

    QTimer *timer;

    int currentReceipt;
    int currentRegisterYear;
    int cashRegisterId;

    QString shopName;
    bool noPrinter;
    QDate lastEOD;
    bool totallyup;

};

#endif // MAINWIDGET_H
