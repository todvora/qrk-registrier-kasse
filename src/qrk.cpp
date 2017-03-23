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

#include "defines.h"
#include "database.h"
#include "aboutdlg.h"
#include "qrk.h"
#include "reports.h"
#include "export/exportdep.h"
#include "export/exportjournal.h"
#include "qrkhome.h"
#include "qrkdocument.h"
#include "qrkregister.h"
#include "manager/managerdialog.h"
#include "utils/demomode.h"
#include "import/csvimportwizard.h"
#include "singleton/spreadsignal.h"
#include "RK/rk_signaturemodulefactory.h"
#include "foninfo.h"
#include "backup.h"
#include "utils/utils.h"
#include "preferences/qrksettings.h"
#include "pluginmanager/pluginmanager.h"
#include "pluginmanager/pluginview.h"

#include <QStackedWidget>
#include <QLCDNumber>
#include <QTimer>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QProcess>
#include <QProgressBar>
#include <QTreeView>
#include <QThread>
#include <QDebug>

//-----------------------------------------------------------------------

QRK::QRK(bool servermode)
    : ui(new Ui::MainWindow), m_servermode(servermode)

{
    connect(Spread::Instance(),SIGNAL(updateProgressBar(int,bool)), this,SLOT(setStatusBarProgressBar(int,bool)));
    connect(Spread::Instance(),SIGNAL(updateProgressBarWait(bool)), this,SLOT(setStatusBarProgressBarWait(bool)));
    connect(Spread::Instance(),SIGNAL(updateSafetyDevice(bool)), this,SLOT(setSafetyDevice(bool)));

    ui->setupUi(this);

    m_dep = new QLabel(this);
    m_dep->setMinimumSize(m_dep->sizeHint());
    m_depPX = new QLabel(this);
    m_depPX->setMinimumSize(m_depPX->sizeHint());
    m_safetyDevicePX = new QLabel(this);
    m_safetyDevicePX->setMinimumSize(m_safetyDevicePX->sizeHint());

    m_currentRegisterYearLabel = new QLabel(this);
    m_currentRegisterYearLabel->setMinimumSize(m_currentRegisterYearLabel->sizeHint());
    m_currentRegisterYearLabel->setToolTip(tr("Kassenjahr"));
    m_cashRegisterIdLabel = new QLabel(this);
    m_cashRegisterIdLabel->setMinimumSize(m_cashRegisterIdLabel->sizeHint());
    m_cashRegisterIdLabel->setToolTip(tr("Kassenidentifikationsnummer"));

    m_progressBar = new QProgressBar(this);
    m_dateLcd = new QLCDNumber (this);
    m_dateLcd->setDigitCount(20);
    m_dateLcd->setMode (QLCDNumber::Dec);
    m_dateLcd->setSegmentStyle (QLCDNumber::Flat);
    m_dateLcd->setFrameStyle (QFrame::NoFrame);

    statusBar()->addPermanentWidget(m_dep,0);
    statusBar()->addPermanentWidget(m_depPX,0);
    statusBar()->addPermanentWidget(m_safetyDevicePX,0);
    statusBar()->addPermanentWidget(m_cashRegisterIdLabel,0);
    statusBar()->addPermanentWidget(m_currentRegisterYearLabel,0);
    statusBar()->addPermanentWidget(m_progressBar,0);
    statusBar()->addPermanentWidget(m_dateLcd,0);

    m_timer = new QTimer (this);
    connect (m_timer, SIGNAL(timeout()), SLOT(timerDone()));

    m_currentRegisterYear = QDateTime::currentDateTime().toString("yyyy").toInt();
    QFont font = QApplication::font();
    font.setPointSize(11);
    QApplication::setFont(font);

    // setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    //Stacked Widget
    m_stackedWidget = new QStackedWidget(this);
    this->setCentralWidget(m_stackedWidget);

    m_timer->start(1000);

    iniStack();

    connect(ui->export_CSV, SIGNAL(triggered()), this, SLOT(export_CSV()));
    connect(ui->import_CSV, SIGNAL(triggered()), this, SLOT(import_CSV()));
    connect(ui->export_JSON, SIGNAL(triggered()), this, SLOT(export_JSON()));
    connect(ui->actionDEPexternalBackup, SIGNAL(triggered()), this, SLOT(backupDEP()));
    connect(ui->actionDatenbank_sichern, SIGNAL(triggered(bool)), this, SLOT(backup()));

    connect(ui->actionAbout_QRK, SIGNAL(triggered()), this, SLOT(actionAbout_QRK()));
    connect(ui->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionQRK_Forum, SIGNAL(triggered()), this, SLOT(actionQRK_Forum()));
    connect(ui->actionDEMO_Daten_zur_cksetzen, SIGNAL(triggered()), this, SLOT(actionResetDemoData()));
    connect(ui->actionDEMOMODUS_Verlassen, SIGNAL(triggered()), this, SLOT(actionLeaveDemoMode()));
    connect(ui->actionInfos_zur_Registrierung_bei_FON, SIGNAL(triggered(bool)), this, SLOT(infoFON()));
    connect(ui->actionPlugins, SIGNAL(triggered(bool)), this, SLOT(plugins()));

    connect(m_qrk_home, SIGNAL(endOfDay()), this, SLOT(endOfDaySlot()));
    connect(m_qrk_home, SIGNAL(endOfMonth()), this, SLOT(endOfMonthSlot()));
    connect(m_qrk_home, SIGNAL(fullScreenButton_clicked()), this, SLOT(fullScreenSlot()));
    connect(m_qrk_home, SIGNAL(exitButton_clicked()), this, SLOT(exitSlot()));

    connect(m_qrk_home, SIGNAL(registerButton_clicked()), this, SLOT(onRegisterButton_clicked()));
    connect(m_qrk_home, SIGNAL(documentButton_clicked()), this, SLOT(onDocumentButton_clicked()));
    connect(m_qrk_home, SIGNAL(managerButton_clicked()), this, SLOT(onManagerButton_clicked()));
    connect(m_qrk_home, SIGNAL(refreshMain()), this, SLOT(init()));

    connect(m_qrk_register, SIGNAL(cancelRegisterButton_clicked()), this, SLOT(onCancelRegisterButton_clicked()));
    connect(m_qrk_register, SIGNAL(finishedReceipt()), this, SLOT(finishedReceipt()));

    connect(m_qrk_document, SIGNAL(cancelDocumentButton_clicked()), this, SLOT(onCancelDocumentButton_clicked()));
    connect(m_qrk_document, SIGNAL(documentButton_clicked()), this, SLOT(onDocumentButton_clicked()));

    QString title = QString("QRK V%1.%2 - Qt Registrier Kasse - %3").arg(QRK_VERSION_MAJOR).arg(QRK_VERSION_MINOR).arg(Database::getShopName());
    setWindowTitle ( title );

    init();

    QrkSettings settings;
    restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
    restoreState(settings.value("mainWindowState").toByteArray());

    PluginManager::instance()->initialize();

}

