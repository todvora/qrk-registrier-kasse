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

#include "fvplatform.h"
#include <QtGlobal>
#include <QDebug>

FvPlatform::FvPlatform(QObject *parent) :
	QObject(parent)
{
	// noop
}

bool FvPlatform::CurrentlyRunningOnPlatform(QString platform)
{
	platform = platform.toUpper().trimmed();
	if (platform.isEmpty()) {
		return false;
	}

	// Defined on AIX.
#ifdef Q_OS_AIX
	if (platform == "Q_OS_AIX") {
		return true;
	}
#endif

	// Q_OS_BSD4 ("Defined on Any BSD 4.4 system") intentionally skipped.

	// Defined on BSD/OS.
#ifdef Q_OS_BSDI
	if (platform == "Q_OS_BSDI") {
		return true;
	}
#endif

	// Defined on Cygwin.
#ifdef Q_OS_CYGWIN
	if (platform == "Q_OS_CYGWIN") {
		return true;
	}
#endif

	// Q_OS_DARWIN ("Defined on Darwin OS (synonym for Q_OS_MAC)") intentionally skipped.

	// Defined on DG/UX.
#ifdef Q_OS_DGUX
	if (platform == "Q_OS_DGUX") {
		return true;
	}
#endif

	// Defined on DYNIX/ptx.
#ifdef Q_OS_DYNIX
	if (platform == "Q_OS_DYNIX") {
		return true;
	}
#endif

	// Defined on FreeBSD.
#ifdef Q_OS_FREEBSD
	if (platform == "Q_OS_FREEBSD") {
		return true;
	}
#endif

	// Defined on HP-UX.
#ifdef Q_OS_HPUX
	if (platform == "Q_OS_HPUX") {
		return true;
	}
#endif

	// Defined on GNU Hurd.
#ifdef Q_OS_HURD
	if (platform == "Q_OS_HURD") {
		return true;
	}
#endif

	// Defined on SGI Irix.
#ifdef Q_OS_IRIX
	if (platform == "Q_OS_IRIX") {
		return true;
	}
#endif

	// Defined on Linux.
#ifdef Q_OS_LINUX
	if (platform == "Q_OS_LINUX") {
		return true;
	}
#endif

	// Defined on LynxOS.
#ifdef Q_OS_LYNX
	if (platform == "Q_OS_LYNX") {
		return true;
	}
#endif

	// Defined on MAC OS (synonym for Darwin).
#ifdef Q_OS_MAC
	if (platform == "Q_OS_MAC") {
		return true;
	}
#endif

	// Q_OS_MSDOS ("Defined on MS-DOS and Windows") intentionally skipped.

	// Defined on NetBSD.
#ifdef Q_OS_NETBSD
	if (platform == "Q_OS_NETBSD") {
		return true;
	}
#endif

	// Defined on OS/2.
#ifdef Q_OS_OS2
	if (platform == "Q_OS_OS2") {
		return true;
	}
#endif

	// Defined on OpenBSD.
#ifdef Q_OS_OPENBSD
	if (platform == "Q_OS_OPENBSD") {
		return true;
	}
#endif

	// Defined on XFree86 on OS/2 (not PM).
#ifdef Q_OS_OS2EMX
	if (platform == "Q_OS_OS2EMX") {
		return true;
	}
#endif

	// Defined on HP Tru64 UNIX.
#ifdef Q_OS_OSF
	if (platform == "Q_OS_OSF") {
		return true;
	}
#endif

	// Defined on QNX Neutrino.
#ifdef Q_OS_QNX
	if (platform == "Q_OS_QNX") {
		return true;
	}
#endif

	// Defined on Reliant UNIX.
#ifdef Q_OS_RELIANT
	if (platform == "Q_OS_RELIANT") {
		return true;
	}
#endif

	// Defined on SCO OpenServer 5.
#ifdef Q_OS_SCO
	if (platform == "Q_OS_SCO") {
		return true;
	}
#endif

	// Defined on Sun Solaris.
#ifdef Q_OS_SOLARIS
	if (platform == "Q_OS_SOLARIS") {
		return true;
	}
#endif

	// Defined on Symbian.
#ifdef Q_OS_SYMBIAN
	if (platform == "Q_OS_SYMBIAN") {
		return true;
	}
#endif

	// Defined on DEC Ultrix.
#ifdef Q_OS_ULTRIX
	if (platform == "Q_OS_ULTRIX") {
		return true;
	}
#endif

	// Q_OS_UNIX ("Defined on Any UNIX BSD/SYSV system") intentionally skipped.

	// Defined on UnixWare 7, Open UNIX 8.
#ifdef Q_OS_UNIXWARE
	if (platform == "Q_OS_UNIXWARE") {
		return true;
	}
#endif

	// Defined on Windows CE (note: goes before Q_OS_WIN32)
#ifdef Q_OS_WINCE
	if (platform == "Q_OS_WINCE") {
		return true;
	}
#endif

	// Defined on all supported versions of Windows.
#ifdef Q_OS_WIN32
	if (platform == "Q_OS_WIN32") {
		return true;
	}
#endif

	// Fallback
	return false;
}
