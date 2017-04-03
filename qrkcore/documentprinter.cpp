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

#include "documentprinter.h"
#include "database.h"
#include "utils/utils.h"
#include "utils/qrcode.h"
#include "reports.h"
#include "singleton/spreadsignal.h"
#include "RK/rk_signaturemodule.h"
#include "preferences/qrksettings.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QAbstractTextDocumentLayout>
#include <QStandardPaths>
#include <QPrinter>
#include <QTextDocument>
#include <QDebug>


DocumentPrinter::DocumentPrinter(QObject *parent)
    :QObject(parent)
{

    QrkSettings settings;
    QList<QString> printerFontList = settings.value("printerfont", "Courier-New,10,100").toString().split(",");
    QList<QString> receiptPrinterFontList = settings.value("receiptprinterfont", "Courier-New,8,100").toString().split(",");

    m_noPrinter = settings.value("noPrinter", false).toBool();
    m_pdfPrinterPath = settings.value("pdfDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+ "/pdf").toString();
    m_printerFont = new QFont(printerFontList.at(0));
    m_printerFont->setPointSize(printerFontList.at(1).toInt());
    m_printerFont->setStretch(printerFontList.at(2).toInt());
    m_printCollectionsReceipt = settings.value("printCollectionReceipt", false).toBool();
    m_collectionsReceiptText  = settings.value("collectionReceiptText", tr("Abholbon für")).toString();

    m_receiptPrinterFont = new QFont(receiptPrinterFontList.at(0));
    m_receiptPrinterFont->setPointSize(receiptPrinterFontList.at(1).toInt());
    m_receiptPrinterFont->setStretch(receiptPrinterFontList.at(2).toInt());

    m_printCompanyNameBold = settings.value("printCompanyNameBold", false).toBool();
    m_printQRCode = settings.value("qrcode", true).toBool();
    m_logoFileName = "";
    if (settings.value("useLogo", false).toBool())
        m_logoFileName = settings.value("logo", "logo.png").toString();

    m_advertisingFileName = "";
    if (settings.value("useAdvertising", false).toBool())
        m_advertisingFileName = settings.value("advertising", "advertising.png").toString();

    m_logoRight = settings.value("logoRight", false).toBool();
    m_numberCopies = settings.value("numberCopies", 1).toInt();
    m_paperFormat = settings.value("paperFormat", "A4").toString();
    m_currency = Database::getCurrency();
    m_smallPrinter = (settings.value("paperWidth", 80).toInt() <= 60)?true :false;

    m_feedProdukt = settings.value("feedProdukt", 5).toInt();
    m_feedCompanyHeader = settings.value("feedCompanyHeader", 5).toInt();
    m_feedCompanyAddress = settings.value("feedCompanyAddress", 5).toInt();
    m_feedCashRegisterid = settings.value("feedCashRegisterid", 5).toInt();
    m_feedTimestamp = settings.value("feedTimestamp", 5).toInt();
    m_feedTax = settings.value("feedTaxSpin", 5).toInt();
    m_feedPrintHeader = settings.value("feedPrintHeader", 5).toInt();
    m_feedHeaderText = settings.value("feedHeaderText", 5).toInt();

    m_printQrCodeLeft = settings.value("qrcodeleft", false).toBool();
    m_useDecimalQuantity = settings.value("useDecimalQuantity", false).toBool();

    if (settings.value("noPrinter", false).toBool()) {
        m_useReportPrinter = false;
    } else {
        m_useReportPrinter = settings.value("useReportPrinter", false).toBool();
        if (m_numberCopies > 1 && m_useReportPrinter) {
            m_numberCopies = 1;
        } else {
            m_useReportPrinter = false;
        }
    }
}

DocumentPrinter::~DocumentPrinter()
{
    SpreadSignal::setProgressBarValue(-1);
}

//--------------------------------------------------------------------------------

void DocumentPrinter::printTestDocument(QFont font)
{
    QTextDocument *testDoc = new QTextDocument();
    testDoc->setHtml(Reports::getReport(2, true));
    testDoc->setDefaultFont(font);
    printDocument(testDoc, "TEST DRUCK");
}

void DocumentPrinter::printDocument(QTextDocument *document, QString title)
{

    SpreadSignal::setProgressBarWait(true);

    QPrinter printer;
    QrkSettings settings;
    bool usePDF = settings.value("reportPrinterPDF", false).toBool();
    if (usePDF) {
        printer.setOutputFormat(QPrinter::PdfFormat);
        QDir dir(m_pdfPrinterPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
    }

    document->setDefaultFont(*m_printerFont);

    if ( m_noPrinter || printer.outputFormat() == QPrinter::PdfFormat) {
        initAlternatePrinter(printer);
        printer.setOutputFileName(QString(m_pdfPrinterPath + "/QRK-REPORT_%1.pdf").arg( title ));
        document->adjustSize();

    } else {
        initAlternatePrinter(printer);
        printer.setPrinterName(settings.value("reportPrinter").toString());
        document->adjustSize();
    }

    QSizeF size(printer.width(), printer.height());
    document->setPageSize(size);

    QPainter painter(&printer);
    document->documentLayout()->setPaintDevice(painter.device());
    document->drawContents(&painter);

//    document->print(&printer);
    SpreadSignal::setProgressBarWait(false);
}

void DocumentPrinter::printReceipt(QJsonObject data)
{
    // print receipt
    QPrinter printer;
    //    printer.setResolution(600);

    if (data.value("isCopy").toBool())
        m_numberCopies = 1;

    m_receiptNum = data.value("receiptNum").toInt();
    SpreadSignal::setProgressBarValue(1);

    if (data.value("isInvoiceCompany").toBool()) {
        if ( initInvoiceCompanyPrinter(printer) )
            printI(data, printer );

    } else {

        if ( initPrinter(printer) )
            printI(data, printer );

        /* Some Printdriver do not accept more than 1 print.
     * so we send a second printjob
     */
        if (m_numberCopies > 1)
            printI(data, printer );

        if (m_useReportPrinter) {
            if (initAlternatePrinter(printer))
                printI( data, printer );
        }
    }
}

void DocumentPrinter::printCollectionReceipt(QJsonObject data, QPrinter &printer)
{

    QFont font(*m_receiptPrinterFont);

    // font.setFixedPitch(true);

    QFont boldFont(*m_receiptPrinterFont);
    boldFont.setBold(true);
    boldFont.setPointSize(m_receiptPrinterFont->pointSize() + 2);

    QFontMetrics boldMetr(boldFont);

    const int WIDTH = printer.pageRect().width();
    int y = 0;

    QString shopName = data.value("shopName").toString();
    QString shopMasterData = data.value("shopMasterData").toString();

    QJsonArray Orders = data["Orders"].toArray();
    foreach (const QJsonValue & item, Orders)
    {

        const QJsonObject& order = item.toObject();

        double count = order.value("count").toDouble();
        bool coupon = order.value("coupon").toString() == "1";
        if (!coupon)
            continue;

        for (int i = 0; i< count; i++) {
            QPainter painter(&printer);
            painter.setFont(font);
            QPen pen(Qt::black);
            painter.setPen(pen);
            QFontMetrics fontMetr = painter.fontMetrics();

           y = 0;

            if (m_printCompanyNameBold) {
                painter.save();
                painter.setFont(boldFont);
                painter.drawText(0, y, WIDTH, boldMetr.height() + 4, Qt::AlignCenter, shopName);
                y += 5;
                painter.restore();
            } else {
                painter.drawText(0, y, WIDTH, fontMetr.height() + 4, Qt::AlignCenter, shopName);
                y += 5;
            }

            int shopMasterDataHeight = shopMasterData.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
            painter.drawText(0, y, WIDTH, shopMasterDataHeight + 4, Qt::AlignCenter, shopMasterData);
            y += 15 + shopMasterDataHeight;


            painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignCenter, tr("Datum: %1 Uhrzeit: %2")
                             .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("dd.MM.yyyy"))
                             .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("hh:mm:ss")));
            y += 5 + fontMetr.height();

            painter.save();
            painter.setFont(boldFont);

            y += 5 + boldMetr.height();
            painter.drawText(0, y, WIDTH, boldMetr.height(), Qt::AlignCenter, m_collectionsReceiptText);
            y += 5 + boldMetr.height() * 2;

            QString product = "1 x " + order.value("product").toString();
            product = Utils::wordWrap(product, WIDTH, boldFont);
            int productHeight = product.split(QRegExp("\n|\r\n|\r")).count() * boldMetr.height();
            painter.drawText(0,  y + 10, WIDTH,  productHeight, Qt::AlignCenter, product);
            painter.end();
        }
    }
}

