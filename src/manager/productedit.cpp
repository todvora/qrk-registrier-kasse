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

#include "productedit.h"
#include "database.h"
#include "utils/utils.h"
#include "preferences/qrksettings.h"

#include <QDoubleValidator>
#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QKeyEvent>
#include <QMessageBox>
#include <QDebug>

//--------------------------------------------------------------------------------
ProductEdit::ProductEdit(QWidget *parent, int id)
    : QDialog(parent), ui(new Ui::ProductEdit), m_id(id)
{
    ui->setupUi(this);

    const QStringList colorNames = QColor::colorNames();
    int index = 0;
    ui->colorComboBox->addItem(tr("gleich wie Gruppe"));
    const QModelIndex idx = ui->colorComboBox->model()->index(index++, 0);
    ui->colorComboBox->model()->setData(idx, "", Qt::BackgroundColorRole);

    foreach (const QString &colorName, colorNames) {
        const QColor color(colorName);
        QString fg = Utils::color_best_contrast(color.name());
        const QColor fg_color(fg);
        ui->colorComboBox->addItem(colorName, color);
        const QModelIndex idx = ui->colorComboBox->model()->index(index++, 0);
        ui->colorComboBox->model()->setData(idx, color, Qt::BackgroundColorRole);
        ui->colorComboBox->model()->setData(idx, fg_color, Qt::ForegroundRole);
    }

    QDoubleValidator *doubleVal = new QDoubleValidator(0.0, 9999999.99, 2, this);
    doubleVal->setNotation(QDoubleValidator::StandardNotation);

    ui->net->setValidator(doubleVal);
    ui->gross->setValidator(doubleVal);

    QSqlDatabase dbc = QSqlDatabase::database("CN");

    m_groupsModel = new QSqlRelationalTableModel(this, dbc);
    m_groupsModel->setQuery("SELECT id, name FROM groups WHERE id > 1", dbc);
    ui->groupComboBox->setModel(m_groupsModel);
    ui->groupComboBox->setModelColumn(1);  // show name

    m_taxModel = new QSqlRelationalTableModel(this, dbc);
    QString q = QString("SELECT id, tax FROM taxTypes WHERE taxlocation='%1'").arg(Database::getTaxLocation());
    m_taxModel->setQuery(q, dbc);
    ui->taxComboBox->setModel(m_taxModel);
    ui->taxComboBox->setModelColumn(1);  // show tax
    ui->taxComboBox->setCurrentIndex(0);

    if ( m_id != -1 )
    {
        QSqlQuery query(QString("SELECT `name`, `group`,`visible`,`net`,`gross`,`tax`, `color`, `itemnum`, `barcode`, `coupon` FROM products WHERE id=%1").arg(id), dbc);
        query.next();

        ui->name->setText(query.value(0).toString());
        ui->name->setReadOnly(true);
        ui->name->setToolTip(tr("Bestehende Artikelnamen dürfen nicht umbenannt werden. Versuchen Sie diesen Artikel zu löschen und legen danach einen neuen Artikel an."));
        ui->visibleCheckBox->setChecked(query.value(2).toBool());
        ui->net->setText(QString::number(query.value(3).toDouble(), 'f', 2));
        ui->gross->setText(QString::number(query.value(4).toDouble(), 'f', 2));
        ui->itemNum->setText(query.value(7).toString());
        ui->barcode->setText(query.value(8).toString());
        ui->collectionReceiptCheckBox->setChecked(query.value(9).toBool());

        int i;
        for (i = 0; i < m_groupsModel->rowCount(); i++)
            if ( query.value(1).toInt() == m_groupsModel->data(m_groupsModel->index(i, 0), Qt::DisplayRole).toInt() )
                break;

        ui->groupComboBox->setCurrentIndex(i);

        for (i = 0; i < m_taxModel->rowCount(); i++)
            if ( query.value(5).toDouble() == m_taxModel->data(m_taxModel->index(i, 1), Qt::DisplayRole).toDouble() )
                break;

        ui->taxComboBox->setCurrentIndex(i);

        for (i = 0; i <= ui->colorComboBox->count(); i++) {
            QString color = ui->colorComboBox->model()->index(i, 0).data(Qt::BackgroundColorRole).toString();
            if ( query.value(6).toString() == color )
                break;
        }

        if (i > ui->colorComboBox->count())
            i = 0;

        QString colorValue = query.value(6).toString().trimmed();
        QPalette palette(ui->colorComboBox->palette());
        QColor color(colorValue);
        palette.setColor(QPalette::Active,QPalette::Button, color);
        palette.setColor(QPalette::Highlight, color);
        ui->colorComboBox->setPalette(palette);
        ui->colorComboBox->setCurrentIndex(i);

    }
    connect (ui->taxComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(taxComboChanged(int)));
    connect (ui->net, SIGNAL(editingFinished()), this, SLOT(netChanged()));
    connect (ui->gross, SIGNAL(editingFinished()), this, SLOT(grossChanged()));
    connect (ui->colorComboBox, SIGNAL(currentIndexChanged(int)),this,SLOT(colorComboChanged(int)));

    connect (ui->okButton, SIGNAL(clicked(bool)),this,SLOT(accept()));
    connect (ui->cancelButton, SIGNAL(clicked(bool)),this,SLOT(reject()));

    QrkSettings settings;
    m_barcodeReaderPrefix = settings.value("barcodeReaderPrefix", Qt::Key_F11).toInt();
    installEventFilter(this);

    bool printCollectionReceipt = settings.value("printCollectionReceipt", false).toBool();
    ui->collectionReceiptLabel->setVisible(printCollectionReceipt);
    ui->collectionReceiptCheckBox->setVisible(printCollectionReceipt);

}

