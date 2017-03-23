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

#include "database.h"
#include "utils/utils.h"
#include "reports.h"
#include "documentprinter.h"
#include "backup.h"
#include "singleton/spreadsignal.h"
#include "RK/rk_signaturemodule.h"

#include <QApplication>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QJsonObject>
#include <QTextDocument>
#include <QDebug>

Reports::Reports(QObject *parent, bool mode)
    : ReceiptItemModel(parent), m_servermode(mode)
{
//    SpreadSignal::setProgressBarValue(1);
    m_journal = new Journal();
}

//--------------------------------------------------------------------------------

Reports::~Reports()
{
    SpreadSignal::setProgressBarValue(-1);
    delete m_journal;
}


/**
 * @brief Reports::getEOFMap
 * @param checkDate
 * @return
 */
QMap<int, QDate> Reports::getEOFMap(QDate checkDate)
{
    QDate last = Database::getLastReceiptDate();
    QMap<int, QDate> map;
    QDate lastEOD = getLastEOD();
    int type = getReportType();

    if (type == 4 || type == 8){
        type = 4;
        last = lastEOD;
    }

    if (type == -1)
        return QMap<int, QDate>();

    // Tagesabschluss von Heute schon gemacht?
    if (lastEOD.isValid() && lastEOD == checkDate) {
        map.insert(3, QDate());
        return map;
    }

    if (!(type ==  3) && !(type == 4)&& checkDate != last)
        map.insert(3, last);

    QString lastMonth = last.toString("yyyyMM");
    QString checkMonth = checkDate.toString("yyyyMM");

    // Monatsabschluss von diesen Monat schon gemacht?
    if (type == 4 && lastMonth == checkMonth) {
        map.insert(4, QDate());
        return map;
    }
    if (lastEOD.isValid() && lastEOD > checkDate) {
        map.insert(4, QDate());
        return map;
    }

    if (!(lastMonth == checkMonth) && !(type ==  4) && checkDate != last)
        map.insert(4, last);

    if ((last.addMonths(1).month() < checkDate.month()) && (type ==  4) && checkDate != last)
        map.insert(4, last.addMonths(1));


    return map;
}

/**
 * @brief Reports::checkEOAny
 * @param checkDate
 * @return
 */
bool Reports::checkEOAny(QDate checkDate, bool checkDay)
{
    bool ret = true;
    QMap<int, QDate> map = getEOFMap(checkDate);
    if (map.isEmpty())
        return true;

    if (map.contains(3) && checkDay) {
        QDate date = map.value(3);
        if (!date.isValid()) {
            if (!m_servermode)
                checkEOAnyMessageBoxInfo(3, QDate::currentDate(), tr("Tagesabschluss wurde bereits erstellt."));
            return false;
        }
    }

    if (map.contains(4)) {
        QDate date = map.value(4);
        if (!date.isValid()) {
            if (!m_servermode)
                checkEOAnyMessageBoxInfo(4, QDate::currentDate(), tr("Monatsabschluss %1 wurde bereits erstellt.").arg( QDate::longMonthName(QDate::currentDate().month())));
            return false;
        }
    }

    QMapIterator<int, QDate> i(map);
    while (ret && i.hasNext()) {
        i.next();

        if (!m_servermode)
            ret = checkEOAnyMessageBoxYesNo(i.key(), i.value());

        if(ret) {
            if (i.key() == 3 && checkDay) {
                ret = endOfDay();
            } else if (i.key() == 4) {
                ret = endOfMonth();
            }
        }
    }

    return ret;
}

bool Reports::checkEOAnyServerMode()
{
    bool ret = checkEOAny();
    return ret;
}

/**
 * @brief Reports::checkEOAnyMessageBoxYesNo
 * @param type
 * @param date
 * @param text
 * @return
 */
