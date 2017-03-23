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

#ifndef BARCODESINTERFACE_H
#define BARCODESINTERFACE_H

#include "plugininterface.h"

class ReceiptItemModel;

class QRK_EXPORT BarcodesInterface : public PluginInterface
{

    Q_OBJECT
    Q_INTERFACES(PluginInterface)

public:
    virtual ~BarcodesInterface() {}
    virtual bool process(ReceiptItemModel *model, int index, QString barcode) = 0;

signals:
    void minusSlot();
    void setColumnHidden(int);
    void finishedReceipt();
};

#define BarcodesInterface_iid "at.ckvsoft.BarcodesInterface"

Q_DECLARE_INTERFACE(BarcodesInterface, BarcodesInterface_iid)

#endif
