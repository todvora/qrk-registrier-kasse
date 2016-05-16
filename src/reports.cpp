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

#include "reports.h"
#include "backup.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QJsonObject>

#include <QDebug>

Reports::Reports(DEP *dep, QProgressBar *pb, QObject *parent, bool mode)
  : QObject(parent), dep(dep), servermode(mode)
{
  this->pb = pb;
  pb->setValue(1);

}

//--------------------------------------------------------------------------------

Reports::~Reports()
{
}

//--------------------------------------------------------------------------------

QDate Reports::getLastEOD()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);
  QString query = QString("SELECT max(timestamp) AS timestamp FROM receipts where payedBy > 2");

  try {
    q.prepare(query);
    q.exec();

    if (q.last()) {
      return q.value(0).toDate();
    }
  } catch (...) {
    qDebug() << "Reports::getLastEOT Error: " << q.lastError().text();
  }

  return QDate();

}

//--------------------------------------------------------------------------------

bool Reports::canCreateEOD(QDate date)
{

  QDateTime f;
  QDateTime t;
  f.setDate(QDate::fromString(date.toString()));
  t.setDate(QDate::fromString(date.toString()));
  f.setTime(QTime::fromString("00:00:00"));
  t.setTime(QTime::fromString("23:59:59"));

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);
  QString query = QString("SELECT timestamp FROM receipts where timestamp BETWEEN '%1' AND '%2' AND payedBy > 2 ORDER BY timestamp DESC LIMIT 1").arg(f.toString(Qt::ISODate)).arg(t.toString(Qt::ISODate));

  // qDebug() << "Reports::canCreateEOD query: " << query;
  try {
    q.prepare(query);
    q.exec();

    if (q.last()) {
      date = q.value(0).toDate();
      return false;
    }
  } catch (QSqlError e) {
    qDebug() << "Reports::canCreateEOT Error: " << e.text();
  }

  return true;
}

//--------------------------------------------------------------------------------

int Reports::getReportType()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);
  QString query = QString("SELECT payedBy, max(id) FROM receipts");
  q.prepare(query);
  q.exec();

  if (q.last()) {
    if (q.value(0).isNull())
      return -1;

    return q.value(0).toInt();
  }
  return -1;
}

//--------------------------------------------------------------------------------

bool Reports::canCreateEOM(QDate date)
{
  QDateTime f;
  QDateTime t;
  f.setDate(QDate::fromString(date.toString()));
  t.setDate(QDate::fromString(date.toString()));
  f.setTime(QTime::fromString("00:00:00"));
  t.setTime(QTime::fromString("23:59:59"));

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);
  QString query = QString("SELECT timestamp FROM receipts where timestamp BETWEEN '%1' AND '%2' AND payedBy = 4 ORDER BY timestamp DESC LIMIT 1").arg(f.toString(Qt::ISODate)).arg(t.toString(Qt::ISODate));

  // qDebug() << "Reports::canCreateEOD query: " << query;
  try {
    q.prepare(query);
    q.exec();

    if (q.last()) {
      date = q.value(0).toDate();
      return false;
    }
  } catch (QSqlError e) {
    qDebug() << "Reports::canCreateEOT Error: " << e.text();
  }

  return true;
}

//--------------------------------------------------------------------------------

void Reports::createEOD(int id, QDate date)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  QElapsedTimer timer;
  timer.start();
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


  insert(eod, id);

  dep->depInsertLine("Beleg", line);

  pb->setValue(100);

  qDebug() << "Reports::createEOD Total elapsed Time: " << timer.elapsed() << "milliseconds";

  QApplication::setOverrideCursor(Qt::ArrowCursor);

}