void DocumentPrinter::printI(QJsonObject data, QPrinter &printer)
{

    int fontsize = m_receiptPrinterFont->pointSize();

    QPainter painter(&printer);
    QFont font(*m_receiptPrinterFont);

    // font.setFixedPitch(true);
    painter.setFont(font);
    QFontMetrics fontMetr = painter.fontMetrics();

    QFont grossFont(font);
    // grossFont.setFixedPitch(true);
    QFontMetrics grossMetrics(grossFont, &printer);

    // QFont boldFont("Courier-New", boldsize, QFont::Bold);  // for sum
    QFont boldFont(*m_receiptPrinterFont);
    boldFont.setBold(true);
    boldFont.setPointSize(m_receiptPrinterFont->pointSize() + 2);

    QFontMetrics boldMetr(boldFont);

    QPen pen(Qt::black);
    painter.setPen(pen);

    QJsonArray Orders = data["Orders"].toArray();
    int oc = Orders.count() + 100;

    const int WIDTH = printer.pageRect().width();

    int y = 0;

    bool logo = false;
    QFileInfo checkFile(m_logoFileName);
    // check if file exists and if yes: Is it really a file and no directory?
    if (checkFile.exists() && checkFile.isFile()) {
        logo = true;
    }

    bool advertising = false;
    QFileInfo checkAdvertisingFile(m_advertisingFileName);
    // check if file exists and if yes: Is it really a file and no directory?
    if (checkAdvertisingFile.exists() && checkAdvertisingFile.isFile()) {
        advertising = true;
    }

    SpreadSignal::setProgressBarValue(((float)15 / (float)oc) * 100);

    bool isInvoiceCompany = data.value("isInvoiceCompany").toBool();

    QPixmap logoPixmap;
    QPixmap advertisingPixmap;

    QString shopName = data.value("shopName").toString();
    QString shopMasterData = data.value("shopMasterData").toString();

    if (!isInvoiceCompany) {
        if (logo) {

            logoPixmap.load(m_logoFileName);

            if (m_logoRight) {
                if (logoPixmap.width() > WIDTH / 2.50)
                    logoPixmap =  logoPixmap.scaled(WIDTH / 2.50, printer.pageRect().height(), Qt::KeepAspectRatio);

                painter.drawPixmap(WIDTH - logoPixmap.width() - 1, y, logoPixmap);

                QRect rect;
                if (m_printCompanyNameBold) {
                    painter.save();
                    painter.setFont(boldFont);
                    rect = painter.boundingRect(0, y, WIDTH - logoPixmap.width(), logoPixmap.height(), Qt::AlignLeft, shopName);
                    painter.drawText(0, y, rect.width(), rect.height(), Qt::AlignLeft, shopName);
                    y += 5;
                    painter.restore();
                } else {
                    rect = painter.boundingRect(0, y, WIDTH - logoPixmap.width(), logoPixmap.height(), Qt::AlignLeft, shopName);
                    painter.drawText(0, y, rect.width(), rect.height(), Qt::AlignLeft, shopName);
                    y += 5;
                }

                rect = painter.boundingRect(0, y, WIDTH - logoPixmap.width(), logoPixmap.height(), Qt::AlignLeft, shopMasterData);
                painter.drawText(0, y, rect.width(), rect.height(), Qt::AlignLeft, shopMasterData);

                y += 5 + qMax(rect.height(), logoPixmap.height()) + 4;
                painter.drawLine(0, y, WIDTH, y);
                y += 5;
            } else {

                if (logoPixmap.width() > WIDTH)
                    logoPixmap =  logoPixmap.scaled(WIDTH, printer.pageRect().height(), Qt::KeepAspectRatio);
                painter.drawPixmap((WIDTH / 2) - (logoPixmap.width()/2) - 1, y, logoPixmap);
                y += 5 + logoPixmap.height() + 4;

                if (m_printCompanyNameBold) {
                    painter.save();
                    painter.setFont(boldFont);
                    painter.drawText(0, y, WIDTH, boldMetr.height() + 4, Qt::AlignCenter, shopName);
                    y += 5;
                    painter.restore();

                } else {
                    painter.drawText(0, y, WIDTH, fontMetr.height() + 4, Qt::AlignCenter, shopName);
                    y += 5;
                }

                int shopMasterDataHeight = shopMasterData.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
                painter.drawText(0, y, WIDTH, shopMasterDataHeight + 4, Qt::AlignCenter, shopMasterData);
                y += 5 + shopMasterDataHeight;

            }
        } else {
            if (m_printCompanyNameBold) {
                painter.save();
                painter.setFont(boldFont);
                painter.drawText(0, y, WIDTH, boldMetr.height() + 4, Qt::AlignCenter, shopName);
                y += m_feedCompanyHeader;
                painter.restore();
            } else {
                painter.drawText(0, y, WIDTH, fontMetr.height() + 4, Qt::AlignCenter, shopName);
                y += m_feedCompanyHeader;
            }

            int shopMasterDataHeight = shopMasterData.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
            painter.drawText(0, y, WIDTH, shopMasterDataHeight + 4, Qt::AlignCenter, shopMasterData);
            y += m_feedCompanyAddress + shopMasterDataHeight;

        }
    }

    if (! data.value("printHeader").toString().isEmpty()) {
        QString printHeader = data.value("printHeader").toString();
        int headerTextHeight = printHeader.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
        painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignCenter, printHeader);
        y += m_feedPrintHeader + headerTextHeight + 4;
        painter.drawLine(0, y, WIDTH, y);
    }

    // CustomerText
    if (! data.value("headerText").toString().isEmpty()) {
        y += 5;
        QString headerText = data.value("headerText").toString();
        int headerTextHeight = headerText.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
        painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignLeft, headerText);
        y += m_feedHeaderText + headerTextHeight + 4;
        painter.drawLine(0, y, WIDTH, y);
        y += 5;
    }

    SpreadSignal::setProgressBarValue(((float)20 / (float)oc) * 100);

    painter.save();
    painter.setFont(boldFont);

    // receiptPrinterHeading or cancellationtext by cancellation
    QString comment = data.value("comment").toString();
    if (!comment.isEmpty()) {
        y += 5 + boldMetr.height();
        painter.drawText(0, y, WIDTH, boldMetr.height(), Qt::AlignCenter, comment);
        y += 5 + boldMetr.height() * 2;
    }

    painter.restore();

    QString copy = "";
    if (data.value("isCopy").toBool())
        copy = tr("( Kopie )");

    if (data.value("isTestPrint").toBool()) {
        data["kassa"] = "DEMO-PRINT-1";
        data["receiptNum"] = 0;
        data["typeText"] = "DEMO";
    }

    if (m_smallPrinter) {
        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                         tr("Kasse: %1")
                         .arg(data.value("kasse").toString()));
        y += m_feedCashRegisterid + fontMetr.height();

        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                         tr("Bon-Nr: %1 %2 - %3")
                         .arg(data.value("receiptNum").toInt())
                         .arg(copy).arg(data.value("typeText").toString()));
        y += m_feedCashRegisterid + fontMetr.height();

        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                         tr("Positionen: %1")
                         .arg(data.value("positions").toInt()));
        y += m_feedCashRegisterid + fontMetr.height();

        SpreadSignal::setProgressBarValue(((float)30 / (float)oc) * 100);

        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Datum: %1")
                         .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("dd.MM.yyyy")));
        y += m_feedTimestamp + fontMetr.height();

        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Uhrzeit: %1")
                         .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("hh:mm:ss")));
        y += m_feedTimestamp + fontMetr.height();

    } else {
        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                         tr("Kasse: %1 Positionen: %2")
                         .arg(data.value("kasse").toString())
                         .arg(data.value("positions").toInt()));
        // TODO: 1 or 2 Lines from config, in my case the text is to long. Do not fit in one line
        y += m_feedCashRegisterid + fontMetr.height();

        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                         tr("Bon-Nr: %1 %2 - %3")
                         .arg(data.value("receiptNum").toInt())
                         .arg(copy).arg(data.value("typeText").toString()));
        y += m_feedCashRegisterid + fontMetr.height();


        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Datum: %1 Uhrzeit: %2")
                         .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("dd.MM.yyyy"))
                         .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("hh:mm:ss")));
        y += m_feedTimestamp + fontMetr.height();

    }

    SpreadSignal::setProgressBarValue(((float)50 / (float)oc) * 100);

    painter.drawLine(0, y, WIDTH, y);
    y += 5;

    // paint orders

    const int X_COUNT = 0;
    const int X_NAME  = 25;

    painter.drawText(X_COUNT, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Anz"));
    painter.drawText(X_NAME,  y, WIDTH - X_COUNT,  fontMetr.height(), Qt::AlignLeft, tr("Artikel"));
    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight, tr("Preis  M%"));
    y += 5 + fontMetr.height();

    SpreadSignal::setProgressBarValue(((float)60 / (float)oc) * 100);

    int progress = 60;

    foreach (const QJsonValue & item, Orders)
    {

        SpreadSignal::setProgressBarValue(((float)progress++ / (float)oc) * 100);

        const QJsonObject& order = item.toObject();

        double count = order.value("count").toDouble();

        QString taxPercent;

        if (Database::getTaxLocation() == "CH")
            taxPercent = QString("%1").arg(QString::number(order.value("tax").toDouble(),'f',2));
        else
            taxPercent = QString("%1").arg(QString::number(order.value("tax").toInt()));

        if (taxPercent == "0") taxPercent = "00";

        QString grossText = QString("%1").arg(QString::number(order.value("gross").toDouble(), 'f', 2));
        QString singleGrossText = QString("%1 x %2").arg(QString::number(count)).arg(QString::number(order.value("singleprice").toDouble(), 'f', 2));

        if (data.value("isTestPrint").toBool()) {
            count = 0.0;
            grossText = "0,0";
            singleGrossText = "0 x 0,0";
        }

        int grossWidth = grossMetrics.boundingRect(grossText + "   " + taxPercent).width();
        QString product = order.value("product").toString();
        product = Utils::wordWrap(product, WIDTH - grossWidth - X_NAME, font);
        int productHeight = product.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();

        // check if new drawText is heigher than page height
        if ( (y + productHeight + 20) > printer.pageRect().height() )
        {
            printer.newPage();
            y = 0;
        }

        QRect usedRect;

        painter.drawText(X_COUNT, y, WIDTH - X_COUNT, fontMetr.height(), Qt::AlignLeft, QString::number(count));
        painter.drawText(X_NAME,  y, WIDTH,  productHeight, Qt::AlignLeft, product, &usedRect);

        if (m_useDecimalQuantity || count > 1 || count < -1) {
            y += m_feedProdukt + usedRect.height();
            painter.drawText(X_NAME,  y, WIDTH - X_NAME - grossWidth - 5,  fontMetr.height(), Qt::AlignLeft, singleGrossText);
        } else {
            y += usedRect.height() - fontMetr.height();
        }

        painter.setFont(grossFont);
        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight, grossText + "   " + taxPercent);
        painter.setFont(font);

        y += m_feedProdukt + fontMetr.height();
    }

    SpreadSignal::setProgressBarValue(((float)progress +10 / (float)oc) * 100);

    painter.drawLine(0, y, WIDTH, y);
    y += 5;

    // if there is not enough space for sum+tax lines, start new page
    if ( (y + (data.value("taxesCount").toInt() * (5 + fontMetr.height())) + boldMetr.height() + 10) > printer.pageRect().height() )
    {
        printer.newPage();
        y = 0;
    }

    SpreadSignal::setProgressBarValue(((float)(progress += 10) / (float)oc) * 100);

    QString sum = QString::number(data.value("sum").toDouble(), 'f', 2);

    if (data.value("isTestPrint").toBool()) {
        sum = "0,0";
    }

    int ySave = y; // save y when QR-Code was printing left
    QString sumText = tr("Gesamt: %1").arg(sum);
    painter.save();
    painter.setFont(boldFont);
    painter.drawText(0, y, WIDTH, boldMetr.height(), Qt::AlignRight, sumText);
    painter.restore();

    y += 5 + boldMetr.height();

    QJsonArray Taxes = data["Taxes"].toArray();
    foreach (const QJsonValue & item, Taxes)
    {
        const QJsonObject& tax = item.toObject();

        QString taxSum = QString::number(tax.value("t2").toString().toDouble(), 'f', 2);
        if (data.value("isTestPrint").toBool()) {
            taxSum = "0,0";
        }

        QString taxValue = tax.value("t1").toString();
        if (taxValue != "0%"){
            painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight,
                             tr("MwSt %1: %2")
                             .arg(tax.value("t1").toString())
                             .arg(taxSum));

            y += m_feedTax + fontMetr.height();
        }
    }
    y += 5 + fontMetr.height();

    // Die Währung müsste sonst neben jeden Preis stehen, darum schreiben wir diesen InfoText
    QString currencyText = tr("(Alle Beträge in %1)").arg(m_currency);
    if (m_printQRCode && m_printQrCodeLeft) {
        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight, currencyText);
        y += 20;
    } else {
        painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignCenter, currencyText);
    }

    y += 5 + fontMetr.height();

    SpreadSignal::setProgressBarValue(((float)(progress += 10) / (float)oc) * 100);

    QString qr_code_rep = shopName + " - " + shopMasterData;
    QString ocr_code_rep = shopName + " - " + shopMasterData;

    if (RKSignatureModule::isDEPactive()) {
        QString signature = Utils::getReceiptSignature(data.value("receiptNum").toInt(), true);
        if (signature.split('.').size() == 3) {
            qr_code_rep = signature.split('.').at(1);
            qr_code_rep = RKSignatureModule::base64Url_decode(qr_code_rep);
            ocr_code_rep = qr_code_rep;
            qr_code_rep = qr_code_rep + "_" + RKSignatureModule::base64Url_decode(signature.split('.').at(2)).toBase64();
            ocr_code_rep = ocr_code_rep + "_" + RKSignatureModule::base32_encode(RKSignatureModule::base64Url_decode(signature.split('.').at(2)));
            if (signature.split('.').at(2) == RKSignatureModule::base64Url_encode("Sicherheitseinrichtung ausgefallen"))
                data["isSEEDamaged"] = true;
            qDebug() << "Function Name: " << Q_FUNC_INFO << " QRCode Representation: " << qr_code_rep;
        } else {
            qInfo() << "Function Name: " << Q_FUNC_INFO << " Print old (before DEP) Receipt Id:" << data.value("receiptNum").toInt();
        }
    }

    if (m_printQRCode && m_printQrCodeLeft) {
        QRCode *qr = new QRCode;
        QPixmap QR = qr->encodeTextToPixmap(qr_code_rep);
        delete qr;

        int sumWidth = boldMetr.boundingRect(sumText).width();

        if (QR.width() > (WIDTH - sumWidth))
            QR =  QR.scaled(WIDTH - sumWidth - 4, printer.pageRect().height(), Qt::KeepAspectRatio);

        painter.drawPixmap( 1, ySave, QR);

        y = 20 + qMax(ySave + QR.height(), y);

        if(data.value("isSEEDamaged").toBool()) {
            y += 5 + fontMetr.height();
            painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignCenter, "Sicherheitseinrichtung ausgefallen");
            y += 5 + fontMetr.height();
        }
    }

    if(isInvoiceCompany) {
        y += 5;
        QString printFooter = tr("Dient als Steuerbeleg für ihr Finanzamt.\n"
                                 "Wichtig: Beleg bitte aufbewahren.\n"
                                 "Diese Rechnung ist nur in Verbindung\n"
                                 "mit dem angehefteten Kassenbon gültig.\n"
                                 ">> Datum = Liefer- und Rechnungsdatum <<\n");

        int headerTextHeight = printFooter.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
        painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignCenter, printFooter);
        y += 5 + headerTextHeight + 4;

    }
    else if (! data.value("printFooter").toString().isEmpty()) {
        y += 5;
        QString printFooter = data.value("printFooter").toString();
        int headerTextHeight = printFooter.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
        painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignCenter, printFooter);
        y += 5 + headerTextHeight + 4;
    }

    SpreadSignal::setProgressBarValue(((float)(progress += 10) / (float)oc) * 100);

    if (m_printQRCode && !m_printQrCodeLeft) {
        QRCode *qr = new QRCode;
        QPixmap QR = qr->encodeTextToPixmap(qr_code_rep);
        delete qr;

        if (QR.width() > WIDTH)
            QR =  QR.scaled(WIDTH, printer.pageRect().height(), Qt::KeepAspectRatio);

            y += 5;
            painter.drawLine(0, y, WIDTH, y);
            y += 5;

            // check if new drawText is heigher than page height
            if ( (y + QR.height() + 20) > printer.pageRect().height() )
            {
                printer.newPage();
                y = 0;
            }
            painter.drawPixmap((WIDTH / 2) - (QR.width()/2) - 1, y, QR);

            y += QR.height() + 20;

    } else if (!m_printQRCode && Database::getTaxLocation() == "AT") {
        y += 5;
        painter.drawLine(0, y, WIDTH, y);
        y += 5;

        int id = QFontDatabase::addApplicationFont(":/font/ocra.ttf");
        QString family = QFontDatabase::applicationFontFamilies(id).at(0);
        QFont ocrfont(family, fontsize);
        painter.setFont(ocrfont);
        QFontMetrics ocrMetr = painter.fontMetrics();

        ocr_code_rep = Utils::wordWrap(ocr_code_rep, WIDTH, ocrfont);
        int ocrHeight = ocr_code_rep.split(QRegExp("\n|\r\n|\r")).count() * ocrMetr.height();

        // check if new drawText is heigher than page height
        if ( (y + ocrHeight + 20) > printer.pageRect().height() )
        {
            printer.newPage();
            y = 0;
        }

        painter.drawText(0,  y, WIDTH,  ocrHeight, Qt::AlignLeft, ocr_code_rep);
        y += ocrHeight + 20;
    }

    if (!m_printQrCodeLeft) {
        if(data.value("isSEEDamaged").toBool()) {
            y += 5 + fontMetr.height();
            painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignCenter, "Sicherheitseinrichtung ausgefallen");
            y += 5 + fontMetr.height();
        }
    }

    if (advertising) {
        y += 5;
        if ( (y + advertisingPixmap.height() + 20) > printer.pageRect().height() )
        {
            printer.newPage();
            y = 0;
        }

        advertisingPixmap.load(m_advertisingFileName);

        if (advertisingPixmap.width() > WIDTH)
            advertisingPixmap = advertisingPixmap.scaled(WIDTH, printer.pageRect().height(), Qt::KeepAspectRatio);
        painter.drawPixmap((WIDTH / 2) - (advertisingPixmap.width()/2) - 1, y, advertisingPixmap);
        y += 5 + advertisingPixmap.height() + 4;

    }

    if (! data.value("printAdvertisingText").toString().isEmpty()) {
        QString printAdvertisingText = data.value("printAdvertisingText").toString();
        int headerTextHeight = printAdvertisingText.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
        painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignCenter, printAdvertisingText);
    }

    painter.end();

    SpreadSignal::setProgressBarValue(100);

    if (m_printCollectionsReceipt)
        printCollectionReceipt(data, printer);

}

