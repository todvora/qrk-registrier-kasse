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

//-----------------------------------------------------------------------

QRK::QRK()
  : ui(new Ui::MainWindow), noPrinter(false)

{

  ui->setupUi(this);

  currentRegisterYearLabel = new QLabel(this);
  cashRegisterIdLabel = new QLabel(this);
  progressBar = new QProgressBar(this);

  dateLcd = new QLCDNumber (this);
  dateLcd->setDigitCount(20);
  dateLcd->setMode (QLCDNumber::Dec);
  dateLcd->setSegmentStyle (QLCDNumber::Flat);
  dateLcd->setFrameStyle (QFrame::NoFrame);

  statusBar()->addPermanentWidget(cashRegisterIdLabel,0);
  statusBar()->addPermanentWidget(currentRegisterYearLabel,0);
  statusBar()->addPermanentWidget(progressBar,0);
  statusBar()->addPermanentWidget(dateLcd,0);

  timer = new QTimer (this);
  connect (timer, SIGNAL(timeout()), SLOT(timerDone()));

  currentRegisterYear = QDateTime::currentDateTime().toString("yyyy").toInt();
  QFont font = QApplication::font();
  font.setPointSize(11);
  QApplication::setFont(font);

  setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);

  //Stacked Widget
  stackedWidget = new QStackedWidget(this);
  this->setCentralWidget(stackedWidget);

  timer->start(1000);

  iniStack();

  connect(ui->actionDEP_Exportieren, SIGNAL(triggered()), this, SLOT(actionDEP_Export()));

  connect(qrk_home, SIGNAL(endOfDay()), this, SLOT(endOfDaySlot()));
  connect(qrk_home, SIGNAL(endOfMonth()), this, SLOT(endOfMonthSlot()));
  connect(qrk_home, SIGNAL(fullScreenButton_clicked()), this, SLOT(fullScreenSlot()));

  connect(qrk_home, SIGNAL(registerButton_clicked()), this, SLOT(onRegisterButton_clicked()));
  connect(qrk_home, SIGNAL(documentButton_clicked()), this, SLOT(onDocumentButton_clicked()));

  connect(qrk_register, SIGNAL(cancelRegisterButton_clicked()), this, SLOT(onCancelRegisterButton_clicked()));
  connect(qrk_register, SIGNAL(registerButton_clicked()), this, SLOT(onRegisterButton_clicked()));

  connect(qrk_document, SIGNAL(cancelDocumentButton_clicked()), this, SLOT(onCancelDocumentButton_clicked()));
  connect(qrk_document, SIGNAL(documentButton_clicked()), this, SLOT(onDocumentButton_clicked()));

}

//--------------------------------------------------------------------------------

QRK::~QRK()
{
  delete ui;
}

//--------------------------------------------------------------------------------

void QRK::iniStack()
{

  qrk_home =  new QRKHome(stackedWidget);
  stackedWidget->addWidget(qrk_home);

  qrk_register =  new QRKRegister(progressBar, stackedWidget);
  stackedWidget->addWidget(qrk_register);

  qrk_document =  new QRKDocument(progressBar, stackedWidget);
  stackedWidget->addWidget(qrk_document);

  stackedWidget->setCurrentIndex(0);
}

//--------------------------------------------------------------------------------

void QRK::timerDone()
{
  QDateTime t = QDateTime::currentDateTime();
  dateLcd->display (t.toString("dd-MM-yyyy  hh:mm:ss"));
}

//--------------------------------------------------------------------------------

void QRK::init()
{
  setShopName();
  cashRegisterId = Database::getCashRegisterId();
}

//--------------------------------------------------------------------------------

void QRK::setShopName()
{
  shopName = Database::getShopName();
}

//--------------------------------------------------------------------------------

void QRK::actionDEP_Export()
{
  DepExportDialog *dlg = new DepExportDialog(this, progressBar);

  if ( dlg->exec() == QDialog::Accepted )
  {
    init();
  }

  delete dlg;
}

//--------------------------------------------------------------------------------

bool QRK::endOfDaySlot()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);
  DEP *dep = new DEP(this);
  Reports *rep = new Reports(dep, this->progressBar, this);
  rep->endOfDay();
  delete dep;
  delete rep;
  this->progressBar->reset();
  QApplication::setOverrideCursor(Qt::ArrowCursor);
  return true;
}

//--------------------------------------------------------------------------------

bool QRK::endOfMonthSlot()
{
  DEP *dep = new DEP(this);
  Reports *rep = new Reports(dep, this->progressBar, this);
  bool ret = rep->endOfMonth();
  lastEOD = Reports::getLastEOD();
  delete dep;
  delete rep;
  this->progressBar->reset();
  this->setCursor(Qt::ArrowCursor);

  return ret;

}

void QRK::onCancelDocumentButton_clicked()
{
  stackedWidget->setCurrentIndex(0);
}

//--------------------------------------------------------------------------------

void QRK::onRegisterButton_clicked()
{
  qrk_register->init();
  qrk_register->newOrder();
  stackedWidget->setCurrentIndex(1);

}

//--------------------------------------------------------------------------------

void QRK::onCancelRegisterButton_clicked()
{
  currentReceipt = 0;
  qrk_register->clearModel();
  stackedWidget->setCurrentIndex(0);
}

//--------------------------------------------------------------------------------

void QRK::onDocumentButton_clicked()
{
  stackedWidget->setCurrentIndex(2);
  qrk_document->documentList();
}

//--------------------------------------------------------------------------------

void QRK::fullScreenSlot()
{

  if ( isFullScreen() )
  {
    showNormal();
    // WeTab does not reparent/show with window frame if not resized
    resize(width() - 150, height());
    showMaximized();
  }
  else
    showFullScreen();
}
