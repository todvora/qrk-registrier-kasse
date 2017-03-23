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

#include "aboutdlg.h"

#include <QFile>
#include <QTextStream>

AboutDlg::AboutDlg( QWidget* parent)
    : QDialog( parent), ui(new Ui::AboutDlg)
{
  ui->setupUi(this);
  readLicense();
    // connect( btClose, SIGNAL( clicked() ), this, SLOT( close() ) );
}

AboutDlg::~AboutDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

void AboutDlg::readLicense()
{
    QFile file;
    file.setFileName(QCoreApplication::applicationDirPath() + "/gpl-3.0." + QLocale::system().name() + ".txt");
    if(!file.exists())
        file.setFileName(QCoreApplication::applicationDirPath() + "/gpl-3.0.txt");

    ui->textBrowser->clear();
    if(file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                ui->textBrowser->append(line);
            }
            file.close();
        }
    } else {
        ui->textBrowser->append(tr("Lizenz Datei (%1) nicht gefunden.").arg(file.fileName()));
    }
}
