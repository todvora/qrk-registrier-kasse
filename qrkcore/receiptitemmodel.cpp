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

#include "defines.h"
#include "receiptitemmodel.h"
#include "database.h"
#include "utils/utils.h"
#include "documentprinter.h"
#include "journal.h"
#include "reports.h"
#include "RK/rk_signaturemodulefactory.h"
#include "utils/demomode.h"
#include "preferences/qrksettings.h"
#include "3rdparty/qbcmath/bcmath.h"

#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonArray>
#include <QDebug>

ReceiptItemModel::ReceiptItemModel(QObject* parent)
    : QStandardItemModel(parent)
{
    m_currency = "";
    m_taxlocation = "";
    m_customerText = "";

    m_isR2B = false;
    m_isReport = false;
    m_totallyup = false;

    connect(this, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(itemChangedSlot(const QModelIndex&, const QModelIndex&)));
}

void ReceiptItemModel::clear()
{
    QStandardItemModel::clear();
    m_currency = Database::getCurrency();
    m_taxlocation = Database::getTaxLocation();
    m_customerText = "";

    m_isR2B = false;
    m_isReport = false;
    m_totallyup = false;

}
void ReceiptItemModel::setReceiptTime(QDateTime receiptTime)
{
    m_receiptTime = receiptTime;
}

void ReceiptItemModel::setCustomerText(QString customerText)
{
    m_customerText = customerText;
}

void ReceiptItemModel::setCurrentReceiptNum(int id)
{
    m_currentReceipt = id;
}

int ReceiptItemModel::getReceiptNum()
{
    return m_currentReceipt;
}

