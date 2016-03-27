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

#include "qrcode.h"

#include <QPixmap>
#include <QPainter>
#include <QDebug>

QRCode::QRCode(QObject *parent)
  : QObject(parent)
{
  image = NULL;
}

QRCode::~QRCode()
{
  if (image != NULL) delete image;
}

QRcode *QRCode::encode(const unsigned char *intext, int length, QRecLevel level)
{
  QRcode *code;

  int version = 0;
  code = QRcode_encodeData(length, intext, version, level);

  return code;
}


QPixmap QRCode::encodeTextToPixmap( QString text, int size, int margin, int ErrCLevel )
{
  QByteArray a = text.toUtf8();

  QRecLevel level;
  switch(ErrCLevel) {
    case 0:
      level = QR_ECLEVEL_L;
      break;
    case 1:
      level = QR_ECLEVEL_M;
      break;
    case 2:
      level = QR_ECLEVEL_Q;
      break;
    case 3:
      level = QR_ECLEVEL_H;
      break;
    default:
      break;
  }

  QRcode *qrcode = encode( (unsigned char*)a.constData(), a.length(), level); // Generate QRCode from string.
  if ( qrcode == NULL ) {
    return QPixmap();
  }
  if (qrcode->width < 21 || qrcode->width > 177) { // qrcode width range is min is "ver-1 = 21 cell", max is ver-40 = 177 cell
    return QPixmap();
  }

  int realwidth = (qrcode->width + margin * 2) * size;

  if (image != NULL) {
    delete(image);
  }

  image = new QImage( realwidth, realwidth, QImage::Format_Mono );
  QPainter painter(image);

  painter.fillRect( QRect( 0, 0, realwidth, realwidth ), QColor( Qt::white ) );

  QColor black( Qt::black );

  int i = 0;
  int x = 0;
  int y = 0;
  QRect rect( 0, 0, size, size );
  while ( y < qrcode->width )
  {
    x = 0;
    while ( x < qrcode->width )
    {
      // only if a block is present (black)
      if ( qrcode->data[i] & 1 )
      {
        rect.moveTo(size * (margin + x),
                    size * (margin + y));
        painter.fillRect( rect, black );
      }
      i++;
      x++;
    }
    y++;
  }

  painter.end();

  QPixmap pixmap = QPixmap::fromImage(*image);

  QRcode_free(qrcode);

  return pixmap;
}
