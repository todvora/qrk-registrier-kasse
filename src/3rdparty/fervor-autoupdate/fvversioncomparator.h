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

#ifndef FVVERSIONCOMPARATOR_H
#define FVVERSIONCOMPARATOR_H

#include <iosfwd>
#include <vector>


class FvVersionComparator
{
public:

	typedef enum {
		kSame = 0,
		kDescending = 1,
		kAscending = -1
	} ComparatorResult;

	static ComparatorResult CompareVersions(std::string versionA,
											std::string versionB);

private:

	FvVersionComparator();

	typedef enum {
		kNumberType,
		kStringType,
		kSeparatorType
	} CharacterType;

	static CharacterType TypeOfCharacter(std::string character);
	static std::vector<std::string> SplitVersionString(std::string version);

};

#endif // FVVERSIONCOMPARATOR_H
