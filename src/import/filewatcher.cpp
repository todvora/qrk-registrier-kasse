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

#include "filewatcher.h"
#include "import.h"

#include <QMessageBox>
#include <QDir>
#include <QTimer>
#include <QDebug>

FileWatcher::FileWatcher (QWidget* parent)
    : QWidget(parent)
{
    this->timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(fileAdded()));

}

FileWatcher::~FileWatcher ()
{
}

void FileWatcher::directoryChanged(const QString &str)
{
    timer->start(200);
    // Q_UNUSED(str)
    // QMessageBox::information(this,"Directory Modified", "Your Directory is modified: " + str);
    appendQueue(str);
}

void FileWatcher::appendQueue(QString path)
{
    qDebug() << "FileWatcher::appendQueue EVENT: " << path;
    QDir directory;

    directory.setPath(path);
    QStringList filter;
    filter.append("*.json");
    list = directory.entryInfoList(filter, QDir::Files, QDir::Time | QDir::Reversed);

}

void FileWatcher::fileAdded()
{
    Q_FOREACH(QFileInfo fileinfo, list)
    qDebug() << "FileWatcher::fileAdded: " << fileinfo.absoluteFilePath();

    // emit addToQueue(list);
    import = new Import(this);

    connect(import,SIGNAL(importInfo(QString)),this,SIGNAL(importInfo(QString)));

    import->loadJSonFile(&list);
    list.clear();
}
