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

#include "a_signacos_04.h"

/*
        RK Suite defined in Detailspezifikation/ABS 2
        suite for a closed system (closed systems are identified by the ZDA-ID AT0)
                R1_AT0("1", "AT0", "ES256", "SHA-256", 8),
        suite for an open system (in this case with the virtual ZDA identified by AT100)
                R1_AT100("1", "AT100", "ES256", "SHA-256", 8);
*/

/**
 * @brief ASignACOS_04::ASignACOS_04
 * @param device_name
 */
ASignACOS_04::ASignACOS_04(QString device_name)
    : ASignSmardCard(device_name)
{
    m_CIN = "";
    m_DF_SIG_Selected = false;
}

/**
 * @brief ASignACOS_04::~ASignACOS_04
 */
ASignACOS_04::~ASignACOS_04()
{
}

QString ASignACOS_04::getCardType()
{
    return QObject::tr("A-Trust ACOS_04");
}

QString ASignACOS_04::signReceipt(QString data)
{
    QString jwsDataToBeSigned = RKSignatureModule::getDataToBeSigned(data);
    QString hashValue = RKSignatureModule::HashValue(jwsDataToBeSigned);

//    QByteArray ba = 0;
    QByteArray ba;
    ba.append(hashValue);
    ba = QByteArray::fromHex(ba);
    unsigned char *hash = (unsigned char*) ba.data();
    unsigned char pin[] = "123456";

    QByteArray JWS_Signature;
    ASignResponse response = signHash(pin, hash);

    for (uint i = 0; i < response.length; i++)
        JWS_Signature[i] = (const char)response.data[i];

    if (response.length > 0) {
        JWS_Signature = JWS_Signature.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    } else
        JWS_Signature.append(base64Url_encode("Sicherheitseinrichtung ausgefallen"));

    return jwsDataToBeSigned + "." + JWS_Signature;

}

/**
 * @brief ASignACOS_04::selectDF_SIG
 * @return
 */
bool ASignACOS_04::selectDF_SIG()
{
    if (m_DF_SIG_Selected)
        return true;

    transmit(ASIGN_ACOS_DF_SIG, 7);
    m_DF_SIG_Selected = true;
    return true;
}

/**
 * @brief ASignACOS_04::selectDF_DEC
 * @return
 */
bool ASignACOS_04::selectDF_DEC()
{
    m_DF_SIG_Selected = false;
    transmit(ASIGN_ACOS_DF_DEC, 7);
    return true;
}

// Default development pin is: "123456"
/**
 * @brief ASignACOS_04::signHash
 * @param pin
 * @param hash
 * @return
 */
ASignResponse ASignACOS_04::signHash(const unsigned char pin[6], const unsigned char hash[32])
{
    /*
        ACOS:
        1. SELECT DF_SIG
        2. MANAGE SECURITY ENVIRONMENT
        3. VERIFY
        4. PUT HASH
        5. COMPUTE DIGITAL SIGNATURE
     */

    selectDF_SIG();
    transmit(ASIGN_CMD_MANAGE_SECURITY_ENVIRONMENT, 11);

    unsigned char cmd[14] = ASIGN_CMD_VERIFY;
    for (int i = 0; i < 8; i++) {
        if (i < 6) {
            cmd[i+5] = pin[i];
        } else {
            cmd[i+5] = 0x00;
        }
    }
    transmit(cmd, 13);

    unsigned char hashcmd[32 + 6] = ASIGN_ACOS_CMD_PUT_HASH;
    memcpy(&hashcmd[5], hash, 32);
    transmit(hashcmd, 37);

    return transmit(ASIGN_CMD_COMPUTE_DIGITAL_SIGNATURE, 5);
}

/**
 * @brief ASignACOS_04::getCertificate
 * @param base64
 * @return
 */
QString ASignACOS_04::getCertificate(bool base64)
{
    /*
        1. SELECT DF_SIG
        2. SELECT EF_C_CH_DS
        3. READ BINARY
        */
    selectDF_SIG();

    transmit(ASIGN_ACOS_EF_C_CH_DS, 7);

    QByteArray certificate;
    certificate = ReadFile();

    if (base64)
        return certificate.toBase64();

    return certificate;
}

/**
 * @brief ASignACOS_04::getCIN
 * @return
 */
QString ASignACOS_04::getCIN()
{
    // 16 characters long
    if ("" == m_CIN) {
        // 1. SELECT DF_DEC / MF
        selectDF_DEC();

        unsigned char cmd[6];
        memcpy(cmd, ASIGN_CMD_READ_BINARY, sizeof(ASIGN_CMD_READ_BINARY));

        // 2. SELECT EF_CIN_CSN
        cmd[2] = 0x86; // short id
        //			Or use: Transmit(ASIGN_SELECT_EF_CIN_CSN, 7);

        // 3. READ BINARY
        ASignResponse response = transmit(cmd, 5);
        QByteArray ba = 0;
        ba.append((char*)response.data,response.length > 16 ? 16 : response.length);

        m_CIN = ba.toHex().toUpper();
    }
    return m_CIN;
}

/**
 * @brief ASignACOS_04::getCertificateSerial
 * @param hex
 * @return
 */
QString ASignACOS_04::getCertificateSerial(bool hex)
{
    unsigned char data[256];

    selectDF_SIG();
    transmit(ASIGN_ACOS_EF_C_CH_DS, 7);
//    unsigned char cmd[6] = ASIGN_CMD_READ_BINARY;
    ASignResponse response = transmit(ASIGN_CMD_READ_BINARY, 5);

    DWORD i = response.length;
    if (i == 0)
        return QString::number(0);

    while (i--) {
        data[i] = response.data[i];
    }

    QByteArray ba = 0;
    ba.append((char*)data,256);

    int len = static_cast<unsigned int>(static_cast<unsigned char>(ba.at(14)));
    long serial = 0;
    for (int i = 0; i < len; i++) {
        serial = (serial << 8) + static_cast<unsigned int>(static_cast<unsigned char>(ba.at(15 + i)));
    }

    // put Certificate to Database
    if (!isCertificateInDB(serial))
        putCertificate(serial, getCertificate(true));

    if (hex) {
        return QString::number(serial,16).toUpper();
    }

    return QString::number(serial);
}
