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

#ifndef RKSIGNATURESMARTCARD_H
#define RKSIGNATURESMARTCARD_H

#include "RK/rk_signaturemodule.h"
#include "qrkcore_global.h"

#include <QString>

#define MAX_APDU_BUFFER_SIZE 260 // Max is 256 bytes, but allow some additional space

class QRK_EXPORT RKSignatureSmartCard :public RKSignatureModule {

public:
    RKSignatureSmartCard(QString readerName);
    ~RKSignatureSmartCard();

    static void getReaders(QStringList *list);
    static QString getMessage(long id);

    bool isCardPresent();
    LPTSTR getReader(QString readerName);

    virtual QString signReceipt(QString data) = 0;
    virtual QString getCertificateSerial(bool hex) = 0;
    virtual QString getCertificate(bool base64 = true) = 0;
    virtual bool selectApplication() = 0;
    virtual QString getCardType() = 0;

protected:
    SCARDHANDLE  m_hCard;
    SCARDCONTEXT m_hContext;
    DWORD        m_activeProtocol;
    QString      m_reader;

    bool connect();
    bool disconnect();
    bool getAtrString(unsigned char *atr, DWORD *atrLen);
    bool transmit(const unsigned char *txBuffer, unsigned long txLength, unsigned char * rxBuffer, DWORD * rxLength);
    bool getATR(unsigned char atr[33], DWORD &length);

};

#endif //RKSIGNATURESMARTCARD_H