bool Reports::checkEOAnyMessageBoxYesNo(int type, QDate date, QString text)
{
    QString infoText;
    if (type == 3) {
        infoText = tr("Tagesabschluss");
        if (text.isEmpty()) text = tr("Tagesabschluss vom %1 muß erstellt werden.").arg(date.toString());
    } else {
        infoText = tr("Monatsabschluss");
        if (text.isEmpty()) text = tr("Monatsabschluss für %1 muß erstellt werden.").arg(date.toString("MMMM yyyy"));
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle(infoText);

    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, tr("Erstellen"));
    msgBox.setButtonText(QMessageBox::No, tr("Abbrechen"));
    msgBox.setDefaultButton(QMessageBox::No);

    if(msgBox.exec() == QMessageBox::Yes)
        return true;

    return false;
}

/**
 * @brief Reports::checkEOAnyMessageBoxInfo
 * @param type
 * @param date
 * @param text
 */
void Reports::checkEOAnyMessageBoxInfo(int type, QDate date, QString text)
{
    QString infoText;
    if (type == 3) {
        infoText = tr("Tagesabschluss");
    } else {
        infoText = tr("Monatsabschluss");
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle(infoText);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(text);
    msgBox.setInformativeText(tr("Erstellungsdatum %1").arg(date.toString()));
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.setButtonText(QMessageBox::Yes, tr("OK"));
    msgBox.exec();
}

bool Reports::endOfDay() {
    return endOfDay(true);
}

/**
 * @brief Reports::endOfDay
 * @return
 */
bool Reports::endOfDay(bool ask)
{
    QDate date = Database::getLastReceiptDate();

    bool create = canCreateEOD(date);
    if (create) {
        if (m_servermode)
            return doEndOfDay(date);

        QString text;
        bool ok = true;
        if (ask && date == QDate::currentDate()) {
            text = tr("Nach dem Erstellen des Tagesabschlusses ist eine Bonierung für den heutigen Tag nicht mehr möglich.");
            ok = checkEOAnyMessageBoxYesNo(3, date, text);
        }

        if (ok) {
            return doEndOfDay(date);
        } else {
            return false;
        }
    } else {
        if (!m_servermode)
            checkEOAnyMessageBoxInfo(3, date, tr("Tagesabschluss wurde bereits erstellt."));
        return false;
    }

    return false;
}

/**
 * @brief Reports::doEndOfDay
 * @param date
 * @return
 */
bool Reports::doEndOfDay(QDate date)
{
    SpreadSignal::setProgressBarValue(1);
    Backup::create();
    m_currentReceipt = createReceipts();
    finishReceipts(3, 0, true);
    createEOD(m_currentReceipt, date);
    printDocument(m_currentReceipt, tr("Tagesabschluss"));
    return true;
}

/**
 * @brief Reports::endOfMonth
 * @return
 */
bool Reports::endOfMonth()
{
    QDate rDate = Database::getLastReceiptDate();
    int type = getReportType();

    if (type == 3) {
        rDate = getLastEOD();
    }

    if (type == 4 || type == 8) {
        rDate = getLastEOD().addMonths(1);
    }

    int receiptMonth = (rDate.year() * 100) + rDate.month();
    int currMonth = (QDate::currentDate().year() * 100) + QDate::currentDate().month();

    bool ok = (rDate.isValid() && receiptMonth <= currMonth);

    if (ok) {
        QDateTime checkdate = QDateTime::currentDateTime();

        if (QDate::currentDate().year() == rDate.year() && QDate::currentDate().month() > 1) {
            checkdate.setDate(QDate::fromString(QString("%1-%2-1")
                                                .arg(rDate.year())
                                                .arg(rDate.month())
                                                , "yyyy-M-d")
                              .addMonths(1).addDays(-1));
        } else {
            checkdate.setDate(QDate::fromString(QString("%1-12-31")
                                                .arg(QDate::currentDate().year()),"yyyy-M-d")
                              .addYears(-1));
        }
        checkdate.setTime(QTime::fromString("23:59:59"));

//        QDateTime dateTime = Database::getLastReceiptDateTime();

        bool canCreateEom = canCreateEOM(rDate);

        if (!(type == 4) && !(type == 8)) {
            bool canCreateEod = canCreateEOD(rDate);

            if (QDateTime(rDate) <= checkdate && canCreateEod) {
                bool doJob = true;
                if (!m_servermode)
                    doJob = checkEOAnyMessageBoxYesNo(3, rDate,tr("Der Tagesabschlusses für %1 muß zuerst erstellt werden.").arg(rDate.toString()));

                if (doJob) {
                    if (! endOfDay())
                        return false;
                } else {
                    return false;
                }
            }
        }

        if (canCreateEom) {
            ok = true;
            if (!m_servermode && receiptMonth == currMonth) {
                QString text = tr("Nach dem Erstellen des Monatsabschlusses ist eine Bonierung für diesen Monat nicht mehr möglich.");
                ok = checkEOAnyMessageBoxYesNo(4, rDate, text);
            }
            if (ok) {
                doEndOfMonth(checkdate.date());
                rDate = rDate.addMonths(1);
                receiptMonth = (rDate.year() * 100) + rDate.month();
                if (rDate.isValid() && receiptMonth < currMonth)
                    ok = checkEOAny();
            }
        }
    } else {
        QDate next = QDate::currentDate();
        next.setDate(next.year(), next.addMonths(1).month(), 1);
        QString text = tr("Der Monatsabschluss kann erst ab %1 gemacht werden.").arg(next.toString());
        checkEOAnyMessageBoxInfo(4, QDate::currentDate(), text);
    }

    return ok;
}

/**
 * @brief Reports::doEndOfMonth
 * @param date
 * @return
 */
bool Reports::doEndOfMonth(QDate date)
{
    SpreadSignal::setProgressBarValue(1);
    bool ret = false;
    Backup::create();
    clear();
    m_currentReceipt =  createReceipts();
    ret = finishReceipts(4, 0, true);
    if (ret) {
        createEOM(m_currentReceipt, date);
        printDocument(m_currentReceipt, tr("Monatsabschluss"));
    }

    if (RKSignatureModule::isDEPactive()) {
        if (date.year() < QDate::currentDate().year() || date.month() == 12)
            createNullReceipt(YEAR_RECEIPT);
        else
            createNullReceipt(MONTH_RECEIPT);
    }

    return ret;
}


/**
 * @brief Reports::createEOD
 * @param id
 * @param date
 */
void Reports::createEOD(int id, QDate date)
{
//    QApplication::setOverrideCursor(Qt::WaitCursor);

//    QElapsedTimer timer;
//    timer.start();
    QDateTime from;
    QDateTime to;

    // ---------- DAY -----------------------------
    from.setDate(QDate::fromString(date.toString()));
    to.setDate(QDate::fromString(date.toString()));
    to.setTime(QTime::fromString("23:59:59"));

    QStringList eod;
    eod.append(createStat(id, "Tagesumsatz", from, to));

    QString line = QString("Tagesbeleg\tTagesbeleg\t\t%1\t%2\t0,0\t0,0\t0,0\t0,0\t0,0\t%3")
            .arg(id)
            .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
            .arg(Utils::getYearlyTotal(date.year()));

    insert(eod, id, to);

    m_journal->journalInsertLine("Beleg", line);

    SpreadSignal::setProgressBarValue(100);

}

/**
 * @brief Reports::createEOM
 * @param id
 * @param date
 */
void Reports::createEOM(int id, QDate date)
{
//    QApplication::setOverrideCursor(Qt::WaitCursor);

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery q(dbc);

    QDateTime from;
    QDateTime to;

    // ---------- MONTH -----------------------------
    //from.setDate(QDate::fromString(date.toString()));
    from.setDate(QDate::fromString(QString("%1-%2-01").arg(date.year()).arg(date.month()),"yyyy-M-d"));
    to.setDate(QDate::fromString(date.toString()));
    to.setTime(QTime::fromString("23:59:59"));

    QStringList eod;
    eod.append(createStat(id, "Monatsumsatz", from, to));

    // ----------- YEAR ---------------------------

    QString fromString = QString("%1-01-01").arg(date.year());
    from.setDate(QDate::fromString(fromString, "yyyy-MM-dd"));
    to.setDate(QDate::fromString(date.toString()));
    to.setTime(QTime::fromString("23:59:59"));

    if (date.month() == 12) {
        eod.append(createYearStat(id, date));
    }

    // ----------------------------------------------

    QString line = QString("Monatsbeleg\tMonatsbeleg\t\t%1\t%2\t0,0\t0,0\t0,0\t0,0\t0,0\t%3")
            .arg(id)
            .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
            .arg(Utils::getYearlyTotal( date.year() ));

    insert(eod, id, to);

    m_journal->journalInsertLine("Beleg", line);

    SpreadSignal::setProgressBarValue(100);

}

/**
 * @brief Reports::getLastEOD
 * @return
 */
QDate Reports::getLastEOD()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT max(timestamp) AS timestamp FROM reports");
    bool ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    if (query.last()) {
        return query.value(0).toDate();
    }

    return QDate();
}

