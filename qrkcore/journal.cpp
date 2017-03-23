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

#include "journal.h"
#include "defines.h"
#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QFile>
#include <QDebug>

Journal::Journal(QObject *parent)
  : QObject(parent)
{
  m_dbc = QSqlDatabase::database("CN");
}

void Journal::journalInsertReceipt(QJsonObject &data)
{
  QSqlQuery query(m_dbc);

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

    bool ok = query.prepare(QString("INSERT INTO journal %1 VALUES(:version,:kasse,:receiptTime,:var)")
        .arg(val));

    query.bindValue(":version", data.value("version").toString());
    query.bindValue(":kasse", data.value("kasse").toString());
    query.bindValue(":receiptTime", data.value("receiptTime").toString());
    query.bindValue(":var", var);

    if (!ok) {
      qCritical() << "Function Name: " << Q_FUNC_INFO << " " << query.lastError().text();
      qCritical() << "Function Name: " << Q_FUNC_INFO << " " << Database::getLastExecutedQuery(query);
    }

    query.exec();

  }

  var.clear();
  var.append(QString("%1\t").arg(data.value("actionText").toString()));
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
  var.append(QString("%1").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));

  bool ok = query.prepare(QString("INSERT INTO journal %1 VALUES(:version,:kasse,:receiptTime,:var)")
      .arg(val));
  query.bindValue(":version", data.value("version").toString());
  query.bindValue(":kasse", data.value("kasse").toString());
  query.bindValue(":receiptTime", data.value("receiptTime").toString());
  query.bindValue(":var", var);

  ok = query.exec();
  if (!ok) {
    qCritical() << "Function Name: " << Q_FUNC_INFO << " " << query.lastError().text();
    qCritical() << "Function Name: " << Q_FUNC_INFO << " " << Database::getLastExecutedQuery(query);
  }
}

void Journal::journalInsertLine(QString title,  QString text)
{

  QDateTime dt = QDateTime::currentDateTime();
  QSqlQuery query(m_dbc);
  QString val = "(version,cashregisterid,datetime,text)";
  bool ok = query.prepare(QString("INSERT INTO journal %1 VALUES(:version, :kasse, :date, :text)")
                      .arg(val));
  query.bindValue(":version", QString("%1.%2").arg(QRK_VERSION_MAJOR).arg(QRK_VERSION_MINOR));
  query.bindValue(":kasse", Database::getCashRegisterId());
  query.bindValue(":date", dt.toString(Qt::ISODate));
  query.bindValue(":text", title + "\t" + text + "\t" + dt.toString(Qt::ISODate));

  if (!ok) {
    qCritical() << "Function Name: " << Q_FUNC_INFO << " " << query.lastError().text();
    qCritical() << "Function Name: " << Q_FUNC_INFO << " " << Database::getLastExecutedQuery(query);
  }

  query.exec();

}
