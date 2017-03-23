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

#ifndef RECEIPTITEMMODEL_H
#define RECEIPTITEMMODEL_H

#include <QStandardItemModel>
#include <QDateTime>
#include "qrkcore_global.h"

enum NULL_RECEIPT
{
  START_RECEIPT = 5,
  CONTROL_RECEIPT,
  COLLECTING_RECEIPT,
  MONTH_RECEIPT,
  CONCLUSION_RECEIPT,
  YEAR_RECEIPT
};

/*  5,'Startbeleg'
    6,'Kontrollbeleg'
    7,'Sammelbeleg'
    8,'Monatsbeleg' == 'Jahresbeleg'
    9,'Schlussbeleg'
*/

class QRK_EXPORT ReceiptItemModel : public QStandardItemModel
{
    Q_OBJECT
  public:
    ReceiptItemModel(QObject* parent = 0);
    QJsonObject compileData(int id = 0);

    void setCurrentReceiptNum(int id);
    void setReceiptTime(QDateTime receiptTime);
    void setCustomerText(QString customerText);
    void newOrder(bool addRow = true);
    void clear();
    void plus();

    bool setReceiptServerMode(QJsonObject obj);
    bool setR2BServerMode(QJsonObject obj);
    bool createNullReceipt(int title);
    bool createStartReceipt();
    bool finishReceipts(int, int = 0, bool = false);
    bool createOrder(bool storno = false);
    bool storno(int id);

    int createReceipts();
    int getReceiptNum();

  signals:
    void setButtonGroupEnabled(bool);
    void finishedItemChanged();
    void finishedPlus();

private slots:
    void itemChangedSlot(const QModelIndex&, const QModelIndex&);

private:
    void setTotallyUp(bool totallyup);
    bool doEndOfDay(QDate date);

    QDate m_lastEOD;
    QString m_currency;
    QString m_taxlocation;
    QString m_customerText;

    QDateTime m_receiptTime;
    bool m_isR2B;
    bool m_isReport;
    bool m_totallyup;

    int m_currentReceipt;
};

#endif // RECEIPTITEMMODEL_H
