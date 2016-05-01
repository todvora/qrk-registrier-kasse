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

#ifndef FVAVAILABLEUPDATE_H
#define FVAVAILABLEUPDATE_H

#include <QObject>
#include <QUrl>

class FvAvailableUpdate : public QObject
{
	Q_OBJECT
public:
	explicit FvAvailableUpdate(QObject *parent = 0);

	QString GetTitle();
	void SetTitle(QString title);

	QUrl GetReleaseNotesLink();
	void SetReleaseNotesLink(QUrl releaseNotesLink);
	void SetReleaseNotesLink(QString releaseNotesLink);

	QString GetPubDate();
	void SetPubDate(QString pubDate);

	QUrl GetEnclosureUrl();
	void SetEnclosureUrl(QUrl enclosureUrl);
	void SetEnclosureUrl(QString enclosureUrl);

	QString GetEnclosureVersion();
	void SetEnclosureVersion(QString enclosureVersion);

	QString GetEnclosurePlatform();
	void SetEnclosurePlatform(QString enclosurePlatform);

	unsigned long GetEnclosureLength();
	void SetEnclosureLength(unsigned long enclosureLength);

	QString GetEnclosureType();
	void SetEnclosureType(QString enclosureType);

private:
	QString m_title;
	QUrl m_releaseNotesLink;
	QString m_pubDate;
	QUrl m_enclosureUrl;
	QString m_enclosureVersion;
	QString m_enclosurePlatform;
	unsigned long m_enclosureLength;
	QString m_enclosureType;

};

#endif // FVAVAILABLEUPDATE_H
