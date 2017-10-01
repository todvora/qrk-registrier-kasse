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
 */

#include "utils.h"
#include "demomode.h"
#include "database.h"
#include "singleton/spreadsignal.h"
#include "RK/rk_signaturemodule.h"
#include "RK/rk_signaturemodulefactory.h"
#include "3rdparty/qbcmath/bcmath.h"
#include "qrcode.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonObject>
#include <QByteArray>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QStringList>
#include <QPixmap>
#include <QtMath>
#include <QDebug>

Utils::Utils()
{
}

Utils::~Utils()
{
}


QString Utils::getSignature(QJsonObject data)
{

    SpreadSignal::setProgressBarWait(true);
    RKSignatureModule *RKSignature = RKSignatureModuleFactory::createInstance("", DemoMode::isDemoMode());
    RKSignature->selectApplication();

    QString taxlocation =  Database::getTaxLocation();
    QSqlDatabase dbc= QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    QJsonObject sign;

    sign["Kassen-ID"] = data.value("kasse").toString();
    sign["Belegnummer"] = QString::number(data.value("receiptNum").toInt());
    sign["Beleg-Datum-Uhrzeit"] = data.value("receiptTime").toString();

    query.prepare("SELECT tax FROM taxTypes WHERE taxlocation=:taxlocation ORDER BY id");
    query.bindValue(":taxlocation", taxlocation);

    bool ok = query.exec();
    if (!ok)
        qWarning() << "Function Name: " << Q_FUNC_INFO << " error: " << query.lastError().text();

    qlonglong counter = 0;
    while(query.next()){
        double tax = query.value(0).toDouble();
        QBCMath gross = data.value(Database::getTaxType(tax)).toDouble();
        gross.round(2);

        sign[Database::getTaxType( tax )] = gross.toString();
        // counter += sign.value(Database::getTaxType( tax )).toString().toDouble() * 100;
        counter += sign.value(Database::getTaxType( tax )).toString().replace(".","").toLongLong();
    }

    QString concatenatedValue = sign["Kassen-ID"].toString() + sign["Belegnummer"].toString();

    QString last_signature = getLastReceiptSignature();
    qlonglong turnOverCounter = getTurnOverCounter();
    turnOverCounter += counter;
    updateTurnOverCounter(turnOverCounter);

    QString symmetricKey = RKSignature->getPrivateTurnoverKey();
    QString base64encryptedTurnOverCounter = RKSignature->encryptTurnoverCounter(concatenatedValue, turnOverCounter, symmetricKey);

    if (data.value("isStorno").toBool(false))
        sign["Stand-Umsatz-Zaehler-AES256-ICM"] = "U1RP";
    else
        sign["Stand-Umsatz-Zaehler-AES256-ICM"] = base64encryptedTurnOverCounter;


    bool safetyDevice;
    QString certificateSerial = RKSignature->getCertificateSerial(true);
    if (certificateSerial == "0") {
        safetyDevice = false;
        certificateSerial = RKSignature->getLastUsedSerial();
        if (!RKSignature->isSignatureModuleSetDamaged())
            RKSignature->setSignatureModuleDamaged();
    } else {
        RKSignature->updateLastUsedSerial(certificateSerial);
        safetyDevice = true;
    }

    sign["Zertifikat-Seriennummer"] = certificateSerial;

    sign["Sig-Voriger-Beleg"] = RKSignature->getLastSignatureValue(last_signature);

    QTime t;
    t.start();
    QString signature = RKSignature->signReceipt(getReceiptShortJson(sign));
    qDebug() << "Function Name: " << Q_FUNC_INFO << "Signature Time elapsed: " << t.elapsed() << " ms";

    delete RKSignature;

    SpreadSignal::setProgressBarWait(false);
    SpreadSignal::setSafetyDevice(safetyDevice);

    return signature;
}

/*
return "_" + rkSuite.getSuiteID() + "_" + cashBoxID + "_" + receiptIdentifier.
+ "_" + dateFormat.format(receiptDateAndTime) + "_" + decimalFormat.format(sumTaxSetNormal).
+ "_" + decimalFormat.format(sumTaxSetErmaessigt1) + "_" + decimalFormat.format(sumTaxSetErmaessigt2).
+ "_" + decimalFormat.format(sumTaxSetNull) + "_" + decimalFormat.format(sumTaxSetBesonders).
+ "_" + encryptedTurnoverValue + "_" + signatureCertificateSerialNumber + "_" + signatureValuePreviousReceipt;
*/

QString Utils::getLastReceiptSignature()
{
    int lastReceiptId = Database::getLastReceiptNum() - 1;
    return getReceiptSignature(lastReceiptId, true);
}

