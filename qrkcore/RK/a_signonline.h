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

#ifndef ASIGNONLINE_H
#define ASIGNONLINE_H



#include "rk_signatureonline.h"
#include <QtNetwork/QtNetwork>

#include "qrkcore_global.h"

class QRK_EXPORT ASignOnline : public RKSignatureOnline
{

public:
    ASignOnline(QString connectionstring);
    ~ASignOnline();

    QString signReceipt(QString data);
    QString getCertificateSerial(bool hex = true);
    QString getCertificate(bool base64=true);
    QString getCIN();
    bool selectApplication();
    QString getCardType();

protected:
    bool doRequest(QNetworkRequest req, QJsonObject &obj);
    bool login();
    bool logout();
    QString getZDA();

    QNetworkAccessManager *m_manager;
    QString m_connection;
    QString m_username;
    QString m_password;
    QString m_url;
    QString m_sessionId;
    QString m_sessionKey;
    QString m_certificateB64;
    QString m_certificateserialHex;
    QString m_alg;
};

#endif // ASIGNONLINE_H