//--------------------------------------------------------------------------------

bool DocumentPrinter::initPrinter(QPrinter &printer)
{
    QrkSettings settings;
    if ( m_noPrinter || printer.outputFormat() == QPrinter::PdfFormat)
        printer.setOutputFileName(QString(m_pdfPrinterPath + "/QRK-BON%1.pdf").arg( m_receiptNum ));
    else
        printer.setPrinterName(settings.value("receiptPrinter").toString());

    printer.setFullPage(true);

    // printer.setResolution(720);

    printer.setPaperSize(QSizeF(settings.value("paperWidth", 80).toInt(),
                                settings.value("paperHeight", 210).toInt()), QPrinter::Millimeter);

    const QMarginsF marginsF(settings.value("marginLeft", 0).toDouble(),
                             settings.value("marginTop", 17).toDouble(),
                             settings.value("marginRight", 5).toDouble(),
                             settings.value("marginBottom", 0).toInt());

    printer.setPageMargins(marginsF,QPageLayout::Millimeter);

    printer.setFullPage(false);

    return true;
}

bool DocumentPrinter::initAlternatePrinter(QPrinter &printer)
{
    QrkSettings settings;
    printer.setPrinterName(settings.value("reportPrinter").toString());

    QString f = settings.value("paperFormat").toString();
    if (f == "A4")
        printer.setPaperSize(printer.A4);
    if (f == "A5")
        printer.setPaperSize(printer.A5);
    if (f == "POS") {
        printer.setFullPage(true);
        printer.setPaperSize(QSizeF(settings.value("paperWidth", 80).toInt(),
                                    settings.value("paperHeight", 210).toInt()), QPrinter::Millimeter);

        const QMarginsF marginsF(settings.value("marginLeft", 0).toDouble(),
                                 settings.value("marginTop", 17).toDouble(),
                                 settings.value("marginRight", 5).toDouble(),
                                 settings.value("marginBottom", 0).toInt());

        printer.setPageMargins(marginsF,QPageLayout::Millimeter);
        printer.setFullPage(false);
    }

    return true;
}

bool DocumentPrinter::initInvoiceCompanyPrinter(QPrinter &printer)
{
    QrkSettings settings;
    printer.setPrinterName(settings.value("invoiceCompanyPrinter").toString());

    QString f = settings.value("invoiceCompanyPaperFormat").toString();
    if (f == "A4")
        printer.setPaperSize(printer.A4);
    if (f == "A5")
        printer.setPaperSize(printer.A5);

    printer.setFullPage(true);

    const QMarginsF marginsF(settings.value("invoiceCompanyMarginLeft", 90).toDouble(),
                             settings.value("invoiceCompanyMarginTop", 50).toDouble(),
                             settings.value("invoiceCompanyMarginRight", 10).toDouble(),
                             settings.value("invoiceCompanyMarginBottom", 0).toInt());

    printer.setPageMargins(marginsF,QPageLayout::Millimeter);

    printer.setFullPage(false);

    return true;
}
