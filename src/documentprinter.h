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

#ifndef DOCUMENTPRINTER_H
#define DOCUMENTPRINTER_H

#include <QObject>
#include <QProgressBar>
#include <QPrinter>
#include <QTextDocument>

class DocumentPrinter : public QObject
{
    Q_OBJECT

  public:
    DocumentPrinter(QObject *parent = 0, QProgressBar *progressBar = 0, bool = false);
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
    QProgressBar *pb;
    bool noPrinter;
    bool smallPrinter;
    bool printQRCode;
    int receiptNum;
    bool logoRight;
    int numberCopies;
    QString paperFormat;
    bool useReportPrinter;
    QString currency;
    QString logoFileName;
    bool printCompanyNameBold;
    QFont *receiptPrinterFont;
    QFont *printerFont;

  signals:

  public slots:

};

#endif // DOCUMENTPRINTER_H
