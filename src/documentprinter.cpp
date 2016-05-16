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

#include "documentprinter.h"
#include "database.h"
#include "utils/utils.h"
#include "utils/qrcode.h"
#include "reports.h"

#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>
#include <QDir>
#include <QAbstractTextDocumentLayout>
#include <QStandardPaths>
#include <QDebug>


DocumentPrinter::DocumentPrinter(QObject *parent, QProgressBar *progressBar, bool noPrinter)
  :QObject(parent), noPrinter(noPrinter)
{

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");

  QList<QString> printerFontList = settings.value("printerfont", "Courier-New,10,100").toString().split(",");
  QList<QString> receiptPrinterFontList = settings.value("receiptprinterfont", "Courier-New,8,100").toString().split(",");

  printerFont = new QFont(printerFontList.at(0));
  printerFont->setPointSize(printerFontList.at(1).toInt());
  printerFont->setStretch(printerFontList.at(2).toInt());

  receiptPrinterFont = new QFont(receiptPrinterFontList.at(0));
  receiptPrinterFont->setPointSize(receiptPrinterFontList.at(1).toInt());
  receiptPrinterFont->setStretch(receiptPrinterFontList.at(2).toInt());

  printCompanyNameBold = settings.value("printCompanyNameBold", false).toBool();
  printQRCode = settings.value("qrcode", true).toBool();
  logoFileName = "";
  if (settings.value("useLogo", false).toBool())
    logoFileName = settings.value("logo", "logo.png").toString();

  logoRight = settings.value("logoRight", false).toBool();
  numberCopies = settings.value("numberCopies", 1).toInt();
  paperFormat = settings.value("paperFormat", "A4").toString();
  currency = Database::getCurrency();
  smallPrinter = (settings.value("paperWidth", 80).toInt() <= 60)?true :false;

  if (noPrinter) {
    useReportPrinter = false;
  } else {
    useReportPrinter = settings.value("useReportPrinter", false).toBool();
    if (numberCopies > 1 && useReportPrinter) {
      numberCopies = 1;
    } else {
      useReportPrinter = false;
    }
  }

  if(!progressBar)
    progressBar = new QProgressBar();

  pb = progressBar;
  pb->setValue(0);
}

