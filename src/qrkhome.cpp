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

#include "qrkhome.h"
#include "database.h"
#include "import/filewatcher.h"

#include <QMessageBox>
#include <QDesktopWidget>
#include <QSettings>
#include <QStandardPaths>

#include <QDebug>

QRKHome::QRKHome(QWidget *parent)
  : QWidget(parent),ui(new Ui::QRKHome), menu(0)

{
  ui->setupUi(this);

  if ( QApplication::desktop()->width() < 1200 )
  {
    ui->documentButton->setMinimumWidth(0);
    ui->pushButton1->setMinimumWidth(0);
    ui->registerButton->setMinimumWidth(0);
    ui->taskButton->setMinimumWidth(0);
  }

  // create the menu popup
  {
    menu = new QFrame(this, Qt::Popup);
    menu->setFrameStyle(QFrame::StyledPanel);
    menu->hide();
    QVBoxLayout *vbox = new QVBoxLayout(menu);

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

  }

  // create the task popup
  {
    task = new QFrame(this, Qt::Popup);
    task->setFrameStyle(QFrame::StyledPanel);
    task->hide();
    QVBoxLayout *vbox = new QVBoxLayout(task);

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
    connect(ui->serverModeCheckBox, SIGNAL(clicked(bool)), this, SLOT(serverModeCheckBox_clicked(bool)));

    // for TESTS
    connect(ui->pushButton1, SIGNAL(clicked()), this, SIGNAL(managerButton_clicked()));


  }

}

// page 1 Main
//--------------------------------------------------------------------------------

void QRKHome::init()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  if (settings.value("backupDirectory").toString().isEmpty()){
    QPixmap pixmap;
    pixmap.load(":icons/cancel.png");
    ui->backupDirIconLabel->setPixmap(pixmap);
    ui->backupDirLabel->setText(tr("n/a"));
  } else {
    ui->backupDirLabel->setText(settings.value("backupDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString());
  }

  QString dataDir = settings.value("sqliteDataDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/data").toString();
  ui->dataDirlabel->setText(dataDir);

  ui->lcdNumberDay->display(Database::getDayCounter());
  ui->lcdNumberMonth->display(Database::getMonthCounter());
  ui->lcdNumberYear->display(Database::getYearCounter());
  ui->serverModeCheckBox->setText(tr("Server Modus (Importverzeichnis: %1)").arg(settings.value("importDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)  + "/import").toString()));

  watcherpath = settings.value("importDirectory", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +  "/import" ).toString();

  watcher.removePaths(watcher.directories());
  if (ui->serverModeCheckBox->isChecked()) {
    watcher.addPath(watcherpath);
    ui->importWidget->setVisible(true);
  } else {
      ui->importWidget->setVisible(false);
  }

//  watcher.addPath(qApp->applicationDirPath() + "/import/" );

  QStringList directoryList = watcher.directories();
  Q_FOREACH(QString directory, directoryList)
    qDebug() << "Directory name" << directory <<"\n";

  FileWatcher *fw = new FileWatcher();
  QObject::connect(&watcher, SIGNAL(directoryChanged(QString)), fw, SLOT(directoryChanged(QString)));
  QObject::connect(fw, SIGNAL(importInfo(QString)), this, SLOT(importInfo(QString)));
  // QObject::connect(fw, SIGNAL(addToQueue(QFileInfoList)), this, SLOT(addToQueue(QFileInfoList)));

//  QObject::connect(&watcher, SIGNAL(fileChanged(QString)), fw, SLOT(showModified(QString)));

}

void QRKHome::menuSlot()
{
  QPoint p(ui->menuButton->x() + ui->menuButton->width() - menu->sizeHint().width(),
           ui->menuButton->y() - menu->sizeHint().height());

  menu->move(mapToGlobal(p));
  menu->show();
}

//--------------------------------------------------------------------------------

void QRKHome::settingsSlot()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  SettingsDialog *tab = new SettingsDialog(settings);
  if (tab->exec() == QDialog::Accepted )
  {
    init();
  }
}

//--------------------------------------------------------------------------------

void QRKHome::taskSlot()
{
  QPoint p(ui->taskButton->x() + ui->taskButton->width() - task->sizeHint().width(),
           ui->taskButton->y() - task->sizeHint().height());

  task->move(mapToGlobal(p));
  task->show();
}

void QRKHome::serverModeCheckBox_clicked(bool checked)
{
  ui->importWidget->setVisible(checked);
  ui->registerButton->setEnabled(!checked);
  ui->taskButton->setEnabled(!checked);

  if (checked) {
      if (Utils::isDirectoryWritable(watcherpath)) {
          watcher.addPath(watcherpath);
          /* create and remove file
           * to scan at servermode startup
           */
          QFile f(watcherpath + "/scan");
          f.open(QFile::WriteOnly);
          f.putChar('s');
          f.close();
          f.remove();
      } else {
          QMessageBox::warning(this,tr("Fehler"),tr("Import Verzeichnis %1 ist nicht beschreibbar.").arg(watcherpath),QMessageBox::Ok);
          ui->serverModeCheckBox->setChecked(false);
          emit serverModeCheckBox_clicked(false);
      }

  } else {
    watcher.removePath(watcherpath);
  }
}

void QRKHome::importInfo(QString str)
{
    ui->importWidget->sortItems(Qt::DescendingOrder);
    ui->importWidget->setWordWrap( true);
    ui->importWidget->addItem(QDateTime::currentDateTime().toString() + ": " + str);
    if (str.startsWith(tr("Import Fehler"))) {
        int count = ui->importWidget->count();
        ui->importWidget->item(count -1)->setTextColor(Qt::red);
    } else {
        int count = ui->importWidget->count();
        ui->importWidget->item(count -1)->setTextColor(Qt::darkGreen);
    }
    ui->importWidget->sortItems(Qt::DescendingOrder);

    ui->lcdNumberDay->display(Database::getDayCounter());
    ui->lcdNumberMonth->display(Database::getMonthCounter());
    ui->lcdNumberYear->display(Database::getYearCounter());

}