//--------------------------------------------------------------------------------

QRK::~QRK()
{
    delete ui;
    PluginManager::instance()->uninitialize();

}

//--------------------------------------------------------------------------------

void QRK::iniStack()
{
    m_qrk_home =  new QRKHome(m_servermode, m_stackedWidget);
    m_stackedWidget->addWidget(m_qrk_home);

    m_qrk_register =  new QRKRegister(m_stackedWidget);
    m_stackedWidget->addWidget(m_qrk_register);

    m_qrk_document =  new QRKDocument(m_stackedWidget);
    m_stackedWidget->addWidget(m_qrk_document);

    m_stackedWidget->setCurrentWidget(m_qrk_home);
}

//--------------------------------------------------------------------------------

void QRK::timerDone()
{
    QDateTime t = QDateTime::currentDateTime();
    m_dateLcd->display (t.toString("dd-MM-yyyy  hh:mm:ss"));
}

//--------------------------------------------------------------------------------

void QRK::setStatusBarProgressBar(int value, bool add)
{
    if (value < 0) {
        if (m_progressBar->value() > -1)
            m_progressBar->setValue(100);

        QTimer::singleShot(2000, m_progressBar, SLOT(reset()));
        if (QApplication::overrideCursor())
            QApplication::restoreOverrideCursor();

    } else {
        if (!QApplication::overrideCursor())
            QApplication::setOverrideCursor(Qt::WaitCursor);

        if (add)
            value = m_progressBar->value() + value;
        m_progressBar->setValue(value);
    }
}

