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

#include "dep.h"
#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QFile>
#include <QDebug>

DEP::DEP(QObject *parent)
  : QObject(parent)
{
  this->dbc = QSqlDatabase::database("CN");
}

void DEP::depInsertReceipt(QJsonObject &data)
{
  QSqlQuery query(dbc);

  // Id Programmversion Kassen-Id Beleg Belegtyp Bemerkung Nachbonierung
  // Belegnummer Datum Umsatz_Normal Umsatz_Ermaessigt1 Umsatz_Ermaessigt2
  // Umsatz_Null Umsatz_Besonders Jahresumsatz_bisher Erstellungsdatum

  QString var;
  QString val = "(version,cashregisterid,datetime,text)";

  QJsonArray a = data.value("Orders").toArray();

  foreach (const QJsonValue & value, a) {
    var.clear();
    QJsonObject o = value.toObject();
    var.append(QString("'Produktposition\t"));
    var.append(QString("%1\t").arg(o["product"].toString()));
    var.append(QString("%1\t").arg(QString::number(o["count"].toDouble(),'f', 2)));
    var.append(QString("%1\t").arg(QString::number(o["singleprice"].toDouble(),'f', 2)));
    var.append(QString("%1\t").arg(QString::number(o["gross"].toDouble(),'f', 2)));
    var.append(QString("%1\t").arg(o["tax"].toDouble()));
    var.append(QString("%1'").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));

    QString q = QString("INSERT INTO dep %1 VALUES(%2,%3,%4,%5)")
        .arg(val)
        .arg(QString("'%1'").arg(data.value("version").toString()))
        .arg(QString("'%1'").arg(data.value("kasse").toString()))
        .arg(QString("'%1'").arg(data.value("receiptTime").toString()))
        .arg(var);

    int ok = query.prepare(q);
    if (!ok)
      qDebug() << "Database::dep " << query.lastError().text();

    query.exec();

  }

  var.clear();
  var.append(QString("'%1\t").arg(data.value("actionText").toString()));
  var.append(QString("%1\t").arg(data.value("typeText").toString()));
  var.append(QString("%1\t").arg(data.value("comment").toString()));
  var.append(QString("%1\t").arg(data.value("totallyup").toString()));
  var.append(QString("%1\t").arg(data.value("receiptNum").toInt()));
  var.append(QString("%1\t").arg(data.value("receiptTime").toString()));
  var.append(QString("%1\t").arg(QString::number(data.value("Satz-Normal").toDouble(),'f',2)));
  var.append(QString("%1\t").arg(QString::number(data.value("Satz-Ermaessigt-1").toDouble(),'f',2)));
  var.append(QString("%1\t").arg(QString::number(data.value("Satz-Ermaessigt-2").toDouble(),'f',2)));
  var.append(QString("%1\t").arg(QString::number(data.value("Satz-Null").toDouble(),'f',2)));
  var.append(QString("%1\t").arg(QString::number(data.value("Satz-Besonders").toDouble(),'f',2)));
  var.append(QString("%1\t").arg(QString::number(data.value("sumYear").toDouble(),'f',2)));
  var.append(QString("%1'").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));

  QString q = QString("INSERT INTO dep %1 VALUES(%2,%3,%4,%5)")
      .arg(val)
      .arg(QString("'%1'").arg(data.value("version").toString()))
      .arg(QString("'%1'").arg(data.value("kasse").toString()))
      .arg(QString("'%1'").arg(data.value("receiptTime").toString()))
      .arg(var);

  int ok = query.prepare(q);
  if (!ok)
    qDebug() << "Database::dep " << query.lastError().text();

  query.exec();

}

void DEP::depInsertLine(QString title,  QString text)
{

  try {
    QDateTime dt = QDateTime::currentDateTime();

    QSqlQuery q(dbc);

    QString val = "(version,cashregisterid,datetime,text)";
    QString query = QString("INSERT INTO dep %1 VALUES('%2','%3','%4','%5')")
        .arg(val)
        .arg( QString("%1.%2").arg(QRK_VERSION_MAJOR).arg(QRK_VERSION_MINOR))
        .arg( Database::getCashRegisterId() )
        .arg(dt.toString(Qt::ISODate))
        .arg(title + "\t" + text + "\t" + QDateTime::currentDateTime().toString(Qt::ISODate));

    //    // qDebug() << query;

    bool ok = q.prepare(query);
    if (!ok)
      qDebug() << "DEP::depInsertLine Error: " << q.lastError().text();

    q.exec();

  } catch (QSqlError e) {
    qDebug() << "DEP::depInsertLine Error: " << e.text();
  }

}
