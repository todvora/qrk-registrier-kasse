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

#include "rk_signaturemodule.h"
#include "base32decode.h"
#include "base32encode.h"

#include <stdio.h>

#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>

#include <QtEndian>
#include <QDataStream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QDebug>

using namespace std;
using namespace CryptoPP;

/**
 * @brief RKSignatureModule::RKSignatureModule
 */
RKSignatureModule::RKSignatureModule()
{
    m_certificateserial = "";
}

/**
 * @brief RKSignatureModule::~RKSignatureModule
 */
RKSignatureModule::~RKSignatureModule()
{
}

/**
 * @brief RKSignatureModule::getDataToBeSigned
 * @param data
 * @return
 */
QString RKSignatureModule::getDataToBeSigned(QString data)
{
    QString JWS_Protected_Header = base64Url_encode("{\"alg\":\"ES256\"}");
    QString JWS_Payload =  base64Url_encode(data);

    return JWS_Protected_Header + "." + JWS_Payload;

}

/**
 * @brief RKSignatureModule::HashValue
 * @param value
 * @return
 */
QByteArray RKSignatureModule::HashValue(QString value)
{
    SHA256 hashtest;
    string message = value.toStdString();
    string digest;

    StringSource s(message, true, new HashFilter(hashtest, new HexEncoder(new StringSink(digest))));
//    StringSource s(message, true, new HashFilter(hashtest, new ArraySink(test, sizeof(test))));

    return QByteArray::fromStdString(digest);
}

/**
 * @brief RKSignatureModule::encryptTurnoverCounter
 * @param concatenated
 * @param turnoverCounter
 * @param symmetricKey
 * @return
 */
QString RKSignatureModule::encryptTurnoverCounter( QString concatenated, qlonglong turnoverCounter, QString symmetricKey)
{
    QString hashValue = HashValue(concatenated);
    return encryptCTR(hashValue.toStdString(), turnoverCounter, symmetricKey.toStdString());
}

/**
 * @brief RKSignatureModule::decryptTurnoverCounter
 * @param concatenated
 * @param encodedTurnoverCounter
 * @param symmetricKey
 * @return
 */
QString RKSignatureModule::decryptTurnoverCounter( QString concatenated, QString encodedTurnoverCounter, QString symmetricKey)
{
    QString hashValue = HashValue(concatenated);
    return decryptCTR(hashValue.toStdString(), encodedTurnoverCounter, symmetricKey.toStdString());
}

/**
 * @brief RKSignatureModule::getLastSignatureValue
 * @param sig
 * @return
 */
QString RKSignatureModule::getLastSignatureValue(QString sig)
{
    QString hashValue = HashValue(sig);
    QByteArray ls;
    ls.append(hashValue);
    ls = QByteArray::fromHex(ls);
    ls.resize(8);

    return ls.toBase64();
}

/**
 * @brief RKSignatureModule::base64_encode
 * @param str
 * @return
 */
QByteArray RKSignatureModule::base64_encode(QString str)
{
    QByteArray ba = 0;
    ba.append(str);

    return ba.toBase64();
}

/**
 * @brief RKSignatureModule::base64Url_encode
 * @param str
 * @return
 */