void QRK::setStatusBarProgressBarWait(bool on_off)
{
    m_progressBar->setMinimum(0);
    if (on_off) {
        m_progressBar->setMaximum(0);
        m_progressBar->setValue(0);

        if (!QApplication::overrideCursor())
            QApplication::setOverrideCursor(Qt::WaitCursor);
   } else {
        m_progressBar->setMaximum(100);
        m_progressBar->reset();

        if (QApplication::overrideCursor())
            QApplication::restoreOverrideCursor();
   }
}

void QRK::setSafetyDevice(bool active)
{
    if (active != m_previousSafetyDeviceState) {
        m_previousSafetyDeviceState = active;
        init();
    }
}

//--------------------------------------------------------------------------------

void QRK::init()
{

    m_currentRegisterYearLabel->setText(QObject::tr(" KJ: %1 ").arg(getCurrentRegisterYear()));
    m_cashRegisterIdLabel->setText(QObject::tr(" KID: %1 ").arg(Database::getCashRegisterId()));

    ui->menuDEMOMODUS->setEnabled(DemoMode::isDemoMode());
    ui->menuDEMOMODUS->menuAction()->setVisible(DemoMode::isDemoMode());
    ui->export_JSON->setVisible(RKSignatureModule::isDEPactive());
    ui->actionInfos_zur_Registrierung_bei_FON->setVisible(RKSignatureModule::isDEPactive());

    setShopName();
    m_cashRegisterId = Database::getCashRegisterId();

    m_qrk_home->init();

    /**
   * @brief DEPaktive
   */
    QPixmap pm1(32, 32);
    QPixmap pm2(32, 32);

    QString DEPtoolTip = tr("DEP Inaktiv");
    QString DEPaktive = tr("Inaktiv");
    m_dep->setText(tr("DEP"));

    if (Database::getTaxLocation() == "AT") {
        m_qrk_home->setExternalDepLabels(true);
        m_dep->setVisible(true);
        m_depPX->setVisible(true);
        m_safetyDevicePX->setVisible(true);

        if (RKSignatureModule::isDEPactive()) {
            RKSignatureModule *signaturinfo = RKSignatureModuleFactory::createInstance("", DemoMode::isDemoMode());

            DEPaktive = tr("Aktiviert");

            QString serial = "0";
            QString cardType = "keine";


            if (signaturinfo->selectApplication()) {
                m_previousSafetyDeviceState = true;
                serial = signaturinfo->getCertificateSerial(true);
                cardType = signaturinfo->getCardType();
                DEPtoolTip = tr("DEP (Daten Erfassungs Protokoll) aktiv, Kartentype: %1 Seriennummer: %2").arg(cardType).arg(serial);
                pm2.fill(Qt::green);
                m_safetyDevicePX->setToolTip(tr("SignaturErstellungsEinheit aktiv, Kartentype: %1 Seriennummer: %2").arg(cardType).arg(serial));
                m_qrk_register->safetyDevice(true);
                m_qrk_home->safetyDevice(true);
            } else {
                m_previousSafetyDeviceState = false;
                DEPtoolTip = tr("DEP (Daten Erfassungs Protokoll) aktiv, SignaturErstellungsEinheit ausgefallen");
                pm2.fill(Qt::red);
                m_safetyDevicePX->setToolTip(tr("SignaturErstellungsEinheit ausgefallen"));
                m_qrk_register->safetyDevice(false);
                m_qrk_home->safetyDevice(false);
            }

            pm1.fill(Qt::green);
            m_depPX->setPixmap(pm1);
            m_depPX->setToolTip(tr("DEP (Daten Erfassungs Protokoll) aktiv"));
            m_safetyDevicePX->setPixmap(pm2);

            delete signaturinfo;
        } else {
            pm1.fill(Qt::red);
            m_depPX->setPixmap(pm1);
            m_safetyDevicePX->setVisible(false);
        }
        m_dep->setToolTip(DEPtoolTip);
    } else {
        m_qrk_home->setExternalDepLabels(false);
        m_dep->setVisible(false);
        m_depPX->setVisible(false);
        m_safetyDevicePX->setVisible(false);
    }
}

