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

#ifndef DOCUMENTPRINTER_H
#define DOCUMENTPRINTER_H

#include <QObject>
#include "qrkcore_global.h"

class QPrinter;
class QTextDocument;

class QRK_EXPORT DocumentPrinter : public QObject
{
    Q_OBJECT

  public:
    DocumentPrinter(QObject *parent = 0);
    ~DocumentPrinter();

    void printReceipt(QJsonObject data);
    void printDocument(QTextDocument *document, QString title);
    void printTestDocument(QFont font);

  private:
    QString wordWrap(QString text, int width, QFont font);
    bool initPrinter(QPrinter &printer);
    bool initAlternatePrinter(QPrinter &printer);
    bool initInvoiceCompanyPrinter(QPrinter &printer);
    void printI(QJsonObject data, QPrinter &printer);
    void printCollectionReceipt(QJsonObject data, QPrinter &printer);

    bool m_noPrinter;
    QString m_pdfPrinterPath;
    bool m_printCollectionsReceipt;
    int m_collectionsReceiptCopies;
    bool m_smallPrinter;
    bool m_printQRCode;
    int m_receiptNum;
    bool m_logoRight;
    int m_numberCopies;
    QString m_paperFormat;
    QString m_collectionsReceiptText;
    bool m_useReportPrinter;
    QString m_currency;
    QString m_logoFileName;
    QString m_advertisingFileName;
    bool m_printCompanyNameBold;
    QFont *m_receiptPrinterFont;
    QFont *m_printerFont;

    int m_feedProdukt;
    int m_feedCompanyHeader;
    int m_feedCompanyAddress;
    int m_feedCashRegisterid;
    int m_feedTimestamp;
    int m_feedTax;
    int m_feedPrintHeader;
    int m_feedHeaderText;
    bool m_printQrCodeLeft;
    bool m_useDecimalQuantity;

};

#endif // DOCUMENTPRINTER_H
