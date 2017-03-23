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

#include <QtWidgets>
#include <QJsonObject>

#include "database.h"
#include "receiptitemmodel.h"
#include "documentprinter.h"
#include "reports.h"
#include "preferences/qrksettings.h"
#include "defines.h"

#include "barcodes.h"

Barcodes::Barcodes()
{
    initBarcodes();

    m_root = new QWidget();
    QVBoxLayout *vl =new QVBoxLayout(m_root);
    QPushButton *genBtn = new QPushButton(m_root);
    genBtn->setText(tr("Save"));
    vl->addWidget(genBtn);
    m_root->setLayout(vl);

}

Barcodes::~Barcodes()
{
}

void Barcodes::initBarcodes() {

    QrkSettings *settings = new QrkSettings(this);
    settings->beginGroup("BarCodesPlugin");

    m_barcode_finishReceipt = settings->value("barcodeFinishReceipt", "100009000001").toString();
    barcodes << m_barcode_finishReceipt;

    m_barcode_removeLastPosition = settings->value("barcodeRemoveLastPosition", "100009000002").toString();
    barcodes << m_barcode_removeLastPosition;

    m_barcode_endOfDay = settings->value("barcodeEndOfDay", "100009000003").toString();
    barcodes << m_barcode_endOfDay;

    m_barcode_discount = settings->value("barcodeDiscount", "100009000007").toString();
    barcodes << m_barcode_discount;

    m_barcode_editPrice = settings->value("barcodeEditPrice", "100009000010").toString();
    barcodes << m_barcode_editPrice;

    m_barcode_printLastReceiptAgain = settings->value("barcodePrintLastReceiptAgain", "100009000005").toString();
    barcodes << m_barcode_printLastReceiptAgain;

    m_barcode_cancelLastReceipt = settings->value("barcodeCancelReceipt", "100009000006").toString();
    barcodes << m_barcode_cancelLastReceipt;

    m_barcode_amount_0 = settings->value("barcodeAmount_0", "100008000000").toString();
    barcodes << m_barcode_amount_0;

    m_barcode_amount_1 = settings->value("barcodeAmount_1", "100008000001").toString();
    barcodes << m_barcode_amount_1;

    m_barcode_amount_2 = settings->value("barcodeAmount_2", "100008000002").toString();
    barcodes << m_barcode_amount_2;

    m_barcode_amount_3 = settings->value("barcodeAmount_3", "100008000003").toString();
    barcodes << m_barcode_amount_3;

    m_barcode_amount_4 = settings->value("barcodeAmount_4", "100008000004").toString();
    barcodes << m_barcode_amount_4;

    m_barcode_amount_5 = settings->value("barcodeAmount_5", "100008000005").toString();
    barcodes << m_barcode_amount_5;

    m_barcode_amount_6 = settings->value("barcodeAmount_6", "100008000006").toString();
    barcodes << m_barcode_amount_6;

    m_barcode_amount_7 = settings->value("barcodeAmount_7", "100008000007").toString();
    barcodes << m_barcode_amount_7;

    m_barcode_amount_8 = settings->value("barcodeAmount_8", "100008000008").toString();
    barcodes << m_barcode_amount_8;

    m_barcode_amount_9 = settings->value("barcodeAmount_9", "100008000009").toString();
    barcodes << m_barcode_amount_9;

    m_barcode_amount_00 = settings->value("barcodeAmount_00", "100008000020").toString();
    barcodes << m_barcode_amount_00;

    m_barcode_amount_000 = settings->value("barcodeAmount_000", "100008000030").toString();
    barcodes << m_barcode_amount_000;

    m_barcode_amount_250 = settings->value("barcodeAmount_250", "100008000250").toString();
    barcodes << m_barcode_amount_250;

    m_barcode_amount_500 = settings->value("barcodeAmount_500", "100008000500").toString();
    barcodes << m_barcode_amount_500;

    settings->endGroup();

}