QString Utils::getReceiptSignature(int id, bool full)
{
    qDebug() << "Function Name: " << Q_FUNC_INFO << " id: " << id;

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare("SELECT data FROM dep WHERE receiptNum=:receiptNum");
    query.bindValue(":receiptNum", id);

    bool ok = query.exec();
    if (!ok)
        qWarning() << "Function Name: " << Q_FUNC_INFO << " error: " << query.lastError().text();

    if (query.next())
    {
        QString s = query.value(0).toString();
        qDebug() << "Function Name: " << Q_FUNC_INFO << " return: " << s;
        if (full)
            return s;

        return s.split('.').at(2);
    }

    qDebug() << "Function Name: " << Q_FUNC_INFO << " return: cashregister id";
    Utils::resetTurnOverCounter();
    return Database::getCashRegisterId();
}

QString Utils::getReceiptShortJson(QJsonObject sig)
{
    // RK Suite defined in Detailspezifikation/ABS 2
    // R1_AT100("1", "AT100", "ES256", "SHA-256", 8);

    QString sign = "_R1-AT1_";
    sign.append( sig.value("Kassen-ID").toString() );
    sign.append( "_" );
    sign.append( sig.value("Belegnummer").toString() );
    sign.append( "_" );
    sign.append( sig.value("Beleg-Datum-Uhrzeit").toString() );
    sign.append( "_" );
    sign.append( sig.value("Satz-Normal").toString().replace(".",","));
    sign.append( "_" );
    sign.append( sig.value("Satz-Ermaessigt-1").toString().replace(".",","));
    sign.append( "_" );
    sign.append( sig.value("Satz-Ermaessigt-2").toString().replace(".",","));
    sign.append( "_" );
    sign.append( sig.value("Satz-Null").toString().replace(".",","));
    sign.append( "_" );
    sign.append( sig.value("Satz-Besonders").toString().replace(".",","));
    sign.append( "_" );
    sign.append( sig.value("Stand-Umsatz-Zaehler-AES256-ICM").toString() );
    sign.append( "_" );
    sign.append( sig.value("Zertifikat-Seriennummer").toString() );
    sign.append( "_" );
    sign.append( sig.value("Sig-Voriger-Beleg").toString() );

    qDebug() << "Function Name: " << Q_FUNC_INFO << " short: " << sign;

    return sign;

}

void Utils::resetTurnOverCounter()
{
    updateTurnOverCounter(0);
}

qlonglong Utils::getTurnOverCounter()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare(QString("SELECT value FROM globals WHERE name='turnovercounter'"));
    bool ok = query.exec();
    if (!ok)
        qCritical() << "Function Name: " << Q_FUNC_INFO << " error: " << query.lastError().text();

    if (query.next())
        return query.value("value").toLongLong();

    return 0;

}

void Utils::updateTurnOverCounter(qlonglong toc)
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare("SELECT value FROM globals WHERE name='turnovercounter'");
    query.exec();

    if (query.next()) {
        query.prepare("UPDATE globals set value=:toc WHERE name='turnovercounter'");
    } else {
        query.prepare("INSERT INTO globals (name, value) VALUES('turnovercounter', :toc)");
    }
    query.bindValue(":toc", toc);
    query.exec();

}

double Utils::getYearlyTotal(int year)
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    QDateTime from;
    QDateTime to;

    QString fromString = QString("%1-01-01").arg(year);
    QString toString = QString("%1-12-31").arg(year);

    from.setDate(QDate::fromString(fromString, "yyyy-MM-dd"));
    to.setDate(QDate::fromString(toString, "yyyy-MM-dd"));
    to.setTime(QTime::fromString("23:59:59"));

    /* Summe */
    query.prepare("SELECT sum(gross) FROM receipts WHERE timestamp BETWEEN :fromDate AND :toDate AND payedBy < 3");
    query.bindValue(":fromDate", from.toString(Qt::ISODate));
    query.bindValue(":toDate", to.toString(Qt::ISODate));

    query.exec();
    query.next();

    double sales = query.value(0).toDouble() ;

    return sales;

}

bool Utils::isDirectoryWritable(QString path)
{
    QFileInfo f(path);
    if (f.exists() && f.isDir()) {
        if (f.isWritable())
            return true;
    }

    return false;
}

QString Utils::wordWrap(QString text, int width, QFont font)
{
  QFontMetrics fm(font);
  QString result;
  int space = 0;

  for (;;) {
    int i = 0;
    while (i < text.length()) {
      if (fm.width(text.left(++i + 1)) > width) {
        int j = text.lastIndexOf(' ', i);
        space = 0;
        if (j > 0) {
          i = j;
          space = 1;
        }
        result += text.left(i);
        result += '\n';
        text = text.mid(i + space);
        break;
      }
    }
    if (i >= text.length())
      break;
  }
  return result + text;
}

