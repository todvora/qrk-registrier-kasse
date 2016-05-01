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

#ifndef QRKREGISTER_H
#define QRKREGISTER_H

#include "defines.h"
#include "database.h"
#include "receiptitemmodel.h"
#include "utils/utils.h"
#include "documentprinter.h"
#include "dep.h"
#include "qrkdelegate.h"
#include "reports.h"
#include "r2bdialog.h"

#include <QProgressBar>
#include <QShortcut>
#include <QSqlQueryModel>

#include "ui_qrkregister.h"

class QRKRegister : public QWidget
{
    Q_OBJECT
  public:
    explicit QRKRegister(QProgressBar *progressBar, QWidget *parent = 0);

    QString getHeaderText();
    void init();
    void updateOrderSum();
    int createReceipts();
    bool finishReceipts(int, int = 0, bool = false);
    void newOrder();

    void clearModel();
    void setItemModel(QSqlQueryModel *model);
    QJsonObject compileData(int id = 0);
    bool createOrder(bool storno = false);
    void setCurrentReceiptNum(int id);
    bool checkEOAny();
    bool checkEOAnyServerMode();
    bool setR2BServerMode(QJsonObject obj);
    void setServerMode(bool);

  signals:
    void cancelRegisterButton_clicked();
    void registerButton_clicked();


  public slots:
  private slots:
    void itemChangedSlot( const QModelIndex& i, const QModelIndex&);
    void plusSlot();
    void minusSlot();
    void onButtonGroup_payNow_clicked(int payedBy);
    void quickProductButtons(int id);
    void addProductToOrderList(int id);
    void totallyupSlot();
    void totallyupExitSlot();
    void receiptToInvoiceSlot();
    void onCancelRegisterButton_clicked();


  private:
    Ui::QRKRegister *ui;

    void setButtonGroupEnabled(bool enabled);
    void quickGroupButtons();
    QString wordWrap(QString text, int width, QFont font);
    void printDocument(int id, QString title);

    int currentReceipt;
    QProgressBar *progressBar;
    QDate lastEOD;
    ReceiptItemModel *orderListModel;
    bool totallyup;
    bool noPrinter;
    bool useInputNetPrice;
    bool useMaximumItemSold;
    bool useDecimalQuantity;
    bool useGivenDialog;
    bool isR2B;
    bool servermode;
    QString currency;
    QString taxlocation;
    QButtonGroup *buttonGroupGroups;
    QButtonGroup *buttonGroupProducts;

};

#endif // REGISTER_H
