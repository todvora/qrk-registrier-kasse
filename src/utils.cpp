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

#include "utils.h"
#include "database.h"
#include "utils/aesutil.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QDebug>

Utils::Utils()
{

}

QString Utils::getSignature(QJsonObject data)
{

  QString taxlocation =  Database::getTaxLocation();
  QSqlDatabase dbc= QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  QJsonObject sign;

  sign["Kassen-ID"] = Database::getCashRegisterId();
  sign["Belegnummer"] = QString::number(data.value("receiptNum").toInt());
  sign["Beleg-Datum-Uhrzeit"] = data.value("receiptTime").toString();

  int ok = q.prepare(QString("SELECT tax FROM taxTypes WHERE taxlocation='%1' ORDER BY id").arg(taxlocation));
  if (!ok)
    qDebug() << "Utils select tax error: " << q.lastError().text();

  q.exec();
  double counter = 0.00;
  while(q.next()){
    double gross = data.value(Database::getTaxType(q.value(0).toDouble())).toDouble();
    double tax = q.value(0).toDouble();
    double net = gross / (1.0 + tax / 100.0);
    sign[Database::getTaxType( tax )] = QString::number( net , 'f', 2);
    counter += net;
  }

  QString concatenatedValue = sign["Kassen-ID"].toString() + sign["Belegnummer"].toString();

  qlonglong turnOverCounter = getTurnOverCounter();
  turnOverCounter += counter * 100;
  updateTurnOverCounter(turnOverCounter);
  QString base64encryptedTurnOverCounter = AESUtil::encryptTurnoverCounter(concatenatedValue, turnOverCounter, AESUtil::getPrivateKey());
  // qlonglong decryptedTurnoverCounter = AESUtil::decryptTurnoverCounter(concatenatedValue, encrypted, AESUtil::getPrivateKey());

  sign["Stand-Umsatz-Zaehler-AES256-ICM"] = base64encryptedTurnOverCounter;

  QString ls = getLastReceiptSignature();

  sign["Zertifikat-Seriennummer"] = "1";
  sign["Sig-Voriger-Beleg"] = ls;

  QJsonDocument doc(sign);
  QByteArray bytes = doc.toJson(doc.Compact);

//  qDebug() << "Utils::getSignature: " << QString::fromStdString( bytes.toStdString() );

  return QString::fromStdString( bytes.toStdString() );
}

/*
return "_" + rkSuite.getSuiteID() + "_" + cashBoxID + "_" + receiptIdentifier.
+ "_" + dateFormat.format(receiptDateAndTime) + "_" + decimalFormat.format(sumTaxSetNormal).
+ "_" + decimalFormat.format(sumTaxSetErmaessigt1) + "_" + decimalFormat.format(sumTaxSetErmaessigt2).
+ "_" + decimalFormat.format(sumTaxSetNull) + "_" + decimalFormat.format(sumTaxSetBesonders).
+ "_" + encryptedTurnoverValue + "_" + signatureCertificateSerialNumber + "_" + signatureValuePreviousReceipt;
*/


QString Utils::getLastReceiptSignature()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  int lastReceiptId = Database::getLastReceiptNum() - 1;
  int ok = q.prepare(QString("SELECT signature FROM receipts WHERE receiptNum=%1").arg(lastReceiptId));
  if (!ok)
    qDebug() << "Utils select signature error: " << q.lastError().text();

  q.exec();

  if (q.next())
  {

    // RK Suite defined in Detailspezifikation/ABS 2
    // R1_AT0("1", "AT0", "ES256", "SHA-256", 8);
    QString s = q.value(0).toString();
    QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());

    if(doc.isObject())
    {
      QJsonObject sig = doc.object();
      QString sign = "_R1_AT0_";
      sign.append( sig.value("Kassen-ID").toString() );
      sign.append( "_" );
      sign.append( sig.value("Belegnummer").toString() );
      sign.append( "_" );
      sign.append( sig.value("Beleg-Datum-Uhrzeit").toString() );
      sign.append( "_" );
      sign.append( sig.value("Normal").toString() );
      sign.append( "_" );
      sign.append( sig.value("Ermaessigt1").toString() );
      sign.append( "_" );
      sign.append( sig.value("Ermaessigt2").toString() );
      sign.append( "_" );
      sign.append( sig.value("Null").toString() );
      sign.append( "_" );
      sign.append( sig.value("Besonders").toString() );
      sign.append( "_" );
      sign.append( sig.value("Stand-Umsatz-Zaehler-AES256-ICM").toString() );
      sign.append( "_" );
      sign.append( sig.value("Zertifikat-Seriennummer").toString() );
      sign.append( "_" );
      sign.append( sig.value("Sig-Voriger-Beleg").toString() );

      return AESUtil::sigLastReceipt(sign);

    }

  }

  return AESUtil::sigLastReceipt(Database::getCashRegisterId());

}

qlonglong Utils::getTurnOverCounter()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  q.prepare(QString("SELECT value FROM globals WHERE name='turnovercounter'"));
  q.exec();

  if (q.next())
    return q.value(0).toLongLong();

  q.prepare(QString("SELECT sum(net) FROM receipts WHERE payedBy < 3"));
  q.exec();
  q.next();

  qlonglong net = q.value(0).toDouble() * 100 ;
  return net;

}

void Utils::updateTurnOverCounter(qlonglong toc)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  q.prepare("SELECT value FROM globals WHERE name='turnovercounter'");
  q.exec();

  if (q.next()) {
    q.prepare(QString("UPDATE globals set value=%1 WHERE name='turnovercounter'").arg(toc));
    q.exec();
  } else {
    q.prepare(QString("INSERT INTO globals (name, value) VALUES('turnovercounter', %1)").arg(toc));
    q.exec();
  }
}

double Utils::getYearlyTotal(int year)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  QDateTime from;
  QDateTime to;

  QString fromString = QString("%1-01-01").arg(year);
  QString toString = QString("%1-12-31").arg(year);

  from.setDate(QDate::fromString(fromString, "yyyy-MM-dd"));
  to.setDate(QDate::fromString(toString, "yyyy-MM-dd"));
  to.setTime(QTime::fromString("23:59:59"));

  /* Summe */
  q.prepare(QString("SELECT sum(gross) FROM receipts WHERE timestamp BETWEEN '%1' AND '%2' AND payedBy < 3")
            .arg(from.toString(Qt::ISODate))
            .arg(to.toString(Qt::ISODate)));

  q.exec();
  q.next();

  double sales = q.value(0).toDouble() ;

  return sales;

}
