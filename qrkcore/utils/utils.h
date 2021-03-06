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

#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QVariant>

#include "qrkcore_global.h"

class RKSignatureModule;

class QRK_EXPORT Utils
{
  public:
    Utils();
    ~Utils();

    QString getSignature(QJsonObject data);

    static bool checkTurnOverCounter();
    static double getYearlyTotal(int year);
    static qlonglong getTurnOverCounter();
    static void resetTurnOverCounter();
    static void updateTurnOverCounter(qlonglong);
    static bool isDirectoryWritable(QString path);
    static QString getReceiptSignature(int id, bool full = false);
    static QString getLastReceiptSignature();
    static QString getReceiptShortJson(QJsonObject obj);
    static QString wordWrap(QString text, int width, QFont font);
    static QString normalizeNumber(QString value);
    static QString color_best_contrast(QString color);
    static QString taxRoundUp(double value,unsigned short np);
    static double getTax(double value, double tax, bool net = false);
    static double getNet(double gross, double tax);
    static double getGross(double net, double tax);
    static QPixmap getQRCode(int id, bool &isDamaged);

};

#endif // UTILS_H