QByteArray RKSignatureModule::base64Url_encode(QString str)
{
    QByteArray ba = 0;
    ba.append(str);

    return ba.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

/**
 * @brief RKSignatureModule::base64_decode
 * @param str
 * @param hex
 * @return
 */
QByteArray RKSignatureModule::base64_decode(QString str, bool hex)
{
    QByteArray ba = 0;
    ba.clear();
    ba.append(str);
    if (hex)
        return QByteArray::fromBase64(ba).toHex();

    return QByteArray::fromBase64(ba);
}

/**
 * @brief RKSignatureModule::base64Url_decode
 * @param QString base64URL_str to decode
 * @return
 */
QByteArray RKSignatureModule::base64Url_decode(QString str)
{
    return QByteArray::fromBase64(str.toUtf8(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

/**
 * @brief RKSignatureModule::base32_encode
 * @param QString str to encode
 * @return
 */
QByteArray RKSignatureModule::base32_encode(QByteArray str)
{
    string decoded = str.toStdString();

    size_t req = Base32Encode::GetLength(decoded.size());
    char encoded[req];

    memset( encoded, 0x00, req);
    size_t size = Base32Encode::Encode(encoded, decoded.data(),decoded.size());

    return  QByteArray::fromStdString(std::string(encoded, size)).simplified();
}

/**
 * @brief RKSignatureModule::base32_decode
 * @param QString base64_str to decode
 * @return
 */
QByteArray RKSignatureModule::base32_decode(QByteArray str)
{

    string encoded = str.toStdString();

    size_t req = Base32Decode::GetLength(encoded.size());
    char decoded[req];

    memset( decoded, 0x00, req);
    size_t size = Base32Decode::Decode(decoded, encoded.data(),encoded.size());

    return  QByteArray::fromStdString(std::string(decoded, size));

}

/**
 * @brief RKSignatureModule::encryptCTR
 * @param string concatenatedHashValue
 * @param qlonglong turnoverCounter
 * @param string symmetricKey
 * @return
 */
QString RKSignatureModule::encryptCTR(std::string concatenatedHashValue, qlonglong turnoverCounter, std::string symmetricKey)
{

    const int RKSuitSize = 8;
    union {
        qlonglong source;
        byte data[RKSuitSize];
    } Union;

    memset( Union.data, 0x00, RKSuitSize );
    Union.source = qToBigEndian(turnoverCounter);

    byte key[ AES::MAX_KEYLENGTH ], iv[ AES::BLOCKSIZE ];
    StringSource ssk(symmetricKey , true, new HexDecoder( new ArraySink(key, AES::MAX_KEYLENGTH)));
    StringSource ssv(concatenatedHashValue, true, new HexDecoder( new ArraySink(iv, AES::BLOCKSIZE)));

    byte out[RKSuitSize];
    memset( out, 0x00, RKSuitSize );

    try {
        CTR_Mode<AES>::Encryption encryption(key,sizeof(key),iv, sizeof(iv));
        encryption.ProcessData(out,Union.data,sizeof(Union.data));
    } catch (Exception &e) {
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << e.what();
    }

    QByteArray ba = 0;
    ba.append(reinterpret_cast<const char *>(out), sizeof(out));

    return ba.toBase64();
}

/**
 * @brief RKSignatureModule::decryptCTR
 * @param string concatenatedHashValue
 * @param QString encryptedTurnoverCounter
 * @param string symmetricKey
 * @return
 */
QString RKSignatureModule::decryptCTR(std::string concatenatedHashValue, QString encryptedTurnoverCounter, std::string symmetricKey)
{
    const int RKSuitSize = 8;
    QByteArray baBase64 = 0;
    baBase64.append(encryptedTurnoverCounter);
    string encryptedTurnoverCounterHex = QByteArray::fromBase64(baBase64).toHex().toStdString();

    byte data[AES::BLOCKSIZE];
    memset( data, 0x00, AES::BLOCKSIZE );

    byte recovered[RKSuitSize];
    memset( recovered, 0x00, RKSuitSize );

    byte key[ AES::MAX_KEYLENGTH ], iv[ AES::BLOCKSIZE ];
    StringSource ssk(symmetricKey , true, new HexDecoder( new ArraySink(key, AES::MAX_KEYLENGTH)));
    StringSource ssv(concatenatedHashValue, true, new HexDecoder( new ArraySink(iv, AES::BLOCKSIZE)));
    StringSource ssd(encryptedTurnoverCounterHex, true, new HexDecoder( new ArraySink(data, AES::BLOCKSIZE)));

    try {
        CTR_Mode<AES>::Decryption decryption(key,sizeof(key),iv, sizeof(iv));
        decryption.ProcessData(recovered,data,AES::BLOCKSIZE);
    } catch (Exception &e) {
        qCritical() << "Function Name: " << Q_FUNC_INFO << " Error: " << e.what();
    }

    /* we need only 8 Bytes for Turnover minimum is 5 Byte */
    union {
        qlonglong source;
        byte data[RKSuitSize];
    } Union;
    memset( Union.data, 0x00, RKSuitSize );
    /* reconstruct reverse */
    for (int i=0; i < 8;i++)
        Union.data[i] = recovered[7 - i];

    return QString::number(Union.source);
}

/**
 * @brief RKSignatureModule::getPrivateTurnoverKey
 * @return QString PrivateTurnoverKey (AES Key)
 */
QString RKSignatureModule::getPrivateTurnoverKey()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT value, strValue FROM globals WHERE name='PrivateTurnoverKey'");
    query.exec();
    if (query.next()) {
        int val = query.value(0).toInt();
        /* increment manual for keyversions check */
        if (val == 1)
            return query.value(1).toString();
    }
    QString key = RKSignatureModule::generatePrivateTurnoverHexKey();
    query.prepare(QString("INSERT INTO globals (name, value, strValue) VALUES('PrivateTurnoverKey', 1, '%1')").arg(key));
    query.exec();

    return key;

}

/**
 * @brief RKSignatureModule::getPrivateTurnoverKeyBase64
 * @return QString AES Key Base64 encoded
 */
QString RKSignatureModule::getPrivateTurnoverKeyBase64()
{
    QString symmetricKey = RKSignatureModule::getPrivateTurnoverKey().toLower();
    QByteArray ba(QByteArray::fromHex(symmetricKey.toUtf8()));
    return ba.toBase64();
}

QString RKSignatureModule::getPrivateTurnoverKeyCheckValueBase64Trimmed()
{
    QString symmetricKeyBase64 = getPrivateTurnoverKeyBase64();
    QByteArray data = QByteArray::fromHex(HashValue(symmetricKeyBase64));
    data.resize(3);
    return data.toBase64().replace("=","");
}

/**
 * @brief RKSignatureModule::getCertificateMap
 * @return QJsonObject certificateMap
 */
QJsonObject RKSignatureModule::getCertificateMap()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT value, strValue FROM globals WHERE name='certificate'");
    query.exec();

    QJsonObject certificateMap;

    while (query.next()) {
        QJsonObject certificate;
        certificate["id"] = QString::number(query.value(0).toString().toInt(), 16).toUpper();
        certificate["signatureDeviceType"] = "CERTIFICATE";
        certificate["signatureCertificateOrPublicKey"] = query.value(1).toString();
        certificateMap[certificate["id"].toString()] = certificate;
    }

    return certificateMap;
}

bool RKSignatureModule::isCertificateInDB(int serial)
{
    if (serial == 0)
        return false;

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT value FROM globals WHERE name='certificate' AND value=:serial");
    query.bindValue(":serial", serial);
    query.exec();
    if (query.next())
        return true;

    return false;
}

/**
 * @brief RKSignatureModule::putCertificate
 * @param int serial
 * @param QString certificateB64
 */
void RKSignatureModule::putCertificate(int serial, QString certificateB64)
{

    if (serial == 0)
        return;

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT value FROM globals WHERE name='certificate' AND value=:serial");
    query.bindValue(":serial", serial);
    query.exec();
    if (query.next())
        return;

    query.prepare("INSERT INTO globals (name, value, strValue) VALUES('certificate', :serial, :certificate)");
    query.bindValue(":serial", serial);
    query.bindValue(":certificate", certificateB64);

    query.exec();
}

QString RKSignatureModule::getLastUsedSerial()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT strValue FROM globals WHERE name='lastUsedCertificate'");
    query.exec();
    if (query.next())
        return query.value(0).toString();

    return "";
}

void RKSignatureModule::updateLastUsedSerial(QString serial)
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    if (RKSignatureModule::getLastUsedSerial() == "") {
        query.prepare("INSERT INTO globals (name, strValue) VALUES('lastUsedCertificate', :serial)");
        query.bindValue(":serial", serial);
    } else {
        query.prepare("UPDATE globals SET strValue=:serial WHERE name='lastUsedCertificate'");
        query.bindValue(":serial", serial);
    }

    query.exec();
}

