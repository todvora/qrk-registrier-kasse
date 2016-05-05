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

#include "backup.h"
#include "JlCompress.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDebug>

void Backup::create()
{
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data";
  create(dataDir);
}

void Backup::create(QString dataDir)
{

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  QString backupDir = settings.value("backupDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backup").toString();

  QString infile = QString("%1/%2-QRK.db").arg(dataDir).arg(QDate::currentDate().year());

  QString outfile = QString("%1/data_%2.zip").arg(backupDir).arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));

  qDebug() << "Backup::create infilefile: " << infile;
  qDebug() << "Backup::create outfilefile: " << outfile;

  bool ok = JlCompress::compressFile( outfile, infile );
  if (!ok)
    qDebug() << "JlCompress::compressFile:" << ok;

}

void Backup::pakLogFile()
{

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  QString backupDir = settings.value("backupDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString();

  QString infile = QString("%1/%2-QRK.db").arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/qrk.log");

  QString outfile = QString("%1/qrk_log_%2.zip").arg(backupDir).arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));

  qDebug() << "Backup::pakLogFile infilefile: " << infile;
  qDebug() << "Backup::pakLogFile outfilefile: " << outfile;

  bool ok = JlCompress::compressFile( outfile, infile );
  if (!ok)
    qDebug() << "JlCompress::compressFile:" << ok;

}

void Backup::cleanUp()
{
  removeDir("./imageformats");
  removeDir("./platforms");
  removeDir("./printsupport");
  removeDir("./sqldrivers");
  QFile::remove("./*.dll");

}

bool Backup::removeDir(const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}
