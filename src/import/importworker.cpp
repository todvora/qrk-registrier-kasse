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

#include "importworker.h"
#include "singleton/spreadsignal.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include <QQueue>
#include <QThread>
#include <QWidget>
#include <QDebug>

ImportWorker::ImportWorker(QQueue<QString> &queue, QWidget *parent)
    :Reports(parent, true)
{
    m_queue = &queue;
    m_isStopped = false;
}

ImportWorker::~ImportWorker()
{
    qDebug() << "Destructor from Worker thread: "<<QThread::currentThreadId();
}

void ImportWorker::stopProcess()
{
    m_isStopped = true;
}

void ImportWorker::process()
{
    while (!m_isStopped && !m_queue->isEmpty()) {
        qDebug() << "From Worker thread: "<<QThread::currentThreadId();

        if (checkEOAnyServerMode()) {
            QString filename = m_queue->first();
            loadJSonFile(filename);
            QThread::msleep(200);
            m_queue->dequeue();
        } else {
            int qsize = m_queue->size();
            while (!m_isStopped && !m_queue->isEmpty())
                fileMover(m_queue->dequeue(), ".false");

            QString info = tr("Import Fehler -> Tages/Monatsabschluß wurde schon erstellt. Es wurden %1 Dateien umbenannt.").arg(qsize);
            SpreadSignal::setImportInfo(info, true);
            break;
        }
    }
    emit finished();
}

bool ImportWorker::loadJSonFile(QString filename)
{

    QString receiptInfo;
    QFile file(filename);

    // in some cases we are to fast here. Try 3 times to open the file while file is in use by create process
    for (int i = 0; i < 3; i++) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            break;
        }
        if (i == 3) {
            SpreadSignal::setImportInfo(tr("Import Fehler -> Datei %1 kann nicht geöffnet werden.").arg(filename), true);
            return false;
        }
        QThread::msleep(300);
    }

    receiptInfo = file.readAll();
    file.close();

    // remove any prefix and suffix from dirty json files
    if (!receiptInfo.startsWith('{')) {
        int begin = receiptInfo.indexOf('{');
        int end = receiptInfo.lastIndexOf('}') - begin + 1;
        receiptInfo = receiptInfo.mid(begin,end);
    }

    QJsonDocument jd = QJsonDocument::fromJson(receiptInfo.toUtf8());
    QJsonObject data = jd.object();

    if (data.contains("r2b")) {
        data["filename"] = file.fileName();
        if (importR2B(data)) {
            SpreadSignal::setImportInfo(tr("Import %1 -> OK").arg(data.value("filename").toString()));
            if (!fileMover(filename, ".old"))
                SpreadSignal::setImportInfo(tr("Import Fehler -> Datei %1 kann nicht umbenannt werden").arg(data.value("filename").toString()), true);

            return true;

        } else {
            SpreadSignal::setImportInfo(tr("Import Fehler -> Falsches Dateiformat (%1).").arg(filename), true);
            fileMover(filename, ".false");
            return false;
        }

    } else if (data.contains("receipt")) {
        data["filename"] = file.fileName();
        if (importReceipt(data)) {
            SpreadSignal::setImportInfo(tr("Import %1 -> OK").arg(data.value("filename").toString()));
            if (!fileMover(filename, ".old"))
                SpreadSignal::setImportInfo(tr("Import Fehler -> Datei %1 kann nicht umbenannt werden.").arg(data.value("filename").toString()), true);

            return true;

        } else {
            if (!fileMover(filename, ".old"))
                SpreadSignal::setImportInfo(tr("Import Fehler -> Datei %1 kann nicht umbenannt werden.").arg(data.value("filename").toString()), true);
        }

    } else {
        SpreadSignal::setImportInfo(tr("Import Fehler -> Falsches Dateiformat (%1).").arg(filename), true);
        fileMover(filename, ".false");
    }

    return false;
}

bool ImportWorker::importR2B(QJsonObject data)
{
    QJsonArray r2bArray = data.value("r2b").toArray();
    bool ok = false;
    foreach (const QJsonValue & value, r2bArray) {
        QJsonObject obj = value.toObject();
        ok = obj.contains("gross") && obj.contains("receiptNum") && obj.contains("payedBy");
        if (ok) {

            newOrder();
            if (! setR2BServerMode(obj)) {
                QString info = tr("Import Fehler -> Rechnungsnummer: %1 aus Importdatei %2 wird schon verwendet!").arg(obj.value("receiptNum").toString()).arg(data.value("filename").toString());
                SpreadSignal::setImportInfo(info, true);
                return false;
            }

            if (int id = createReceipts()) {
                setCurrentReceiptNum(id);
                if (createOrder()) {
                    if (finishReceipts(obj.value("payedBy").toString().toInt(0))) {
                    }
                }
            }
        } else {
            QString info = tr("Import Fehler -> Falsches JSON Format, Dateiname: %1").arg(data.value("filename").toString());
            SpreadSignal::setImportInfo(info, true);
            return false;
        }
    }
    return true;
}

bool ImportWorker::importReceipt(QJsonObject data)
{
    QJsonArray receiptData = data.value("receipt").toArray();
    bool ok = false;
    foreach (const QJsonValue & value, receiptData) {
        QJsonObject obj = value.toObject();
        ok = obj.contains("payedBy") && obj.contains("items");
        if (ok) {

            newOrder();
            if (! setReceiptServerMode(obj)) {
                QString info = tr("Import Fehler -> Importdatei %1!").arg(data.value("filename").toString());
                SpreadSignal::setImportInfo(info, true);
                return false;
            }
            if (int id = createReceipts()) {
                setCurrentReceiptNum(id);
                if (createOrder()) {
                    if (finishReceipts(obj.value("payedBy").toString().toInt(0))) {
                        return true;
                    }
                }
            }
        } else {
            QString info = tr("Import Fehler -> Falsches JSON Format, Dateiname: %1").arg(data.value("filename").toString());
            SpreadSignal::setImportInfo(info, true);
            return false;
        }

    }
    return true;
}

bool ImportWorker::fileMover(QString filename, QString ext)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO << " filename: " << filename << " ext: " << ext;
    QString originalfilename = filename;
    QFileInfo fi(filename.replace(".json", ext));
    QDir directory(fi.absoluteDir());
    QStringList filter;
    filter.append(fi.baseName() + ext + "*");
    QFileInfoList fIL;
    fIL = directory.entryInfoList(filter, QDir::Files, QDir::Name);
    if (fIL.size()>0) {
        QFileInfo fi = fIL.last();
        QString e = fi.fileName().section(".", -1,-1);
        if (e == ext.section(".", -1,-1)) {
            ext = ".001";
        } else {
            int i = e.toInt() +1;
            ext = "." + QString("%1").arg(i, 3, 'g', -1, '0');;
        }
        QFile f(originalfilename);
        return f.rename(filename + ext);
    }
    QFile f(originalfilename);

    return f.rename(filename);
}
