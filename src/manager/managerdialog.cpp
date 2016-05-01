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

#include <QtWidgets>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QPrinterInfo>

#include "managerdialog.h"
#include "manager/groupswidget.h"
#include "manager/productswidget.h"

ManagerDialog::ManagerDialog(QWidget *parent)
  : QDialog(parent)
{

  groups = new GroupsWidget(this);
  products = new ProductsWidget(this);

  tabWidget = new QTabWidget;
  tabWidget->addTab(groups, tr("Gruppen"));
  tabWidget->addTab(products, tr("Produkte"));

  QPushButton *pushButton = new QPushButton;
  pushButton->setMinimumHeight(60);
  pushButton->setMinimumWidth(0);

  QIcon icon = QIcon(":icons/ok.png");
  QSize size = QSize(32,32);
  pushButton->setIcon(icon);
  pushButton->setIconSize(size);
  pushButton->setText(tr("OK"));
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding );
  buttonLayout->addItem(spacer);
  buttonLayout->addWidget(pushButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(tabWidget);
  mainLayout->addLayout(buttonLayout);
  setLayout(mainLayout);

  setWindowTitle(tr("Manager"));
  setMinimumWidth(800);
  setMinimumHeight(400);

  connect(pushButton, SIGNAL(clicked()), this, SLOT(accept()));

}

void ManagerDialog::accept()
{
  QDialog::accept();

}