bool Barcodes::process(ReceiptItemModel *model, int currIndex, QString barcode)
{
    m_model = model;
    m_index = currIndex;
    if (m_index < 0)
        return false;

    switch(barcodes.indexOf(barcode)) {
    case 0:  {//m_barcode_finishReceipt
        if (m_model->item(m_index,REGISTER_COL_PRODUCT)->text() == "")
            break;
        if (m_model->rowCount() == 0)
            break;
        if (int id = m_model->createReceipts()) {
            m_model->setCurrentReceiptNum(id);
            if (m_model->createOrder()) {
                m_model->finishReceipts(PAYED_BY_CASH);
                emit finishedReceipt();
            }
        }
        break;
    }
    case 1: {//m_barcode_removeLastPosition
        emit minusSlot();
        break;
    }
    case 2:  {//m_barcode_endOfDay
        Reports *rep = new Reports(this);
        rep->endOfDay(false);
        delete rep;
        break;
    }
    case 3:  {//m_barcode_discount
        break;
    }
    case 4:  {//m_barcode_editPrice
        initAmount();
        initAppendType();
        m_model->item(m_index, REGISTER_COL_COUNT_TYPE_STR)->setText("1");
        resetAmount();
        break;
    }
    case 5:  {//m_barcode_printLastReceiptAgain
        printReceipt();
        break;
    }
    case 6:  {//m_barcode_cancelLastReceipt
        break;
    }
    case 7:  {//m_barcode_amount_0
        handleAmount("0");
        break;
    }
    case 8:  {//m_barcode_amount_1
        handleAmount("1");
        break;
    }
    case 9:  {//m_barcode_amount_2
        handleAmount("2");
        break;
    }
    case 10:  {//m_barcode_amount_3
        handleAmount("3");
        break;
    }
    case 11:  {//m_barcode_amount_4
        handleAmount("4");
        break;
    }
    case 12:  {//m_barcode_amount_5
        handleAmount("5");
        break;
    }
    case 13:  {//m_barcode_amount_6
        handleAmount("6");
        break;
    }
    case 14:  {//m_barcode_amount_7
        handleAmount("7");
        break;
    }
    case 15:  {//m_barcode_amount_8
        handleAmount("8");
        break;
    }
    case 16:  {//m_barcode_amount_9
        handleAmount("9");
        break;
    }
    case 17:  {//m_barcode_amount_00
        handleAmount("00");
        break;
    }
    case 18:  {//m_barcode_amount_000
        handleAmount("000");
        break;
    }
    case 19:  {//m_barcode_amount_250
        handleAmount("250");
        break;
    }
    case 20:  {//m_barcode_amount_500
        handleAmount("500");
        break;
    }
    default: {
        return false;
    }
    }

    return true;

}

void Barcodes::printReceipt()
{
    int id = Database::getLastReceiptNum(true);

    ReceiptItemModel *reg = new ReceiptItemModel(this);
    reg->setCurrentReceiptNum(id);

    QrkSettings settings;
    QJsonObject data = reg->compileData();
    data["isCopy"] = true;
    int storno = Database::getStorno(id);
    if (storno == 2) {
        id = Database::getStornoId(id);
        data["comment"] = (id > 0)? tr("Storno für Beleg Nr: %1").arg(id):settings.value("receiptPrinterHeading", "KASSABON").toString();
    }

    data["headerText"] = Database::getCustomerText(id);

//    QApplication::setOverrideCursor(Qt::WaitCursor);
    DocumentPrinter *p = new DocumentPrinter(this);
    p->printReceipt(data);
    delete p;
    QApplication::restoreOverrideCursor();

}

void Barcodes::stornoReceipt()
{
    int id = Database::getLastReceiptNum(true);
    int payedBy = Database::getPayedBy(id);

    int storno = Database::getStorno(id);
    if (storno)
        return; // kann nicht storniert werden. wurde schon storniert oder ist ein storno Beleg

    // Hier wird gecheckt ob ein Tags/Monatsabschluss gemacht werden muss
    Reports *rep = new Reports(this,true);
    bool ret = rep->checkEOAnyServerMode();
    delete rep;
    if (! ret) {
        return; // Fehler ???
    }

    ReceiptItemModel *reg = new ReceiptItemModel(this);
    reg->newOrder();
    reg->storno(id);

    int currentReceipt = reg->createReceipts();
    if ( currentReceipt ) {
        reg->setCurrentReceiptNum(currentReceipt);
        if ( reg->createOrder(true) ) {
            if ( reg->finishReceipts(payedBy, id) ) {
                delete reg;
                return;
            }
        }
    }
    delete reg;
}

void Barcodes::handleAmount(QString amount) {

    if(m_index>=0) {

        initAmount();
        initAppendType();

        QStandardItem* item = m_model->item(m_index, REGISTER_COL_COUNT_TYPE_STR);
        if(QString::compare(item->text(), "0", Qt::CaseInsensitive)==0) {
            appendToAmount(amount);
        } else {
            appendToPrice(amount);
        }

    }
}

void Barcodes::appendToAmount(QString digits) {

    QString val = m_model->item(m_index, REGISTER_COL_COUNT_STR)->text()+digits;
    m_model->item(m_index, REGISTER_COL_COUNT_STR)->setText(val);

    QString valueAsString = QString::number(val.toFloat()/1000.0);
    m_model->item(m_index, REGISTER_COL_COUNT)->setText(valueAsString);
}

void Barcodes::initAmount() {
    init(REGISTER_COL_COUNT_STR, "");
}

void Barcodes::initAppendType() {
    init(REGISTER_COL_COUNT_TYPE_STR, "0");
}

void Barcodes::resetAmount() {

    m_model->item(m_index, REGISTER_COL_COUNT_STR)->setText("");
}

void Barcodes::appendToPrice(QString digits) {

    QString val = m_model->item(m_index, REGISTER_COL_COUNT_STR)->text()+digits;
    m_model->item(m_index, REGISTER_COL_COUNT_STR)->setText(val);

    QString valueAsString = QString::number(val.toFloat()/100.0);
    m_model->item(m_index, REGISTER_COL_SINGLE)->setText(valueAsString);
}

void Barcodes::init(int col, QString val) {

    QStandardItem* item = m_model->item(m_index, col);
    if(item == 0) {
        item = new QStandardItem(val);
        m_model->setItem(m_index, col, item);
        emit setColumnHidden(col);
    }
}

QWidget *Barcodes::SettingsWidget()
{
    return m_root;
}