//--------------------------------------------------------------------------------
void Reports::fixMonth(int id)
{

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);
  QString query = QString("SELECT timestamp FROM receipts WHERE receiptNum=%1").arg(id);
  q.exec(query);
  if (!q.next())
    return;

  if (!q.value("timestamp").toString().isEmpty())
    return;

  query = QString("SELECT timestamp FROM receipts WHERE receiptNum=%1").arg(id +1);

  q.exec(query);
  if (!q.next())
    return;

  QDate fromOld = q.value("timestamp").toDate();

  QDate date;
    date.setDate(fromOld.year(), fromOld.month(), 1);
    date = date.addDays(-1);

    query = QString("DELETE FROM reports WHERE receiptNum=%1").arg(id);
    q.exec(query);

    query = QString("UPDATE receipts SET timestamp=%1 WHERE receiptNum=%2").arg(date.toString(Qt::ISODate)).arg(id);
    q.exec(query);

    QElapsedTimer timer;
    timer.start();
    QDateTime from;
    QDateTime to;

    // ---------- MONTH -----------------------------
    from.setDate(QDate::fromString(date.toString()));
    from.setDate(QDate::fromString(QString("%1-%2-01").arg(date.year()).arg(date.month()),"yyyy-MM-dd"));
    to.setDate(QDate::fromString(date.toString()));
    to.setTime(QTime::fromString("23:59:59"));

    QStringList eod;
    eod.append(createStat(id, "Monatsumsatz", from, to));

    insert(eod, id);

//    dep->depInsertLine("Beleg", line);

    pb->setValue(100);

    qDebug() << "Reports::createEOM Total elapsed Time: " << timer.elapsed() << "milliseconds";
    QApplication::setOverrideCursor(Qt::ArrowCursor);


}

//--------------------------------------------------------------------------------

void Reports::createEOM(int id, QDate date)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  QElapsedTimer timer;
  timer.start();
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

  insert(eod, id);

  dep->depInsertLine("Beleg", line);

  pb->setValue(100);

  qDebug() << "Reports::createEOM Total elapsed Time: " << timer.elapsed() << "milliseconds";
  QApplication::setOverrideCursor(Qt::ArrowCursor);

}

//--------------------------------------------------------------------------------