DocumentPrinter::~DocumentPrinter()
{
  pb->setMaximum(100);
  pb->reset();

  // qDebug() << "delete ->>>> ~DocumentPrinter()";
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
  QPrinter printer;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  bool usePDF = settings.value("reportPrinterPDF", false).toBool();
  if (usePDF) {
    printer.setOutputFormat(QPrinter::PdfFormat);
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/pdf");
    if (!dir.exists()) {
      dir.mkpath(".");
    }
  }

  document->setDefaultFont(*printerFont);

  if ( noPrinter || printer.outputFormat() == QPrinter::PdfFormat) {
    initAlternatePrinter(printer);
    printer.setOutputFileName(QString(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/pdf/QRK-REPORT_%1.pdf").arg( title ));
    document->adjustSize();

  } else {
    initAlternatePrinter(printer);
    printer.setPrinterName(settings.value("reportPrinter").toString());
    document->adjustSize();

  }

  document->print(&printer);

}

void DocumentPrinter::printReceipt(QJsonObject data)
{
  // print receipt
  QPrinter printer;
  //    printer.setResolution(600);

  if (data.value("isCopy").toBool())
    numberCopies = 1;

  receiptNum = data.value("receiptNum").toInt();
  pb->setValue(1);

  if (data.value("isInvoiceCompany").toBool()) {
    if ( initInvoiceCompanyPrinter(printer) )
      printI(data, printer );

  } else {

    if ( initPrinter(printer) )
      printI(data, printer );

    /* Some Printdriver do not accept more than 1 print.
     * so we send a second printjob
     */
    if (numberCopies > 1)
      printI(data, printer );

    if (useReportPrinter) {
      if (initAlternatePrinter(printer))
        printI( data, printer );
    }
  }
}

void DocumentPrinter::printI(QJsonObject data, QPrinter &printer)
{


  int fontsize = receiptPrinterFont->pointSize();
/*  int boldsize = 10;

   if (smallPrinter) {
    fontsize = 6;
    boldsize = 8;
  }
*/

  QPainter painter(&printer);

//  int id = QFontDatabase::addApplicationFont(":/font/Oxygen-Sans.ttf");
//  QString family = QFontDatabase::applicationFontFamilies(id).at(0);

  // QFont font("Courier-New", fontsize);

  QFont font(*receiptPrinterFont);

  // font.setFixedPitch(true);
  painter.setFont(font);
  QFontMetrics fontMetr = painter.fontMetrics();
  qDebug() << "Font: " << font.family();

  QFont grossFont(font);
  // grossFont.setFixedPitch(true);
  QFontMetrics grossMetrics(grossFont, &printer);

  // QFont boldFont("Courier-New", boldsize, QFont::Bold);  // for sum
  QFont boldFont(*receiptPrinterFont);
  boldFont.setBold(true);
  boldFont.setPointSize(receiptPrinterFont->pointSize() + 2);

  QFontMetrics boldMetr(boldFont);

  QPen pen(Qt::black);
  painter.setPen(pen);

  const int WIDTH = printer.pageRect().width();

  int y = 0;

  bool logo = false;
  QFileInfo checkFile(logoFileName);
  // check if file exists and if yes: Is it really a file and no directory?
  if (checkFile.exists() && checkFile.isFile()) {
    logo = true;
  }

  pb->setValue(10);

  bool isInvoiceCompany = data.value("isInvoiceCompany").toBool();

  QPixmap logoPixmap;
  QString shopName = data.value("shopName").toString();
  QString shopMasterData = data.value("shopMasterData").toString();


  if (!isInvoiceCompany) {
    if (logo) {

      logoPixmap.load(logoFileName);

      if (logoRight) {

        if (logoPixmap.width() > printer.pageRect().width() / 2.50)
          logoPixmap =  logoPixmap.scaled(printer.pageRect().width() / 2.50, printer.pageRect().height(), Qt::KeepAspectRatio);

        painter.drawPixmap(WIDTH - logoPixmap.width() - 1, y, logoPixmap);

        QRect rect;
        if (printCompanyNameBold) {
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

        if (logoPixmap.width() > printer.pageRect().width())
          logoPixmap =  logoPixmap.scaled(printer.pageRect().width(), printer.pageRect().height(), Qt::KeepAspectRatio);
        painter.drawPixmap((WIDTH / 2) - (logoPixmap.width()/2) - 1, y, logoPixmap);
        y += 5 + logoPixmap.height() + 4;

        if (printCompanyNameBold) {
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
      if (printCompanyNameBold) {
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
  }

  pb->setValue(15);

  if (! data.value("printHeader").toString().isEmpty()) {
    QString printHeader = data.value("printHeader").toString();
    int headerTextHeight = printHeader.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
    painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignCenter, printHeader);
    y += 5 + headerTextHeight + 4;
    painter.drawLine(0, y, WIDTH, y);
  }

  if (! data.value("headerText").toString().isEmpty()) {
    y += 5;
    QString headerText = data.value("headerText").toString();
    int headerTextHeight = headerText.split(QRegExp("\n|\r\n|\r")).count() * fontMetr.height();
    painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignLeft, headerText);
    y += 5 + headerTextHeight + 4;
    painter.drawLine(0, y, WIDTH, y);
    y += 5;
  }

  pb->setValue(20);

  painter.save();
  painter.setFont(boldFont);

  y += 5 + boldMetr.height();
  painter.drawText(0, y, WIDTH, boldMetr.height(), Qt::AlignCenter, data.value("comment").toString());
  y += 5 + boldMetr.height() * 2;

  painter.restore();

  QString copy = "";
  if (data.value("isCopy").toBool())
    copy = tr("( Kopie )");

  if (data.value("isTestPrint").toBool()) {
    data["kassa"] = "DEMO-PRINT-1";
    data["receiptNum"] = 0;
    data["typeText"] = "DEMO";
  }

  if (smallPrinter) {
    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                     tr("Kasse: %1")
                     .arg(data.value("kasse").toString()));
    y += 5 + fontMetr.height();

    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                     tr("Bon-Nr: %1 %2")
                     .arg(data.value("receiptNum").toInt())
                     .arg(copy));
    y += 5 + fontMetr.height();

    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                     tr("Positionen: %1")
                     .arg(data.value("positions").toInt()));
    y += 5 + fontMetr.height();


    pb->setValue(30);

    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Datum: %1")
                     .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("dd.MM.yyyy")));
    y += 5 + fontMetr.height();

    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Uhrzeit: %1")
                     .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("hh:mm:ss")));
    y += 5 + fontMetr.height();

  } else {
    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                     tr("Kasse: %1 Positionen: %2")
                     .arg(data.value("kasse").toString())
                     .arg(data.value("positions").toInt()));
    y += 5 + fontMetr.height();

    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                     tr("Bon-Nr: %1 %2")
                     .arg(data.value("receiptNum").toInt())
                     .arg(copy));
    y += 5 + fontMetr.height();


    pb->setValue(30);

    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Datum: %1 Uhrzeit: %2")
                     .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("dd.MM.yyyy"))
                     .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("hh:mm:ss")));
    y += 5 + fontMetr.height();

  }
  pb->setValue(40);

  painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                   data.value("typeText").toString());
  y += 5 + fontMetr.height();

  painter.drawLine(0, y, WIDTH, y);
  y += 5;

  pb->setValue(50);

  // paint orders

  const int X_COUNT = 0;
  const int X_NAME  = 25;

  painter.drawText(X_COUNT, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Anz"));
  painter.drawText(X_NAME,  y, WIDTH - X_COUNT,  fontMetr.height(), Qt::AlignLeft, tr("Produkt"));
  painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight, tr("Preis"));
  y += 5 + fontMetr.height();

  pb->setValue(60);

  QJsonArray Orders = data["Orders"].toArray();

  int oc = Orders.count();
  pb->setMaximum(60 + oc);
  int progress = 60;

  foreach (const QJsonValue & item, Orders)
  {

    pb->setValue(progress++);

    const QJsonObject& order = item.toObject();

    double count = order.value("count").toDouble();
    QString grossText = QString("%1").arg(QString::number(order.value("gross").toDouble(), 'f', 2));
    QString singleGrossText = QString("%1 x %2").arg(QString::number(count)).arg(QString::number(order.value("singleprice").toDouble(), 'f', 2));

    if (data.value("isTestPrint").toBool()) {
      count = 0.0;
      grossText = "0,0";
      singleGrossText = "0 x 0,0";
    }

    int grossWidth = grossMetrics.boundingRect(grossText).width();
    QString product = order.value("product").toString();
    product = wordWrap(product, WIDTH - grossWidth - 5, font);
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

    y += 5 + usedRect.height();

    painter.drawText(X_NAME,  y, WIDTH - X_NAME - grossWidth - 5,  fontMetr.height(), Qt::AlignLeft, singleGrossText);
    painter.setFont(grossFont);
    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight, grossText);
    painter.setFont(font);

    y += 5 + fontMetr.height();
  }

  pb->setValue(progress + 10);

  painter.drawLine(0, y, WIDTH, y);
  y += 5;

  // if there is not enough space for sum+tax lines, start new page
  if ( (y + (data.value("taxesCount").toInt() * (5 + fontMetr.height())) + boldMetr.height() + 10) > printer.pageRect().height() )
  {
    printer.newPage();
    y = 0;
  }

  pb->setValue(progress + 10);

  QString sum = QString::number(data.value("sum").toDouble(), 'f', 2);

  if (data.value("isTestPrint").toBool()) {
    sum = "0,0";
  }

  painter.save();
  painter.setFont(boldFont);
  painter.drawText(0, y, WIDTH, boldMetr.height(), Qt::AlignRight,
                   tr("Gesamt: %1").arg(sum));
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

    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight,
                     tr("MwSt %1: %2")
                     .arg(tax.value("t1").toString())
                     .arg(taxSum));

    y += 5 + fontMetr.height();
  }
  y += 5 + fontMetr.height();

  QString currencyText = tr("(Alle Beträge in %1)").arg(currency);
  painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignCenter, currencyText);
  y += 5 + fontMetr.height();


  pb->setValue(progress + 10);

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

  QString sign = Utils::getReceiptSignature(data.value("receiptNum").toInt());

  if (printQRCode) {

    y += 5;
    painter.drawLine(0, y, WIDTH, y);
    y += 5;
    QRCode *qr = new QRCode;
    QPixmap QR = qr->encodeTextToPixmap(sign);
    delete qr;

    if (QR.width() > printer.pageRect().width())
      QR =  QR.scaled(printer.pageRect().width(), printer.pageRect().height(), Qt::KeepAspectRatio);

    // check if new drawText is heigher than page height
    if ( (y + QR.height() + 20) > printer.pageRect().height() )
    {
      printer.newPage();
      y = 0;
    }

    painter.drawPixmap((WIDTH / 2) - (QR.width()/2) - 1, y, QR);

  } else {
    if (Database::getTaxLocation() == "AT" && QDate::currentDate().year() > 2016) {
      y += 5;
      painter.drawLine(0, y, WIDTH, y);
      y += 5;

      int id = QFontDatabase::addApplicationFont(":/font/ocra.ttf");
      QString family = QFontDatabase::applicationFontFamilies(id).at(0);
      QFont ocrfont(family, fontsize);
      painter.setFont(ocrfont);
      QFontMetrics ocrMetr = painter.fontMetrics();

      sign = wordWrap(sign, WIDTH, ocrfont);
      int ocrHeight = sign.split(QRegExp("\n|\r\n|\r")).count() * ocrMetr.height();

      // check if new drawText is heigher than page height
      if ( (y + ocrHeight + 20) > printer.pageRect().height() )
      {
        printer.newPage();
        y = 0;
      }

      painter.drawText(0,  y, WIDTH,  ocrHeight, Qt::AlignLeft, sign);
    }

  }

  painter.end();

  pb->setValue(progress + 10);

}

//--------------------------------------------------------------------------------

bool DocumentPrinter::initPrinter(QPrinter &printer)
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");

  if ( noPrinter || printer.outputFormat() == QPrinter::PdfFormat)
    printer.setOutputFileName(QString(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/pdf/QRK-BON%1.pdf").arg( receiptNum ));
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
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");

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
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");

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

QString DocumentPrinter::wordWrap(QString text, int width, QFont font)
{
  QFontMetrics fm(font);
  QString result;
  int space = 0;

  for (;;) {
    int i = 0;
    while (i < text.length()) {
      if (fm.width(text.left(++i + 1)) > width) {
        int j = text.lastIndexOf(' ', i);
        space = 0;
        if (j > 0) {
          i = j;
          space = 1;
        }
        result += text.left(i);
        result += '\n';
        text = text.mid(i + space);
        break;
      }
    }
    if (i >= text.length())
      break;
  }
  return result + text;
}
