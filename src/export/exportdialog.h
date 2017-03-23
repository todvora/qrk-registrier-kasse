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

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

namespace Ui {
  class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

  public:
    ExportDialog(bool save2dir, QWidget *parent = 0);
    ~ExportDialog();
    QString getFilename();
    QString getFrom();
    QString getTo();

signals:

  private slots:
    void onDateEditTo_dateChanged(const QDate &date);
    void onDateEditFrom_dateChanged(const QDate &date);
    void onExportButton_clicked();

  private:
    Ui::ExportDialog *ui;
    bool m_saveTodir;
    QDateTime *m_fromDT;
    QDateTime *m_toDT;
    QString m_filename;

};

#endif // EXPORTDIALOG_H
