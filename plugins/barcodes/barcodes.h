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

#ifndef BARCODES_H
#define BARCODES_H

#include <QObject>
#include <QtPlugin>

#include "pluginmanager/Interfaces/barcodesinterface.h"

class ReceiptItemModel;
class Barcodes : public BarcodesInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "at.ckvsoft.BarcodesInterface" FILE "barcodes.json")
    Q_INTERFACES(BarcodesInterface)

public:
    Barcodes();
    ~Barcodes();

    bool process(ReceiptItemModel *model, int index, QString barcode);
    QWidget *SettingsWidget();

private:
    void initBarcodes();
    void printReceipt();
    void stornoReceipt();
    void handleAmount(QString amount);
    void appendToAmount(QString digits);
    void initAmount();
    void initAppendType();
    void resetAmount();
    void appendToPrice(QString digits);
    void init(int col, QString val);

    ReceiptItemModel *m_model = 0;
    int m_index;
    QStringList barcodes;

    QString m_barcode_finishReceipt;
    QString m_barcode_removeLastPosition;
    QString m_barcode_endOfDay;
    QString m_barcode_discount;
    QString m_barcode_editPrice;

    QString m_barcode_printLastReceiptAgain;
    QString m_barcode_cancelLastReceipt;

    QString m_barcode_amount_0;
    QString m_barcode_amount_1;
    QString m_barcode_amount_2;
    QString m_barcode_amount_3;
    QString m_barcode_amount_4;
    QString m_barcode_amount_5;
    QString m_barcode_amount_6;
    QString m_barcode_amount_7;
    QString m_barcode_amount_8;
    QString m_barcode_amount_9;
    QString m_barcode_amount_00;
    QString m_barcode_amount_000;
    QString m_barcode_amount_250;
    QString m_barcode_amount_500;

    QWidget *m_root;
};

#endif
