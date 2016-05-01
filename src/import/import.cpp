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

#include "import.h"
#include "qrkregister.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#include <QDebug>

Import::Import(QObject *parent) : QObject(parent)
{

}

void Import::loadJSonFile(QFileInfoList *filenames)
{

  Q_FOREACH(QFileInfo fileinfo, *filenames) {

    QString receiptInfo;
    qDebug() << "Import::loadJSonFile filenames in queue before -> " << filenames->size();
    filenames->removeAll(fileinfo);
    qDebug() << "Import::loadJSonFile filenames in queue after -> " << filenames->size();
    qDebug() << "Import::loadJSonFile " << fileinfo.absoluteFilePath();
    QFile file;
    file.setFileName(fileinfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Import::loadJSonFile continue " << fileinfo.absoluteFilePath();
        continue;
    }
    receiptInfo = file.readAll();
    QJsonDocument jd = QJsonDocument::fromJson(receiptInfo.toUtf8());
    QJsonObject data = jd.object();
    if (data.contains("r2b")) {
      data["filename"] = file.fileName();
      qDebug() << "Import::loadJSonFile type = r2b";
      if (importR2B(data)) {
          file.close();
          qDebug() << "Import::loadJSonFile Import " << data.value("filename").toString() << " OK";
          emit importInfo(QString("import %1 -> OK").arg(data.value("filename").toString()));
          QString filename = file.fileName().replace("json", "old");
          QFile::remove(filename);
          if (!file.rename(filename)) {
            qDebug() << "Import::loadJSonFile Can't rename " << data.value("filename").toString();
            emit importInfo(QString("Import Fehler -> Datei %1 kann nicht umbenannt werden").arg(data.value("filename").toString()));
          }

      } else {
          qDebug() << "Import::loadJSonFile rename to false";
          QString filename = file.fileName().replace("json", "false");
          file.close();
          QFile::remove(filename);
          file.rename(filename);

      }
    } else if (data.contains("receipt")) {
      qDebug() << "Import::loadJSonFile type = receipt";

    }

  }

}

bool Import::importR2B(QJsonObject data)
{
  QJsonArray r2bArray = data.value("r2b").toArray();
  bool ok = false;
  foreach (const QJsonValue & value, r2bArray) {
      QJsonObject obj = value.toObject();
      ok = obj.contains("gross") && obj.contains("receiptNum") && obj.contains("payedBy");
      if (ok) {
        QProgressBar *bar = new QProgressBar(0);
        QRKRegister *reg = new QRKRegister(bar);
        reg->init();
        reg->setServerMode(true);
        reg->newOrder();
        if (! reg->setR2BServerMode(obj)) {
            QString info = tr("Import Fehler -> Rechnungsnummer: %1 aus Importdatei %2 wird schon verwendet!").arg(obj.value("receiptNum").toString()).arg(data.value("filename").toString());
            emit importInfo(info);
            qDebug() << "Import::importR2B Receipt " << obj.value("receiptNum").toString() << "  already exists!";
            return false;
        }

        if (reg->checkEOAnyServerMode()) {
          qDebug() << "Import::importR2B checkEOAnyServerMode. OK";
          if (int receiptNum = reg->createReceipts()) {
            qDebug() << "Import::importR2B createReceipts. ReceiptNumber: " << receiptNum;
            reg->setCurrentReceiptNum(receiptNum);
            if (reg->createOrder())
              qDebug() << "Import::importR2B createOrder. OK";
              if (reg->finishReceipts(obj.value("payedBy").toString().toInt(0))) {
                  qDebug() << "Import::importR2B finishReceipts. OK";
                  return true;
              }
          }
        }

      } else {
          QString info = tr("Import Fehler -> Falsches JSON Format, Dateiname: %1").arg(data.value("filename").toString());
          emit importInfo(info);
          qDebug() << "Import::importR2B Invalid JSon Format.";
        return false;
      }
  }
  qDebug() << "Import::importR2B we shouldn't be here ...";
  return false;
}

void Import::importReceipt(QJsonObject data)
{
  QJsonArray r2bData = data.value("0").toArray();

}
