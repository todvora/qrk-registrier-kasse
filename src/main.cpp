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
 */

#include "qrk.h"
#include "settingsdialog.h"
#include "fvupdater.h"
#include "utils/demomode.h"

#include "defines.h"
#include "database.h"
#include "stdio.h"
#include "signal.h"

#include <QTranslator>
#include <QLibraryInfo>
#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include <QStyleFactory>
#include <QtPlugin>

//--------------------------------------------------------------------------------
#include <QFile>
#include <QTextStream>
#include <QDebug>

void QRKMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & str)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("%1 Debug: %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).arg(str);
        break;
    case QtWarningMsg:
        txt = QString("%1 Warning: %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).arg(str);
    break;
    case QtCriticalMsg:
        txt = QString("%1 Critical: %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).arg(str);
    break;
    case QtFatalMsg:
        txt = QString("%1 Fatal: %2").arg(QDateTime::currentDateTime().toString(Qt::ISODate)).arg(str);
    break;
    }
    QFile outFile(qApp->applicationDirPath() + "/qrk.log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt.simplified() << endl;
}

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
  QApplication::setStyle(QStyleFactory::create("Fusion"));

  qInstallMessageHandler(QRKMessageHandler);

  // Prerequisite for the Fervor updater
  QApplication::setOrganizationName("ckvsoft");
  QApplication::setOrganizationDomain("ckvsoft.at");
  QApplication::setApplicationName("QRK");
  QApplication::setApplicationVersion(QString("%1.%2").arg(QRK_VERSION_MAJOR).arg(QRK_VERSION_MINOR));

  // Set feed URL before doing anything else
  FvUpdater::sharedUpdater()->SetFeedURL("http://service.ckvsoft.at/swupdates/Appcast.xml");
  FvUpdater::sharedUpdater()->setRequiredSslFingerPrint("d7e0f2fc899f5ec4456300c7061ff1da");	// Optional
  FvUpdater::sharedUpdater()->setHtAuthCredentials("swupdates", "updatepw");	// Optional
  FvUpdater::sharedUpdater()->setSkipVersionAllowed(true);	// Optional
  FvUpdater::sharedUpdater()->setRemindLaterAllowed(true);	// Optional
  // Finish Up old Updates
  FvUpdater::sharedUpdater()->finishUpdate();

  // Check for updates automatically
  FvUpdater::sharedUpdater()->CheckForUpdatesSilent();

  /*
  QSharedMemory mem("QRK");
  if(!mem.create(1))
  {
    QMessageBox::critical(0,"Instanz erkannt!","QRK wird bereits ausgeführt!\nQRK wird geschlossen...","Ok");
    exit(0);
  }
*/

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

  /*check if we have SET Demomode*/
  if (DemoMode::isModeNotSet()) {
    QMessageBox messageBox(QMessageBox::Question,
                QObject::tr("DEMOMODUS"),
                QObject::tr("Wird QRK (Qt Registrier Kasse) im Echtbetrieb verwendet?\n\nJA wenn QRK im produktiven Einsatz ist.\n\nNEIN wenn DEMO Daten verwendet werden."),
                QMessageBox::Yes | QMessageBox::No,
                0);
        messageBox.setButtonText(QMessageBox::Yes, QObject::tr("Ja"));
        messageBox.setButtonText(QMessageBox::No, QObject::tr("Nein"));

    if (messageBox.exec() == QMessageBox::Yes )
    {
      DemoMode::leaveDemoMode();
    }
  }

  /*Check for MasterData*/
  QString cri = Database::getCashRegisterId();
  if ( cri.isEmpty() ) {
    QMessageBox::warning(0, QObject::tr("Kassenidentifikationsnummer"), QObject::tr("Stammdaten müssen vollständig ausgefüllt werden.."));
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
    SettingsDialog *tab = new SettingsDialog(settings);
    if (tab->exec() == QDialog::Accepted )
    {
      if ( Database::getCashRegisterId().isEmpty() )
        return 0;
    }
  }

  QString title = QString("QRK V%1.%2 - Qt Registrier Kasse - %3").arg(QRK_VERSION_MAJOR).arg(QRK_VERSION_MINOR).arg(Database::getShopName());
  mainWidget->currentRegisterYearLabel->setText(QObject::tr(" Kassenjahr: %1 ").arg(mainWidget->getCurrentRegisterYear()));
  mainWidget->cashRegisterIdLabel->setText(QObject::tr(" Kassenidentifikationsnummer: %1 ").arg(Database::getCashRegisterId()));
  mainWidget->setLastEOD(Reports::getLastEOD());

  mainWidget->setShopName();
  mainWidget->setWindowTitle ( title );
  mainWidget->init();


  signal(SIGTERM, sighandler);

  return app.exec();
}
