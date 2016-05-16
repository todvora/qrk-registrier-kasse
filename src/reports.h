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

#ifndef REPORTS_H
#define REPORTS_H

#include "utils/utils.h"
#include "dep.h"
#include "database.h"
#include "qrkregister.h"

class Reports : public QObject
{
    Q_OBJECT
  public:
    Reports(DEP *dep, QProgressBar *pb, QObject *parent = 0, bool mode = false);
    ~Reports();

    static bool canCreateEOD(QDate);
    static bool canCreateEOM(QDate);
    static int getReportType();
    static QDate getLastEOD();
    int getCurrentId();

    void createEOD(int, QDate);
    void insert(QStringList, int);
    void createEOM(int, QDate);

    bool endOfMonth();
    bool endOfDay();
    void fixMonth(int);


    static QString getReport(int id, bool test = false);

  signals:

  public slots:

  private:
    QStringList createStat(int, QString, QDateTime, QDateTime);
    QStringList createYearStat(int, QDate);
    bool doEndOfDay(QDate date);

    DEP *dep;
    QProgressBar *pb;
    QString yearsales;
    int currentReceipt;
    bool servermode;

};

#endif // REPORTS_H