/**
* @brief Reports::canCreateEOD
* @param date
* @return
*/
bool Reports::canCreateEOD(QDate date)
{
    QDateTime f;
    QDateTime t;
    f.setDate(QDate::fromString(date.toString()));
    t.setDate(QDate::fromString(date.toString()));
    f.setTime(QTime::fromString("00:00:00"));
    t.setTime(QTime::fromString("23:59:59"));

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT reports.timestamp FROM reports, receipts where reports.timestamp BETWEEN :fromDate AND :toDate AND receipts.payedBy > 2 AND reports.receiptNum=receipts.receiptNum ORDER BY receipts.timestamp DESC LIMIT 1");
    query.bindValue(":fromDate", f.toString(Qt::ISODate));
    query.bindValue(":toDate", t.toString(Qt::ISODate));

    bool ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    if (query.last()) {
        date = query.value(0).toDate();
        return false;
    }

    return true;
}

/**
 * @brief Reports::canCreateEOM
 * @param date
 * @return
 */
bool Reports::canCreateEOM(QDate date)
{
    QDateTime f;
    QDateTime t;
    f.setDate(QDate::fromString(date.toString()));
    t.setDate(QDate::fromString(date.toString()));
    f.setTime(QTime::fromString("00:00:00"));
    t.setTime(QTime::fromString("23:59:59"));

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT reports.timestamp FROM reports, receipts where reports.timestamp BETWEEN :fromDate AND :toDate AND receipts.payedBy = 4 AND reports.receiptNum=receipts.receiptNum ORDER BY receipts.timestamp DESC LIMIT 1");
    query.bindValue(":fromDate", f.toString(Qt::ISODate));
    query.bindValue(":toDate", t.toString(Qt::ISODate));

    bool ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    if (query.last()) {
        date = query.value(0).toDate();
        return false;
    }

    return true;
}