void RKSignatureModule::setSignatureModuleDamaged()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare("INSERT INTO globals (name, strValue) VALUES('signatureModuleIsDamaged', :date)");
    query.bindValue(":date", QDateTime::currentDateTime().toString(Qt::ISODate) );

    query.exec();
}

bool RKSignatureModule::isSignatureModuleSetDamaged()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    query.prepare("SELECT strValue FROM globals WHERE name='signatureModuleIsDamaged'");
    query.exec();

    if (query.next())
        return true;

    return false;
}

QString RKSignatureModule::resetSignatureModuleDamaged()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);

    QString ISODate = "";

    query.prepare("SELECT strValue FROM globals WHERE name='signatureModuleIsDamaged'");
    query.exec();

    if (query.next())
        ISODate = query.value(0).toString();

    query.prepare("DELETE FROM globals WHERE name='signatureModuleIsDamaged'");
    query.exec();

    return ISODate;

}

/**
 * @brief RKSignatureModule::generatePrivateTurnoverHexKey
 * @return AES Key in Hex
 */
QString RKSignatureModule::generatePrivateTurnoverHexKey()
{

    AutoSeededRandomPool prng;

    /* generate key for encrypt */
    unsigned char key[AES::MAX_KEYLENGTH];
    prng.GenerateBlock(key, sizeof(key));

    string encoded;
    encoded.clear();

    StringSource ssk(key, sizeof(key), true, new HexEncoder( new StringSink(encoded)));

    return QString::fromStdString(encoded).toLower();
}

/**
 * @brief RKSignatureModule::isDEPactive
 * @return bool
 */
bool RKSignatureModule::isDEPactive()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT value FROM globals WHERE name='DEP'");
    query.exec();
    if (query.next()) {
        bool active = query.value(0).toBool();
        return active;
    }

    return false;

}

/**
 * @brief RKSignatureModule::setDEPactive
 * @param bool active
 */
void RKSignatureModule::setDEPactive(bool active)
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare("SELECT value FROM globals WHERE name='DEP'");
    query.exec();
    if (query.next()) {
        query.prepare("UPDATE globals SET value=:active WHERE name='DEP'");
        query.bindValue(":active", active);
    } else {
        query.prepare("INSERT INTO globals (name, value) VALUES('DEP', :active)");
        query.bindValue(":active", active);
    }
    query.exec();
}
