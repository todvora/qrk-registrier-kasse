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

#ifndef QRKREGISTER_H
#define QRKREGISTER_H

#include "receiptitemmodel.h"
#include "preferences/qrksettings.h"
#include "pluginmanager/Interfaces/barcodesinterface.h"

#include "ui_qrkregister.h"

//class QSqlQueryModel;

class QRKRegister : public QWidget
{
    Q_OBJECT
  public:
    explicit QRKRegister(QWidget *parent = 0);
    ~QRKRegister();
    void init();
    void newOrder();
    void clearModel();

  signals:
    void cancelRegisterButton_clicked();
    void finishedReceipt();

  public slots:
    void safetyDevice(bool active);

  private slots:
    void barcodeChangedSlot();
    void plusSlot();
    void minusSlot();
    void onButtonGroup_payNow_clicked(int payedBy);
    void quickProductButtons(int id);
    void addProductToOrderList(int id);
    void totallyupSlot();
    void totallyupExitSlot();
    void receiptToInvoiceSlot();
    void onCancelRegisterButton_clicked();
    void setButtonGroupEnabled(bool enabled);
    void finishedItemChanged();
    void finishedPlus();
    void createCheckReceipt(bool);
    void setColumnHidden(int col);

  protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void keyPressEvent(QKeyEvent *event);

  private:
    Ui::QRKRegister *ui;

    void updateOrderSum();
    void setCurrentReceiptNum(int id);
    void quickGroupButtons();
    void handleAmount(QString amount);
    void initAmount();
    void resetAmount();
    void initAppendType();
    void init(int col, QString val);
    void appendToAmount(QString digits);
    void appendToPrice(QString digits);

    bool finishReceipts(int, int = 0, bool = false);

    void initPlugins();
    BarcodesInterface *barcodesInterface;

    int m_currentReceipt;
    ReceiptItemModel *m_orderListModel;

    bool m_totallyup;
    bool m_useInputNetPrice;
    bool m_useMaximumItemSold;
    bool m_useDecimalQuantity;
    bool m_useGivenDialog;
    bool m_isR2B;
    bool m_receiptPrintDialog;

    QButtonGroup *m_buttonGroupGroups;
    QButtonGroup *m_buttonGroupProducts;

    int m_barcodeReaderPrefix;


};

#endif // REGISTER_H
