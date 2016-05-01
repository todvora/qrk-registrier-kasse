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

#include "qsortfiltersqlquerymodel.h"

#include <QSqlDriver>
#include <QSqlField>
#include <QSqlRecord>

QSortFilterSqlQueryModel::QSortFilterSqlQueryModel(QObject *parent) :
  QSqlQueryModel(parent)
{

}

void QSortFilterSqlQueryModel::setQuery(const QString &query, const QSqlDatabase &db)
{
  queryClause = query;
  dbc = db;

  filterString.clear();
  //filterColumn.clear();
  filterFlags = Qt::MatchStartsWith;
  sortKeyColumn = -1;
  sortOrder = Qt::DescendingOrder;
}

void QSortFilterSqlQueryModel::select()
{

  if (queryClause.isEmpty() || (!dbc.isValid()))
    return;

  QString query = queryClause;

  if (!filterString.isEmpty() && !filterColumn.isEmpty()) {
    QString whereClause;
    QString esFilterString = filterString;
    QString esFilterColumn = filterColumn;

    if (filterFlags & Qt::MatchExactly) {
      whereClause = "WHERE %1 = %2";
    } else if (filterFlags & Qt::MatchStartsWith) {
      whereClause = "WHERE %1 LIKE %2";
      esFilterString.append("%");
    } else if (filterFlags & Qt::MatchEndsWith) {
      whereClause = "WHERE %1 LIKE %2";
      esFilterString.prepend("%");
    } else if (filterFlags & Qt::MatchContains) {
      whereClause = "WHERE %1 LIKE %2";
      esFilterString.append("%");
      esFilterString.prepend("%");
    } else { return; } // unhandled filterflag

    QSqlDriver *driver = dbc.driver();
    esFilterColumn = driver->escapeIdentifier(filterColumn, QSqlDriver::FieldName);
    QSqlField field; field.setType(QVariant::String); field.setValue(esFilterString);
    esFilterString = driver->formatValue(field);

    whereClause = whereClause.arg(esFilterColumn).arg(esFilterString);
    query.append(" " + whereClause);
  }


  if (sortKeyColumn >= 0) {
    QString orderClause;
    orderClause = "ORDER BY " + QString::number(sortKeyColumn+1) + " " + ((sortOrder == Qt::AscendingOrder) ? "ASC" : "DESC");
    query.append(" " + orderClause);
  }

  QSqlQueryModel::setQuery(query, dbc);

}

void QSortFilterSqlQueryModel::setSort(int column, Qt::SortOrder order)
{
  sortKeyColumn = column;
  sortOrder = order;
  QSortFilterSqlQueryModel::setFilterColumn(this->record().fieldName( column ));
  emit sortChanged();
}

void QSortFilterSqlQueryModel::sort(int column, Qt::SortOrder order)
{
  if ((sortKeyColumn != column) || (sortOrder != order)) {
    setSort(column, order);
    select();
  }
}

void QSortFilterSqlQueryModel::setFilterColumn(const QString &column)
{
  filterColumn = column;
}

void QSortFilterSqlQueryModel::setFilter(const QString &filter) {
  filterString = filter;
}

void QSortFilterSqlQueryModel::setFilterFlags(const Qt::MatchFlag flags)
{
  filterFlags = flags;
}

void QSortFilterSqlQueryModel::filter(const QString &filter)
{
  if (filterString != filter) {
    setFilter(filter);
    select();
  }
}

QString QSortFilterSqlQueryModel::getFilterColumnName()
{
  return this->headerData( sortKeyColumn, Qt::Horizontal ).toString();
}
