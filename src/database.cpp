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

#include "database.h"
#include "utils/demomode.h"

#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDate>

Database::Database(QObject *parent)
  : QObject(parent)
{
}

//--------------------------------------------------------------------------------

Database::~Database()
{
}

QString Database::getDayCounter()
{

  QDateTime dateFrom;
  QDateTime dateTo;

  dateFrom.setDate(QDate::currentDate());
  dateTo.setDate(QDate::currentDate());
  dateTo.setTime(QTime::currentTime());

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  /* Summe */
  query.prepare(QString("SELECT sum(gross) FROM receipts WHERE timestamp BETWEEN '%1' AND '%2' AND payedBy < 3").arg(dateFrom.toString(Qt::ISODate)).arg(dateTo.toString(Qt::ISODate)));
  query.exec();
  query.next();

  return QString::number(query.value(0).toDouble(), 'f', 2);

}

QString Database::getMonthCounter()
{

  QDateTime dateFrom;
  QDateTime dateTo;

  dateFrom.setDate(QDate::fromString(QString("%1-%2-1")
                                     .arg(QDate::currentDate().year())
                                     .arg(QDate::currentDate().month())
                                     , "yyyy-M-d"));
  dateTo.setDate(QDate::currentDate());
  dateTo.setTime(QTime::currentTime());

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  /* Summe */
  query.prepare(QString("SELECT sum(gross) FROM receipts WHERE timestamp BETWEEN '%1' AND '%2' AND payedBy < 3").arg(dateFrom.toString(Qt::ISODate)).arg(dateTo.toString(Qt::ISODate)));
  query.exec();
  query.next();

  return QString::number(query.value(0).toDouble(), 'f', 2);

}

QString Database::getYearCounter()
{

  QDateTime dateFrom;
  QDateTime dateTo;

  dateFrom.setDate(QDate::fromString(
                     QString("%1-1-1")
                     .arg(QDate::currentDate().year())
                     , "yyyy-M-d")
                   );

  dateTo.setDate(QDate::currentDate());
  dateTo.setTime(QTime::currentTime());

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  /* Summe */
  query.prepare(QString("SELECT sum(gross) FROM receipts WHERE timestamp BETWEEN '%1' AND '%2' AND payedBy < 3").arg(dateFrom.toString(Qt::ISODate)).arg(dateTo.toString(Qt::ISODate)));
  query.exec();
  query.next();

  return QString::number(query.value(0).toDouble(), 'f', 2);

}

//--------------------------------------------------------------------------------

void Database::updateProductSold(double count, QString product)
{
    if (product.startsWith("Zahlungsbeleg für Rechnung"))
        return;

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare(QString("UPDATE products SET sold=sold+%1 WHERE name='%2'").arg(count).arg(product));
    query.exec();

}

//--------------------------------------------------------------------------------

QString Database::getTaxLocation()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare("SELECT strValue FROM globals WHERE name='taxlocation'");
  query.exec();
  if (query.next())
    return query.value(0).toString();

  return "AT";

}

//--------------------------------------------------------------------------------

QString Database::getDefaultTax()
{

  QString taxlocation = getTaxLocation();

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare(QString("SELECT tax FROM taxTypes WHERE taxlocation = '%1' LIMIT 1").arg(taxlocation));
  query.exec();
  if (query.next())
    return query.value(0).toString();

  return "20";

}

//--------------------------------------------------------------------------------

QString Database::getShortCurrency()
{
  QString currency = getCurrency();
  if (currency == "CHF")
    return "Fr";
  else
    return "€";

}

//--------------------------------------------------------------------------------

QString Database::getCurrency()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare("SELECT strValue FROM globals WHERE name='currency'");
  query.exec();
  if (query.next())
    return query.value(0).toString();

  return "EUR";
}

//--------------------------------------------------------------------------------

QString Database::getCashRegisterId()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare("SELECT strValue FROM globals WHERE name='shopCashRegisterId'");
  query.exec();
  if (query.next()) {
    if (DemoMode::isDemoMode())
      return "DEMO-" + query.value(0).toString();

    return query.value(0).toString();
  }

  return QString();
}

//--------------------------------------------------------------------------------

