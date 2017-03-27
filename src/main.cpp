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

#if defined(_WIN32) || defined(__APPLE__)
#include "3rdparty/fervor-autoupdate/fvupdater.h"
#endif


#include "qrk.h"
#include "preferences/settingsdialog.h"
#include "preferences/qrksettings.h"
#include "utils/demomode.h"
#include "utils/utils.h"
#include "backup.h"
#include "reports.h"

#include "defines.h"
#include "database.h"
#include "stdio.h"
#include "signal.h"

#include <QDir>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSharedMemory>
#include <QStyleFactory>
#include <QStandardPaths>
#include <QCommandLineParser>

//--------------------------------------------------------------------------------
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDebug>

void QRKMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & str)
{
    QString txt = "";
    bool debug = qApp->property("debugMsg").toBool();
    switch (type) {
    case QtDebugMsg:
        if (debug)
            txt = QString("%1 %2 Debug: %3").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz")).arg(QApplication::applicationVersion()).arg(str);
        break;
    case QtInfoMsg:
        txt = QString("%1 %2 Info: %3").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz")).arg(QApplication::applicationVersion()).arg(str);
        break;
    case QtWarningMsg:
        txt = QString("%1 %2 Warning: %3").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz")).arg(QApplication::applicationVersion()).arg(str);
        break;
    case QtCriticalMsg:
        txt = QString("%1 %2 Critical: %3").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz")).arg( QApplication::applicationVersion()).arg(str);
        break;
    case QtFatalMsg:
        txt = QString("%1 %2 Fatal: %3").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss.zzz")).arg( QApplication::applicationVersion()).arg(str);
        break;
    }

    QFile outFile( QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/qrk.log");
    if (outFile.size() > 20000000) /*20 Mega*/
        Backup::pakLogFile();

    if (!txt.isEmpty()) {
        outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        QTextStream ts(&outFile);
        ts << txt.simplified() << endl;
        outFile.close();
    }
}

void createAppDataLocation()
{
  QDir dir;
  dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
}

void setApplicationFont()
{
  QrkSettings settings;
    QList<QString> systemFontList = settings.value("systemfont").toString().split(",");
  if (systemFontList.count() < 3)
    return;

  QFont sysFont(systemFontList.at(0));
  sysFont.setPointSize(systemFontList.at(1).toInt());
  sysFont.setStretch(systemFontList.at(2).toInt());

  QApplication::setFont(sysFont);

}

void loadStyleSheet(const QString &sheetName)
{
    QFile file(sheetName);
    if (file.exists()) {
        file.open(QFile::ReadOnly);
        QString styleSheet = QString::fromLatin1(file.readAll());

        qApp->setStyleSheet(styleSheet);
    } else {
        qWarning() << "Function Name: " << Q_FUNC_INFO << " QSS File <" << sheetName << "> not found";
    }
}

void setNoPrinter(bool noPrinter = false)
{
    QrkSettings settings;
    settings.save2Settings("noPrinter", noPrinter, false);
}

bool isQRKrunning()
{
  QString s;
  QMessageBox msgWarning(QMessageBox::Warning,"","");

  QrkSettings settings;

  bool rc = settings.value("QRK_RUNNING", false).toBool();

  if(rc)
  {
    msgWarning.setWindowTitle(QObject::tr("Warnung"));
    s = QObject::tr("Das Programm läuft bereits oder wurde das letzte Mal gewaltsam beendet. Bitte überprüfen Sie das!\n(beim nächsten Programmstart wird diese Meldung nicht mehr angezeigt)");
    msgWarning.setText(s);
    msgWarning.exec();
    settings.removeSettings("QRK_RUNNING", false);
  }
  else
  {
    settings.save2Settings("QRK_RUNNING", true, false);
  }
  return rc;
}

//--------------------------------------------------------------------------------

void sighandler(int /*sig*/)
{
    QrkSettings settings;
    settings.removeSettings("QRK_RUNNING", false);
    qApp->exit();
}

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    app.setProperty("debugMsg", true);

    qInstallMessageHandler(QRKMessageHandler);

    QApplication::setOrganizationName("ckvsoft");
    QApplication::setOrganizationDomain("ckvsoft.at");
    QApplication::setApplicationName("QRK");
    QApplication::setApplicationVersion(QString("%1.%2").arg(QRK_VERSION_MAJOR).arg(QRK_VERSION_MINOR));

    createAppDataLocation();

    setApplicationFont();

    QString locale = QLocale::system().name();

    QTranslator trans;
    QString translationPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if ( (trans.load(QLocale(), QLatin1String("qt"), QLatin1String("_"),translationPath) ||
          trans.load(QLocale(), QLatin1String("qt"), QLatin1String("_"),QCoreApplication::applicationDirPath() + QDir::separator() + QLatin1String("i18n"))) )
        app.installTranslator(&trans);

    // QRK translation file is searched in the application install dir (e.g. /opt/qrk)
    // and also in /usr/share/qrk
    QTranslator trans2;  // if current language is not de and translation not found, use english
    if ( !locale.startsWith(QLatin1String("de")) &&
         (trans2.load(QLatin1String("QRK_") + locale, QCoreApplication::applicationDirPath() + QDir::separator() + QLatin1String("i18n")) ||
          trans2.load(QLatin1String("QRK_") + locale, QLatin1String("/usr/share/qrk")) ||
          trans2.load(QLatin1String("QRK_en"), QCoreApplication::applicationDirPath()  + QDir::separator() + QLatin1String("i18n")) ||
          trans2.load(QLatin1String("QRK_en"), QLatin1String("/usr/share/qrk"))) )
        app.installTranslator(&trans2);


    QCommandLineParser parser;
    parser.setApplicationDescription("QRK");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption fullScreenOption(QStringList() << "f" << "fullscreen", QObject::tr("Startet im Vollbild Modus."));
    parser.addOption(fullScreenOption);

    QCommandLineOption minimizeScreenOption(QStringList() << "m" << "minimize", QObject::tr("Startet im Minimize Modus."));
    parser.addOption(minimizeScreenOption);

    QCommandLineOption noPrinterOption(QStringList() << "n" << "noprinter", QObject::tr("Verwendet den internen PDF Drucker."));
    parser.addOption(noPrinterOption);

    QCommandLineOption dbSelectOption("db", QObject::tr("Datenbank Definition Dialog."));
    parser.addOption(dbSelectOption);

    QCommandLineOption restartOption("r", QObject::tr("Erzwingt den Start."));
    parser.addOption(restartOption);

    QCommandLineOption styleSheetOption(QStringList() << "s" << "stylesheet", QObject::tr("StyleSheet <QSS Datei>."), QObject::tr("QSS Datei"));
    parser.addOption(styleSheetOption);

    QCommandLineOption serverModeOption(QStringList() << "servermode", QObject::tr("Startet QRK im Servermode"));
    parser.addOption(serverModeOption);

    QCommandLineOption debugModeOption(QStringList() << "d" << "debug", QObject::tr("Schreibt DEBUG Ausgaben in die Log-Datei"));
    parser.addOption(debugModeOption);

    QCommandLineOption reminderOffOption(QStringList() << "noreminder", QObject::tr("Unterdrückt die RKSV Information"));
    parser.addOption(reminderOffOption);

    parser.process(app);

    bool dbSelect = parser.isSet(dbSelectOption);
    bool fullScreen = parser.isSet(fullScreenOption);
    bool minimize = parser.isSet(minimizeScreenOption);
    bool noPrinter = parser.isSet(noPrinterOption);
    bool servermode = parser.isSet(serverModeOption);
    bool debugMsg = parser.isSet(debugModeOption);
    bool reminderOff  = parser.isSet(reminderOffOption);

#ifdef QT_DEBUG
    debugMsg = true;
    noPrinter = true;
#endif

    if (parser.isSet(restartOption)) {
        QrkSettings settings;
        settings.removeSettings("QRK_RUNNING", false);
    }

    if (parser.isSet(styleSheetOption)) {
        loadStyleSheet(parser.value(styleSheetOption));
    }

    if (isQRKrunning())
      return 0;

    if ( !Database::open( dbSelect) ) {
        sighandler(0);
        return 0;
    }

    QRK *mainWidget = new QRK(servermode);
    mainWidget->show();

    if (!Database::isCashRegisterInAktive() && !DemoMode::isDemoMode() && RKSignatureModule::isDEPactive() && !Utils::checkTurnOverCounter()) {
        QMessageBox messageBox(QMessageBox::Critical,
                               QObject::tr("DEP Fehler"),
                               QObject::tr("ACHTUNG! Leider sind Sie von einem seltenen Rundungsfehler betroffen.\n Bitte sichern Sie Ihre Daten und Melden Sie die Kasse bei FON ab und nochmals neu an.\nInformationen bitte per eMail (info@ckvsoft.at) anfordern.\nBis dahin müssen Sie Belege per Hand erstellen und danch in der neuen Kasse erfassen."),
                               QMessageBox::Yes | QMessageBox::No,
                               0);
        messageBox.setButtonText(QMessageBox::Yes, QObject::tr("Kasse außer Betrieb nehmen?"));
        messageBox.setButtonText(QMessageBox::No, QObject::tr("Weiter machen"));

        if (messageBox.exec() == QMessageBox::Yes )
        {
            mainWidget->closeCashRegister();
            return 0;
        }
    }

    mainWidget->setResuscitationCashRegister(Database::isCashRegisterInAktive());

    mainWidget->statusBar()->setStyleSheet(
                "QStatusBar { border-top: 1px solid lightgrey; border-radius: 1px;"
                "background: lightgrey; spacing: 3px; /* spacing between items in the tool bar */ }"
                );

    setNoPrinter(noPrinter);

    if (DemoMode::isDemoMode())
        debugMsg = true;

    app.setProperty("debugMsg", debugMsg);

    if (fullScreen && !minimize)
        mainWidget->setWindowState(mainWidget->windowState() ^ Qt::WindowFullScreen);
    else if (minimize && !fullScreen)
        mainWidget->setWindowState(mainWidget->windowState() ^ Qt::WindowMinimized);

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

    if (!(Database::getTaxLocation() == "AT"))
        reminderOff = true;

    /* RKSV Reminder */
    if (!RKSignatureModule::isDEPactive() && !reminderOff) {
        QMessageBox messageBox(QMessageBox::Information,
                               QObject::tr("RKSV"),
                               QObject::tr("Ab dem 1.4.2017 benötigen Sie ein Zertifikat welches Sie bei uns beziehen können. http://www.ckvsoft.at"),
                               QMessageBox::Yes,
                               0);
        messageBox.setButtonText(QMessageBox::Yes, QObject::tr("OK"));

        messageBox.exec();
    }

    /*Check for MasterData*/
    QString cri = Database::getCashRegisterId();
    if ( cri.isEmpty() ) {
        QMessageBox::warning(0, QObject::tr("Kassenidentifikationsnummer"), QObject::tr("Stammdaten müssen vollständig ausgefüllt werden.."));
        SettingsDialog tab;
        tab.exec();
        if ( Database::getCashRegisterId().replace("DEMO-", "").isEmpty() ) {
            sighandler(0);
            return 0;
        }
    }

    mainWidget->setShopName();

#if defined(_WIN32) || defined(__APPLE__)
    // Set feed URL before doing anything else
    FvUpdater::sharedUpdater()->SetFeedURL("http://service.ckvsoft.at/qrk/updates/Appcast.xml");
    FvUpdater::sharedUpdater()->setRequiredSslFingerPrint("c3b038cb348c7d06328579fb950a48eb");	// Optional
    FvUpdater::sharedUpdater()->setHtAuthCredentials("username", "password");	// Optional
    FvUpdater::sharedUpdater()->setUserCredentials(Database::getShopName() + "/" + Database::getCashRegisterId() + "/" + QApplication::applicationVersion());
    FvUpdater::sharedUpdater()->setSkipVersionAllowed(true);	// Optional
    FvUpdater::sharedUpdater()->setRemindLaterAllowed(true);	// Optional
    // Finish Up old Updates
    FvUpdater::sharedUpdater()->finishUpdate();

    // Check for updates automatically
    FvUpdater::sharedUpdater()->CheckForUpdatesSilent();
#endif

    signal(SIGTERM, sighandler);

    return app.exec();
}
