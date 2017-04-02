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

#include "a_signonline.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

/**
 * @brief ASignOnline::ASignOnline
 */
ASignOnline::ASignOnline(QString connectionstring)
{
    m_manager = new QNetworkAccessManager;
    m_sessionId = QString();
    m_sessionKey = QString();
    m_certificateB64 = "";
    m_alg = "";
    m_connection = connectionstring;

    if (m_connection.split("@").size() == 3) {
        m_username = m_connection.split("@").at(0);
        m_password = m_connection.split("@").at(1);
        m_url = m_connection.split("@").at(2);
    }
}

ASignOnline::~ASignOnline()
{
    logout();
}

QString ASignOnline::getCardType()
{
    if(m_username == "u123456789")
        return QObject::tr("A-Trust Online (Test Zugang)");

    return QObject::tr("A-Trust Online (Live Zugang)");
}

QString ASignOnline::signReceipt(QString data)
{
    QString ZDA = getZDA();

    QString jwsDataToBeSigned = RKSignatureModule::getDataToBeSigned(data);
    QString hashValue = RKSignatureModule::HashValue(jwsDataToBeSigned);

    QByteArray ba = 0;
    ba.append(hashValue);
    ba = QByteArray::fromHex(ba);

    QUrl reqUrl(m_url + "/Session/" + m_sessionId + "/Sign/Hash");
    QNetworkRequest req(reqUrl);

    //Creating the JSON-Data
    QJsonObject *jsondata = new QJsonObject();

    jsondata->insert("request", "POST");
    jsondata->insert("sessionKey", m_sessionKey);
    jsondata->insert("hash", (QString)ba.toBase64());

    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    req.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(QJsonDocument(*jsondata).toJson().size()));

    if (doRequest(req, *jsondata)) {
        QString JWS_Signature =  jsondata->value("signature").toString();
        return jwsDataToBeSigned + "." + JWS_Signature;
    }

    QString JWS_Signature = base64Url_encode("Sicherheitseinrichtung ausgefallen");
    return jwsDataToBeSigned + "." + JWS_Signature;
}


QString ASignOnline::getZDA()
{
    QUrl reqUrl(m_url + "/" + m_username + "/ZDA");
    QNetworkRequest req(reqUrl);

    //Creating the JSON-Data
    QJsonObject *jsondata = new QJsonObject();
    jsondata->insert("request", "GET");

    if (doRequest(req, *jsondata)) {
        return jsondata->value("zdaid").toString();
    }

    qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << jsondata->value("errorstring").toString();
    return "AT1";
}

/**
 * @brief ASignOnline::getCertificate
 * @param base64
 * @return
 */
QString ASignOnline::getCertificate(bool base64)
{
    Q_UNUSED(base64);

    QUrl reqUrl(m_url + "/" + m_username + "/Certificate");
    QNetworkRequest req(reqUrl);

    //Creating the JSON-Data
    QJsonObject *jsondata = new QJsonObject();
    jsondata->insert("request", "GET");

    if (doRequest(req, *jsondata)) {
        m_certificateB64 = jsondata->value("Signaturzertifikat").toString();
        m_certificateserialHex = jsondata->value("ZertifikatsseriennummerHex").toString();
        m_certificateserial = jsondata->value("Zertifikatsseriennummer").toString().toUtf8();
        m_alg = jsondata->value("alg").toString();
        return m_certificateB64;
    }

    qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << jsondata->value("errorstring").toString();
    return "";
}

/**
 * @brief ASignOnline::getCertificateSerial
 * @param hex
 * @return
 */
QString ASignOnline::getCertificateSerial(bool hex)
{
    QStringList l = signReceipt("check").split('.');
    if (l.size() == 3) {
        if (RKSignatureModule::base64Url_decode(l.at(2)) == "Sicherheitseinrichtung ausgefallen")
            return "0";
    }

    if (m_certificateserial.isEmpty())
        getCertificate();

    if (hex)
        return m_certificateserialHex;

    return m_certificateserial;
}

/**
 * @brief ASignOnline::selectApplication
 * @return
 */
bool ASignOnline::selectApplication()
{
    if (m_username.isEmpty() || m_password.isEmpty() || m_url.isEmpty())
        return false;

    if(!login())
        return false;

    return true;
}

bool ASignOnline::login(){
    //  / asignrkonline /v2/ Session /{Benutzername} tent Type: application / json

    QUrl reqUrl(m_url + "/Session/" + m_username);
    QNetworkRequest req(reqUrl);
    //Creating the JSON-Data
    QJsonObject *jsondata = new QJsonObject();

    jsondata->insert("request", "PUT");
    jsondata->insert("password", m_password);

    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    req.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(QJsonDocument(*jsondata).toJson().size()));

    if (doRequest(req, *jsondata)) {
        m_sessionId = jsondata->value("sessionid").toString();
        m_sessionKey = jsondata->value("sessionkey").toString();
        // put Certificate to Database
        if (!isCertificateInDB(getCertificateSerial(false).toInt()))
            putCertificate(getCertificateSerial(false).toInt(), getCertificate(true));

        return true;
    }
    //failure
    qCritical() << "Function Name: " << Q_FUNC_INFO << " error: " << jsondata->value("errorstring").toString();
    return false;
}

bool ASignOnline::logout()
{
    if (!m_sessionId.isEmpty()) {
        QUrl reqUrl(m_url + "/Session/" + m_sessionId);
        QNetworkRequest req(reqUrl);
        //Creating the JSON-Data
        QJsonObject *jsondata = new QJsonObject();
        jsondata->insert("request", "DELETE");

        if (doRequest(req, *jsondata)) {
            m_sessionId = "";
            m_sessionKey = "";
            return true;
        }
        //failure
        qWarning() << "Function Name: " << Q_FUNC_INFO << " error: " << jsondata->value("errorstring").toString();
        return false;
    }
    return true;
}

bool ASignOnline::doRequest(QNetworkRequest req, QJsonObject &obj)
{

    // Connection via HTTPS
    QSslSocket* sslSocket = new QSslSocket();
    QSslConfiguration configuration = req.sslConfiguration();
    configuration.setPeerVerifyMode(QSslSocket::VerifyNone);
    configuration.setProtocol(QSsl::AnyProtocol);

    sslSocket->setSslConfiguration(configuration);
    req.setSslConfiguration(configuration);

    QEventLoop eventLoop;

    //Sending the Request
    QString request = obj.value("request").toString();
    QNetworkReply *reply;

    if (request == "POST")
        reply = m_manager->post(req, QJsonDocument(obj).toJson());
    else if (request == "PUT")
        reply = m_manager->put(req, QJsonDocument(obj).toJson());
    else if (request == "GET")
        reply = m_manager->get(req);
    else if (request == "DELETE")
        reply = m_manager->deleteResource(req);
    else
        return false;

    QObject::connect(m_manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    // the HTTP request
    eventLoop.exec(); // blocks stack until "finished()" has been called

    if (reply->error() == QNetworkReply::NoError) {
        //success
        obj = QJsonDocument::fromJson(reply->readAll()).object();
        delete reply;
        return true;
    }
    else {
        //failure
        obj["errorstring"] = reply->errorString();
        delete reply;
        return false;
    }
    return false;
}