//--------------------------------------------------------------------------------

void QRK::setShopName()
{
    m_shopName = Database::getShopName();
}

//--------------------------------------------------------------------------------

void QRK::actionAbout_QRK()
{
    AboutDlg *dlg = new AboutDlg(this);

    dlg->exec();
    dlg->close();

    delete dlg;
}

void QRK::actionQRK_Forum()
{
    QString link = "http://forum.ckvsoft.at/";
    QDesktopServices::openUrl(QUrl(link));
}

//--------------------------------------------------------------------------------
void QRK::import_CSV()
{
    CsvImportWizard *importWizard = new CsvImportWizard(this);
    importWizard->exec();
}

void QRK::export_CSV()
{
    ExportJournal *xport = new ExportJournal(this);
    xport->Export();
    delete xport;
}

void QRK::export_JSON()
{
    ExportDEP *xport = new ExportDEP(this);
    xport->Export();
    delete xport;
}

void QRK::infoFON()
{
    FONInfo *finfo = new FONInfo(this);
    finfo->exec();
    delete finfo;
}

//--------------------------------------------------------------------------------

void QRK::endOfDaySlot()
{
    Reports *rep = new Reports(this);
    rep->endOfDay();
    delete rep;
}

//--------------------------------------------------------------------------------

void QRK::endOfMonthSlot()
{
    Reports *rep = new Reports(this);
    rep->endOfMonth();
    delete rep;
}

void QRK::onCancelDocumentButton_clicked()
{
    m_stackedWidget->setCurrentWidget(m_qrk_home);
}

//--------------------------------------------------------------------------------

void QRK::onRegisterButton_clicked()
{
    m_qrk_register->init();
    m_qrk_register->newOrder();
    m_stackedWidget->setCurrentWidget(m_qrk_register);
}

void QRK::onManagerButton_clicked()
{
    ManagerDialog *manager = new ManagerDialog(this);
    manager->exec();
}

//--------------------------------------------------------------------------------

void QRK::onCancelRegisterButton_clicked()
{
    m_qrk_register->clearModel();
    m_stackedWidget->setCurrentWidget(m_qrk_home);
    m_qrk_home->init();
}

void QRK::finishedReceipt()
{
    m_qrk_register->clearModel();
    m_qrk_home->init();
    m_qrk_register->init();
    m_qrk_register->newOrder();
}

//--------------------------------------------------------------------------------

void QRK::onDocumentButton_clicked()
{
    m_stackedWidget->setCurrentWidget(m_qrk_document);
    m_qrk_document->documentList();
}

//--------------------------------------------------------------------------------

void QRK::fullScreenSlot()
{
    QrkSettings settings;
    if ( isFullScreen() )
    {
        setWindowState(windowState() ^ Qt::WindowFullScreen);
        restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
        restoreState(settings.value("mainWindowState").toByteArray());
    } else {
        settings.value("mainWindowGeometry", saveGeometry());
        settings.value("mainWindowState", saveState());
        showFullScreen();
    }
}

//--------------------------------------------------------------------------------

