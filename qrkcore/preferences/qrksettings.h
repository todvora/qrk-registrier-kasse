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

#ifndef QRKSETTINGS_H
#define QRKSETTINGS_H

#include <QObject>
#include <QVariant>
#include <QSettings>

#include "qrkcore_global.h"

class Journal;

class QRK_EXPORT QrkSettings : public QSettings
{
    Q_OBJECT
public:
    QrkSettings(QObject *parent = 0);
    ~QrkSettings();

    void save2Database(QString name, QString value);
    void save2Settings(QString name, QVariant value, bool journaling = true);
    void removeSettings(QString name, bool journaling = true);

    QVariant value(QString key, QVariant defaultValue = QVariant());

    void beginGroup(const QString &prefix);
    void endGroup();

    private:
    QSettings *m_settings;
    Journal *m_journal;
};

#endif // QRKSETTINGS_H
