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

#include "a_signsmardcard.h"
#include <QDebug>
#include <iostream>

/*
        RK Suite defined in Detailspezifikation/ABS 2
        suite for a closed system (closed systems are identified by the ZDA-ID AT0)
                R1_AT0("1", "AT0", "ES256", "SHA-256", 8),
        suite for an open system (in this case with the virtual ZDA identified by AT100)
                R1_AT100("1", "AT100", "ES256", "SHA-256", 8);
*/

/**
 * @brief ASignSmardCard::ASignSmardCard
 * @param device_name
 */
ASignSmardCard::ASignSmardCard(QString device_name)
    : RKSignatureSmartCard(device_name)
{
}

/**
 * @brief ASignSmardCard::~ASignSmardCard
 */
ASignSmardCard::~ASignSmardCard()
{
}

/**
 * @brief ASignSmardCard::getMessage
 * @param code
 * @return
 */
QString ASignSmardCard::getMessage(const unsigned char code[2]) {
  unsigned short c = (code[0] * 256) + code[1];
  switch (c)
  {
    case 0x6281: return "A.Sign Warning: Part of returned data may be corrupted";
    case 0x6282: return "A.Sign Warning: End of file reached before reading Le bytes";
    case 0x6283: return "A.Sign Warning: Selected file invalidated";
    case 0x6284: return "A.Sign Warning: FCI not formatted according to 5.1.5";
    case 0x63C0: return "A.Sign Warning: Incorrect PIN, 0 attempts left";
    case 0x63C1: return "A.Sign Warning: Incorrect PIN, 1 attempt left";
    case 0x63C2: return "A.Sign Warning: Incorrect PIN, 2 attempts left";
    case 0x63C3: return "A.Sign Warning: Incorrect PIN, 3 attempts left";
    case 0x63C4: return "A.Sign Warning: Incorrect PIN, 4 attempts left";
    case 0x63C5: return "A.Sign Warning: Incorrect PIN, 5 attempts left";
    case 0x63C6: return "A.Sign Warning: Incorrect PIN, 6 attempts left";
    case 0x63C7: return "A.Sign Warning: Incorrect PIN, 7 attempts left";
    case 0x63C8: return "A.Sign Warning: Incorrect PIN, 8 attempts left";
    case 0x63C9: return "A.Sign Warning: Incorrect PIN, 9 attempts left";
    case 0x63CA: return "A.Sign Warning: Incorrect PIN, 10 attempts left";
    case 0x63CB: return "A.Sign Warning: Incorrect PIN, 11 attempts left";
    case 0x63CC: return "A.Sign Warning: Incorrect PIN, 12 attempts left";
    case 0x63CD: return "A.Sign Warning: Incorrect PIN, 13 attempts left";
    case 0x63CE: return "A.Sign Warning: Incorrect PIN, 14 attempts left";
    case 0x63CF: return "A.Sign Warning: Incorrect PIN, 15 attempts left";
    case 0x6581: return "A.Sign Error: Memory failure (unsuccessful writing)";
    case 0x6700: return "A.Sign Error: LC invalid";
    case 0x6883: return "A.Sign Error: Command chain not finished";
    case 0x6981: return "A.Sign Error: Command incompatible with file structure";
    case 0x6982: return "A.Sign Error: Security status not satisfied";
    case 0x6983: return "A.Sign Error: PIN blocked";
    case 0x6985: return "A.Sign Error: Conditions of use not satisfied";
    case 0x6986: return "A.Sign Error: Command not allowed (no current EF)";
    case 0x6A00: return "A.Sign Error: KID not found";
    case 0x6A80: return "A.Sign Error: Invalid parameter in data field";
    case 0x6A81: return "A.Sign Error: Function not supported";
    case 0x6A82: return "A.Sign Error: File not found";
    case 0x6A86: return "A.Sign Error: P1 / P2 invalid";
    case 0x6A87: return "A.Sign Error: LC inconsistent with P1-P2";
    case 0x6A88: return "A.Sign Error: Referenced data (data objects) not found";
    case 0x6B00: return "A.Sign Error: Wrong parameters";
//    case 0x6CXX: return "A.Sign Error: Wrong length (wrong Le field: 'XX' indicates the exact length)";
    case 0x6D00: return "A.Sign Error: Instruction code not supported or invalid";
    case 0x6E00: return "A.Sign Error: Class not supported";
    case 0x6F02: return "A.Sign Error: Data Overflow";
    case 0x6F03: return "A.Sign Error: Command sequence not correct";
    case 0x6F05: return "A.Sign Error: No security environment";
    case 0x9615: return "A.Sign Error: Clear text PIN not allowed";
    case 0x9801: return "A.Sign Error: TIM_ERROR_TLV";
    case 0x9802: return "A.Sign Error: TIM_ERROR_VALUE";
    case 0x9803: return "A.Sign Error: TIM_ERROR_DATA_MISSING";
    case 0x9804: return "A.Sign Error: TIM_ERROR_INVALID_CHARACTER";
    case 0x9811: return "A.Sign Error: TIM_ERROR_DATE_FORMAT";
    case 0x9812: return "A.Sign Error: TIM_ERROR_DATE_OUT_OF_RANGE";
    case 0x9813: return "A.Sign Error: TIM_ERROR_CURRENCY";
    case 0x9821: return "A.Sign Error: TIM_ERROR_TAX_VERIFICATION_FAILED";
    case 0x9822: return "A.Sign Error: TIM_ERROR_NEGATIVE_TURNOVER";
    case 0x9831: return "A.Sign Error: TIM_ERROR_SIGNATURE_INVALID";
    case 0x9841: return "A.Sign Error: TIM_ERROR_INVALID_LIVECYCLE";
    case 0x98E1: return "A.Sign Error: TIM_ERROR_MEMORY_FAILURE";
    case 0x98E2: return "A.Sign Error: TIM_ERROR_DATA_CORRUPTED";
    case 0x98FF: return "A.Sign Error: TIM_ERROR_NOT_SUPPORTED";
    default:
      return "Unknown A.Sign Error: 0000";
  }
}