//--------------------------------------------------------------------------------
bool ProductEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        switch (keyEvent->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            focusNextChild();
            break;

        default:
            if (keyEvent->key() == m_barcodeReaderPrefix) {
                ui->barcode->setFocus(Qt::OtherFocusReason);
                break;
            }
            return QObject::eventFilter(obj, event);
        }

        return true;
    }

    return QObject::eventFilter(obj, event);

}

void ProductEdit::colorComboChanged(int idx)
{

    QString colorValue = ui->colorComboBox->itemData(idx, Qt::BackgroundColorRole).toString(); // ->itemText(idx); //   ->model()->index(ui->colorComboBox->currentIndex(), 0).data(Qt::BackgroundColorRole).toString();
    QPalette palette(ui->colorComboBox->palette());
    QColor color(colorValue);
    palette.setColor(QPalette::Active,QPalette::Button, color);
    palette.setColor(QPalette::Highlight, color);
    ui->colorComboBox->setPalette(palette);
}

void ProductEdit::taxComboChanged(int)
{
    double tax = m_taxModel->data(m_taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double net = ui->net->text().replace(",",".").toDouble();
    double gross = net * (1.0 + tax / 100.0);

    ui->gross->setText(QString::number(gross, 'f', 2));

}

void ProductEdit::netChanged()
{
    double tax = m_taxModel->data(m_taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double net = ui->net->text().replace(",",".").toDouble();
    double gross = net * (1.0 + tax / 100.0);

    ui->gross->setText(QString::number(gross, 'f', 2));

}

void ProductEdit::grossChanged()
{
    double tax = m_taxModel->data(m_taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double gross = ui->gross->text().replace(",",".").toDouble();
    double net = gross / (1.0 + tax / 100.0);

    ui->net->setText(QString::number(net, 'f', 2));

}

void ProductEdit::accept()
{

    int id = Database::getProductIdByName(ui->name->text());
    if (m_id != id) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Artikel schon vorhanden"));
        msgBox.setText(tr("Der Artikel '%1' ist schon vorhanden. Möchten Sie diesen überschreiben?").arg(ui->name->text()));
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setButtonText(QMessageBox::Yes, tr("Überschreiben"));
        msgBox.setButtonText(QMessageBox::No, tr("Abbrechen"));
        msgBox.setDefaultButton(QMessageBox::No);

        if(msgBox.exec() == QMessageBox::No)
            return;

    }
    m_id = id;

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    QString color = ui->colorComboBox->model()->index(ui->colorComboBox->currentIndex(), 0).data(Qt::BackgroundColorRole).toString();

    double tax = m_taxModel->data(m_taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double net = ui->gross->text().replace(",",".").toDouble() / (1.0 + tax / 100.0);
    bool collectionReceipt = ui->collectionReceiptCheckBox->isChecked();

    if ( m_id == -1 )  // new entry
    {
        query.prepare(QString("INSERT INTO products (name, `group`, itemnum, barcode, visible, net, gross, tax, color, coupon) VALUES(:name, :group, :itemnum, :barcode, :visible, :net, :gross, :tax, :color, :coupon)"));
    } else {
        query.prepare(QString("UPDATE products SET name=:name, itemnum=:itemnum, barcode=:barcode, `group`=:group, visible=:visible, net=:net, gross=:gross, tax=:tax, color=:color, coupon=:coupon WHERE id=:id"));
        query.bindValue(":id", m_id);
    }

    QString itemNum = ui->itemNum->text();
    query.bindValue(":name", ui->name->text());
    query.bindValue(":itemnum", itemNum);
    query.bindValue(":barcode", ui->barcode->text());
    query.bindValue(":group", m_groupsModel->data(m_groupsModel->index(ui->groupComboBox->currentIndex(), 0)).toInt());
    query.bindValue(":visible", ui->visibleCheckBox->isChecked());
    query.bindValue(":net", net);
    query.bindValue(":gross", ui->gross->text().replace(",",".").toDouble());
    query.bindValue(":tax", tax);
    query.bindValue(":color", color);
    query.bindValue(":coupon", collectionReceipt);

    bool ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    QDialog::accept();
}

//--------------------------------------------------------------------------------