QStringList Reports::createStat(int id, QString type, QDateTime from, QDateTime to)
{

  QElapsedTimer timer;
  timer.start();

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  int value = pb->value();
  pb->setValue(value + 8);

  /* Anzahl verkaufter Produkte oder Leistungen */
  QString query = QString("SELECT sum(orders.count) FROM orders WHERE receiptId IN (SELECT id FROM receipts WHERE timestamp BETWEEN '%1' AND '%2')").arg(from.toString(Qt::ISODate)).arg(to.toString(Qt::ISODate));
  q.prepare(query);
  q.exec();
  q.next();

  double sumProducts = q.value(0).toDouble();

  QStringList stat;
  stat.append(QString("Anzahl verkaufter Produkte oder Leistungen: %1").arg(QString::number(sumProducts,'f',2).replace(".",",")));

  pb->setValue(value + 8);

  /* Anzahl Zahlungen */
  q.prepare(QString("SELECT count(id) FROM receipts WHERE timestamp BETWEEN '%1' AND '%2' AND payedBy <= 2").arg(from.toString(Qt::ISODate)).arg(to.toString(Qt::ISODate)));
  q.exec();
  q.next();

  stat.append(QString("Anzahl Zahlungen: %1").arg(q.value(0).toInt()));

  pb->setValue(value + 8);

  /* Anzahl Stornos */
  q.prepare(QString("SELECT count(id) FROM receipts WHERE timestamp BETWEEN '%1' AND '%2' AND storno = 1").arg(from.toString(Qt::ISODate)).arg(to.toString(Qt::ISODate)));
  q.exec();
  q.next();

  stat.append(QString("Anzahl Stornos: %1").arg(q.value(0).toInt()));
  stat.append("-");

  pb->setValue(value + 8);

  /* Umsätze Zahlungsmittel */
  // query = QString("SELECT c.actionText, a.tax, a.gross FROM (SELECT * from orders where receiptId IN (select id from receipts where timestamp between '%1' AND '%2')) AS a INNER JOIN receipts AS b ON a.receiptId = b.id INNER JOIN actionTypes AS c ON c.actionId=b.payedBy GROUP BY b.payedBy, a.tax").arg(from.toString(Qt::ISODate)).arg(to.toString(Qt::ISODate));
  query = QString("SELECT actionTypes.actionText, orders.tax, SUM(orders.count * orders.gross) from orders "
                  " LEFT JOIN receipts on orders.receiptId=receipts.receiptNum"
                  " LEFT JOIN actionTypes on receipts.payedBy=actionTypes.actionId"
                  " WHERE receipts.timestamp between '%1' AND '%2' AND receipts.payedBy < 3"
                  " GROUP BY orders.tax, receipts.payedBy ORDER BY receipts.payedBy, orders.tax")
      .arg(from.toString(Qt::ISODate))
      .arg(to.toString(Qt::ISODate));

  // qDebug() << "Umsätze Zahlungsmittel: " << query;
  q.prepare(query);

  pb->setValue(value + 8);

  q.exec();
  stat.append(tr("Umsätze nach Zahlungsmittel"));

  QString zm = "";
  while (q.next())
  {
    if (!(zm == q.value(0).toString())){
      zm = q.value(0).toString();
      stat.append(QString("%1").arg(q.value(0).toString()));
    }
    stat.append(QString("%1%: %2")
                .arg(q.value(1).toInt())
                .arg(QString::number(q.value(2).toDouble(), 'f', 2).replace(".",",")));
  }
  stat.append("-");

  pb->setValue(value + 8);

  /* Umsätze Steuern */
  query = QString("SELECT orders.tax, SUM(orders.count * orders.gross) from receipts LEFT JOIN orders on orders.receiptId=receipts.receiptNum WHERE receipts.timestamp between '%1' AND '%2' AND receipts.payedBy < 3 GROUP by orders.tax ORDER BY orders.tax")
      .arg(from.toString(Qt::ISODate))
      .arg(to.toString(Qt::ISODate));

  // qDebug() << "query: "<< query;

  q.exec(query);

  stat.append(tr("Umsätze nach Steuersätzen"));
  while (q.next())
  {
    stat.append(QString("%1%: %2")
                .arg(q.value(0).toString())
                .arg(QString::number(q.value(1).toDouble(), 'f', 2).replace(".",",")));
  }
  stat.append("-");

  pb->setValue(value + 8);

  /* Summe */
  q.prepare(QString("SELECT sum(gross) FROM receipts WHERE timestamp BETWEEN '%1' AND '%2' AND payedBy < 3").arg(from.toString(Qt::ISODate)).arg(to.toString(Qt::ISODate)));
  q.exec();
  q.next();

  QString sales = QString::number(q.value(0).toDouble(),'f',2).replace(".",",");

  if (type == "Jahresumsatz") {
    yearsales = sales;
  } else {
    QJsonObject data;
    data["receiptNum"] = id;
    data["receiptTime"] = to.toString(Qt::ISODate);

    QString signature = Utils::getSignature(data);

    query = QString("UPDATE receipts SET gross=%1, timestamp='%2', signature='%3' WHERE receiptNum=%4")
        .arg(QString::number(q.value(0).toDouble(),'f',2))
        .arg(to.toString(Qt::ISODate))
        .arg(signature)
        .arg(id);

    bool ok = q.exec(query);
    if (!ok) {
      qDebug() << "Reports::createStat query: " << query;
      qDebug() << "Reports::createStat error: " << q.lastError().text();
    }
  }

  stat.append(QString("%1: %2").arg(type).arg(sales));
  stat.append("=");

  query = QString("SELECT sum(orders.count) AS count, products.name, orders.gross, sum(orders.count * orders.gross) AS total, orders.tax FROM orders LEFT JOIN products ON orders.product=products.id  LEFT JOIN receipts ON receipts.receiptNum=orders.receiptId WHERE receipts.timestamp BETWEEN '%1' AND '%2' GROUP BY products.name, orders.gross ORDER BY orders.tax, products.name ASC").arg(from.toString(Qt::ISODate)).arg(to.toString(Qt::ISODate));
  q.exec(query);

  stat.append(tr("Verkaufte Produkte oder Leistungen (Gruppiert) Gesamt: %1").arg(QString::number(sumProducts,'f',2).replace(".",",")));
  while (q.next())
  {
    stat.append(QString("%1: %2: %3: %4: %5%")
                .arg(q.value(0).toString())
                .arg(q.value(1).toString())
                .arg(QString::number(q.value(2).toDouble(),'f',2).replace(".",","))
                .arg(QString::number(q.value(3).toDouble(),'f',2).replace(".",","))
                .arg(q.value(4).toDouble()));
  }

  qDebug() << "Reports::createStat(" << type << ") elapsed Time: " << timer.elapsed() << "milliseconds";

  return stat;
}

