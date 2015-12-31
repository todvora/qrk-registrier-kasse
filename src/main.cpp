/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015 Christian Kvasny <chris@ckvsoft.at>
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
 */

#include "qrk.h"
#include "database.h"
#include "stdio.h"
#include "signal.h"

#include <QTranslator>
#include <QLibraryInfo>
#include <QApplication>

//--------------------------------------------------------------------------------

void printHelp()
{
  printf("QRK understands:\n"
         "-db ... open database defintion dialog\n"
         "-f | --fullscreen ... work in fullscreen mode\n"
         "-n | --noPrinter ... for tests, do not print to printer but to PDF\n"
         "--help ... this text\n");
}

//--------------------------------------------------------------------------------

void sighandler(int /*sig*/)
{
  qApp->exit();
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QString locale = QLocale::system().name();

  QTranslator trans;
  QString translationPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
  if ( trans.load(QLatin1String("qt_") + locale, translationPath) )
    app.installTranslator(&trans);

  // QRK translation file is searched in the application install dir (e.g. /opt/QRK)
  // and also in /usr/share/QRK
  QTranslator trans2;  // if current language is not de and translation not found, use english
  if ( !locale.startsWith(QLatin1String("de")) &&
       (trans2.load(QLatin1String("QRK_") + locale, QCoreApplication::applicationDirPath()) ||
        trans2.load(QLatin1String("QRK_") + locale, QLatin1String("/usr/share/QRK")) ||
        trans2.load(QLatin1String("QRK_en"), QCoreApplication::applicationDirPath()) ||
        trans2.load(QLatin1String("QRK_en"), QLatin1String("/usr/share/QRK"))) )
    app.installTranslator(&trans2);

  QRK *mainWidget = new QRK;

  bool dbSelect = false;

  foreach(const QString &arg, QApplication::arguments())
  {
    if ( arg[0] != '-' )
      continue;

    if ( arg == "-db" )
      dbSelect = true;
    else if ( (arg == "--fullscreen") || (arg == "-f") )
    {
      mainWidget->setWindowState(mainWidget->windowState() ^ Qt::WindowFullScreen);
    }
    else if ( (arg == "--noPrinter") || (arg == "-n") )
    {
      mainWidget->setNoPrinter();
    }
    else // if ( arg == "--help" )  // --help or everything else we do not understand
    {
      printHelp();
      return 0;
    }
  }

  // Global
  mainWidget->statusBar()->setStyleSheet(
      "QStatusBar::item { border: 1px solid lightgrey; border-radius: 3px;} "
      );

  mainWidget->show();

  if ( !Database::open( dbSelect) )
    return 0;

  QString title = QString("QRK V%1.%2 - Qt Registrier Kasse - %3").arg(QRK_VERSION_MAJOR).arg(QRK_VERSION_MINOR).arg(Database::getShopName());
  mainWidget->currentRegisterYearLabel->setText(QObject::tr(" Kassenjahr: %1 ").arg(mainWidget->getCurrentRegisterYear()));
  mainWidget->cashRegisterIdLabel->setText(QObject::tr(" Kassenidentifikationsnummer: %1 ").arg(mainWidget->getCashRegisterId()));
  mainWidget->setLastEOD(Reports::getLastEOD());

  mainWidget->setShopName();
  mainWidget->setWindowTitle ( title );

  signal(SIGTERM, sighandler);

  return app.exec();
}