bool Utils::checkTurnOverCounter()
{
    QString key = RKSignatureModule::getPrivateTurnoverKey();
    RKSignatureModule *sm = RKSignatureModuleFactory::createInstance("", DemoMode::isDemoMode());

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare("SELECT data FROM dep ORDER BY id");
    query.exec();

    qlonglong counter = 0;
    bool ret = true;
    while(query.next()) {
        QString payload = RKSignatureModule::base64Url_decode(query.value(0).toString().split('.').at(1));
        QStringList list = payload.split('_');
        qlonglong counter2 = 0;
        QString con = list.at(2) + list.at(3);
        QString encTOC = list.at(10);
        for (int y = 5; y < 9; y++) {
            QString current = list.at(y);
            counter2 += current.replace(",","").toLongLong();
        }
        counter += counter2;
        QString newEncTOC = sm->encryptTurnoverCounter(con,counter,key);

        if (newEncTOC.compare(encTOC)) {
            if (encTOC != "U1RP")
                ret = false;
        }
    }
    delete sm;
    return ret;
}

QString Utils::normalizeNumber(QString value)
{
    value.replace(".", ",");
    int pos = value.lastIndexOf(",");
    if (pos >= 0)
        value = value.left(pos) + "." + value.mid(pos+1);

    return value.replace(",", "");
}

QString Utils::color_best_contrast(QString color){
    bool hash = false;
    bool ok;
    if (color.startsWith("#")) {
        color = color.replace("#", "");
        hash = true;
    }
    QString r, g, b;
    if (color.length() == 3){
        QString tmp = "";
        tmp += color.mid(0, 1) + color.mid(0, 1);
        tmp += color.mid(2, 1) + color.mid(2, 1);
        tmp += color.mid(3, 1) + color.mid(3, 1);
        r = (tmp.mid(0, 2).toInt(&ok,16) < 128)?"FF":"00";
        g = (tmp.mid(2, 2).toInt(&ok,16) < 128)?"FF":"00";
        b = (tmp.mid(4, 2).toInt(&ok,16) < 128)?"FF":"00";
    } else if (color.length() == 6) {
        r = (color.mid(0, 2).toInt(&ok,16) < 128)?"FF":"00";
        g = (color.mid(2, 2).toInt(&ok,16) < 128)?"FF":"00";
        b = (color.mid(4, 2).toInt(&ok,16) < 128)?"FF":"00";
    } else {
        r = "00";
        g = "00";
        b = "00";
    }

    return ((hash)?"#":"") + r + g + b;
}

QString Utils::taxRoundUp(double value,unsigned short np)
{
    double x,factor;
    QBCMath result;
    factor = pow(10, np);
    x = floor(value*factor+0.9)*factor;
    result = floor(x/factor)/factor;
    return result.toString();
}

double Utils::getTax(double value, double tax, bool net)
{
    QBCMath v(value);
    v.round(2);
    QBCMath t(100 + tax);
    t.round(2);
    QBCMath result;

    if (net) {
        result = v / 100 *t;
        result -= v;
    } else {
        result = v / t * 100;
        result = v - result;
    }

    result.round(2);
    return result.toDouble();
}

double Utils::getNet(double gross, double tax)
{
    double g = QString::number(gross, 'f',2).toDouble();
    double t = Utils::getTax(gross, tax);
    return QString::number(g - t, 'f',2).toDouble();
}

double Utils::getGross(double net, double tax)
{
    double n = QString::number(net, 'f',2).toDouble();
    double t = Utils::getTax(net, tax, true);
    return QString::number(n + t, 'f',2).toDouble();
}

QPixmap Utils::getQRCode(int id, bool &isDamaged )
{
    if (id < 1)
        return QPixmap();

    isDamaged = false;

    QString qr_code_rep = "";
    QString signature = Utils::getReceiptSignature(id,true);
    if (signature.split('.').size() == 3) {
        qr_code_rep = signature.split('.').at(1);
        qr_code_rep = RKSignatureModule::base64Url_decode(qr_code_rep);
        qr_code_rep = qr_code_rep + "_" + RKSignatureModule::base64Url_decode(signature.split('.').at(2)).toBase64();
        if (signature.split('.').at(2) == RKSignatureModule::base64Url_encode("Sicherheitseinrichtung ausgefallen"))
            isDamaged = true;
    }
    QRCode *qr = new QRCode;
    QPixmap QR = qr->encodeTextToPixmap(qr_code_rep);
    delete qr;

    return QR;
}
