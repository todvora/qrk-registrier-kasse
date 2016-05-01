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

#ifndef _GROUPS_WIDGET_H_
#define _GROUPS_WIDGET_H_

#include <QDialog>
class QSqlTableModel;
class QSortFilterProxyModel;

#include "ui_groupwidget.h"

namespace Ui {
  class GroupsWidget;
}

class GroupsWidget : public QWidget
{
  Q_OBJECT

  public:
    GroupsWidget(QWidget *parent);

  private slots:
    void filterGroup(const QString &filter);
    void plusSlot();
    void minusSlot();
    void editSlot();

  private:
    Ui::GroupsWidget *ui;
    QSqlTableModel *model;
    QSortFilterProxyModel *proxyModel;

};

#endif
