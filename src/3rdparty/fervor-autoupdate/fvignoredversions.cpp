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

#include "fvignoredversions.h"
#include "fvversioncomparator.h"
#include <QSettings>
#include <QCoreApplication>
#include <string>

// QSettings key for the latest skipped version
#define FV_IGNORED_VERSIONS_LATEST_SKIPPED_VERSION_KEY	"FVLatestSkippedVersion"


FVIgnoredVersions::FVIgnoredVersions(QObject *parent) :
QObject(parent)
{
	// noop
}

bool FVIgnoredVersions::VersionIsIgnored(QString version)
{
	// We assume that variable 'version' contains either:
	//	1) The current version of the application (ignore)
	//	2) The version that was skipped before and thus stored in QSettings (ignore)
	//	3) A newer version (don't ignore)
	// 'version' is not likely to contain an older version in any case.
    
	if (version == QCoreApplication::applicationVersion()) {
		return true;
	}
    
    QSettings settings(QSettings::NativeFormat,
					   QSettings::UserScope,
					   QCoreApplication::organizationDomain(),
					   QCoreApplication::applicationName());
    
	//QSettings settings;
	if (settings.contains(FV_IGNORED_VERSIONS_LATEST_SKIPPED_VERSION_KEY)) {
		QString lastSkippedVersion = settings.value(FV_IGNORED_VERSIONS_LATEST_SKIPPED_VERSION_KEY).toString();
		if (version == lastSkippedVersion) {
			// Implicitly skipped version - skip
			return true;
		}
	}
    
	std::string currentAppVersion = QCoreApplication::applicationVersion().toStdString();
	std::string suggestedVersion = std::string(version.toStdString());
	if (FvVersionComparator::CompareVersions(currentAppVersion, suggestedVersion) == FvVersionComparator::kAscending) {
		// Newer version - do not skip
		return false;
	}
    
	// Fallback - skip
	return true;
}

void FVIgnoredVersions::IgnoreVersion(QString version)
{
	if (version == QCoreApplication::applicationVersion()) {
		// Don't ignore the current version
		return;
	}
    
	if (version.isEmpty()) {
		return;
	}
    
    QSettings settings(QSettings::NativeFormat,
					   QSettings::UserScope,
					   QCoreApplication::organizationDomain(),
					   QCoreApplication::applicationName());
    

	settings.setValue(FV_IGNORED_VERSIONS_LATEST_SKIPPED_VERSION_KEY, version);
    
	return;
}
