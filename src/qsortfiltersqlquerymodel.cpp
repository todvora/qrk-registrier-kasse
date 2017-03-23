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
  m_queryClause = query;
  m_dbc = db;

  m_filterString.clear();
  //filterColumn.clear();
  m_filterFlags = Qt::MatchStartsWith;
  m_sortKeyColumn = -1;
  m_sortOrder = Qt::DescendingOrder;
}

void QSortFilterSqlQueryModel::select()
{

  if (m_queryClause.isEmpty() || (!m_dbc.isValid()))
    return;

  QString query = m_queryClause;

  if (!m_filterString.isEmpty() && !m_filterColumn.isEmpty()) {
    QString whereClause;
    QString esFilterString = m_filterString;
    QString esFilterColumn = m_filterColumn;

    if (m_filterFlags & Qt::MatchExactly) {
      whereClause = "WHERE %1 = %2";
    } else if (m_filterFlags & Qt::MatchStartsWith) {
      whereClause = "WHERE %1 LIKE %2";
      esFilterString.append("%");
    } else if (m_filterFlags & Qt::MatchEndsWith) {
      whereClause = "WHERE %1 LIKE %2";
      esFilterString.prepend("%");
    } else if (m_filterFlags & Qt::MatchContains) {
      whereClause = "WHERE %1 LIKE %2";
      esFilterString.append("%");
      esFilterString.prepend("%");
    } else { return; } // unhandled filterflag

    QSqlDriver *driver = m_dbc.driver();
    esFilterColumn = driver->escapeIdentifier(m_filterColumn, QSqlDriver::FieldName);
    QSqlField field; field.setType(QVariant::String); field.setValue(esFilterString);
    esFilterString = driver->formatValue(field);

    whereClause = whereClause.arg(esFilterColumn).arg(esFilterString);
    query.append(" " + whereClause);
  }


  if (m_sortKeyColumn >= 0) {
    QString orderClause;
    orderClause = "ORDER BY " + QString::number(m_sortKeyColumn+1) + " " + ((m_sortOrder == Qt::AscendingOrder) ? "ASC" : "DESC");
    query.append(" " + orderClause);
  }

  QSqlQueryModel::setQuery(query, m_dbc);

}

void QSortFilterSqlQueryModel::setSort(int column, Qt::SortOrder order)
{
  m_sortKeyColumn = column;
  m_sortOrder = order;
  QSortFilterSqlQueryModel::setFilterColumn(this->record().fieldName( column ));
  emit sortChanged();
}

void QSortFilterSqlQueryModel::sort(int column, Qt::SortOrder order)
{
  if ((m_sortKeyColumn != column) || (m_sortOrder != order)) {
    setSort(column, order);
    select();
  }
}

void QSortFilterSqlQueryModel::setFilterColumn(const QString &column)
{
  m_filterColumn = column;
}

void QSortFilterSqlQueryModel::setFilter(const QString &filter) {
  m_filterString = filter;
}

void QSortFilterSqlQueryModel::setFilterFlags(const Qt::MatchFlag flags)
{
  m_filterFlags = flags;
}

void QSortFilterSqlQueryModel::filter(const QString &filter)
{
  if (m_filterString != filter) {
    setFilter(filter);
    select();
  }
}

QString QSortFilterSqlQueryModel::getFilterColumnName()
{
  return this->headerData( m_sortKeyColumn, Qt::Horizontal ).toString();
}
