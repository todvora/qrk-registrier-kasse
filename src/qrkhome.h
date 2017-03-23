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

#ifndef QRKHOME_H
#define QRKHOME_H

#include <QFileInfoList>
#include <QFileSystemWatcher>

#include "ui_qrkhome.h"

class FileWatcher;

class QRKHome : public QWidget
{
    Q_OBJECT
  public:
    explicit QRKHome(bool servermode = false, QWidget *parent = 0);
    ~QRKHome();

    void init();
    bool isServerMode();
    void setExternalDepLabels(bool visible);

  signals:
    void registerButton_clicked();
    void documentButton_clicked();
    void managerButton_clicked();
    void fullScreenButton_clicked();
    void exitButton_clicked();
    void stopWatcher();
    void refreshMain();

    void endOfDay();
    void endOfMonth();

  public slots:
    void safetyDevice(bool);

  private slots:
    void menuSlot();
    void taskSlot();
    void settingsSlot();
    void serverModeCheckBox_clicked(bool checked);
    void importInfo(QString str, bool isError);

  private:
    Ui::QRKHome *ui;
    QWidget *m_menu;
    QFrame *m_task;
    QFileSystemWatcher m_watcher;
    FileWatcher *m_fw;
    QString m_watcherpath;
    bool m_previousSafetyDeviceState;

};

#endif // HOME_H
