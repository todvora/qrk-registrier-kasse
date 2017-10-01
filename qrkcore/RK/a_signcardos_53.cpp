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

#include "a_signcardos_53.h"

/*
        RK Suite defined in Detailspezifikation/ABS 2
        suite for a closed system (closed systems are identified by the ZDA-ID AT0)
                R1_AT0("1", "AT0", "ES256", "SHA-256", 8),
        suite for an open system (in this case with the virtual ZDA identified by AT100)
                R1_AT100("1", "AT100", "ES256", "SHA-256", 8);
*/

//---------------------------------------------------------------------------

/**
 * @brief ASignCARDOS_53::ASignCARDOS_53
 * @param device_name
 */
ASignCARDOS_53::ASignCARDOS_53(QString device_name)
    : ASignSmardCard(device_name)
{
    m_CIN = "";
    m_DF_SIG_Selected = false;
}

/**
 * @brief ASignCARDOS_53::~ASignCARDOS_53
 */
ASignCARDOS_53::~ASignCARDOS_53()
{
}

QString ASignCARDOS_53::getCardType()
{
    return QObject::tr("A-Trust CARDOS_53");
}

QString ASignCARDOS_53::signReceipt(QString data)
{
    QString jwsDataToBeSigned = RKSignatureModule::getDataToBeSigned(data);
    QString hashValue = RKSignatureModule::HashValue(jwsDataToBeSigned);

    QByteArray ba = 0;
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
 * @brief ASignCARDOS_53::selectDF_SIG
 * @return
 */
bool ASignCARDOS_53::selectDF_SIG()
{
    if (m_DF_SIG_Selected)
        return true;

    transmit(ASIGN_OS53_MASTER_FILE,7);
    transmit(ASIGN_OS53_DF_SIG, 7);
    m_DF_SIG_Selected = true;
    return true;
}

/**
 * @brief ASignCARDOS_53::selectDF_DEC
 * @return
 */
bool ASignCARDOS_53::selectDF_DEC()
{
    m_DF_SIG_Selected = false;
    transmit(ASIGN_OS53_MASTER_FILE, 7);
    return true;
}

// Default development pin is: "123456"
/**
 * @brief ASignCARDOS_53::signHash
 * @param pin
 * @param hash
 * @return
 */
ASignResponse ASignCARDOS_53::signHash(const unsigned char pin[6], const unsigned char hash[32])
{
    /*
        Card OS 5.3:
        1. SELECT DF_SIG
        2. VERIFY
        3. COMPUTE DIGITAL SIGNATURE
        */
    selectDF_SIG();
    /*
                Format 2 PIN block
                The format 2 PIN block is constructed thus:
                1 nibble with the value of 2, which identifies this as a format 2 block
                1 nibble encoding the length N of the PIN
                N nibbles, each encoding one PIN digit
                14-N nibbles, each holding the "fill" value 15
                */

    unsigned char cmd[14] = ASIGN_CMD_VERIFY;
    cmd[5]  = 0x26; // (byte)((2 << 4) | pin.Length)
    cmd[6]  = ((pin[0]-'0') * 16) + (pin[1]-'0'); // First 2 PIN numbers
    cmd[7]  = ((pin[2]-'0') * 16) + (pin[3]-'0'); // Second 2 PIN numbers
    cmd[8]  = ((pin[4]-'0') * 16) + (pin[5]-'0'); // Third 2 PIN numbers
    cmd[9]  = 0xFF;
    cmd[10] = 0xFF;
    cmd[11] = 0xFF;
    cmd[12] = 0xFF;
    ASignSmardCard::transmit(cmd, 13);

    unsigned char hashcmd[32 + 6] = ASIGN_OS53_CMD_PUT_HASH;
    memcpy(&hashcmd[5], hash, 32);
    return ASignSmardCard::transmit(hashcmd,sizeof(hashcmd));
}

/**
 * @brief ASignCARDOS_53::getCertificate
 * @param base64
 * @return
 */
QString ASignCARDOS_53::getCertificate(bool base64)
{

    /*
        1. SELECT DF_SIG
        2. SELECT EF_C_CH_DS
        3. READ BINARY
        */
    selectDF_SIG();

    ASignSmardCard::transmit(ASIGN_OS53_EF_C_CH_DS, 7);
    QByteArray certificate;
    certificate = ReadFile();

    if (base64)
        return certificate.toBase64();

    return certificate;
}

/**
 * @brief ASignCARDOS_53::getCIN
 * @return
 */
QString ASignCARDOS_53::getCIN()
{
    // 16 characters long
    if ("" == m_CIN) {
        // 1. SELECT DF_DEC / MF
        selectDF_DEC();

        unsigned char cmd[6];
        memcpy(cmd, ASIGN_CMD_READ_BINARY, sizeof(ASIGN_CMD_READ_BINARY));

        // 2. SELECT EF_CIN_CSN
        ASignSmardCard::transmit(ASIGN_SELECT_EF_CIN_CSN, 7);

        // 3. READ BINARY
        ASignResponse response = ASignSmardCard::transmit(cmd, 5);
        QByteArray ba = 0;
        ba.append((char*)response.data,response.length > 16 ? 16 : response.length);

        m_CIN = ba.toHex().toUpper();
    }
    return m_CIN;
}

/**
 * @brief ASignCARDOS_53::getCertificateSerial
 * @param hex
 * @return
 */
QString ASignCARDOS_53::getCertificateSerial(bool hex)
{
    unsigned char data[256];

    selectDF_SIG();
    ASignSmardCard::transmit(ASIGN_OS53_EF_C_CH_DS, 7);
    unsigned char cmd[6];
    memcpy(cmd, ASIGN_CMD_READ_BINARY, sizeof(ASIGN_CMD_READ_BINARY));

    ASignResponse response = ASignSmardCard::transmit(cmd, 5);

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
