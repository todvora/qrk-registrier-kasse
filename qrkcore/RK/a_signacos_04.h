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

#ifndef ASIGNACOS_04_H
#define ASIGNACOS_04_H

#define ASIGN_ACOS_CMD_PUT_HASH               "\x00\x2A\x90\x81\x20" // + 32 byte SHA256

const unsigned char ASIGN_ACOS_DF_SIG[] = {0x00,0xA4,0x00,0x0C,0x02,0xDF,0x70};
const unsigned char ASIGN_ACOS_AID_SIG[] = {0x00,0xA4,0x04,0x00,0x07,0xA0,0x00,0x00,0x01,0x18,0x45,0x43};
const unsigned char ASIGN_ACOS_AID_DEC[] = {0x00,0xA4,0x04,0x00,0x07,0xA0,0x00,0x00,0x01,0x18,0x45,0x4E};
const unsigned char ASIGN_ACOS_EF_C_CH_DS[] = {0x00,0xA4,0x00,0x0C,0x02,0xc0,0x02};
const unsigned char ASIGN_ACOS_DF_DEC[] = {0x00,0xA4,0x00,0x0C,0x02,0xDF,0x71};

#include "RK/a_signsmardcard.h"

#include "qrkcore_global.h"

class QRK_EXPORT ASignACOS_04 : public ASignSmardCard
{

public:
    ASignACOS_04(QString device_name);
    ~ASignACOS_04();

    QString signReceipt(QString data);
    QString getCertificateSerial(bool hex = true);
    QString getCertificate(bool base64=true);
    QString getCIN();
    QString getCardType();

protected:
    bool selectDF_SIG();
    bool selectDF_DEC();
    void getPrivateKey(unsigned char * pkey);

    ASignResponse signHash(const unsigned char pin[6], const unsigned char hash[32]);

};

#endif // ASIGNACOS_04_H