QStringList Database::getMaximumItemSold()
{
    QStringList list;
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT max(sold), name, tax, gross FROM products LIMIT 1;");
    query.exec();
    if (query.next()) {
        list << query.value("name").toString()
             << query.value("tax").toString()
             << query.value("gross").toString();
        return list;
    }

    list << "" << "20" << "0,00";
    return list;

}

//--------------------------------------------------------------------------------

bool Database::addCustomerText(int id, QString text)
{

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  QString q = QString("INSERT INTO customer (receiptNum, text) VALUES (%1, '%2')")
          .arg(id)
          .arg(text);

  bool ok = query.prepare(q);

  if (!ok) {
    qDebug() << "Error: " << query.lastError().text();
    qDebug() << "Query: " << q;
  }

  if( query.exec() ){
    return true;
  } else {
    qDebug() << "Database::addCustomerText error: " << query.lastError().text();
  }

  // qDebug() << "Error: " << query.lastError().text();
  return false;
}

//--------------------------------------------------------------------------------

QString Database::getCustomerText(int id)
{

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  QString q = QString("SELECT text FROM customer WHERE receiptNum=%1")
          .arg(id);

  bool ok = query.prepare(q);

  if (!ok) {
    qDebug() << "Error: " << query.lastError().text();
    qDebug() << "Query: " << q;
  }

  if( query.exec() ){
    if (query.next())
      return query.value("text").toString();

    return "";
  } else {
    qDebug() << "Database::getCustomerText error: " << query.lastError().text();
  }

  return "";
}

//--------------------------------------------------------------------------------

bool Database::addProduct(const QList<QVariant> &data)
{
  bool productExists = Database::exists("products", data.at(0).toString());

  int visible = data.at(4).toInt();
  int group = 2;
  if ( data.at(0).toString().startsWith("Zahlungsbeleg für Rechnung") ) {
      if (productExists)
          return false;

      /* We do not need to diplay this in the Manager*/
      visible = 0;
      group = 1;
  }

  if (productExists)
      return true;


  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  QString q = QString("INSERT INTO products (name, tax, net, gross, visible, `group`) VALUES ('%1', %2, %3, %4, %5, %6)")
          .arg(data.at(0).toString())
          .arg(data.at(1).toInt())
          .arg(data.at(2).toDouble())
          .arg(data.at(3).toDouble())
          .arg(visible)
          .arg(group);

  bool ok = query.prepare(q);

  if (!ok) {
    qDebug() << "Database::addProduct Error: " << query.lastError().text();
    qDebug() << "Database::addProduct Query: " << q;
  }

  if( query.exec() ){
    return true;
  } else {
    qDebug() << "Database::addProduct error: " << query.lastError().text();
  }

  // qDebug() << "Error: " << query.lastError().text();
  return false;
}

//--------------------------------------------------------------------------------

bool Database::exists(const QString type, const QString &name)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare(QString("SELECT id FROM %1 WHERE name='%2'").arg(type).arg(name));
  query.exec();
  if (query.next())
    return true;

  return false;
}

//--------------------------------------------------------------------------------

int Database::getLastReceiptNum()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare("SELECT value FROM globals WHERE name='lastReceiptNum'");
  query.exec();

  if ( query.next() )
    return query.value(0).toInt();

  return 0;
}

QStringList Database::getLastReceipt()
{

  QStringList list;

  int lastReceipt = getLastReceiptNum();

  if (lastReceipt == 0)
    return list;

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  query.prepare(QString("SELECT timestamp, receiptNum, payedBy, gross FROM receipts WHERE receiptNum=%1").arg(lastReceipt));
  query.exec();
  query.next();
  list << query.value(0).toString() << query.value(1).toString() << query.value(2).toString() << query.value(3).toString();

  return list;
}

//--------------------------------------------------------------------------------

QDate Database::getLastReceiptDate()
{
  return getLastReceiptDateTime().date();
}

//--------------------------------------------------------------------------------

QDateTime Database::getLastReceiptDateTime()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare("SELECT timestamp FROM receipts where receiptNum IN (SELECT value FROM globals WHERE name='lastReceiptNum')");
  query.exec();
  if (query.next())
    return query.value(0).toDateTime();

  return QDateTime();
}

//--------------------------------------------------------------------------------

QString Database::getDatabaseType()
{
  // read global defintions (DB, ...)
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  return settings.value("DB_type").toString();
}

