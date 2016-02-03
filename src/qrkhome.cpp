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

#include "qrkhome.h"
#include "database.h"

#include <QMessageBox>
#include <QDesktopWidget>
#include <QSettings>


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
    ui->backupDirLabel->setText(settings.value("backupDirectory", qApp->applicationDirPath()).toString());
  }

  ui->dataDirlabel->setText(qApp->applicationDirPath() + "/data");

  ui->lcdNumberDay->display(Database::getDayCounter());
  ui->lcdNumberMonth->display(Database::getMonthCounter());
  ui->lcdNumberYear->display(Database::getYearCounter());

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
