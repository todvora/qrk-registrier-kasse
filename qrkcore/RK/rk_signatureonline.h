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

#ifndef RKSIGNATUREONLINE_H
#define RKSIGNATUREONLINE_H

#include "RK/rk_signaturemodule.h"

#include "qrkcore_global.h"

class QRK_EXPORT RKSignatureOnline : public RKSignatureModule{

public:

    virtual QString getCertificateSerial(bool hex) = 0;
    virtual QString getCertificate(bool base64 = true) = 0;
    virtual bool selectApplication() = 0;
    virtual QString getCardType() = 0;

protected:

};

#endif // RKSIGNATUREONLINE_H