//--------------------------------------------------------------------------------

QString Database::getShopName()
{
  QString name;
  QString tmp;
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare("SELECT strValue FROM globals WHERE name='shopName'");
  query.exec();
  query.next();

  name = query.value(0).toString();

  return name;

}

QString Database::getShopMasterData()
{
  QString name;
  QString tmp;
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  query.prepare("SELECT strValue FROM globals WHERE name='shopOwner'");
  query.exec();
  query.next();
  tmp = query.value(0).toString();

  name = (tmp.isEmpty())? "": "\n" + tmp;

  query.prepare("SELECT strValue FROM globals WHERE name='shopAddress'");
  query.exec();
  query.next();
  tmp = query.value(0).toString();

  name += (tmp.isEmpty())? "": "\n" + tmp;

  query.prepare("SELECT strValue FROM globals WHERE name='shopUid'");
  query.exec();
  query.next();
  tmp = query.value(0).toString();

  name += (tmp.isEmpty())? "": "\n" + tmp;

  return name;

}

//--------------------------------------------------------------------------------

bool Database::open(bool dbSelect)
{
    const int CURRENT_SCHEMA_VERSION = 10;
  // read global defintions (DB, ...)
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");

  QString dbType = getDatabaseType();

  if ( dbType.isEmpty() || dbSelect )
  {
    DatabaseDefinition dialog(0);

    if ( dialog.exec() == QDialog::Rejected )
      return false;

    dbType = dialog.getDbType();

    settings.setValue("DB_type", dbType);
    settings.setValue("DB_userName", dialog.getUserName());
    settings.setValue("DB_password", dialog.getPassword());
    settings.setValue("DB_hostName", dialog.getHostName());
  }

  QDate date = QDate::currentDate();

  QString dataDir = qApp->applicationDirPath() + "/data";
  QDir dir(dataDir);
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  if (QFile::exists(QString(dataDir + "/%1-QRK.db").arg(date.year() -1)))
    QFile::copy(QString(dataDir + "/%1-QRK.db").arg(date.year() -1),QString(dataDir + "/%1-QRK.db").arg(date.year()));


  QSqlDatabase currentConnection;
  // setup database connection
  if ( dbType == "QSQLITE" )
  {
    currentConnection = QSqlDatabase::addDatabase("QSQLITE", "CN");
    currentConnection.setDatabaseName(QString(dataDir + "/%1-QRK.db").arg(date.year()));
  }
  else if ( dbType == "QMYSQL" )
  {
    QString userName = settings.value("DB_userName", "QRK").toString();
    QString password = settings.value("DB_password", "").toString();
    QString hostName = settings.value("DB_hostName", "localhost").toString();

    currentConnection = QSqlDatabase::addDatabase("QMYSQL", "CN");
    currentConnection.setHostName(hostName);
    currentConnection.setUserName(userName);
    currentConnection.setPassword(password);
  }

  bool ok = currentConnection.open();

  if ( !ok )
  {
    QMessageBox errorDialog;
    errorDialog.setIcon(QMessageBox::Critical);
    errorDialog.addButton(QMessageBox::Ok);
    errorDialog.setText(currentConnection.lastError().text());
    errorDialog.setWindowTitle(QObject::tr("Datenbank Verbindungsfehler"));
    errorDialog.exec();
    return false;
  }

  if ( dbType == "QMYSQL" )
  {
    QSqlQuery query(currentConnection);
    query.exec("SHOW DATABASES LIKE 'QRK'");
    if ( !query.next() )  // db does not exist
      query.exec("CREATE DATABASE QRK");

    currentConnection.close();
    currentConnection.setDatabaseName("QRK");
    if ( !currentConnection.open() )
      return false;
  }

  QSqlQuery query(currentConnection);
  query.exec("SELECT 1 FROM globals");
  if ( !query.next() )  // empty DB, create all tables
  {
    QFile f;

    if ( dbType == "QSQLITE" )
    {
      f.setFileName(":/sql/QRK-sqlite.sql");
    }
    else if ( dbType == "QMYSQL" )
    {
      f.setFileName(":/sql/QRK-mysql.sql");
    }

    f.open(QIODevice::ReadOnly);
    QString cmd = f.readAll();
    QStringList commands = cmd.split(';', QString::SkipEmptyParts);
    foreach (const QString &command, commands)
      query.exec(command);

    // just some default tables
    for (int i = 1; i <= 5; i++)
      query.exec(QString("INSERT INTO `tables` (id, name) VALUES(%1, '%2')").arg(i).arg(i));

    query.exec("INSERT INTO globals (name, value) VALUES('numPictureButtons', 0)");
    query.exec("INSERT INTO globals (name, value) VALUES('lastReceiptNum', 0)");
    query.exec("INSERT INTO globals (name, strValue) VALUES('shopName', '')");
    query.exec("INSERT INTO globals (name, strValue) VALUES('backupTarget', '')");

    query.exec(QString("INSERT INTO globals (name, value) VALUES('schemaVersion', %1)")
               .arg(CURRENT_SCHEMA_VERSION));

    query.exec(QString("INSERT INTO `dep`(id,version,cashregisterid,datetime,text) VALUES (NULL,'0.15.1222',0,CURRENT_TIMESTAMP, 'Id\tProgrammversion\tKassen-Id\tProduktposition\tBeschreibung\tMenge\tEinzelpreis\tGesamtpreis\tUSt. Satz\tErstellungsdatum')"));
    query.exec(QString("INSERT INTO `dep`(id,version,cashregisterid,datetime,text) VALUES (NULL,'0.15.1222',0,CURRENT_TIMESTAMP, 'Id\tProgrammversion\tKassen-Id\tBeleg\tBelegtyp\tBemerkung\tNachbonierung\tBelegnummer\tDatum\tUmsatz Normal\tUmsatz Ermaessigt1\tUmsatz Ermaessigt2\tUmsatz Null\tUmsatz Besonders\tJahresumsatz bisher\tErstellungsdatum')"));
    query.exec(QString("INSERT INTO `dep`(id,version,cashregisterid,datetime,text) VALUES (NULL,'0.15.1222',0,CURRENT_TIMESTAMP, 'Id\tProgrammversion\tKassen-Id\tBeleg-Textposition\tText\tErstellungsdatum')"));

  }
  else  // db already exists; check if we need to run an update
  {
    int schemaVersion = 1;
    query.exec("SELECT value FROM globals WHERE name='schemaVersion'");
    if ( query.next() )
      schemaVersion = query.value(0).toInt();
    else  // schemaVersion not set in globals, must be version 1
      query.exec("INSERT INTO globals (name, value) VALUES('schemaVersion', 1)");

    // run all db update scripts from the db version + 1 to what the program currently needs
    for (int i = schemaVersion + 1; i <= CURRENT_SCHEMA_VERSION; i++)
    {
      QFile f;

      if ( dbType == "QSQLITE" )
      {
        f.setFileName(QString(":/sql/QRK-sqlite-update-%1.sql").arg(i));
      }
      else if ( dbType == "QMYSQL" )
      {
        f.setFileName(QString(":/sql/QRK-mysql-update-%1.sql").arg(i));
      }

      if ( !f.open(QIODevice::ReadOnly) )
      {
        qWarning("could not load internal update file %s", qPrintable(f.fileName()));
        return false;  // should never happen
      }

      QString cmd = f.readAll();
      QStringList commands = cmd.split(';', QString::SkipEmptyParts);
      foreach (const QString &command, commands)
        query.exec(command);

      // changes which are not possible in sql file needed for update-7
      if ( i == 7 )
      {
//          query.exec(QString("UPDATE products SET \"group\"=2 WHERE name NOT LIKE 'Zahlungsbeleg für Rechnung%'"));
//          query.exec(QString("UPDATE products SET \"group\"=1 WHERE name LIKE 'Zahlungsbeleg für Rechnung%'"));
          query.exec(QString("UPDATE products SET `group`=2 WHERE name NOT LIKE 'Zahlungsbeleg für Rechnung%'"));
          query.exec(QString("UPDATE products SET `group`=1 WHERE name LIKE 'Zahlungsbeleg für Rechnung%'"));
      }
    }

    if ( schemaVersion != CURRENT_SCHEMA_VERSION )
    {
      query.exec(QString("UPDATE globals SET value=%1 WHERE name='schemaVersion'")
                 .arg(CURRENT_SCHEMA_VERSION));
    }

  }

  if ( dbType == "QSQLITE" )
  {
    // enforce foreign key constraint
    query.exec("PRAGMA foreign_keys = 1;");
  }

  return true;
}

