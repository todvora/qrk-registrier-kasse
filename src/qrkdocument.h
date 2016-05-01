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

#ifndef QRKDOCUMENT_H
#define QRKDOCUMENT_H

#include "defines.h"
#include "qsortfiltersqlquerymodel.h"
#include "qrkdelegate.h"
#include "database.h"
#include "reports.h"
#include "documentprinter.h"

#include "ui_qrkdocument.h"

class QRKDocument : public QWidget
{
    Q_OBJECT

  public:

    explicit QRKDocument(QProgressBar *progressBar, QWidget *parent = 0);
    void documentList();

  signals:
    void cancelDocumentButton_clicked();
    void documentButton_clicked();

  public slots:

  private slots:
    void onCancellationButton_clicked();
    void onPrintcopyButton_clicked(bool = false);
    void onInvoiceCompanyButton_clicked();
    void sortChanged();


  protected slots:
    void onDocumentSelectionChanged(const QItemSelection &, const QItemSelection &);

  private:
    Ui::QRKDocument *ui;
    QSqlQueryModel *documentContentModel;
    QSortFilterSqlQueryModel *documentListModel;

    int currentReceipt;
    QProgressBar *progressBar;
};

#endif // QRKDOCUMENT_H
