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
#include "qrkhome.h"
#include "import/filewatcher.h"
#include "singleton/spreadsignal.h"
#include "qrkprogress.h"
#include "preferences/settingsdialog.h"
#include "preferences/qrksettings.h"
#include "database.h"
#include "utils/utils.h"

#include <QMessageBox>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>

#include <QDebug>

QRKHome::QRKHome(bool servermode, QWidget *parent)
    : QWidget(parent),ui(new Ui::QRKHome), m_menu(0)
{
    connect(Spread::Instance(),SIGNAL(updateSafetyDevice(bool)), this,SLOT(safetyDevice(bool)));

    ui->setupUi(this);

    ui->signatureDamagedLabel->setVisible(false);

    if ( QApplication::desktop()->width() < 1200 )
    {
        ui->documentButton->setMinimumWidth(0);
        ui->managerButton->setMinimumWidth(0);
        ui->registerButton->setMinimumWidth(0);
        ui->taskButton->setMinimumWidth(0);
    }

    // create the menu popup
    {

        m_menu = new QWidget(this, Qt::Popup);
        m_menu->hide();
        QVBoxLayout *vbox = new QVBoxLayout(m_menu);

        QPushButton *b;
        b = new QPushButton(QIcon(":/icons/exit.png"), tr("Beenden"));
        b->setFixedHeight(MIN_HEIGHT);
        b->setIconSize(QSize(32, 32));
        connect(b, SIGNAL(clicked()), this, SIGNAL(exitButton_clicked()));
        vbox->addWidget(b);

        b = new QPushButton(QIcon(":/icons/view-fullscreen.png"), tr("Vollbild"));
        b->setFixedHeight(MIN_HEIGHT);
        b->setIconSize(QSize(32, 32));
        connect(b, SIGNAL(clicked()), this, SIGNAL(fullScreenButton_clicked()));
        vbox->addWidget(b);

        b = new QPushButton(QIcon(":/icons/settings.png"), tr("Einstellungen"));
        b->setFixedHeight(MIN_HEIGHT);
        b->setIconSize(QSize(32, 32));
        connect(b, SIGNAL(clicked()), this, SLOT(settingsSlot()));
        vbox->addWidget(b);

        /*    b = new QPushButton(QIcon(":/icons/reports.png"), tr("Berichte"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    connect(b, SIGNAL(clicked()), this, SLOT(reportsSlot()));
    vbox->addWidget(b);
*/
        connect(ui->menuButton, SIGNAL(clicked()), this, SLOT(menuSlot()));
        // connect(ui->menuButton, SIGNAL(clicked()), this, SLOT(settingsSlot()));

    }

    // create the task popup
    {
        m_task = new QFrame(this, Qt::Popup);
        m_task->setFrameStyle(QFrame::StyledPanel);
        m_task->hide();
        QVBoxLayout *vbox = new QVBoxLayout(m_task);

        QPushButton *b;
        b = new QPushButton(QIcon(":/icons/day.png"), tr("Tagesabschluss"));
        b->setFixedHeight(MIN_HEIGHT);
        b->setIconSize(QSize(32, 32));
        connect(b, SIGNAL(clicked()), this, SIGNAL(endOfDay()));
        vbox->addWidget(b);

        b = new QPushButton(QIcon(":/icons/month.png"), tr("Monatsabschluss"));
        b->setFixedHeight(MIN_HEIGHT);
        b->setIconSize(QSize(32, 32));
        connect(b, SIGNAL(clicked()), this, SIGNAL(endOfMonth()));
        vbox->addWidget(b);

        connect(ui->taskButton, SIGNAL(clicked()), this, SLOT(taskSlot()));
        connect(ui->registerButton, SIGNAL(clicked()), this, SIGNAL(registerButton_clicked()));
        connect(ui->documentButton, SIGNAL(clicked()), this, SIGNAL(documentButton_clicked()));
        connect(ui->managerButton, SIGNAL(clicked()), this, SIGNAL(managerButton_clicked()));
    }

    connect(ui->serverModeCheckBox, SIGNAL(clicked(bool)), this, SLOT(serverModeCheckBox_clicked(bool)));
    connect(Spread::Instance(),SIGNAL(updateImportInfo(QString, bool)), this,SLOT(importInfo(QString, bool)));

    ui->importWidget->setVisible(true);
    setMinimumHeight(400);

    m_fw = new FileWatcher();
    QObject::connect(&m_watcher, SIGNAL(directoryChanged(QString)), m_fw, SLOT(directoryChanged(QString)));

    ui->serverModeCheckBox->setChecked(servermode);
}

