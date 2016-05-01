/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015-2016 Christian Kvasny <chris@ckvsoft.at>
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

#ifndef FVUPDATEDOWNLOADPROGRESS_H
#define FVUPDATEDOWNLOADPROGRESS_H

#include <QWidget>
#include "ui_fvupdatedownloadprogress.h"

class FvUpdateDownloadProgress : public QWidget
{
	Q_OBJECT

public:
	FvUpdateDownloadProgress(QWidget *parent = 0);
	~FvUpdateDownloadProgress();

public slots:
	void downloadProgress ( qint64 bytesReceived, qint64 bytesTotal );
	void close();

private:
	Ui::FvUpdateDownloadProgress ui;
};

#endif // FVUPDATEDOWNLOADPROGRESS_H
