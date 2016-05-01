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

#ifndef QRK_DELEGATE_H
#define QRK_DELEGATE_H

#include <QStyledItemDelegate>

class QrkDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:

    enum TYPE
    {
      SPINBOX,
      COMBO_TAX,
      PRODUCTS,
      NUMBERFORMAT_DOUBLE,
      NUMBERFORMAT_INT,
      DOUBLE_SPINBOX
    };

    explicit QrkDelegate(int type, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    virtual QString displayText(const QVariant &value, const QLocale &locale) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const;


  signals:

  private slots:
    void commitAndCloseEditor() ;

  private:
    int type;
    QString shortcurrency;
    QString taxlocation;

};

#endif // QRK_DELEGATE_H
