/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015 Christian Kvasny <chris@ckvsoft.at>
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
