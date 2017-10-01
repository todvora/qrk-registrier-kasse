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

#include "filewatcher.h"
#include "importworker.h"
#include "singleton/spreadsignal.h"

#include <QDir>
#include <QCollator>
#include <QQueue>
#include <QThread>
#include <QDebug>

FileWatcher::FileWatcher (QWidget* parent)
    : QWidget(parent)
{
    m_isBlocked = false;
    m_queue = new QQueue<QString>;
}

FileWatcher::~FileWatcher ()
{

}

void FileWatcher::directoryChanged(const QString &path)
{

    if(m_isBlocked)
        return;

    m_isBlocked = true;

    if (!m_watchingPathList.contains(path))
        m_watchingPathList.append(path);

    QDir directory;
    directory.setFilter(QDir::Files | QDir::NoSymLinks);
    directory.setSorting(QDir::NoSort);  // will sort manually with std::sort

    directory.setPath(path);
    QStringList filter;
    filter.append("*.json");
    QStringList fileList;
    fileList.clear();
    fileList = directory.entryList(filter);

    qSort(
        fileList.begin(),
        fileList.end(), compareNames);


    foreach (const QString str, fileList) {
        QFile f(path + "/" + str);
        if (!m_queue->contains(path + "/" + str) && f.exists()) {

            m_queue->append(path + "/" + str);
        }

        if (m_queue->size() > 50)
            break;
    }

    if (m_isBlocked && !m_queue->isEmpty())
        start();
    else
        m_isBlocked = false;
}

void FileWatcher::removeDirectories()
{
    m_watchingPathList.clear();
    if (m_queue->isEmpty())
        emit workerStopped();
}

void FileWatcher::finished()
{

    m_isBlocked = false;
    emit workerStopped();
    foreach (const QString str, m_watchingPathList) {
        emit directoryChanged(str);
    }
}

bool FileWatcher::compareNames(const QString& s1,const QString& s2)
{
    {
        // ignore common prefix..
        int i = 0;
        while ((i < s1.length()) && (i < s2.length()) && (s1.at(i).toLower() == s2.at(i).toLower()))
            ++i;
        ++i;
        // something left to compare?
        if ((i < s1.length()) && (i < s2.length()))
        {
            // get number prefix from position i - doesnt matter from which string
            int k = i-1;
            //If not number return native comparator
            if(!s1.at(k).isNumber() || !s2.at(k).isNumber())
            {
                return QString::compare(s1, s2, Qt::CaseSensitive) < 0;
            }
            QString n = "";
            k--;
            while ((k >= 0) && (s1.at(k).isNumber()))
            {
                n = s1.at(k)+n;
                --k;
            }
            // get relevant/signficant number string for s1
            k = i-1;
            QString n1 = "";
            while ((k < s1.length()) && (s1.at(k).isNumber()))
            {
                n1 += s1.at(k);
                ++k;
            }

            // get relevant/signficant number string for s2
            //Decrease by
            k = i-1;
            QString n2 = "";
            while ((k < s2.length()) && (s2.at(k).isNumber()))
            {
                n2 += s2.at(k);
                ++k;
            }

            // got two numbers to compare?
            if (!n1.isEmpty() && !n2.isEmpty())
            {
                return (n+n1).toInt() < (n+n2).toInt();
            }
            else
            {
                // not a number has to win over a number.. number could have ended earlier... same prefix..
                if (!n1.isEmpty())
                    return false;
                if (!n2.isEmpty())
                    return true;
                return s1.at(i) < s2.at(i);
            }
        }
        else {
            // shortest string wins
            return s1.length() < s2.length();
        }
    }
}

void FileWatcher::start()
{
    qDebug()<<"From main thread: "<<QThread::currentThreadId();

    QThread *thread = new QThread;
    thread->sleep(1); // Sometimes the File was not finished
    m_worker = new ImportWorker(*m_queue);
    m_worker->clear();
    m_worker->moveToThread(thread);

    connect(thread, SIGNAL(started()), m_worker, SLOT(process()));
    connect(this, SIGNAL(stopWorker()), m_worker, SLOT(stopProcess()), Qt::DirectConnection);
    connect(m_worker, SIGNAL(finished()), this, SLOT(finished()));
    connect(m_worker, SIGNAL (finished()), thread, SLOT (quit()));
    connect(m_worker, SIGNAL (finished()), m_worker, SLOT (deleteLater()));
    connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));

    thread->start();
//    worker->process();
}