/**
 * @brief Reports::getReportType
 * @return
 */
int Reports::getReportType()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("select payedBy,receiptNum from receipts where id=(select max(id) from receipts);");
    bool ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    if (query.last()) {
        if (query.value(0).isNull())
            return -1;
        return query.value(0).toInt();
    }
    return -1;
}

/**
 * @brief Reports::createStat
 * @param id
 * @param type
 * @param from
 * @param to
 * @return
 */
QStringList Reports::createStat(int id, QString type, QDateTime from, QDateTime to)
{

//    QElapsedTimer timer;
//    timer.start();

    QString toS = to.toString("yyyyMMdd");
    QString toS2 = QDate::currentDate().toString("yyyyMMdd");

    if (to.toString("yyyyMMdd") == QDate::currentDate().toString("yyyyMMdd"))
        to.setTime(QTime::currentTime());

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    /* Anzahl verkaufter Artikel oder Leistungen */
    query.prepare("SELECT sum(orders.count) FROM orders WHERE receiptId IN (SELECT id FROM receipts WHERE timestamp BETWEEN :fromDate AND :toDate AND payedBy <= 2)");
    query.bindValue(":fromDate", from.toString(Qt::ISODate));
    query.bindValue(":toDate", to.toString(Qt::ISODate));
    bool ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    query.next();

    double sumProducts = query.value(0).toDouble();

    QStringList stat;
    stat.append(QString("Anzahl verkaufter Artikel oder Leistungen: %1").arg(QString::number(sumProducts,'f',2).replace(".",",")));

    /* Anzahl Zahlungen */
    query.prepare("SELECT count(id) FROM receipts WHERE timestamp BETWEEN :fromDate AND :toDate AND payedBy <= 2 AND storno < 2");
    query.bindValue(":fromDate", from.toString(Qt::ISODate));
    query.bindValue(":toDate", to.toString(Qt::ISODate));
    ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    query.next();

    stat.append(QString("Anzahl Zahlungen: %1").arg(query.value(0).toInt()));

    /* Anzahl Stornos */
    query.prepare("SELECT count(id) FROM receipts WHERE timestamp BETWEEN :fromDate AND :toDate AND storno = 2");
    query.bindValue(":fromDate", from.toString(Qt::ISODate));
    query.bindValue(":toDate", to.toString(Qt::ISODate));

    ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    query.next();

    stat.append(QString("Anzahl Stornos: %1").arg(query.value(0).toInt()));
    stat.append("-");

    /* Umsätze Zahlungsmittel */
    // query = QString("SELECT c.actionText, a.tax, a.gross FROM (SELECT * from orders where receiptId IN (select id from receipts where timestamp between '%1' AND '%2')) AS a INNER JOIN receipts AS b ON a.receiptId = b.id INNER JOIN actionTypes AS c ON c.actionId=b.payedBy GROUP BY b.payedBy, a.tax").arg(from.toString(Qt::ISODate)).arg(to.toString(Qt::ISODate));
    query.prepare("SELECT actionTypes.actionText, orders.tax, SUM(orders.count * orders.gross) from orders "
                  " LEFT JOIN receipts on orders.receiptId=receipts.receiptNum"
                  " LEFT JOIN actionTypes on receipts.payedBy=actionTypes.actionId"
                  " WHERE receipts.timestamp between :fromDate AND :toDate AND receipts.payedBy < 3"
                  " GROUP BY orders.tax, receipts.payedBy ORDER BY receipts.payedBy, orders.tax");
    query.bindValue(":fromDate", from.toString(Qt::ISODate));
    query.bindValue(":toDate", to.toString(Qt::ISODate));

    ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    stat.append(tr("Umsätze nach Zahlungsmittel"));

    QString zm = "";
    while (query.next())
    {
        if (!(zm == query.value(0).toString())){
            zm = query.value(0).toString();
            stat.append(QString("%1").arg(query.value(0).toString()));
        }
        stat.append(QString("%1%: %2")
                    .arg(query.value(1).toInt())
                    .arg(QString::number(query.value(2).toDouble(), 'f', 2).replace(".",",")));
    }
    stat.append("-");

    /* Umsätze Steuern */
    query.prepare("SELECT orders.tax, SUM(orders.count * orders.gross) from receipts LEFT JOIN orders on orders.receiptId=receipts.receiptNum WHERE receipts.timestamp between :fromDate AND :toDate AND receipts.payedBy < 3 GROUP by orders.tax ORDER BY orders.tax");
    query.bindValue(":fromDate", from.toString(Qt::ISODate));
    query.bindValue(":toDate", to.toString(Qt::ISODate));

    ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    stat.append(tr("Umsätze nach Steuersätzen"));
    while (query.next())
    {
        stat.append(QString("%1%: %2")
                    .arg(query.value(0).toString())
                    .arg(QString::number(query.value(1).toDouble(), 'f', 2).replace(".",",")));
    }
    stat.append("-");

    /* Summe */
    query.prepare("SELECT sum(gross) FROM receipts WHERE timestamp BETWEEN :fromDate AND :toDate AND payedBy < 3");
    query.bindValue(":fromDate", from.toString(Qt::ISODate));
    query.bindValue(":toDate", to.toString(Qt::ISODate));
    ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    query.next();

    double gross = QString::number(query.value(0).toDouble(),'f',2).toDouble();
    QString sales = QString::number(query.value(0).toDouble(),'f',2).replace(".",",");

    if (type == "Jahresumsatz") {
        m_yearsales = sales;
    } else {
        QJsonObject data;
        data["receiptNum"] = id;
        data["receiptTime"] = to.toString(Qt::ISODate);

        query.prepare("UPDATE receipts SET gross=:gross, timestamp=:timestamp, infodate=:infodate WHERE receiptNum=:receiptNum");
        query.bindValue(":gross", gross);
        query.bindValue(":timestamp", QDateTime::currentDateTime().toString(Qt::ISODate));
        query.bindValue(":infodate", to.toString(Qt::ISODate));
        query.bindValue(":receiptNum", id);

        bool ok = query.exec();

        if (!ok) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
        }
    }

    stat.append(QString("%1: %2").arg(type).arg(sales));
    stat.append("=");

    query.prepare("SELECT sum(orders.count) AS count, products.name, orders.gross, sum(orders.count * orders.gross) AS total, orders.tax FROM orders LEFT JOIN products ON orders.product=products.id  LEFT JOIN receipts ON receipts.receiptNum=orders.receiptId WHERE receipts.timestamp BETWEEN :fromDate AND :toDate GROUP BY products.name, orders.gross ORDER BY orders.tax, products.name ASC");
    query.bindValue(":fromDate", from.toString(Qt::ISODate));
    query.bindValue(":toDate", to.toString(Qt::ISODate));
    //  qInfo() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);

    ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    stat.append(tr("Verkaufte Artikel oder Leistungen (Gruppiert) Gesamt %1").arg(QString::number(sumProducts,'f',2).replace(".",",")));
    while (query.next())
    {
        stat.append(QString("%1: %2: %3: %4: %5%")
                    .arg(query.value(0).toString())
                    .arg(query.value(1).toString())
                    .arg(QString::number(query.value(2).toDouble(),'f',2).replace(".",","))
                    .arg(QString::number(query.value(3).toDouble(),'f',2).replace(".",","))
                    .arg(query.value(4).toDouble()));
    }

    return stat;
}

