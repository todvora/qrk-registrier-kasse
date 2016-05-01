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

#include "qrkdelegate.h"
#include "database.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCompleter>
#include <QPainter>
#include <QDebug>


QrkDelegate::QrkDelegate(int type, QObject *parent)
  :QStyledItemDelegate(parent), type(type)
{
  shortcurrency = Database::getShortCurrency();
  taxlocation = Database::getTaxLocation();
}

QWidget* QrkDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &item, const QModelIndex &index) const
{
  if (this->type == SPINBOX) {
    QSpinBox *spinbox = new QSpinBox(parent);
    spinbox->setMinimum(-99999);
    spinbox->setMaximum(99999);
    spinbox->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

    connect( spinbox , SIGNAL( valueChanged(int) ), this , SLOT( commitAndCloseEditor() ) ) ;

    return spinbox;

  } else if (this->type == DOUBLE_SPINBOX) {
      QDoubleSpinBox *spinbox = new QDoubleSpinBox(parent);
      spinbox->setMinimum(-99999);
      spinbox->setMaximum(99999);
      spinbox->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

      connect( spinbox , SIGNAL( valueChanged(int) ), this , SLOT( commitAndCloseEditor() ) ) ;

      return spinbox;

  } else if (this->type == COMBO_TAX) {
    QComboBox *combo = new QComboBox(parent);
    combo->setEditable(false);
    combo->setInsertPolicy(QComboBox::InsertAfterCurrent);
    combo->setDuplicatesEnabled(false);
    QSqlDatabase dbc= QSqlDatabase::database("CN");
    QSqlQuery q(dbc);
    q.prepare(QString("SELECT tax FROM taxTypes WHERE taxlocation='%1' ORDER BY id").arg(taxlocation));
    if(!q.exec()){
      qDebug()<<"Can't get taxType list!";
    }
    while(q.next()){
      combo->addItem(q.value(0).toString());
    }
    combo->setCurrentIndex(combo->findText(index.data().value<QString>()));
    return combo;

  } else if (this->type == PRODUCTS) {
    QLineEdit *editor = new QLineEdit( parent );
    editor->setPlaceholderText(tr("Artikelname"));
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
//    query.prepare("SELECT name FROM products WHERE name NOT LIKE 'Zahlungsbeleg für Rechnung%'");
    query.prepare("SELECT name FROM products WHERE visible = 1");
    query.exec();

    QStringList* list = new QStringList();

    while(query.next()){
      QString value = query.value("name").toString();
      *list   << value;
    }

    QCompleter * editorCompleter = new QCompleter( *list ) ;
    editorCompleter->setCaseSensitivity( Qt::CaseInsensitive ) ;
    editorCompleter->setFilterMode( Qt::MatchContains );
    editor->setCompleter( editorCompleter );

    connect( editor , SIGNAL( editingFinished() ), this , SLOT( commitAndCloseEditor() ) ) ;

    return editor ;

  } else if (this->type == NUMBERFORMAT_DOUBLE) {
    QLineEdit* editor = new QLineEdit(parent);
    QRegExpValidator* rxv = new QRegExpValidator(QRegExp("[-+]?[0-9]*[\\.,]?[0-9]+([eE][-+]?[0-9]+)?"));
    editor->setValidator(rxv);
    editor->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    return editor;
  }
  // qDebug() << "QrkDelegate::Editor";

  return QStyledItemDelegate::createEditor(parent, item, index);

}

QString QrkDelegate::displayText(const QVariant &value, const QLocale &locale) const
{

  if (this->type == NUMBERFORMAT_DOUBLE) {
    int x = QString::number(value.toDouble()).length() - QString::number(value.toDouble()).indexOf(".");
    QString formattedNum;
    if (x > 3 && QString::number(value.toDouble()).indexOf(".") > 0) {
      formattedNum = QString("%1").arg(locale.toString(value.toDouble(), 'f', 3));
      formattedNum = QString("%1.. %2").arg(formattedNum.left(formattedNum.length() - 1)).arg(shortcurrency);
    } else {
      formattedNum = QString("%1 %2").arg(locale.toString(value.toDouble(), 'f', 2)).arg(shortcurrency);
    }
    return formattedNum;

  } else if (this->type == COMBO_TAX) {
    QString formattedNum;
    if (taxlocation == "CH")
      formattedNum = QString("%1 %").arg(locale.toString(value.toDouble()));
    else
      formattedNum = QString("%1 %").arg(locale.toString(value.toInt()));

    return formattedNum;
  }

  return QStyledItemDelegate::displayText(value, locale);
}


void QrkDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

  if (this->type == SPINBOX) {
    // Get the value via index of the Model
    int value = index.model()->data(index, Qt::EditRole).toInt();
    // Put the value into the SpinBox
    QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
    spinbox->setValue(value);
  } else if (this->type == DOUBLE_SPINBOX) {
      // Get the value via index of the Model
      double value = index.model()->data(index, Qt::EditRole).toDouble();
      // Put the value into the SpinBox
      QDoubleSpinBox *spinbox = static_cast<QDoubleSpinBox*>(editor);
      spinbox->setValue(value);

  } else if (this->type == COMBO_TAX) {
    if(index.data().canConvert<QString>()){
      QString taxTitle= index.data().value<QString>();
      QComboBox *combo= qobject_cast<QComboBox *>(editor);
      combo->setCurrentIndex(combo->findText(taxTitle));
    }

  } else if(this->type == PRODUCTS){
    QLineEdit *edit = qobject_cast<QLineEdit*>( editor ) ;
    edit->setText( index.data(Qt::EditRole).toString());
  } else if (this->type == NUMBERFORMAT_DOUBLE) {
    QString v = index.model()->data(index,Qt::EditRole).toString();
    v.replace(",",".");
    // qDebug() << "QrkDelegate::setEditorData v:" << v;
    double value = v.toDouble();
    // qDebug() << "QrkDelegate::setEditorData value:" << value;

    QLineEdit* line = static_cast<QLineEdit*>(editor);
    line->setText(QString().setNum(value));
  }
  // qDebug() << "QrkDelegate::setEditorData";

}

void QrkDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{

  if (this->type == SPINBOX) {
    QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
    spinbox->interpretText();
    int value = spinbox->value();
    model->setData(index, value);
  } else if (this->type == DOUBLE_SPINBOX) {
      QDoubleSpinBox *spinbox = static_cast<QDoubleSpinBox*>(editor);
      spinbox->interpretText();
      double value = spinbox->value();
      model->setData(index, value);

  } else if (this->type == COMBO_TAX) {
    if (index.data().canConvert(QMetaType::QString)){
      QComboBox *combo = qobject_cast<QComboBox *>(editor);
      model->setData(index, QVariant::fromValue(combo->currentText().trimmed()));
    }

  } else if (this->type == PRODUCTS) {
    QLineEdit *edit = qobject_cast<QLineEdit *>( editor ) ;
    model->setData( index, edit->text() ) ;

  }else if (this->type == NUMBERFORMAT_DOUBLE) {
    QLineEdit* line = static_cast<QLineEdit*>(editor);
    QString value = line->text();
    model->setData(index,value);
  }
  // qDebug() << "QrkDelegate::setModelData";
  QStyledItemDelegate::setModelData(editor, model, index);

}

void QrkDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{

  editor->setGeometry(option.rect);
  // qDebug() << "QrkDelegate::updateEditorGeometry";

}

void QrkDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{

  switch (this->type)
  {
    case NUMBERFORMAT_DOUBLE:
    case NUMBERFORMAT_INT:
    case COMBO_TAX:
      QStyleOptionViewItem myOption = option;
      myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

      QStyledItemDelegate::paint(painter, myOption, index);
      return;
  }

  QStyledItemDelegate::paint(painter, option, index);
}

void QrkDelegate::commitAndCloseEditor()
{
  if (this->type == COMBO_TAX) {
    QComboBox *combo= qobject_cast<QComboBox *>(sender());
    emit commitData(combo);
    emit closeEditor(combo);
  } else if (this->type == SPINBOX) {
    QSpinBox *spinbox= qobject_cast<QSpinBox *>(sender());
    emit commitData(spinbox);
  } else if (this->type == DOUBLE_SPINBOX) {
    QDoubleSpinBox *spinbox= qobject_cast<QDoubleSpinBox *>(sender());
    emit commitData(spinbox);
  } else if (this->type == PRODUCTS) {
    QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
    emit commitData( editor ) ;
  }
}
