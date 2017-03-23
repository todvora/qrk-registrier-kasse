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
#include "a_signacos_04.h"
#include "a_signonline.h"

#include "rk_signaturemodulefactory.h"
#include "rk_smartcardinfo.h"

#include "preferences/qrksettings.h"

#include <QDebug>

/**
 * @brief RKSignatureModuleFactory::createInstance
 * @param reader
 * @return
 */
RKSignatureModule *RKSignatureModuleFactory::createInstance(QString reader, bool demomode) {

    QrkSettings settings;
    if (reader.isNull() || reader.isEmpty())
        reader = settings.value("currentCardReader", "").toString();

    if (demomode && reader.isEmpty()) {
        reader = "u123456789@123456789@https://hs-abnahme.a-trust.at/asignrkonline/v2";
    } else if (reader.isEmpty()) {
        reader = settings.value("atrust_connection", "").toString();
    }

    RKSignatureModule *signatureModule = 0;

    if (reader.isEmpty())
        return new RKSmartCardInfo(reader);

    if (reader.split("@").size() == 3) {
        qDebug("A-Trust Online");
        return new ASignOnline(reader);
    }

    QString ATR;
    ATR = getATR(reader);

    if (ATR == "3BBF11008131FE45455041000000000000000000000000F1" || ATR == "3BBF13008131FE45455041" || ATR == "3BBF11008131FE45455041000000000000000000000000F1") {
        // cardInstance = new SmardCard_ACOS(reader);
        signatureModule = new ASignACOS_04(reader);
        qDebug("Card: ACOS04");
    } else if (ATR == "3BDF18008131FE588031905241016405C903AC73B7B1D444") {
        // cardInstance = new SmartCard_CardOS_5_3(reader);
        signatureModule = new ASignCARDOS_53(reader);
        qDebug("Card: CARDOS 5.3");
    } else {
        signatureModule = new RKSmartCardInfo(reader);
        qWarning() << "Keine oder falsche Karte! ATR:" << ATR;
    }

    signatureModule->selectApplication();
    return signatureModule;
}

/**
 * @brief RKSignatureModuleFactory::getATR
 * @param reader
 * @return
 */
QString RKSignatureModuleFactory::getATR(QString reader)
{
    RKSmartCardInfo *sc = new RKSmartCardInfo(reader);
    QString ATR = sc->getATR();
    delete sc;
    return ATR;
}