//--------------------------------------------------------------------------------

int Database::getPayedBy(int id)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare(QString("SELECT payedBy FROM receipts WHERE receiptNum=%1").arg(id));
  query.exec();
  query.next();

  return query.value(0).toInt();

}

//--------------------------------------------------------------------------------

int Database::getActionTypeByName(const QString &name)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare(QString("SELECT actionId FROM actionTypes WHERE actionText='%1'").arg(name));
  query.exec();
  query.next();

  return query.value(0).toInt();

}

//--------------------------------------------------------------------------------

QString Database::getActionType(int id)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare(QString("SELECT actionText FROM actionTypes WHERE actionId=%1").arg(id));
  query.exec();
  query.next();

  return query.value(0).toString();

}

//--------------------------------------------------------------------------------

QString Database::getTaxType(int id)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  query.prepare(QString("SELECT comment FROM taxTypes WHERE tax=%1").arg(id));
  query.exec();
  query.next();

  return query.value(0).toString();

}

//--------------------------------------------------------------------------------

void Database::setStorno(int id, int value)
{
  try {
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare(QString("UPDATE receipts SET storno=%1 WHERE receiptNum=%2").arg(value).arg(id));
    query.exec();
  } catch (QSqlError &e) {
    qDebug() << "Database::setReceiptStorno " << e.text();
  }

}