bool ReceiptItemModel::finishReceipts(int payedBy, int id, bool isReport)
{

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    bool ok = false;
    ok = query.exec(QString("UPDATE globals SET value=%1 WHERE name='lastReceiptNum'").arg(m_currentReceipt));

    if (!ok) {
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    QBCMath sum = 0.0;
    QBCMath net = 0.0;

    if (!isReport) {

        QSqlQuery orders(dbc);
        orders.prepare(QString("SELECT orders.count, orders.gross, orders.tax, orders.discount FROM orders WHERE orders.receiptId=%1")
                       .arg(m_currentReceipt));

        ok = orders.exec();
        if (!ok) {
            qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
            qCritical() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
        }

        while ( orders.next() )
        {
            QBCMath count = orders.value("count").toDouble();
            count.round(2);
            QBCMath singlePrice = orders.value("gross").toDouble();
            singlePrice.round(2);
            double tax = QString::number(orders.value("tax").toDouble(),'f',2).toDouble();
            QBCMath discount = orders.value("discount").toDouble();
            discount.round(2);

            QBCMath gross = singlePrice * count;
            gross = gross - ((gross / 100) * discount.toDouble());
            gross.round(2);
            sum += gross;
            net += gross / (1.0 + tax / 100.0);
        }
    }

    setReceiptTime(QDateTime::currentDateTime());
    query.prepare(QString("UPDATE receipts SET timestamp=:timestamp, infodate=:infodate, receiptNum=:receiptNum, payedBy=:payedBy, gross=:gross, net=:net WHERE id=:receiptNum"));
    query.bindValue(":timestamp", m_receiptTime.toString(Qt::ISODate));
    query.bindValue(":infodate", m_receiptTime.toString(Qt::ISODate));
    query.bindValue(":receiptNum", m_currentReceipt);
    query.bindValue(":payedBy", payedBy);
    query.bindValue(":gross", sum.toDouble());
    query.bindValue(":net", net.toString());

    ok = query.exec();
    if (!ok) {
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    QJsonObject data = compileData(id);
    if (!m_isReport && m_isR2B){
        data["isR2B"] = m_isR2B;
    }

    if (RKSignatureModule::isDEPactive()) {
        Utils *utils = new Utils();
        QString signature =  utils->getSignature(data);
        delete utils;

        query.prepare(QString("INSERT INTO dep (receiptNum, data) VALUES (:receiptNum, :data)"));
        query.bindValue(":receiptNum", m_currentReceipt);
        query.bindValue(":data", signature);

        qDebug() << "Function Name: " << Q_FUNC_INFO << " Signature Data: " << signature;

        ok = query.exec();
        if (!ok) {
            qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
            qCritical() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
        }
    }
    if (isReport)
        return true;

    if (id)
        Database::setStornoId(m_currentReceipt, id);

    DocumentPrinter *p = new DocumentPrinter();
    p->printReceipt(data);
    delete p;

    Journal *journal = new Journal();
    journal->journalInsertReceipt(data);
    delete journal;

    return true;

}

QJsonObject ReceiptItemModel::compileData(int id)
{

    QSqlDatabase dbc = QSqlDatabase::database("CN");

    QSqlQuery query(dbc);
    QJsonObject Root;//root object

    // receiptNum, ReceiptTime
    query.prepare(QString("SELECT `receiptNum`,`timestamp`, `payedBy` FROM receipts WHERE id=%1").arg(m_currentReceipt));
    bool ok = query.exec();

    if (!ok) {
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    query.next();
    int receiptNum = query.value(0).toInt();
    QDateTime receiptTime = query.value(1).toDateTime();
    int payedBy = query.value(2).toInt();

    // Positions
    query.prepare(QString("SELECT COUNT(*) FROM orders WHERE receiptId=%1").arg(m_currentReceipt));
    ok = query.exec();

    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    query.next();
    int positions = query.value(0).toInt();

    // sum Year
    QString year = receiptTime.toString("yyyy");
    query.prepare(QString("SELECT SUM(gross) AS Total FROM receipts where timestamp like '%1%'").arg(year));
    query.exec();
    query.next();
    double sumYear = query.value(0).toDouble();

    // AdvertisingText
    query.prepare("SELECT strValue FROM globals WHERE name='printAdvertisingText'");
    query.exec();
    query.next();
    Root["printAdvertisingText"] = query.value(0).toString();

    // Header
    query.prepare("SELECT strValue FROM globals WHERE name='printHeader'");
    query.exec();
    query.next();
    Root["printHeader"] = query.value(0).toString();

    // Footer
    query.prepare("SELECT strValue FROM globals WHERE name='printFooter'");
    query.exec();
    query.next();
    Root["printFooter"] = query.value(0).toString();

    // TaxTypes

    QSqlQuery taxTypes(dbc);
    taxTypes.prepare(QString("SELECT tax, comment FROM taxTypes WHERE taxlocation=:taxlocation ORDER BY id"));
    query.bindValue(":taxlocation", m_taxlocation);
    taxTypes.exec();
    while(taxTypes.next())
    {
        Root[taxTypes.value(1).toString()] = 0.0;
    }

    // Orders
    QSqlQuery orders(dbc);
    orders.prepare(QString("SELECT orders.count, products.name, orders.gross, orders.tax, products.coupon, orders.discount FROM orders INNER JOIN products ON products.id=orders.product WHERE orders.receiptId=:id"));
    orders.bindValue(":id", m_currentReceipt);

    orders.exec();

    QrkSettings settings;

    // ZNr Programmversion Kassen-Id Beleg Belegtyp Bemerkung Nachbonierung
    // Belegnummer Datum Umsatz_Normal Umsatz_Ermaessigt1 Umsatz_Ermaessigt2
    // Umsatz_Null Umsatz_Besonders Jahresumsatz_bisher Erstellungsdatum

    Root["version"] = QString("%1.%2").arg(QRK_VERSION_MAJOR).arg(QRK_VERSION_MINOR);
    Root["action"] = JOURNAL_RECEIPT;
    Root["kasse"] = Database::getCashRegisterId();
    Root["actionText"] = tr("Beleg");
    Root["typeText"] = Database::getActionType((payedBy > PAYED_BY_START_RECEIPT)?0:payedBy);
    Root["shopName"] = Database::getShopName();
    Root["shopMasterData"] = Database::getShopMasterData();
    Root["headerText"] = m_customerText;
    Root["totallyup"] = (m_totallyup)? "Nachbonierung":"";
    Root["comment"] = (id > 0)? tr("Storno für Beleg Nr: %1").arg(id):settings.value("receiptPrinterHeading", "KASSABON").toString();
    Root["isStorno"] = (id > 0);
    Root["receiptNum"] = receiptNum;
    Root["receiptTime"] = receiptTime.toString(Qt::ISODate);
    Root["currentRegisterYear"] = QDate::currentDate().year();

    QJsonArray Orders;

    double sum = 0;
    QMap<double, double> taxes; // <tax-percent, sum>

    while(orders.next())//load all data from the database
    {
        QBCMath discount = orders.value("discount").toDouble();
        discount.round(2);
        double count = orders.value(0).toDouble();
        count = QString::number(count, 'f', 2).toDouble();
        double singlePrice = orders.value(2).toDouble();
        double gross = singlePrice * count;
        gross = gross - ((gross / 100) * discount.toDouble());
        gross = QString::number(gross, 'f', 2).toDouble();
        double tax = orders.value(3).toDouble();

        sum += gross;

        if ( taxes.contains(tax) )
            taxes[tax] += Utils::getTax(gross,tax);
        else
            taxes[tax] = Utils::getTax(gross,tax);

        QJsonObject order;
        order["count"] = count;
        order["product"] = orders.value(1).toString();
        order["discount"] = discount.toDouble();
        order["gross"] = gross;
        order["singleprice"] = singlePrice;
        order["tax"] = tax;
        order["coupon"] = orders.value(4).toString();
        Orders.append(order);

        QString taxType = Database::getTaxType(tax);
        Root[taxType] = Root[taxType].toDouble() + gross; /* last Info: we need GROSS :)*/

    }

    QJsonArray Taxes;
    QList<double> keys = taxes.keys();
    for (double i = 0.0; i < keys.count(); i++)
    {
        QJsonObject tax;
        tax["t1"] = QString("%1%").arg( keys[i] );
        tax["t2"] = QString::number(taxes[keys[i]], 'f', 2);
        Taxes.append(tax);
    }

    Root["Orders"] = Orders;
    Root["sum"] = sum;
    Root["sumYear"] = sumYear;
    Root["positions"] = positions;
    Root["Taxes"] = Taxes;
    Root["taxesCount"] = taxes.count();

    return Root;

}

void ReceiptItemModel::newOrder( bool  addRow )
{

    m_currentReceipt = 0;  // a new receipt not yet in the DB

    clear();

    setColumnCount(8);
    setHeaderData(REGISTER_COL_COUNT, Qt::Horizontal, QObject::tr("Anzahl"));
    setHeaderData(REGISTER_COL_PRODUCT, Qt::Horizontal, QObject::tr("Artikel"));
    setHeaderData(REGISTER_COL_NET, Qt::Horizontal, QObject::tr("E-Netto"));
    setHeaderData(REGISTER_COL_TAX, Qt::Horizontal, QObject::tr("MwSt."));
    setHeaderData(REGISTER_COL_SINGLE, Qt::Horizontal, QObject::tr("E-Preis"));
    setHeaderData(REGISTER_COL_DISCOUNT, Qt::Horizontal, QObject::tr("Rabatt %"));
    setHeaderData(REGISTER_COL_TOTAL, Qt::Horizontal, QObject::tr("Preis"));
    setHeaderData(REGISTER_COL_SAVE, Qt::Horizontal, QObject::tr(" "));

    if (addRow)
        plus();
}

void ReceiptItemModel::plus()
{
    int row = rowCount();
    insertRow(row);
    blockSignals(true);

    QString defaultTax = Database::getDefaultTax();

//    setColumnCount(8);
    setItem(row, REGISTER_COL_COUNT, new QStandardItem(QString("1")));
    setItem(row, REGISTER_COL_PRODUCT, new QStandardItem(QString("")));
    setItem(row, REGISTER_COL_TAX, new QStandardItem(defaultTax));
    setItem(row, REGISTER_COL_NET, new QStandardItem(QString("0")));
    setItem(row, REGISTER_COL_SINGLE, new QStandardItem(QString("0")));
    setItem(row, REGISTER_COL_DISCOUNT, new QStandardItem("0"));
    setItem(row, REGISTER_COL_TOTAL, new QStandardItem(QString("0")));

    QStandardItem* itemSave = new QStandardItem(false);
    itemSave->setCheckable(true);
    itemSave->setCheckState(Qt::Unchecked);
    itemSave->setEditable(false);
    itemSave->setText(tr("Speichern"));

    setItem(row, REGISTER_COL_SAVE, itemSave);

    item(row ,REGISTER_COL_COUNT)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
    item(row ,REGISTER_COL_TAX)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

    blockSignals(false);
    emit finishedPlus();

}

bool ReceiptItemModel::createNullReceipt(int type)
{

    QString typeText = "";
    int payType = 0;
    switch (type) {
    case START_RECEIPT:
        typeText = "Startbeleg";
        payType = PAYED_BY_START_RECEIPT;
        break;
    case MONTH_RECEIPT:
        typeText = "Monatsbeleg";
        payType = PAYED_BY_MONTH_RECEIPT;
        break;
    case YEAR_RECEIPT:
        typeText = "Jahresbeleg";
        payType = PAYED_BY_MONTH_RECEIPT;
        break;
    case COLLECTING_RECEIPT:
        typeText = QString("Sammelbeleg - Signatureinheit ausgefallen von %1 bis %2").arg(RKSignatureModule::resetSignatureModuleDamaged()).arg(QDateTime::currentDateTime().toString(Qt::ISODate));
        payType = PAYED_BY_COLLECTING_RECEIPT;
        break;
    case CONTROL_RECEIPT:
        typeText = "Kontroll NULL Beleg";
        payType = PAYED_BY_COLLECTING_RECEIPT;
        break;
    case CONCLUSION_RECEIPT:
        typeText = "SchlussBeleg";
        payType = PAYED_BY_CONCLUSION_RECEIPT;
        break;

    default:
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: False type";
        return false;
    }

    bool ret = false;
    newOrder();
    int rc = rowCount();
    if (rc == 0) {
        plus();
        rc = rowCount();
    }

    removeColumn(REGISTER_COL_SAVE);

    QList<QVariant> list;
    list << typeText
         << "0"
         << "0.0"
         << "0.0"
         << "0"
         << "1";

    ret = Database::addProduct(list);
    if (ret) {
        item(rc -1, REGISTER_COL_COUNT)->setText( "1" );
        item(rc -1, REGISTER_COL_PRODUCT)->setText( typeText );
        item(rc -1, REGISTER_COL_TAX)->setText( "0" );
        item(rc -1, REGISTER_COL_SINGLE)->setText( "0" );

    } else {
        return false;
    }

    if (int id = createReceipts()) {
        setCurrentReceiptNum(id);
        if (createOrder()) {
            if (type == START_RECEIPT)
                RKSignatureModule::setDEPactive(true);
            if (finishReceipts( payType )) {
                return true;
            }
        }
    }

    return false;
}

bool ReceiptItemModel::createStartReceipt()
{
    if (RKSignatureModule::isDEPactive())
        return false;

    bool ret = createNullReceipt(START_RECEIPT);

    if (ret)
        return true;

    RKSignatureModule::setDEPactive(false);
    return false;
}

int ReceiptItemModel::createReceipts()
{

    // Check if RKSignatureModule
    if (RKSignatureModule::isDEPactive() && RKSignatureModule::isSignatureModuleSetDamaged()) {
        RKSignatureModule *sigModule = RKSignatureModuleFactory::createInstance("", DemoMode::isDemoMode());
        sigModule->selectApplication();
        int certificateSerial = sigModule->getCertificateSerial(false).toInt();
        delete sigModule;
        if (certificateSerial != 0) {
            ReceiptItemModel *receipt = new ReceiptItemModel();
            receipt->createNullReceipt(COLLECTING_RECEIPT);
            delete receipt;
        }
    }

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    int ok = query.exec(QString("INSERT INTO receipts (timestamp, infodate) VALUES('%1','%2')")
                        .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                        .arg(QDateTime::currentDateTime().toString(Qt::ISODate)
                        ));
    if (!ok) {
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    QString driverName = dbc.driverName();
    if ( driverName == "QMYSQL" ) {
        query.prepare("SELECT LAST_INSERT_ID()");
        query.exec();
    }
    else if ( driverName == "QSQLITE" ) {
        query.prepare("SELECT last_insert_rowid()");
        query.exec();
    }

    query.next();
    m_currentReceipt = query.value(0).toInt();

    return m_currentReceipt;
}

bool ReceiptItemModel::createOrder(bool storno)
{

    bool ret = false;

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc) ;
    query.prepare(QString("INSERT INTO orders (receiptId, product, count, net, discount, gross, tax) SELECT :receiptId, id, :count, :net, :discount, :egross, :tax FROM products WHERE name=:name"));

    int row_count = rowCount();
    for (int row = 0; row < row_count; row++)
    {
        double count = data(index(row, REGISTER_COL_COUNT, QModelIndex())).toDouble();
        if (storno)
            count *= -1;

        QString product = data(index(row, REGISTER_COL_PRODUCT, QModelIndex())).toString();
        double tax = data(index(row, REGISTER_COL_TAX, QModelIndex())).toDouble();
        double egross = data(index(row, REGISTER_COL_SINGLE, QModelIndex())).toDouble();
        double discount = data(index(row, REGISTER_COL_DISCOUNT, QModelIndex())).toDouble();

        Database::updateProductSold(count, product);

        //double net = egross - (egross * tax / (100 + tax));

        double net = egross - Utils::getTax(egross, tax);

        query.bindValue(":receiptId", m_currentReceipt);
        query.bindValue(":count", count);
        query.bindValue(":net", net);
        query.bindValue(":discount", discount);
        query.bindValue(":egross", egross);
        query.bindValue(":tax", tax);
        query.bindValue(":name", product);

        ret = query.exec();

        if (!ret) {
            qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
            qCritical() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
        }
    }

    query.clear();

    return ret;
}

bool ReceiptItemModel::setR2BServerMode(QJsonObject obj)
{
    QString product = QString("Zahlungsbeleg für Rechnung %1 - nicht für den Vorsteuerabzug geeignet" ).arg(obj.value("receiptNum").toString());

    QString gross = Utils::normalizeNumber( obj.value("gross").toString() );

    item(0, REGISTER_COL_COUNT)->setText( "1" );
    item(0, REGISTER_COL_PRODUCT)->setText( product );
    item(0, REGISTER_COL_TAX)->setText( "0" );
    item(0, REGISTER_COL_SINGLE)->setText( gross );

    if (!obj.value("customerText").toString().isEmpty())
        setCustomerText(obj.value("customerText").toString());

    m_isR2B = true;

    QList<QVariant> list;

    list << data(index(0, REGISTER_COL_PRODUCT, QModelIndex())).toString()
         << data(index(0, REGISTER_COL_TAX, QModelIndex())).toString()
         << data(index(0, REGISTER_COL_NET, QModelIndex())).toString()
         << data(index(0, REGISTER_COL_SINGLE, QModelIndex())).toString()
         << "1";

    bool ret = Database::addProduct(list);

    return ret;
}

bool ReceiptItemModel::setReceiptServerMode(QJsonObject obj)
{

    int rc = rowCount();
    QJsonArray receiptItems = obj.value("items").toArray();
    bool ok = false;
    bool ret = false;
    foreach (const QJsonValue & value, receiptItems) {
        QJsonObject jsonItem = value.toObject();
        ok = jsonItem.contains("count") && jsonItem.contains("name") && jsonItem.contains("gross") && jsonItem.contains("tax");
        if(!ok)
            continue;

        QList<QVariant> list;

        QString gross = Utils::normalizeNumber(jsonItem.value("gross").toString());
        QString net = Utils::normalizeNumber(jsonItem.value("net").toString());
        QString tax = Utils::normalizeNumber(jsonItem.value("tax").toString());
        QString count = Utils::normalizeNumber(jsonItem.value("count").toString());
        QString discount;
        if (jsonItem.value("discount").isUndefined())
            discount = "0.00";
        else
            discount = jsonItem.value("discount").toString();

        list << jsonItem.value("name").toString()
             << tax
             << net
             << gross
             << "1";

        ret = Database::addProduct(list);

        if (ret) {

            bool newItem = item(rc -1, REGISTER_COL_PRODUCT)->text().isEmpty();
            if (!newItem) {
                plus();
                rc = rowCount();
            }

            item(rc -1, REGISTER_COL_COUNT)->setText( count );
            item(rc -1, REGISTER_COL_PRODUCT)->setText( jsonItem.value("name").toString() );
            item(rc -1, REGISTER_COL_TAX)->setText( tax );
            item(rc -1, REGISTER_COL_DISCOUNT)->setText( discount );
            item(rc -1, REGISTER_COL_SINGLE)->setText( gross );

        } else {
            ret = false;
            break;
        }
    }

    if (!obj.value("customerText").toString().isEmpty())
        setCustomerText(obj.value("customerText").toString());

    return ret;
}

void ReceiptItemModel::itemChangedSlot( const QModelIndex& i, const QModelIndex&)
{

    int row = i.row();
    int col = i.column();

    double sum(0.0);
    double net(0.0);
    double tax(0.0);

    QString s = data(index(row, col, QModelIndex())).toString();

    //  QString s2;
    QStringList temp = data(index(row, REGISTER_COL_SINGLE, QModelIndex())).toString().split(" ");
    double d2 = temp[0].replace(",", ".").toDouble();

    /* initialize QSqlDatabase dbc & QSqlQuery query(dbc)
   * will not work in a switch block
   */

    if (col == REGISTER_COL_PRODUCT) {
        QSqlDatabase dbc = QSqlDatabase::database("CN");
        QSqlQuery query(dbc);
        query.prepare(QString("SELECT gross, tax FROM products WHERE name=:name"));
        query.bindValue(":name", s);
        query.exec();
        if (query.next()) {
            item(row, REGISTER_COL_TAX)->setText(query.value(1).toString());
            item(row, REGISTER_COL_SINGLE)->setText(query.value(0).toString());
        } else {
            item(row, REGISTER_COL_SINGLE)->setText("0");
        }

        emit setButtonGroupEnabled(! s.isEmpty());
    }

    switch( col )
    {
    case REGISTER_COL_COUNT:
        if (s.toDouble() == 0) {
            sum = 0;
        } else {
            sum =  s.toDouble() * d2;
            sum = sum - ((sum / 100) * data(index(row, REGISTER_COL_DISCOUNT, QModelIndex())).toDouble());
        }

        s = QString::number(sum, 'f', 2);
//        if (s.toDouble() == 0)
            blockSignals(true);

        item(row, REGISTER_COL_TOTAL)->setText( s );

//        if (s.toDouble() == 0)
            blockSignals(false);

        break ;
    case REGISTER_COL_TAX:
        net = data(index(row, REGISTER_COL_NET, QModelIndex())).toDouble();
        sum = net * (1.0 + s.replace(" %", "").toDouble() / 100.0);
        item(row, REGISTER_COL_SINGLE)->setText( QString("%1").arg(sum) );
        break;
    case REGISTER_COL_NET:
        s.replace(",", ".");
        blockSignals(true);
        tax = data(index(row, REGISTER_COL_TAX, QModelIndex())).toDouble();
        s = QString::number(s.toDouble() * ((100 + tax) / 100), 'f', 2);
        item(row, REGISTER_COL_SINGLE)->setText( s );
        sum = s.toDouble() * data(index(row, REGISTER_COL_COUNT, QModelIndex())).toDouble();
        sum = sum - ((sum / 100) * data(index(row, REGISTER_COL_DISCOUNT, QModelIndex())).toDouble());
        s = QString::number(sum, 'f', 2);
        item(row, REGISTER_COL_TOTAL)->setText( s );
        blockSignals(false);
        break;
    case REGISTER_COL_SINGLE:
        s.replace(",", ".");
        blockSignals(true);
        item(row, REGISTER_COL_SINGLE)->setText( s );
        tax = data(index(row, REGISTER_COL_TAX, QModelIndex())).toDouble();
        net = s.toDouble() / (1.0 + tax / 100.0);
        item(row, REGISTER_COL_NET)->setText( QString("%1").arg(net) );
        sum = s.toDouble() * data(index(row, REGISTER_COL_COUNT, QModelIndex())).toDouble();
        sum = sum - ((sum / 100) * data(index(row, REGISTER_COL_DISCOUNT, QModelIndex())).toDouble());
        s = QString::number(sum, 'f', 2);
        item(row, REGISTER_COL_TOTAL)->setText( s );
        blockSignals(false);
        break ;
    case REGISTER_COL_DISCOUNT:
        blockSignals(true);
        s.replace(",", ".");
        sum = data(index(row, REGISTER_COL_SINGLE, QModelIndex())).toDouble();
        sum = sum * data(index(row, REGISTER_COL_COUNT, QModelIndex())).toDouble();
        sum = sum - ((sum / 100) * s.toDouble());
        s = QString::number(sum, 'f', 2);
        item(row, REGISTER_COL_TOTAL)->setText( s );
        blockSignals(false);
        break ;
    case REGISTER_COL_TOTAL:
        s.replace(",", ".");
        s = QString::number(s.toDouble(), 'f', 2);
        blockSignals(true);
        item(row, REGISTER_COL_TOTAL)->setText( s );
        sum = s.toDouble() / data(index(row, REGISTER_COL_COUNT, QModelIndex())).toDouble();
        tax = data(index(row, REGISTER_COL_TAX, QModelIndex())).toDouble();
        net = sum / (1.0 + tax / 100.0);
        item(row, REGISTER_COL_NET)->setText( QString("%1").arg(net) );
        s = QString::number(sum, 'f', 2);
        item(row, REGISTER_COL_SINGLE)->setText( s );
        blockSignals(false);
        break ;
    }

    emit finishedItemChanged();

}

bool ReceiptItemModel::storno(int id)
{

    int rc = rowCount();

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT orders.count, products.name, orders.tax, orders.net, orders.gross, orders.discount FROM orders INNER JOIN products ON products.id=orders.product WHERE orders.receiptId=:id");
    query.bindValue(":id", id);

    bool ret = query.exec();

    if (!ret) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    while (query.next()) {
        bool newItem = item(rc -1, REGISTER_COL_PRODUCT)->text().isEmpty();
        if (!newItem) {
            plus();
            rc = rowCount();
        }

        item(rc -1, REGISTER_COL_COUNT)->setText(query.value(0).toString());
        item(rc -1, REGISTER_COL_PRODUCT)->setText(query.value(1).toString());
        item(rc -1, REGISTER_COL_TAX)->setText(query.value(2).toString());
        item(rc -1, REGISTER_COL_DISCOUNT)->setText(query.value("discount").toString());
        item(rc -1, REGISTER_COL_NET)->setText(query.value(3).toString());
        item(rc -1, REGISTER_COL_SINGLE)->setText(query.value(4).toString());
    }

    return ret;
}
