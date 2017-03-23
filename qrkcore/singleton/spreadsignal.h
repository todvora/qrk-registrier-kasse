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

#ifndef SPREADSIGNAL_H
#define SPREADSIGNAL_H

#include <QObject>
#include "Singleton.h"

#include "qrkcore_global.h"

class QRK_EXPORT SpreadSignal : public QObject
{
    Q_OBJECT
public:
    explicit SpreadSignal(QObject *parent = 0);

    static void setProgressBarValue(int value, bool add = false);
    static void setProgressBarWait(bool on_off);
    static void setImportInfo(QString message, bool isError = false);
    static void setSafetyDevice(bool active);

signals:
    void updateProgressBar(int value, bool add);
    void updateProgressBarWait(bool on_off);
    void updateImportInfo(QString message, bool isError = false);
    void updateSafetyDevice(bool active);

public slots:

};

//Global variable
typedef Singleton<SpreadSignal> Spread;

#endif // SPREADSIGNAL_H
