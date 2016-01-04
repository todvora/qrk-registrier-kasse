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

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>

Utils::Utils()
{

}

QString Utils::getSignature(QDateTime datetime, double sum, double net, int num)
{
  QString signature = QString("D(%1)G(%2)N(%3)I(%5)")
      .arg(datetime.toString(Qt::ISODate))
      .arg(QString("%1").arg(QString::number(sum, 'f', 2)))
      .arg(QString("%1").arg(QString::number(net, 'f', 2)))
      .arg(num);

  return signature;
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
