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

#ifndef IMPORT_H
#define IMPORT_H

#include "reports.h"

class ImportWorker : public Reports
{
    Q_OBJECT

public:
    ImportWorker(QQueue<QString> &queue, QWidget *parent = 0);
    ~ImportWorker();

signals:
    void finished();

public slots:
    void process();
    void stopProcess();

private:
    bool loadJSonFile(QString filename);
    bool importR2B(QJsonObject data);
    bool importReceipt(QJsonObject data);
    bool fileMover(QString filename, QString ext);

    QQueue<QString> *m_queue;
    bool m_isStopped;

};

#endif // IMPORT_H