//--------------------------------------------------------------------------------

void Reports::insert(QStringList list, int id)
{
  QElapsedTimer timer;
  timer.start();

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  int count = list.count();
  pb->setValue(1);
  pb->setMaximum(count);
  int i=0;
  QString line;
  foreach (line, list) {
    dep->depInsertLine("Textposition", line);
    QString query = QString("INSERT INTO reports (receiptNum, text) VALUES(%1, '%2')").arg(id).arg(line);
    q.prepare(query);
    q.exec();
    pb->setValue(i++);
  }

  qDebug() << "Reports::insert elapsed Time: " << timer.elapsed() << "milliseconds";

}

//--------------------------------------------------------------------------------

QStringList Reports::createYearStat(int id, QDate date)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  QElapsedTimer timer;
  timer.start();

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

  qDebug() << "Reports::createEOY Total elapsed Time: " << timer.elapsed() << "milliseconds";

  return eoy;
}

//--------------------------------------------------------------------------------

QString Reports::getReport(int id, bool test)
{

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  q.prepare(QString("SELECT payedBy, timestamp FROM receipts WHERE receiptNum=%1").arg(id));
  q.exec();
  q.next();

  int type = q.value(0).toInt();
  QString format = (type == 4)? "MMMM yyyy": "dd MMMM yyyy";

  QString header;
  if (test)
    header = QString("TESTDRUCK für SCHRIFTART");
  else
    header = QString("BON # %1, %2 - %3").arg(id).arg(Database::getActionType(type)).arg(q.value(1).toDate().toString(format));

  q.prepare(QString("SELECT text FROM reports WHERE receiptNum=%1").arg(id));
  q.exec();

  QString text;

  text.append("<!DOCTYPE html><html><head>\n");
  text.append("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
  text.append("</head><body>\n<table cellpadding=\"3\" cellspacing=\"1\" width=\"100%\">\n");

  int x = 0;
  int span = 5;
  bool needOneMoreCol = false;

  text.append(QString("<tr><th colspan=\"%1\">%2</th></tr>").arg(span).arg(header) );
  text.append(QString("<tr><th colspan=\"%1\"></th></tr>").arg(span));

  while (q.next()){
    needOneMoreCol = false;
    span = 5;

    QString t = q.value(0).toString();
    x++;
    QString color = "";
    if (x % 2 == 1)
      color = "bgcolor='#F5F5F5'";
    else
      color = "bgcolor='#FFFFFF'";

    text.append("<tr>");

    QStringList list;

    if (t.indexOf('-') == 0) {
      t.replace('-',"<hr>");
      text.append(QString("<td colspan=\"%1\" %2>%3</td>").arg(span).arg(color).arg(t));
      needOneMoreCol = false;
    } else if (t.indexOf('=') == 0) {
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
    } else if (t.indexOf(QRegularExpression("^\\d:")) != -1) {
      list = t.split(":",QString::SkipEmptyParts);
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

//--------------------------------------------------------------------------------

bool Reports::endOfDay()
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(tr("Tagesabschluss"));

  QDate date = Database::getLastReceiptDate();
  bool create = Reports::canCreateEOD(date);
  if (create) {
    if (servermode)
      return doEndOfDay(date);

    if (date == QDate::currentDate()) {
      msgBox.setText(tr("Nach dem Erstellen des Tagesabschlusses ist eine Bonierung für den heutigen Tag nicht mehr möglich."));
    } else {
      msgBox.setText(tr("Nach dem Erstellen des Tagesabschlusses für %1 ist eine Nachbonierung erst ab %2 möglich.").arg(date.toString()).arg(date.addDays(1).toString()));
    }
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, tr("Erstellen"));
    msgBox.setButtonText(QMessageBox::No, tr("Abbrechen"));
    msgBox.setDefaultButton(QMessageBox::No);
    if(msgBox.exec() == QMessageBox::Yes){
      return doEndOfDay(date);
    } else {
      return false;
    }
  } else {
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(tr("Tagesabschluss wurde bereits erstellt."));
    msgBox.setInformativeText(tr("Erstellungsdatum %1").arg(date.toString()));
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.setButtonText(QMessageBox::Yes, tr("OK"));
    msgBox.exec();
    return false;
  }

  return false;
}

//--------------------------------------------------------------------------------

bool Reports::endOfMonth()
{

  // check if current month is newer than last Receipt Month

  QDate rDate = Database::getLastReceiptDate();
  int receiptMonth = (rDate.year() * 100) + rDate.month();
  int currMonth = (QDate::currentDate().year() * 100) + QDate::currentDate().month();

  bool ok = (rDate.isValid() && receiptMonth < currMonth);

  if (ok) {
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Monatsabschluss"));
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

    QDateTime dateTime = Database::getLastReceiptDateTime();

    bool canCreateEOD = Reports::canCreateEOD(dateTime.date());
    bool canCreateEOM = Reports::canCreateEOM(dateTime.date());

    if (dateTime <= checkdate && canCreateEOD) {
      msgBox.setText(tr("Der Tagesabschlusses für %1 muß zuerst erstellt werden.").arg(dateTime.date().toString()));
      msgBox.setStandardButtons(QMessageBox::Yes);
      msgBox.addButton(QMessageBox::No);
      msgBox.setButtonText(QMessageBox::Yes, tr("Erstellen"));
      msgBox.setButtonText(QMessageBox::No, tr("Abbrechen"));
      msgBox.setDefaultButton(QMessageBox::No);
      if (servermode) {
        if (! endOfDay())
          return false;
      } else if(msgBox.exec() == QMessageBox::Yes){
        if (! endOfDay())
          return false;
      } else {
        return false;
      }
    }
    if (canCreateEOM) {
      QRKRegister *reg = new QRKRegister(pb);
      currentReceipt =  reg->createReceipts();
      reg->finishReceipts(4, 0, true);
      createEOM(currentReceipt, checkdate.date());
    } else {
      QDate next = QDate::currentDate();
      next.setDate(next.year(), next.addMonths(1).month(), 1);
      QMessageBox::information(0, "Montatsabschluss",
                               tr("Der Monatsabschluss kann erst ab %1 gemacht werden.").arg(next.toString()));
    }
  } else {
    QDate next = QDate::currentDate();
    next.setDate(next.year(), next.addMonths(1).month(), 1);
    QMessageBox::information(0, "Montatsabschluss", tr("Der Monatsabschluss kann erst ab %1 gemacht werden.").arg(next.toString()));
  }

  return true;
}

bool Reports::doEndOfDay(QDate date)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  Backup::cleanUp();
  Backup::create();
  QRKRegister *reg = new QRKRegister(pb);
  currentReceipt = reg->createReceipts();
  reg->finishReceipts(3, 0, true);
  createEOD(currentReceipt, date);
  return true;
}

int Reports::getCurrentId()
{
  return currentReceipt;
}
