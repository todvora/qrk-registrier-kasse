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

#include "productedit.h"
#include "database.h"

#include <QDoubleValidator>
#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

//--------------------------------------------------------------------------------
ProductEdit::ProductEdit(QWidget *parent, int theId)
    : QDialog(parent), ui(new Ui::ProductEdit), id(theId)
{
    ui->setupUi(this);

    const QStringList colorNames = QColor::colorNames();
    int index = 0;
    ui->colorComboBox->addItem(tr("gleich wie Gruppe"));
    const QModelIndex idx = ui->colorComboBox->model()->index(index++, 0);
    ui->colorComboBox->model()->setData(idx, "", Qt::BackgroundColorRole);

    foreach (const QString &colorName, colorNames) {
        const QColor color(colorName);
        ui->colorComboBox->addItem(colorName, color);
        const QModelIndex idx = ui->colorComboBox->model()->index(index++, 0);
        ui->colorComboBox->model()->setData(idx, color, Qt::BackgroundColorRole);
    }


    QDoubleValidator *doubleVal = new QDoubleValidator(0.0, 9999999.99, 2, this);
    doubleVal->setNotation(QDoubleValidator::StandardNotation);

    ui->net->setValidator(doubleVal);
    ui->gross->setValidator(doubleVal);

    QSqlDatabase dbc = QSqlDatabase::database("CN");

    groupsModel = new QSqlRelationalTableModel(this, dbc);
    groupsModel->setQuery("SELECT id, name FROM groups WHERE id > 1", dbc);
    ui->groupComboBox->setModel(groupsModel);
    ui->groupComboBox->setModelColumn(1);  // show name

    taxModel = new QSqlRelationalTableModel(this, dbc);
    QString q = QString("SELECT id, tax FROM taxTypes WHERE taxlocation='%1'").arg(Database::getTaxLocation());
    taxModel->setQuery(q, dbc);
    ui->taxComboBox->setModel(taxModel);
    ui->taxComboBox->setModelColumn(1);  // show tax
    ui->taxComboBox->setCurrentIndex(0);

    if ( id != -1 )
    {
        QSqlQuery query(QString("SELECT `name`,`group`,`visible`,`net`,`gross`,`tax`, `color` FROM products WHERE id=%1").arg(id), dbc);
        query.next();

        ui->name->setText(query.value(0).toString());
        ui->visibleCheckBox->setChecked(query.value(2).toBool());
        ui->net->setText(QString::number(query.value(3).toDouble(), 'f', 2));
        ui->gross->setText(QString::number(query.value(4).toDouble(), 'f', 2));

        int i;
        for (i = 0; i < groupsModel->rowCount(); i++)
            if ( query.value(1).toInt() == groupsModel->data(groupsModel->index(i, 0), Qt::DisplayRole).toInt() )
                break;

        ui->groupComboBox->setCurrentIndex(i);

        for (i = 0; i < taxModel->rowCount(); i++)
            if ( query.value(5).toDouble() == taxModel->data(taxModel->index(i, 1), Qt::DisplayRole).toDouble() )
                break;

        ui->taxComboBox->setCurrentIndex(i);

        for (i = 0; i <= ui->colorComboBox->count(); i++) {
            QString color = ui->colorComboBox->model()->index(i, 0).data(Qt::BackgroundColorRole).toString();
            if ( query.value(6).toString() == color )
                break;
        }

        if (i > ui->colorComboBox->count())
          i = 0;

        ui->colorComboBox->setCurrentIndex(i);
        QPalette palette(ui->colorComboBox->palette());
        QColor color(query.value(6).toString());
        palette.setColor(QPalette::Active,QPalette::Button, color);
        palette.setColor(QPalette::Highlight, color);
//        palette.setColor(QPalette::ButtonText, Qt::gray);
        ui->colorComboBox->setPalette(palette);

    }
    connect (ui->taxComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(taxComboChanged(int)));
    connect (ui->net, SIGNAL(editingFinished()), this, SLOT(netChanged()));
    connect (ui->gross, SIGNAL(editingFinished()), this, SLOT(grossChanged()));

}

//--------------------------------------------------------------------------------

void ProductEdit::taxComboChanged(int)
{
    double tax = taxModel->data(taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double net = ui->net->text().replace(",",".").toDouble();
    double gross = net * (1.0 + tax / 100.0);

    ui->gross->setText(QString::number(gross, 'f', 2));

}

void ProductEdit::netChanged()
{
    double tax = taxModel->data(taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double net = ui->net->text().replace(",",".").toDouble();
    double gross = net * (1.0 + tax / 100.0);

    ui->gross->setText(QString::number(gross, 'f', 2));

}

void ProductEdit::grossChanged()
{
    double tax = taxModel->data(taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double gross = ui->gross->text().replace(",",".").toDouble();
    double net = gross / (1.0 + tax / 100.0);

    ui->net->setText(QString::number(net, 'f', 2));

}

void ProductEdit::accept()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    QString color = ui->colorComboBox->model()->index(ui->colorComboBox->currentIndex(), 0).data(Qt::BackgroundColorRole).toString();

    double tax = taxModel->data(taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double net = ui->gross->text().toDouble() / (1.0 + tax / 100.0);

    if ( id == -1 )  // new entry
    {
        bool ok = query.exec(QString("INSERT INTO products (name, `group`, visible, net, gross, tax, color) VALUES('%1', %2, %3, %4, %5, %6, '%7')")
                             .arg(ui->name->text())
                             .arg(groupsModel->data(groupsModel->index(ui->groupComboBox->currentIndex(), 0)).toInt())
                             .arg(ui->visibleCheckBox->isChecked())
                             .arg(net)
                             .arg(ui->gross->text().toDouble())
                             .arg(tax)
                             .arg(color));

        if (!ok) {
            qDebug() << "ProductEdit::accept() error: " << query.lastError().text();
            qDebug() << "ProductEdit::accept() query: " << query.lastQuery();

        }
    }
    else
    {
        bool ok = query.exec(QString("UPDATE products SET name='%1', `group`=%2,visible=%3,net=%4,gross=%5,tax=%6, color='%7' WHERE id=%8")
                             .arg(ui->name->text())
                             .arg(groupsModel->data(groupsModel->index(ui->groupComboBox->currentIndex(), 0)).toInt())
                             .arg(ui->visibleCheckBox->isChecked())
                             .arg(net)
                             .arg(ui->gross->text().toDouble())
                             .arg(tax)
                             .arg(color)
                             .arg(id));
        if (!ok) {
            qDebug() << "ProductEdit::accept() error: " << query.lastError().text();
            qDebug() << "ProductEdit::accept() query: " << query.lastQuery();

        }

    }

    QDialog::accept();
}

//--------------------------------------------------------------------------------
