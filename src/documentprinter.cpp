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
 */

#include "documentprinter.h"

#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QFileInfo>
#include <QDateTime>

#include <QDebug>

DocumentPrinter::DocumentPrinter(QObject *parent, QProgressBar *progressBar, bool noPrinter)
  :QObject(parent), noPrinter(false)
{

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");

  logoRight = settings.value("logoRight", false).toBool();
  numberCopies = settings.value("numberCopies", 1).toInt();
  paperFormat = settings.value("paperFormat", "A4").toString();

  if (noPrinter) {
    useReportPrinter = false;
  } else {
    useReportPrinter = settings.value("useReportPrinter", true).toBool();
    if (numberCopies > 1 && useReportPrinter) {
      numberCopies = 1;
    }
  }

  this->noPrinter = noPrinter;
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

void DocumentPrinter::printDocument(QTextDocument *document)
{
  QPrinter printer;
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  printer.setPrinterName(settings.value("reportPrinter").toString());

  document->print(&printer);

}

void DocumentPrinter::printReceipt(QJsonObject data)
{
  // print receipt
  QPrinter printer;
  //    printer.setResolution(600);

  receiptNum = data.value("receiptNum").toInt();

  pb->setValue(1);
  if ( initPrinter(printer) )
    printI(data, printer );

  if (useReportPrinter) {
    if (initAlternatePrinter(printer))
      printI( data, printer );
  }

}

void DocumentPrinter::printI(QJsonObject data, QPrinter &printer)
{

  printer.setNumCopies(numberCopies);

  QPainter painter(&printer);
  QFont font("Arial", 8);
  painter.setFont(font);
  QFontMetrics fontMetr = painter.fontMetrics();

  QFont grossFont(font);
  grossFont.setFixedPitch(true);
  QFontMetrics grossMetrics(grossFont, &printer);

  QFont boldFont("Arial", 10, 100);  // for sum
  QFontMetrics boldMetr(boldFont);


  const int WIDTH = printer.pageRect().width();
  // const int WIDTH = printer.pageLayout().paintRect().width();

  int y = 0;

  bool logo = false;
  QFileInfo checkFile("logo.png");
  // check if file exists and if yes: Is it really a file and no directory?
  if (checkFile.exists() && checkFile.isFile()) {
    logo = true;
  }


  /*
    QPixmap logoPixmap;
    logoPixmap.load(":/icons/logo.png");

    painter.drawPixmap(WIDTH - logoPixmap.width() - 1, y, logoPixmap);
    QRect rect = painter.boundingRect(0, y, WIDTH - logoPixmap.width(), logoPixmap.height(), Qt::AlignCenter, shopName);
    painter.drawText(0, y, rect.width(), rect.height(), Qt::AlignCenter, shopName);
    y += 5 + qMax(rect.height(), logoPixmap.height()) + 4;
    painter.drawLine(0, y, WIDTH, y);

*/

  pb->setValue(10);


  QPixmap logoPixmap;
  QString shopName = data.value("shopName").toString();

  if (logo) {

    logoPixmap.load("logo.png");
    /*
        QRect rect = painter.viewport();
        QSize size = logoImage.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(),size.width(), size.height());
        painter.setWindow(logoImage.rect());
*/

    // qDebug() << "Printer resolution: " << printer.resolution();

    //        logoPixmap.size().scale(rect.size(), Qt::KeepAspectRatio);


    if (logoRight) {

      painter.drawPixmap(WIDTH - logoPixmap.width() - 1, y, logoPixmap);
      QRect rect = painter.boundingRect(0, y, WIDTH - logoPixmap.width(), logoPixmap.height(), Qt::AlignCenter, shopName);
      painter.drawText(0, y, rect.width(), rect.height(), Qt::AlignCenter, shopName);

      y += 5 + qMax(rect.height(), logoPixmap.height()) + 4;
      painter.drawLine(0, y, WIDTH, y);

    } else {

      painter.drawPixmap((WIDTH / 2) - (logoPixmap.width()/2) - 1, y, logoPixmap);
      int height= logoPixmap.height();
      y += 5 + qMax(height, logoPixmap.height()) + 4;
    }
  }

  //    painter.drawLine(0, y, WIDTH, y);

  //    y += 9;
  pb->setValue(15);

  if (! logoRight || !logo) {
    int shopnameHeight = shopName.split(QRegExp("\n|\r\n|\r"),QString::SkipEmptyParts).count() * fontMetr.height();
    painter.drawText(0, y, WIDTH, shopnameHeight + 4, Qt::AlignCenter, shopName);

    y += 5 + shopnameHeight;
  }
  //    painter.drawLine(0, y, WIDTH, y);

  if (! data.value("printHeader").toString().isEmpty()) {
    y += 5;
    QString printHeader = data.value("printHeader").toString();
    int headerTextHeight = printHeader.split(QRegExp("\n|\r\n|\r"),QString::SkipEmptyParts).count() * fontMetr.height();
    painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignCenter, printHeader);
    y += 5 + headerTextHeight + 4;
    painter.drawLine(0, y, WIDTH, y);
  }


  if (! data.value("headerText").toString().isEmpty()) {
    y += 5;
    QString headerText = data.value("headerText").toString();
    int headerTextHeight = headerText.split(QRegExp("\n|\r\n|\r"),QString::SkipEmptyParts).count() * fontMetr.height();
    painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignLeft, headerText);
    y += 5 + headerTextHeight + 4;
    painter.drawLine(0, y, WIDTH, y);
  }

  pb->setValue(20);

  painter.save();
  painter.setFont(boldFont);

  y += 5 + fontMetr.height();
  painter.drawText(0, y, WIDTH, boldMetr.height(), Qt::AlignCenter, data.value("comment").toString());

  painter.restore();

  QString copy = "";
  if (data.value("isCopy").toBool())
    copy = tr("( Kopie )");

  y += 5 + boldMetr.height();
  painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                   tr("Pos: %1 Kasse: %2  Bon-Nr: %3-%4 %5")
                   .arg(data.value("positions").toInt())
                   .arg(data.value("kasse").toInt())
                   .arg(data.value("currentRegisterYear").toInt())
                   .arg(data.value("receiptNum").toInt())
                   .arg(copy));

  pb->setValue(30);

  /*
    y += 5 + fontMetr.height();
    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                     tr("Re-Nr:: %1-%2-%3")
                     .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("yyyyMMdd"))
                     .arg(data.value("kasse").toInt() , 2, 10, QChar('0'))
                     .arg(data.value("receiptNum").toInt()));
*/

  y += 5 + fontMetr.height();
  painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft, tr("Datum: %1 Uhrzeit: %2")
                   .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("dd.MM.yyyy"))
                   .arg(QDateTime::fromString(data.value("receiptTime").toString(), Qt::ISODate).toString("hh:mm:ss")));

  pb->setValue(40);

  y += 5 + fontMetr.height();
  painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignLeft,
                   data.value("typeText").toString());

  y += 5 + fontMetr.height();
  painter.drawLine(0, y, WIDTH, y);

  pb->setValue(50);

  // paint orders

  y += 5;

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

    int count = order.value("count").toInt();
    QString grossText = QString("%1 %2").arg(QString::number(order.value("gross").toDouble(), 'f', 2)).arg(tr("€"));
    //      if ( count > 1 || count < -1)
    QString singleGrossText = QString("%1 x %2 %3").arg(QString::number(count)).arg(QString::number(order.value("singleprice").toDouble(), 'f', 2)).arg(tr("€"));
    int grossWidth = grossMetrics.boundingRect(grossText).width();

    //      QString product = fontMetr.elidedText(order.value("product").toString(), Qt::ElideMiddle, WIDTH - grossWidth - 5);
    QString product = order.value("product").toString();
    product = wordWrap(product, WIDTH - grossWidth - 5, font);
    int productHeight = product.split(QRegExp("\n|\r\n|\r"),QString::SkipEmptyParts).count() * fontMetr.height();

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

  // if there is not enough space for sum+tax lines, start new page
  if ( (y + (data.value("taxesCount").toInt() * (5 + fontMetr.height())) + boldMetr.height() + 10) > printer.pageRect().height() )
  {
    printer.newPage();
    y = 0;
  }

  pb->setValue(progress + 10);

  y += 5;
  painter.save();
  painter.setFont(boldFont);
  painter.drawText(0, y, WIDTH, boldMetr.height(), Qt::AlignRight,
                   tr("Gesamt: %1 €").arg(QString::number(data.value("sum").toDouble(), 'f', 2)));
  painter.restore();
  y += boldMetr.height() + 10;


  QJsonArray Taxes = data["Taxes"].toArray();
  foreach (const QJsonValue & item, Taxes)
  {
    const QJsonObject& tax = item.toObject();
    painter.drawText(0, y, WIDTH, fontMetr.height(), Qt::AlignRight,
                     tr("MwSt %1: %2 €")
                     .arg(tax.value("t1").toString())
                     .arg(QString::number(tax.value("t2").toString().toDouble(), 'f', 2)));

    y += 5 + fontMetr.height();
  }

  pb->setValue(progress + 10);

  if (! data.value("printFooter").toString().isEmpty()) {
    y += 10;
    QString printFooter = data.value("printFooter").toString();
    int headerTextHeight = printFooter.split(QRegExp("\n|\r\n|\r"),QString::SkipEmptyParts).count() * fontMetr.height();
    painter.drawText(0, y, WIDTH, headerTextHeight, Qt::AlignCenter, printFooter);
    y += 5 + headerTextHeight + 4;
  }


  painter.end();

  pb->setValue(progress + 10);

}

