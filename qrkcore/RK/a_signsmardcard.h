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

#ifndef ASIGNSMARDCARD_H
#define ASIGNSMARDCARD_H



// a.sign Commands

#define ASIGN_CMD_VERIFY                      "\x00\x20\x00\x81\x08\x00\x00\x00\x00\x00\x00\x00\x00"

const unsigned char ASIGN_CMD_READ_BINARY[] = {0x00,0xB0,0x00,0x00,0x00};
const unsigned char ASIGN_CMD_COMPUTE_DIGITAL_SIGNATURE[] =  {0x00,0x2A,0x9E,0x9A,0x00};
const unsigned char ASIGN_CMD_MANAGE_SECURITY_ENVIRONMENT[] = {0x00,0x22,0x41,0xB6,0x06,0x84,0x01,0x88,0x80,0x01,0x44};
const unsigned char ASIGN_SELECT_EF_CIN_CSN[] = {0x00,0xA4,0x00,0x0C,0x02,0xD0,0x01};

#include "rk_signaturesmartcard.h"
#include "qrkcore_global.h"

class QString;

class QRK_EXPORT ASignSmardCard : public RKSignatureSmartCard
{
public:
    ASignSmardCard(QString device_name);
    ~ASignSmardCard();

    bool selectApplication();
    virtual QString getCardType() = 0;

protected:
    QString m_CIN;
    bool m_DF_SIG_Selected;

    virtual ASignResponse signHash(const unsigned char pin[6], const unsigned char hash[32]) = 0;

    ASignResponse transmit(const unsigned char *txBuffer, DWORD txLength);
    QString getMessage(const unsigned char code[2]);
    QByteArray ReadFile();

};

//---------------------------------------------------------------------------

#endif // ASIGNSMARDCARD_H
