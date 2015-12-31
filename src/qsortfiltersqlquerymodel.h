/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015 Christian Kvasny <chris@ckvsoft.at>
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

#ifndef QSORTFILTERSQLQUERYMODEL_H
#define QSORTFILTERSQLQUERYMODEL_H

#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QSqlField>
#include <QString>


class QSortFilterSqlQueryModel : public QSqlQueryModel
{
    Q_OBJECT

  public:

    explicit QSortFilterSqlQueryModel(QObject *parent = 0);

    //void setQuery(const QSqlQuery &query);
    void setQuery(const QString & query, const QSqlDatabase & dbc = QSqlDatabase::database() );

  public slots:

    void setFilterColumn (const QString & column);
    void setFilterFlags (const Qt::MatchFlag flags);
    void setFilter( const QString & filter );
    void filter( const QString & filter );

    void select();

    virtual void setSort (int column, Qt::SortOrder order);
    virtual void sort (int column, Qt::SortOrder order);

  private:

    QSqlDatabase dbc;
    QString queryClause;
    Qt::MatchFlag filterFlags;
    QString filterString;
    QString filterColumn;
    int sortKeyColumn;
    Qt::SortOrder sortOrder;

  signals:

};

#endif // QSORTFILTERSQLQUERYMODEL_H