/**
 * @brief Reports::insert
 * @param list
 * @param id
 */
void Reports::insert(QStringList list, int id, QDateTime to)
{

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    int count = list.count();
    SpreadSignal::setProgressBarValue(0);

    int i=0;
    QString line;
    foreach (line, list) {
        m_journal->journalInsertLine("Textposition", line);
        query.prepare("INSERT INTO reports (receiptNum, timestamp, text) VALUES(:receiptNum, :timestamp, :text)");
        query.bindValue(":receiptNum", id);
        query.bindValue(":timestamp", to.toString(Qt::ISODate));
        query.bindValue(":text", line);

        bool ok = query.exec();
        if (!ok) {
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
            qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
        }

        SpreadSignal::setProgressBarValue(((float)i++ / (float)count) * 100 );

    }
}

/**
 * @brief Reports::createYearStat
 * @param id
 * @param date
 * @return
 */
QStringList Reports::createYearStat(int id, QDate date)
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery q(dbc);

    QDateTime from;
    QDateTime to;

    // ----------- YEAR ---------------------------
    QStringList eoy;
    QString fromString = QString("%1-01-01").arg(date.year());
    from.setDate(QDate::fromString(fromString, "yyyy-MM-dd"));
    to.setDate(QDate::fromString(date.toString()));
    to.setTime(QTime::fromString("23:59:59"));

    eoy.append(QString("Jahressummen %1:").arg(date.year()));
    eoy.append("-");

    eoy.append(createStat(id, "Jahresumsatz", from, to));

    return eoy;
}