/**
 * @brief ASignSmardCard::transmit
 * @param txBuffer
 * @param txLength
 * @return
 */
ASignResponse ASignSmardCard::transmit(const unsigned char * txBuffer, DWORD txLength)
{
    ASignResponse result;
    memset(&result, 0, sizeof(ASignResponse));

    if (RKSignatureSmartCard::transmit(txBuffer, txLength, result.data, &result.length)) {
        result.code[0] = result.data[result.length - 2];
        result.code[1] = result.data[result.length - 1];
        result.length -= 2;
        if (result.code[0] != 0x90 || result.code[1] != '\0') {
            qCritical() << "Function Name: " << Q_FUNC_INFO << getMessage(result.code);
        }
    }

    return result;
}

/**
 * @brief ASignSmardCard::selectApplication
 * @return
 */
bool ASignSmardCard::selectApplication()
{
    if (!isCardPresent()) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << "There is no Smart card in the reader";
        return false;
    }
    if (!connect()) {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << "There is a Smart card in the reader but the connection failed";
        return false;
    }
    // There is a Smart card in the reader and connected successful
    return true;
}

QByteArray ASignSmardCard::ReadFile()
{
    QByteArray ba = 0;

    unsigned char cmd[6];
    memcpy(cmd, ASIGN_CMD_READ_BINARY, sizeof(ASIGN_CMD_READ_BINARY));
    ASignResponse response = transmit(cmd, 5);

    DWORD offset = response.length;
    for( DWORD i = 0; i < response.length; i++) {
        ba.append(response.data[i]);
    }
    while (256 == response.length) {
        /*
         * 2.1.6.1 Explicit selection
         * For explicit selection the exact offset is given.
         * OH Offset High Byte
         * OL Offset Low Byte
         *
         * 2.1.6.2 Implicit selection
         * When using implicit selection, the data of the EF with the given short identier (SID) is read.
         * The Short Identier always refers to an EF in the DF just selected before.
         * Therefore, 3 bits of OH are used to indicate the implicit selection
         * B8 B7 B6 B5 B4 B3 B2 B1
         * 1  0  0  x  x  x  x  x
         *          XXXXX is the Short File Identier. The Lower Offset OL is always set to 0x00.
         *
         * 2.1.6.3 Le
         * Le (length expected), 0x00 all digits until the le end but max. 256 bytes
         */

        cmd[2] = (offset >> 8) & 0xFF;
        cmd[3] = offset & 0xFF;
        response = transmit(cmd, 5);
        for( DWORD i = 0; i < response.length; i++) {
            ba.append(response.data[i]);
        }
        offset += response.length;
    }

    return ba;
}
