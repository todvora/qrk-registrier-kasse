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

#include "fvavailableupdate.h"

FvAvailableUpdate::FvAvailableUpdate(QObject *parent) :
	QObject(parent)
{
	// noop
}

QString FvAvailableUpdate::GetTitle()
{
	return m_title;
}

void FvAvailableUpdate::SetTitle(QString title)
{
	m_title = title;
}

QUrl FvAvailableUpdate::GetReleaseNotesLink()
{
	return m_releaseNotesLink;
}

void FvAvailableUpdate::SetReleaseNotesLink(QUrl releaseNotesLink)
{
	m_releaseNotesLink = releaseNotesLink;
}

void FvAvailableUpdate::SetReleaseNotesLink(QString releaseNotesLink)
{
	SetReleaseNotesLink(QUrl(releaseNotesLink));
}

QString FvAvailableUpdate::GetPubDate()
{
	return m_pubDate;
}

void FvAvailableUpdate::SetPubDate(QString pubDate)
{
	m_pubDate = pubDate;
}

QUrl FvAvailableUpdate::GetEnclosureUrl()
{
	return m_enclosureUrl;
}

void FvAvailableUpdate::SetEnclosureUrl(QUrl enclosureUrl)
{
	m_enclosureUrl = enclosureUrl;
}

void FvAvailableUpdate::SetEnclosureUrl(QString enclosureUrl)
{
	SetEnclosureUrl(QUrl(enclosureUrl));
}

QString FvAvailableUpdate::GetEnclosureVersion()
{
	return m_enclosureVersion;
}

void FvAvailableUpdate::SetEnclosureVersion(QString enclosureVersion)
{
	m_enclosureVersion = enclosureVersion;
}

QString FvAvailableUpdate::GetEnclosurePlatform()
{
	return m_enclosurePlatform;
}

void FvAvailableUpdate::SetEnclosurePlatform(QString enclosurePlatform)
{
	m_enclosurePlatform = enclosurePlatform;
}

unsigned long FvAvailableUpdate::GetEnclosureLength()
{
	return m_enclosureLength;
}

void FvAvailableUpdate::SetEnclosureLength(unsigned long enclosureLength)
{
	m_enclosureLength = enclosureLength;
}

QString FvAvailableUpdate::GetEnclosureType()
{
	return m_enclosureType;
}

void FvAvailableUpdate::SetEnclosureType(QString enclosureType)
{
	m_enclosureType = enclosureType;
}
