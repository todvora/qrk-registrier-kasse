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

#include "csvimportwizardpage3.h"
#include "ui_csvimportwizardpage3.h"
#include "database.h"
#include "backup.h"

#include <QTableView>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QDateTime>
#include <QDebug>

CsvImportWizardPage3::CsvImportWizardPage3(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::CsvImportWizardPage3)
{
    ui->setupUi(this);

    connect(ui->startImportButton, SIGNAL(clicked(bool)),this, SLOT(save(bool)));
}

CsvImportWizardPage3::~CsvImportWizardPage3()
{
    delete ui;
}

bool CsvImportWizardPage3::isComplete() const
{
    return m_finished;
}

void CsvImportWizardPage3::initializePage()
{

    m_finished = false;
    ui->startImportButton->setEnabled(true);
    m_autoGroup = field("autogroup").toBool();
    m_guessGroup = field("guessgroup").toBool();
    m_ignoreExistingProduct = field("ignoreexisting").toBool();
    m_updateExistingProduct = field("updateexisting").toBool();
    m_importType = field("importtype").toString();

    m_visibleGroup = field("visiblegroup").toBool();
    m_visibleProduct = field("visibleproduct").toBool();

}

void CsvImportWizardPage3::setModel(QStandardItemModel *model)
{
    m_model =  model;

}

void CsvImportWizardPage3::setMap(QMap<QString, QVariant> *map)
{
    m_map =  map;
}

void CsvImportWizardPage3::importFinished()
{
    m_finished = true;
    ui->startImportButton->setEnabled(false);

    emit completeChanged();
    emit info(tr("Der Datenimport wurde abgeschlossen."));

}

void CsvImportWizardPage3::info(QString info)
{
    info = QString("%1 %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).arg(info);
    ui->listWidget->addItem(info);
}

void CsvImportWizardPage3::save(bool)
{
    emit info(tr("Backup der Daten wurde gestartet."));
    Backup::create();
    emit info(tr("Backup der Daten fertig."));
    emit completeChanged();
    emit info(tr("Der DatenImport wurde gestarted."));
    ImportData *import = new ImportData(m_model, m_map, m_ignoreExistingProduct, m_guessGroup, m_autoGroup, m_visibleGroup, m_visibleProduct, m_updateExistingProduct);
    QThread *thread = new QThread(this);
    import->moveToThread(thread);

    connect(thread, SIGNAL(started()), import, SLOT(run()));

    connect(import, SIGNAL(percentChanged(int)), ui->progressBar, SLOT(setValue(int)));
    connect(import, SIGNAL(finished()), this, SLOT(importFinished()));
    connect(import, SIGNAL(info(QString)), this, SLOT(info(QString)));

    thread->start();

}

//---- ImportData ------------------------------------------------------------------------------------------------------

ImportData::ImportData(QStandardItemModel *model, QMap<QString, QVariant> *map, bool ignoreExistingProduct, bool guessGroup, bool autoGroup,  bool visibleGroup, bool visibleProduct, bool updateExistingProduct)
{
    m_model = model;
    m_map = map;
    m_autoGroup = autoGroup;
    m_guessGroup = guessGroup;
    m_ignoreExistingProduct = ignoreExistingProduct;
    m_updateExistingProduct = updateExistingProduct;
    m_visibleGroup = visibleGroup;
    m_visibleProduct = visibleProduct;
}

ImportData::~ImportData()
{
}

void ImportData::run()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    int count = m_model->rowCount();
    // count = 100; /* for testing */

    for (int row = 0; row < count; row++) {

        int percent = ((float)row / (float)count) * 100;
        emit percentChanged(percent);

        QString itemnum = getItemValue(row, m_map->value(tr("Artikelnummer")).toInt());
        QString barcode = getItemValue(row, m_map->value(tr("Barcode")).toInt());
        QString name = getItemValue(row, m_map->value(tr("Artikelname")).toInt());
        QString color = getItemValue(row, m_map->value(tr("Farbe")).toInt());
        QString coupon = getItemValue(row, m_map->value(tr("Extrabon")).toInt());

        if (m_ignoreExistingProduct && exists(itemnum, barcode, name)) {
            emit info(tr("%1 ist schon vorhanden. Der Import wurde ignoriert").arg(name));
            continue;
        }

        bool ok = false;
        int id = exists(itemnum, barcode, name);
        if (id == 0)
            continue;

        if (m_updateExistingProduct && id > 0) {
            ok= query.prepare("UPDATE products SET name=:name, itemnum=:itemnum, barcode=:barcode, tax=:tax, net=:net, gross=:gross, visible=:visible, color=:color, coupon=:coupon,  `group`=:group WHERE id=:id");
            query.bindValue(":id", id);
            emit info(tr("Update %1").arg(name));
        } else {
            ok= query.prepare("INSERT INTO products (name, `group`, itemnum, barcode, visible, net, gross, tax, color, coupon) VALUES (:name, :group, :itemnum, :barcode, :visible, :net, :gross, :tax, :color, :coupon)");
        }

        if (!ok) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
        }

        double net = getItemValue(row, m_map->value(tr("Netto Preis")).toInt(),true).toDouble();
        double gross = getItemValue(row, m_map->value(tr("Brutto Preis")).toInt(),true).toDouble();
        double tax;
        if (m_map->value(tr("Steuersatz")).toInt() == 0)
            tax = Database::getDefaultTax().toDouble();
        else
            tax = getItemValue(row, m_map->value(tr("Steuersatz")).toInt(),true).toDouble();


        if (tax < 1) tax = tax * 100.0;
        if (gross < net) gross = net * (1.0 + tax / 100.0);
        if (gross != 0.00 && net == 0.00) net = gross / (1.0 + tax / 100.0);

        query.bindValue(":itemnum", itemnum);
        query.bindValue(":barcode", barcode);
        query.bindValue(":name", name);
        query.bindValue(":net", net);
        query.bindValue(":gross", gross);
        query.bindValue(":tax", tax);
        query.bindValue(":visible", m_visibleProduct);
        query.bindValue(":color", color);
        query.bindValue(":coupon", (coupon.toInt() == 0)? false: true);

        QString group = getItemValue(row, m_map->value(tr("Gruppe")).toInt());
        if (group.toInt() > 0) {
            if (getGroupByName(getGroupById(group.toInt())) == group.toInt()) {
                query.bindValue(":group", group.toInt());
            } else {
                if(m_guessGroup) {
                    int id = createGroup(getGuessGroup(name));
                    query.bindValue(":group", id);
                } else {
                    query.bindValue(":group", 2);
                }
            }
        } else {
            if (group.isEmpty()) {
                if(m_guessGroup) {
                    int id = createGroup(getGuessGroup(name));
                    query.bindValue(":group", id);
                } else {
                    query.bindValue(":group", 2);
                }
            } else {
                int id = getGroupByName(group);
                if (id > 0) {
                    query.bindValue(":group", id);
                } else {
                    if(m_autoGroup) {
                        int id = createGroup(group);
                        query.bindValue(":group", id);
                    } else {
                        query.bindValue(":group", 2);
                    }
                }
            }
        }

        ok = query.exec();
        qDebug() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);

        if (!ok) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
        }
    }

    emit percentChanged(100);
    emit finished();

}

