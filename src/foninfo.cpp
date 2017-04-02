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

#include "foninfo.h"
#include "database.h"
#include "RK/rk_signaturemodulefactory.h"
#include "utils/demomode.h"

#include <QDebug>

#include "ui_foninfo.h"

FONInfo::FONInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FONInfo)
{
    ui->setupUi(this);
    RKSignatureModule *signatureModule = RKSignatureModuleFactory::createInstance("", DemoMode::isDemoMode());
    signatureModule->selectApplication();
    ui->aesKeyLabel->setText(signatureModule->getPrivateTurnoverKeyBase64());
    ui->cashRegisterIdLabel->setText(Database::getCashRegisterId());
    ui->checkSumLabel->setText(signatureModule->getPrivateTurnoverKeyCheckValueBase64Trimmed());
    ui->descriptionLabel->setText("QRK Registrier Kasse # " + Database::getCashRegisterId());
    ui->providerLabel->setText("AT1 A-Trust");
    if (signatureModule->getCardType().contains("Online"))
        ui->securityModuleLabel->setText("Hardware-Sicherheitsmodul (HSM) eines Dienstleister");
    else
        ui->securityModuleLabel->setText("Signaturkarte");

    QString serial = signatureModule->getCertificateSerial(true);
    ui->serialLabel->setText(serial);

    delete signatureModule;
    connect(ui->pushButton, SIGNAL(clicked(bool)),this, SLOT(close()));

}

FONInfo::~FONInfo()
{
    delete ui;
}
