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

#ifndef IMPORT_H
#define IMPORT_H

#include <QObject>
#include <QFileInfoList>

class Import : public QObject
{
    Q_OBJECT
  public:
    explicit Import(QObject *parent = 0);
    void loadJSonFile(QFileInfoList *filename);

  signals:
    void updateProgressBar(int value);
    void importInfo(QString str);

  public slots:

  private:
    bool importR2B(QJsonObject data);
    void importReceipt(QJsonObject data);
    QFileInfoList list;

};

#endif // IMPORT_H