//--------------------------------------------------------------------------------

void Database::setStornoId(int sId, int id)
{
  try {
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    // Beleg wurde von 'sId' storniert
    query.prepare(QString("UPDATE receipts SET stornoId=%1 WHERE receiptNum=%2").arg(sId).arg(id));
    query.exec();

    // Beleg ist Stornobeleg von Beleg Nr: 'id'
    query.prepare(QString("UPDATE receipts SET stornoId=%1 WHERE receiptNum=%2").arg(id).arg(sId));
    query.exec();
    setStorno(id); // Beleg wurde storniert
    setStorno(sId, 2); // Beleg ist StornoBeleg

  } catch (QSqlError &e) {
    qDebug() << "Database::setReceiptStorno " << e.text();
  }

}

//--------------------------------------------------------------------------------

int Database::getStorno(int id)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  try {
    query.prepare(QString("SELECT storno FROM receipts WHERE receiptNum=%1").arg(id));
    query.exec();
  } catch (QSqlError &e) {
    qDebug() << "Database::getStorno " << e.text();
  }

  query.next();
  return query.value(0).toInt();
}

//--------------------------------------------------------------------------------

int Database::getStornoId(int id)
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);
  try {
    query.prepare(QString("SELECT stornoId FROM receipts WHERE receiptNum=%1").arg(id));
    query.exec();
  } catch (QSqlError &e) {
    qDebug() << "Database::getStornoId " << e.text();
  }

  query.next();
  return query.value(0).toInt();
}

void Database::resetAllData()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);

  q.prepare("DELETE FROM dep;");
  q.exec();

  q.prepare("DELETE FROM orders;");
  q.exec();

  q.prepare("DELETE FROM receipts;");
  q.exec();

  q.prepare("DELETE FROM reports;");
  q.exec();

  q.prepare("DELETE FROM products WHERE `group`=1;");
  q.exec();

  q.prepare("UPDATE globals SET value = 0 WHERE name = 'lastReceiptNum';");
  q.exec();

  q.prepare("UPDATE globals SET value = 0 WHERE name = 'turnovercounter';");
  q.exec();

  QString dbType = getDatabaseType();

  if ( dbType == "QMYSQL") {
    /*MYSQL*/
    q.prepare("ALTER TABLE dep AUTO_INCREMENT = 1;");
    q.exec();

    q.prepare("ALTER TABLE orders AUTO_INCREMENT = 1;");
    q.exec();

    q.prepare("ALTER TABLE receipts AUTO_INCREMENT = 1;");
    q.exec();

  } else {
    /*SQLITE*/
    q.prepare("delete from sqlite_sequence where name='dep';");
    q.exec();

    q.prepare("delete from sqlite_sequence where name='orders';");
    q.exec();

    q.prepare("delete from sqlite_sequence where name='receipts';");
    q.exec();
  }

}