//--------------------------------------------------------------------------------

bool DocumentPrinter::initPrinter(QPrinter &printer)
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");

  qDebug() << "Print Resolution: " << printer.resolution();

  //  printer.setResolution(150);

  if ( noPrinter || printer.outputFormat() == QPrinter::PdfFormat)
    printer.setOutputFileName(QString("QRK-BON%1.pdf").arg( receiptNum ));
  else
    printer.setPrinterName(settings.value("receiptPrinter").toString());

  // defaults fit for 2"(width) x 4"(height) settings for the page of a TSC/TTP-245C printer
  printer.setPaperSize(QSizeF(settings.value("paperWidth", 80).toInt(),
                              settings.value("paperHeight", 210).toInt()), QPrinter::Millimeter);

  printer.setPageMargins(settings.value("marginLeft", 0).toInt(),
                         settings.value("marginTop", 17).toInt(),
                         settings.value("marginRight", 5).toInt(),
                         settings.value("marginBottom", 0).toInt(),
                         QPrinter::Millimeter);


  return true;
}

bool DocumentPrinter::initAlternatePrinter(QPrinter &printer)
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");

  printer.setPrinterName(settings.value("reportPrinter").toString());

  // defaults fit for 2"(width) x 4"(height) settings for the page of a TSC/TTP-245C printer

  /*
 *   QString f = settings.value("paperFormat").toString();
  if (f == "A4")
      printer.setPaperSize(printer.A4);
  if (f == "A5")
      printer.setPaperSize(printer.A5);
*/

  return true;
}

QString DocumentPrinter::wordWrap(QString text, int width, QFont font)
{
  QFontMetrics fm(font);
  QString result;

  for (;;) {
    int i = 0;
    while (i < text.length()) {
      if (fm.width(text.left(++i + 1)) > width) {
        int j = text.lastIndexOf(' ', i);
        if (j > 0)
          i = j;
        result += text.left(i);
        result += '\n';
        text = text.mid(i+1);
        break;
      }
    }
    if (i >= text.length())
      break;
  }
  return result + text;
}