/**
 * @brief Reports::getReport
 * @param id
 * @param test
 * @return
 */
QString Reports::getReport(int id, bool test)
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare("SELECT receipts.payedBy, reports.timestamp FROM receipts JOIN reports ON receipts.receiptNum=reports.receiptNum WHERE receipts.receiptNum=:id");
    query.bindValue(":id", id);
    bool ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    query.next();

    int type = query.value(0).toInt();
    QString format = (type == 4)? "MMMM yyyy": "dd MMMM yyyy";

    QString header;
    if (test)
        header = QString("TESTDRUCK für SCHRIFTART");
    else
        header = QString("BON # %1, %2 - %3").arg(id).arg(Database::getActionType(type)).arg(query.value(1).toDate().toString(format));

    query.prepare("SELECT text FROM reports WHERE receiptNum=:id");
    query.bindValue(":id", id);
    ok = query.exec();
    if (!ok) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << query.lastError().text();
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Query: " << Database::getLastExecutedQuery(query);
    }

    QString text;

    text.append("<!DOCTYPE html><html><head>\n");
    text.append("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
    text.append("</head><body>\n<table cellpadding=\"3\" cellspacing=\"1\" width=\"100%\">\n");

    int x = 0;
    int span = 5;
    bool needOneMoreCol = false;

    text.append(QString("<tr><th colspan=\"%1\">%2</th></tr>").arg(span).arg(header) );
    text.append(QString("<tr><th colspan=\"%1\"></th></tr>").arg(span));

    while (query.next()){
        needOneMoreCol = false;
        span = 5;

        QString t = query.value(0).toString();
        x++;
        QString color = "";
        if (x % 2 == 1)
            color = "bgcolor='#F5F5F5'";
        else
            color = "bgcolor='#FFFFFF'";

        text.append("<tr>");

        QStringList list;

        if (t.indexOf('-') == 0 && t.size() == 1) {
            t.replace('-',"<hr>");
            text.append(QString("<td colspan=\"%1\" %2>%3</td>").arg(span).arg(color).arg(t));
            needOneMoreCol = false;
        } else if (t.indexOf('=') == 0  && t.size() == 1) {
            t.replace('=',"<hr size=\"5\">");
            text.append(QString("<td colspan=\"%1\" %2>%3</td>").arg(span).arg(color).arg(t));
            needOneMoreCol = false;
        } else if (t.indexOf(QRegularExpression("[0-9]{1,2}%:")) != -1) {
            list = t.split(":");
            span = span - list.count();
            foreach (const QString &str, list) {
                if (test)
                    text.append(QString("<td align=\"right\" colspan=\"%1\" %2>%3</td>").arg(span).arg(color).arg("0,00"));
                else
                    text.append(QString("<td align=\"right\" colspan=\"%1\" %2>%3</td>").arg(span).arg(color).arg(str));
                span = 1;
            }
            needOneMoreCol = true;
        } else if (t.indexOf(QRegularExpression("^-*\\d+:")) != -1) {
            list = t.split(": ",QString::SkipEmptyParts);
            span = span - list.count();
            int count = 0;

            QString align = "left";
            foreach (const QString &str, list) {
                if (count > 1) align="right";
                if (test && count > 1)
                    text.append(QString("<td align=\"%1\" colspan=\"%2\" %3>%4</td>").arg(align).arg(span).arg(color).arg("0,00"));
                else
                    text.append(QString("<td align=\"%1\" colspan=\"%2\" %3>%4</td>").arg(align).arg(span).arg(color).arg(str));

                count++;
                span = 1;
            }
            needOneMoreCol = false;
        } else {
            list = t.split(":",QString::SkipEmptyParts);
            span = span - list.count();
            int count = 0;

            QString align = "left";
            foreach (const QString &str, list) {
                if (count > 0) align="right";
                if (test && count > 0)
                    text.append(QString("<td align=\"%1\" colspan=\"%2\" %3>%4</td>").arg(align).arg(span).arg(color).arg("0,00"));
                else
                    text.append(QString("<td align=\"%1\" colspan=\"%2\" %3>%4</td>").arg(align).arg(span).arg(color).arg(str));

                count++;
                span = 1;
            }
            needOneMoreCol = true;
        }

        if (needOneMoreCol)
            text.append(QString("<td colspan=\"%1\" %2></td>").arg(span).arg(color));

        text.append("</tr>");
    }
    text.append("</table></body></html>\n");

    return text;
}

void Reports::printDocument(int id, QString title)
{
    QString DocumentTitle = QString("BON_%1_%2").arg(id).arg(title);
    QTextDocument doc;
    doc.setHtml(Reports::getReport(id));
    DocumentPrinter *p = new DocumentPrinter();
    p->printDocument(&doc, DocumentTitle);
    delete p;
}