void QRK::closeEvent (QCloseEvent *event)
{
    if (m_qrk_home->isServerMode()) {
        QMessageBox::information(this, tr("Server Modus aktiv"), tr("Beenden Sie bitte zuerst den Servermodus."));
        event->ignore();
        return;
    }

    QMessageBox mb(tr("Beenden"),
                   tr("Möchten sie wirklich beenden ?"),
                   QMessageBox::Question,
                   QMessageBox::Yes | QMessageBox::Default,
                   QMessageBox::No | QMessageBox::Escape,
                   QMessageBox::NoButton);
    mb.setButtonText(QMessageBox::Yes, tr("Ja"));
    mb.setButtonText(QMessageBox::No, tr("Nein"));

    if (mb.exec() == QMessageBox::Yes) {
        QrkSettings settings;
        settings.removeSettings("QRK_RUNNING", false);
        if ( !isFullScreen() ) {
            settings.save2Settings("mainWindowGeometry", saveGeometry(), false);
            settings.save2Settings("mainWindowState", saveState(), false);
        }
        PluginManager::instance()->uninitialize();
        QApplication::exit();
    } else {
        event->ignore();
    }
}

//--------------------------------------------------------------------------------

void QRK::exitSlot()
{
    QCloseEvent *event = new QCloseEvent();
    closeEvent(event);
}

void QRK::actionLeaveDemoMode()
{
    Database::resetAllData();
    DemoMode::leaveDemoMode();
    restartApplication();
}

void QRK::actionResetDemoData()
{
    Database::resetAllData();
    restartApplication();
}

void QRK::backupDEP()
{
    if (ExportDEP::getLastMonthReceiptId() == -1) {
        QMessageBox::information(this, tr("DEP Datensicherung"), tr("Es wurde kein Monatsbeleg gefunden. Versuchen Sie die Sicherung nach einen Monatsabschlusses erneut."));
        return;
    }

    QrkSettings settings;
    QString directoryname = settings.value("externalDepDirectory", "").toString();
    if (!Utils::isDirectoryWritable(directoryname)) {
        QMessageBox messageBox(QMessageBox::Question,
                               QObject::tr("Externes DEP Backup"),
                               QObject::tr("Das externe Medium %1 ist nicht vorhanden oder nicht beschreibbar. Bitte beheben Sie den Fehler und dann drücken OK. Wenn Sie das Backup zu einen späteren Zeitpunkt durchführen möchten dann klicken Sie abbrechen.").arg(directoryname),
                               QMessageBox::Yes | QMessageBox::No,
                               0);
        messageBox.setButtonText(QMessageBox::Yes, QObject::tr("OK"));
        messageBox.setButtonText(QMessageBox::No, QObject::tr("Abbrechen"));

        if (messageBox.exec() == QMessageBox::No )
        {
            return;
        }
    }

    if (Utils::isDirectoryWritable(directoryname)) {
        ExportDEP *xDep = new ExportDEP();
        bool ok = xDep->createBackup();
        delete xDep;
        if (ok) {
            QMessageBox::information(this, tr("DEP Datensicherung"), tr("DEP Datensicherung abgeschlossen."));
            return;
        } else {
            QMessageBox::information(this, tr("DEP Datensicherung"), tr("DEP Datensicherung fehlgeschlagen."));
            return;
        }
    }

    backupDEP();
}

void QRK::backup()
{
    setStatusBarProgressBarWait(true);
    Backup::create();
    setStatusBarProgressBarWait(false);
    QMessageBox::information(this, tr("Datensicherung"), tr("Datensicherung abgeschlossen."));
}

void QRK::plugins()
{
    PluginView view;
    view.exec();
}

void QRK::restartApplication()
{
    // Spawn a new instance of myApplication:
    QString app = QApplication::applicationFilePath();
    QStringList arguments = QApplication::arguments();
    arguments << "-r";
    QString wd = QDir::currentPath();
    QProcess::startDetached(app, arguments, wd);
    QApplication::exit();
}