QString ImportData::getItemValue(int row, int col, bool replace)
{
    col--;
    if (col >= 0) {
        QStandardItem *item;
        item = m_model->item(row, col);
        if (!item)
            return "";

        if (replace)
            return item->text().remove(QRegExp("[^0-9,.]")).replace(",",".");

        QString text = item->text();
        if (text.isNull())
            return "";
        return item->text();
    }

    return "";

}

QString ImportData::getGroupById(int id)
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare("SELECT name FROM groups WHERE id=:id");
    query.bindValue(":id", id);

    query.exec();
    if (query.next())
        return query.value(0).toString();

    return "";
}

int ImportData::getGroupByName(QString name)
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare("SELECT id FROM groups WHERE name=:name");
    query.bindValue(":name", name);

    query.exec();
    if (query.next())
        return query.value(0).toInt();

    return -1;

}

QString ImportData::getGuessGroup(QString name)
{
    QString group = name.split(" ").at(0);
    return group;
}

int ImportData::createGroup(QString name)
{

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare(QString("SELECT id FROM groups WHERE name=:name"));
    query.bindValue(":name", name);

    bool ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    if(query.next())
        return query.value(0).toInt();


    query.prepare(QString("INSERT INTO groups (name, visible) VALUES(:name, :visible)"));
    query.bindValue(":name", name);
    query.bindValue(":visible", (m_visibleGroup)? 1: 0);

    ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    emit info(tr("Gruppe %1 wurde erstellt.").arg(name));

    return getGroupByName(name);

}

int ImportData::exists(QString itemnum, QString barcode, QString name)
{

    if (name.isEmpty())
        return 0;

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    if (!itemnum.isEmpty()) {
        bool ok = query.prepare("SELECT id, name FROM products WHERE itemnum=:itemnum");
        query.bindValue(":itemnum", itemnum);

        if (!ok) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
        }

        query.exec();
        if (query.next()) {
            if (query.value(1).toString() != name) {
                emit info(tr("Artiklenummer %1 (%2) ist bereits für Artikel %3 vergeben. Kein Import möglich.").arg(itemnum).arg(name).arg(query.value(1).toString()));
                return 0;
            }
            return query.value(0).toInt();
        }
    }
    if (!barcode.isEmpty()) {

        bool ok = query.prepare("SELECT id, name FROM products WHERE barcode=:barcode");
        query.bindValue(":barcode", barcode);

        if (!ok) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
        }

        query.exec();
        if (query.next()) {
            if (query.value(1).toString() != name) {
                emit info(tr("Barcode %1 (%2) ist bereits für Artikel %3 vergeben. Kein Import möglich.").arg(barcode).arg(name).arg(query.value(1).toString()));
                return 0;
            }

            return query.value(0).toInt();
        }
    }

    bool ok = query.prepare("SELECT id FROM products WHERE name=:name");
    query.bindValue(":name", name);
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    query.exec();
    if (query.next())
        return query.value(0).toInt();

    return -1;
}
