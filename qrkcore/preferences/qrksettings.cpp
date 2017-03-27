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

#include "qrksettings.h"
#include "database.h"
#include "journal.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSettings>
#include <QDebug>

QrkSettings::QrkSettings(QObject *parent) : QSettings(parent)
{
    m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK", this);
    m_journal = new Journal(this);
}

QrkSettings::~QrkSettings()
{
}

void QrkSettings::beginGroup(const QString &prefix)
{
    m_settings->beginGroup(prefix);
}

void QrkSettings::endGroup()
{
    m_settings->endGroup();
}

void QrkSettings::save2Database(QString name, QString value)
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    QString oldValue = "";
    query.prepare("SELECT id, strValue FROM globals WHERE name=:name;");
    query.bindValue(":name", name);
    bool ok = query.exec();

    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " " << Database::getLastExecutedQuery(query);
    }

    int id = -1;
    if (query.next()) {
        id = query.value("id").toInt();
        oldValue = query.value("strValue").toString();
    }

    if (oldValue.isEmpty() && oldValue != value) {
        if (id > 0)
            ok = query.prepare("UPDATE globals set strValue=:value WHERE name=:name;");
        else
            ok = query.prepare("INSERT INTO globals (name, strValue) VALUES(:name, :value);");

        if (!ok) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " " << query.lastError().text();
            qWarning() << "Function Name: " << Q_FUNC_INFO << " " << Database::getLastExecutedQuery(query);
        }

        query.bindValue(":value", value);
        query.bindValue(":name", name);
        ok = query.exec();

        qDebug() << "Function Name: " << Q_FUNC_INFO << " " << Database::getLastExecutedQuery(query);

        if (!ok) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " " << query.lastError().text();
            qWarning() << "Function Name: " << Q_FUNC_INFO << " " << Database::getLastExecutedQuery(query);
        }

        QString text = QString("Parameter '%1' aus der Datenbanktabelle 'globals' wurde von '%2' auf '%3' geändert").arg(name).arg(oldValue).arg(value);
        m_journal->journalInsertLine("Konfigurationsänderung", text);
    }
}

void QrkSettings::save2Settings(QString name, QVariant value, bool journaling)
{
    QVariant oldValue = m_settings->value(name);
    if (oldValue != value) {
        m_settings->setValue(name, value);
        QString text = QString("Parameter '%1' aus der Konfigurationsdatei wurde von '%2' auf '%3' geändert").arg(name).arg(oldValue.toString()).arg(value.toString());
        if (journaling)
            m_journal->journalInsertLine("Konfigurationsänderung", text);
    }
}

void QrkSettings::removeSettings(QString name, bool journaling)
{
    QVariant oldValue = m_settings->value(name);
    if (oldValue.isValid()) {
        QString text = QString("Parameter '%1' aus der Konfigurationsdatei wurde von '%2' auf 'standard' geändert").arg(name).arg(oldValue.toString());
        if (journaling)
            m_journal->journalInsertLine("Konfigurationsänderung", text);
    }

    QString f = m_settings->fileName();

    m_settings->remove(name);
}

QVariant QrkSettings::value(QString key, QVariant defaultValue)
{
    return m_settings->value(key,defaultValue);
}
