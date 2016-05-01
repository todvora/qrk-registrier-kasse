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

#ifndef AESUTIL_H
#define AESUTIL_H

#include <iostream>
#include <QVariant>

class QString;

class AESUtil
{
  public:
    static QString getPrivateKey();

    static QString encryptTurnoverCounter(QString concatenated, qlonglong turnoverCounter, QString symmetricKey);
    static QString decryptTurnoverCounter(QString concatenated, QString encryptedTurnoverCounter, QString symmetricKey);
    static QString sigReceipt(QString);
    static QString generateKey();

    static void test();

  private:
    static QString encryptCTR(std::string, qlonglong, std::string);
    static QString decryptCTR(std::string, QString, std::string);
    static QString HashValue(QString);

    static QByteArray base64_encode(QString string);
    static QByteArray base64Url_encode(QString string);
    static QByteArray base64_decode(QString string, bool hex = false);
    static QByteArray base32_encode(QString str);
    static QByteArray base32_decode(QString str);

    static QString toHex(QByteArray ba);

};

#endif // AESUTIL_H
