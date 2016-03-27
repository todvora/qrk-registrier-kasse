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

#ifndef QRCODE_H
#define QRCODE_H

#include <qrencode.h>
#include <QObject>

class QRCode : public QObject
{
    Q_OBJECT

  public:
    QRCode(QObject *parent = 0);
    ~QRCode();
    QPixmap encodeTextToPixmap( QString text, int size = 4, int margin = 3, int ErrCLevel = 0 );

  private:
    QRcode *encode(const unsigned char *intext, int length, QRecLevel level = QR_ECLEVEL_L);
    QImage *image;

};

#endif // QRCODE_H