QRKHome::~QRKHome()
{
    delete m_fw;
}

void QRKHome::safetyDevice(bool active)
{
    if (active != m_previousSafetyDeviceState) {
        m_previousSafetyDeviceState = active;
        if (!active) {
            ui->signatureDamagedLabel->setVisible(true);
            importInfo(tr("Signaturerstellungseinrichtung ausgefallen!"), true);
            if (isServerMode())
                emit serverModeCheckBox_clicked(false);
        } else {
            ui->signatureDamagedLabel->setVisible(false);
        }
    }
}

void QRKHome::setExternalDepLabels(bool visible)
{
    ui->externalDepDescriptionLabel->setVisible(visible);
    ui->externalDepDirIconLabel->setVisible(visible);
    ui->externalDepDirlabel->setVisible(visible);

}

void QRKHome::init()
{
    QrkSettings settings;
    if (settings.value("backupDirectory").toString().isEmpty()){
        QPixmap pixmap;
        pixmap.load(":icons/cancel.png");
        ui->backupDirIconLabel->setPixmap(pixmap);
        ui->backupDirLabel->setText(tr("n/a"));
    } else {
        QString backupDir = settings.value("backupDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString();
        if (!Utils::isDirectoryWritable(backupDir)) {
            QPixmap pixmap;
            pixmap.load(":icons/cancel.png");
            ui->backupDirIconLabel->setPixmap(pixmap);
        }
        ui->backupDirLabel->setToolTip(backupDir);
        ui->backupDirLabel->setText(backupDir);
    }

    // QFontMetrics metricsDataDir(ui->dataDirlabel->font());
    // ui->dataDirlabel->setText(metricsDataDir.elidedText(dataDir, Qt::ElideMiddle, ui->dataDirlabel->width()));
    QString dataDir = settings.value("sqliteDataDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data").toString();
    if (!Utils::isDirectoryWritable(dataDir)) {
        QPixmap pixmap;
        pixmap.load(":icons/cancel.png");
        ui->dataDirIconLabel->setPixmap(pixmap);
    }
    ui->dataDirlabel->setToolTip(dataDir);
    ui->dataDirlabel->setText(dataDir);

    if (settings.value("pdfDirectory").toString().isEmpty()){
        QPixmap pixmap;
        pixmap.load(":icons/cancel.png");
        ui->pdfDirIconLabel->setPixmap(pixmap);
        ui->pdfDirLabel->setText(tr("n/a"));
    } else {
        QString pdfDir = settings.value("pdfDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString();
        if (!Utils::isDirectoryWritable(pdfDir)) {
            QPixmap pixmap;
            pixmap.load(":icons/cancel.png");
            ui->pdfDirIconLabel->setPixmap(pixmap);
        }
        ui->pdfDirLabel->setToolTip(pdfDir);
        ui->pdfDirLabel->setText(pdfDir);
    }

    if (settings.value("externalDepDirectory").toString().isEmpty()){
        QPixmap pixmap;
        pixmap.load(":icons/cancel.png");
        ui->externalDepDirIconLabel->setPixmap(pixmap);
        ui->externalDepDirlabel->setText(tr("n/a"));
    } else {
        QString externalDepDir = settings.value("externalDepDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString();
        if (!Utils::isDirectoryWritable(externalDepDir)) {
            QPixmap pixmap;
            pixmap.load(":icons/cancel.png");
            ui->externalDepDirIconLabel->setPixmap(pixmap);
        }
        ui->externalDepDirlabel->setToolTip(externalDepDir);
        ui->externalDepDirlabel->setText(externalDepDir);
    }

    ui->lcdNumberDay->display(Database::getDayCounter());
    ui->lcdNumberMonth->display(Database::getMonthCounter());
    ui->lcdNumberYear->display(Database::getYearCounter());
    ui->serverModeCheckBox->setText(tr("Server Modus (Importverzeichnis: %1)").arg(settings.value("importDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString()));

    m_watcherpath = settings.value("importDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString();

    if (!m_watcher.directories().isEmpty()) {
        m_watcher.removePaths(m_watcher.directories());
        qDebug() << "Function Name: " << Q_FUNC_INFO << " remove directories from WatchList";
    }

    if (isServerMode()) {
        m_watcher.addPath(m_watcherpath);
        ui->importWidget->setVisible(true);
        emit serverModeCheckBox_clicked(true);
    } else {
        ui->importWidget->setVisible(false);
        // emit serverModeCheckBox_clicked(false);
    }

    if (Database::isCashRegisterInAktive()) {
        ui->registerButton->setEnabled(false);
        ui->taskButton->setEnabled(false);
        ui->serverModeCheckBox->setEnabled(false);
    }

    // Remove old Importfiles
    QDir dir(m_watcherpath);
    dir.setNameFilters(QStringList() << "*.false" << "*.old");
    dir.setFilter(QDir::Files);
    QStringList list = dir.entryList();
    while(list.size() > 0){
        QString f = list.takeFirst();
        if (QFileInfo(dir.absoluteFilePath(f)).created() < QDateTime::currentDateTime().addDays(-7)) {
            QFile::remove(dir.absoluteFilePath(f));
            qInfo() << "Function Name: " << Q_FUNC_INFO << " Remove file older than 7 Days FileName: " << f;
        }
    }
}

bool QRKHome::isServerMode()
{
    return ui->serverModeCheckBox->isChecked();
}

void QRKHome::menuSlot()
{
    QPoint p(ui->menuButton->x() + ui->menuButton->width() - m_menu->sizeHint().width(),
             ui->menuButton->y() - m_menu->sizeHint().height());

    m_menu->move(mapToGlobal(p));
    m_menu->show();
}

//--------------------------------------------------------------------------------

void QRKHome::settingsSlot()
{
    SettingsDialog *tab = new SettingsDialog();
    if (tab->exec() == QDialog::Accepted )
    {
        init();
        emit refreshMain();
    }
    delete tab;
}

//--------------------------------------------------------------------------------

void QRKHome::taskSlot()
{
    QPoint p(ui->taskButton->x() + ui->taskButton->width() - m_task->sizeHint().width(),
             ui->taskButton->y() - m_task->sizeHint().height());

    m_task->move(mapToGlobal(p));
    m_task->show();
}

void QRKHome::serverModeCheckBox_clicked(bool checked)
{
    ui->importWidget->setVisible(checked);
    ui->registerButton->setEnabled(!checked);
    ui->taskButton->setEnabled(!checked);

    if (checked) {
        if (Utils::isDirectoryWritable(m_watcherpath)) {
            m_watcher.addPath(m_watcherpath);
            /* create and remove file
           * to scan at servermode startup
           */
            QFile f(m_watcherpath + "/scan");
            f.open(QFile::WriteOnly);
            f.putChar('s');
            f.close();
            f.remove();
        } else {
            QMessageBox::warning(this,tr("Fehler"),tr("Import Verzeichnis %1 ist nicht beschreibbar.").arg(m_watcherpath),QMessageBox::Ok);
            ui->serverModeCheckBox->setChecked(false);
            emit serverModeCheckBox_clicked(false);
        }

    } else {

        m_watcher.removePath(m_watcherpath);
        connect(this, SIGNAL(stopWatcher()), m_fw, SIGNAL(stopWorker()));
        emit stopWatcher();

        QRKProgress *p = new QRKProgress(this);
        connect(m_fw, SIGNAL(workerStopped()), p, SLOT(close()));

        p->setText(tr("Warte auf die Fertigstellung der aktuellen Importdatei."));
        p->setWaitMode();
        p->show();
        m_fw->removeDirectories();
        ui->serverModeCheckBox->setChecked(false);
    }
}

void QRKHome::importInfo(QString str, bool isError)
{
    ui->importWidget->addItem(QDateTime::currentDateTime().toString() + ": " + str);
    if (isError) {
        int count = ui->importWidget->count();
        ui->importWidget->item(count -1)->setTextColor(Qt::red);
    } else {
        int count = ui->importWidget->count();
        ui->importWidget->item(count -1)->setTextColor(Qt::darkGreen);
    }

    ui->importWidget->sortItems(Qt::DescendingOrder);
    ui->importWidget->setWordWrap( true);
//    ui->importWidget->repaint();

    ui->lcdNumberDay->display(Database::getDayCounter());
    ui->lcdNumberMonth->display(Database::getMonthCounter());
    ui->lcdNumberYear->display(Database::getYearCounter());

}
