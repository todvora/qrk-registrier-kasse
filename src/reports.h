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
 */

#ifndef REPORTS_H
#define REPORTS_H

#include "utils.h"
#include "dep.h"
#include "database.h"
#include "qrkregister.h"

class Reports : public QObject
{
    Q_OBJECT
  public:
    Reports(DEP *dep, QProgressBar *pb, QObject *parent = 0);
    ~Reports();

    static bool canCreateEOD(QDate);
    static bool canCreateEOM(QDate);
    static int getReportId();
    static QDate getLastEOD();

    void createEOD(int, QDate);
    void insert(QStringList, int);
    void createEOM(int, QDate);

    bool endOfMonth();
    bool endOfDay();

    static QString getReport(int);

  signals:

  public slots:

  private:
    QStringList createStat(int, QString, QDateTime, QDateTime);
    QStringList createYearStat(int, QDate);

    DEP *dep;
    QProgressBar *pb;
    QString yearsales;
    int currentReceipt;

};

#endif // REPORTS_H
