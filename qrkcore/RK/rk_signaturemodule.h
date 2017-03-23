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

#ifndef RKSIGNATUREMODULE_H
#define RKSIGNATUREMODULE_H

#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#endif

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QJsonObject>

#include "qrkcore_global.h"

#define MAX_APDU_BUFFER_SIZE 260 // Max is 256 bytes, but allow some additional space

typedef struct
{
    unsigned char code[2];
    unsigned char data[MAX_APDU_BUFFER_SIZE];
    DWORD length;
} ASignResponse;

class QRK_EXPORT RKSignatureModule
{
public:
    RKSignatureModule();

    virtual ~RKSignatureModule();

    static QByteArray base64_encode(QString str);
    static QByteArray base64Url_encode(QString str);
    static QByteArray base64_decode(QString str, bool hex);
    static QByteArray base64Url_decode(QString str);
    static QByteArray base32_encode(QByteArray str);
    static QByteArray base32_decode(QByteArray str);
    static QString getPrivateTurnoverKey();
    static QString getPrivateTurnoverKeyBase64();
    static QJsonObject getCertificateMap();
    static bool isDEPactive();
    static void setDEPactive(bool active=true);
    static bool isCertificateInDB(int serial);
    static QString getLastUsedSerial();
    static void updateLastUsedSerial(QString serial);
    static void setSignatureModuleDamaged();
    static bool isSignatureModuleSetDamaged();
    static QString resetSignatureModuleDamaged();

    QString getPrivateTurnoverKeyCheckValueBase64Trimmed();
    QString encryptTurnoverCounter( QString concatenated, qlonglong turnoverCounter, QString symmetricKey);
    QString decryptTurnoverCounter( QString concatenated, QString encodedTurnoverCounter, QString symmetricKey);
    QString getLastSignatureValue(QString sig);

    virtual QString signReceipt(QString data) = 0;
    virtual QString getCertificateSerial(bool hex) = 0;
    virtual QString getCertificate(bool base64 = true) = 0;
    virtual bool selectApplication() = 0;
    virtual QString getCardType() = 0;

protected:
    QByteArray m_certificateserial;

    static QString generatePrivateTurnoverHexKey();

    virtual QString getDataToBeSigned(QString data);

    QByteArray HashValue(QString value);
    void putCertificate(int serial, QString certificateB64);
    QString encryptCTR(std::string concatenatedHashValue, qlonglong turnoverCounter, std::string symmetricKey);
    QString decryptCTR(std::string concatenatedHashValue, QString encryptedTurnoverCounter, std::string symmetricKey);


};

#endif // RKSIGNATUREMODULE_H
