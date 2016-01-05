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

#include "qrkdocument.h"

#include <QMessageBox>
#include <QJsonObject>
#include <QDesktopWidget>

QRKDocument::QRKDocument(QProgressBar *progressBar, QWidget *parent)
  : QWidget(parent), ui(new Ui::QRKDocument)

{

  this->progressBar = progressBar;
  ui->setupUi(this);

  ui->textBrowser->setHidden(true);

  if ( QApplication::desktop()->width() < 1200 )
  {
    ui->cancelDocumentButton->setMinimumWidth(0);
    ui->cancellationButton->setMinimumWidth(0);
    ui->pushButton->setMinimumWidth(0);
    ui->pushButton_2->setMinimumWidth(0);
  }

  connect(ui->cancelDocumentButton, SIGNAL(clicked()), this, SIGNAL(cancelDocumentButton_clicked()));
  connect(ui->printcopyButton, SIGNAL(clicked()), this, SLOT(onPrintcopyButton_clicked()));
  connect(ui->cancellationButton, SIGNAL(clicked()), this, SLOT(onCancellationButton_clicked()));

  // ----------------------------------------------------------------------------

}

//--------------------------------------------------------------------------------

void QRKDocument::documentList()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");

  ui->documentLabel->setText("");
  documentContentModel = new QSqlQueryModel;

  documentListModel = new QSortFilterSqlQueryModel;
  documentListModel->setQuery("SELECT receipts.receiptNum, actionTypes.actionText, receipts.gross, receipts.timestamp FROM receipts INNER JOIN actionTypes ON receipts.payedBy=actionTypes.actionId", dbc);


  documentListModel->setFilterColumn("receiptNum");
  documentListModel->setFilterFlags(Qt::MatchStartsWith);
  documentListModel->setFilter("");
  documentListModel->select();
  documentListModel->sort(DOCUMENT_COL_RECEIPT, Qt::DescendingOrder);

  documentListModel->setHeaderData(DOCUMENT_COL_RECEIPT, Qt::Horizontal, tr("Beleg"));
  documentListModel->setHeaderData(DOCUMENT_COL_TYPE, Qt::Horizontal, tr("Type"));
  documentListModel->setHeaderData(DOCUMENT_COL_TOTAL, Qt::Horizontal, tr("Preis"));
  documentListModel->setHeaderData(DOCUMENT_COL_DATE, Qt::Horizontal, tr("Datum"));

  ui->documentList->setModel(documentListModel);
  ui->documentList->setItemDelegateForColumn(DOCUMENT_COL_TOTAL, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));

  ui->documentContent->setModel(documentContentModel);
  ui->documentContent->setShowGrid(false);

  ui->documentList->resizeColumnsToContents();
  ui->documentContent->resizeColumnsToContents();

  connect(ui->documentFilterEdit, SIGNAL (textChanged(QString)), documentListModel, SLOT (filter(QString)));
  connect(ui->documentList->selectionModel(), SIGNAL (selectionChanged ( const QItemSelection &, const QItemSelection &)),this, SLOT (onDocumentSelectionChanged(const QItemSelection &, const QItemSelection &)));

}

//----------------SLOTS-----------------------------------------------------------

void QRKDocument::onDocumentSelectionChanged(const QItemSelection &, const QItemSelection & )
{
  QModelIndexList indexList = ui->documentList->selectionModel()->selectedIndexes();
  int row = -1;
  foreach (QModelIndex index, indexList) {
    row = index.row();
  }

  int receiptNum = documentListModel->data(documentListModel->index(row, DOCUMENT_COL_RECEIPT, QModelIndex())).toInt();
  QString payedByText = documentListModel->data(documentListModel->index(row, DOCUMENT_COL_TYPE, QModelIndex())).toString();
  double price = documentListModel->data(documentListModel->index(row, DOCUMENT_COL_TOTAL, QModelIndex())).toDouble();
  int type = Database::getActionTypeByName(payedByText);

  if (type == PAYED_BY_REPORT_EOD || type == PAYED_BY_REPORT_EOM) { /* actionType Tagesbeleg*/
    ui->documentContent->setHidden(true);
    ui->textBrowser->setHidden(false);
    ui->cancellationButton->setHidden(true);
    ui->documentLabel->setText(QString("Beleg Nr: %1\t%2\t%3").arg(receiptNum).arg(payedByText).arg(QString::number(price, 'f', 2)));

    ui->textBrowser->setHtml(Reports::getReport(receiptNum));

  } else {
    ui->documentContent->setHidden(false);
    ui->textBrowser->setHidden(true);
    ui->cancellationButton->setHidden(false);

    QString stornoText = "";
    if (Database::getStorno(receiptNum) == 1)
      stornoText = tr("(Stornierter Beleg, siehe Beleg Nr: %1)").arg(Database::getStornoId(receiptNum));
    else if (Database::getStorno(receiptNum) == 2)
      stornoText = tr("(Storno Beleg für Beleg Nr: %1)").arg(Database::getStornoId(receiptNum));

    ui->documentLabel->setText(QString("Beleg Nr: %1\t%2\t%3\t\t%4").arg(receiptNum).arg(payedByText).arg(QString::number(price, 'f', 2)).arg(stornoText));

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    int id = ui->documentList->model()->data(documentListModel->index(row, 0, QModelIndex())).toInt();

    documentContentModel->setQuery(QString("SELECT orders.count, products.name, orders.tax, orders.gross, orders.count * orders.gross AS Price FROM orders INNER JOIN products ON products.id=orders.product WHERE orders.receiptId=%1").arg(id), dbc);
    documentContentModel->setHeaderData(REGISTER_COL_COUNT, Qt::Horizontal, tr("Anz."));
    documentContentModel->setHeaderData(REGISTER_COL_PRODUCT, Qt::Horizontal, tr("Artikel"));
    documentContentModel->setHeaderData(REGISTER_COL_TAX, Qt::Horizontal, tr("MwSt."));
    documentContentModel->setHeaderData(REGISTER_COL_SINGLE, Qt::Horizontal, tr("E-Preis"));
    documentContentModel->setHeaderData(REGISTER_COL_TOTAL, Qt::Horizontal, tr("Preis"));
    ui->documentContent->setItemDelegateForColumn(REGISTER_COL_SINGLE, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));
    ui->documentContent->setItemDelegateForColumn(REGISTER_COL_TOTAL, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));

    ui->documentContent->resizeColumnsToContents();
    ui->documentContent->horizontalHeader()->setSectionResizeMode(REGISTER_COL_PRODUCT, QHeaderView::Stretch);

  }
}

//--------------------------------------------------------------------------------

void QRKDocument::onCancellationButton_clicked()
{

  ui->documentLabel->setText("");

  QModelIndex idx = ui->documentList->currentIndex();
  int row = idx.row();
  int id = documentListModel->data(documentListModel->index(row, DOCUMENT_COL_RECEIPT, QModelIndex())).toInt();

  int storno = Database::getStorno(id);
  if (storno)
  {
    QString stornoText = "";
    if (storno == 1)
      stornoText = tr("Beleg mit der Nummer %1 wurde bereits storniert. Siehe Beleg Nr: ").arg(id).arg(Database::getStornoId(id));
    else
      stornoText = tr("Beleg mit der Nummer %1 ist ein Stornobeleg von Beleg Nummer %2 und kann nicht storniert werden. Erstellen Sie einen neuen Kassebon").arg(id).arg(Database::getStornoId(id));

    QMessageBox::warning( this, tr("Storno"), stornoText);
    emit documentButton_clicked();
    return;
  }

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQueryModel *model = new QSqlQueryModel;
  model->setQuery(QString("SELECT orders.count, products.name, orders.tax, orders.gross FROM orders INNER JOIN products ON products.id=orders.product WHERE orders.receiptId=%1").arg(id), dbc);

  int payedBy = Database::getPayedBy(id);

  QRKRegister *reg = new QRKRegister(progressBar);
  reg->setItemModel(model);
  if (! reg->checkEOAny())
    emit documentButton_clicked();

  currentReceipt = reg->createReceipts();
  if ( currentReceipt )
  {
    reg->setCurrentReceiptNum(currentReceipt);
    if ( reg->createOrder(true) )
    {
      if ( reg->finishReceipts(payedBy, id) )
      {
        emit documentButton_clicked();
        return;
      }
    }
  }
  // qDebug() << "QRK::onCancellationButton_clicked()";
}

//--------------------------------------------------------------------------------

void QRKDocument::onPrintcopyButton_clicked()
{

  QModelIndex idx = ui->documentList->currentIndex();
  int row = idx.row();
  int id = documentListModel->data(documentListModel->index(row, DOCUMENT_COL_RECEIPT, QModelIndex())).toInt();

  if (!id)
    return;

  QString payedByText = documentListModel->data(documentListModel->index(row, DOCUMENT_COL_TYPE, QModelIndex())).toString();
  int type = Database::getActionTypeByName(payedByText);

  if (type == PAYED_BY_REPORT_EOD || type == PAYED_BY_REPORT_EOM) { /* actionType Tagesbeleg*/
    QTextDocument doc;
    doc.setHtml(Reports::getReport(id));

    QApplication::setOverrideCursor(Qt::WaitCursor);
    DocumentPrinter *p = new DocumentPrinter(this, progressBar);
    p->printDocument(&doc);
    delete p;
    QApplication::setOverrideCursor(Qt::ArrowCursor);

  } else {

    currentReceipt = id;

    QRKRegister *reg = new QRKRegister(progressBar);
    reg->setCurrentReceiptNum(id);

    QJsonObject data = reg->compileData();

    data["isCopy"] = true;
    int storno = Database::getStorno(id);
    if (storno == 2) {
      id = Database::getStornoId(id);
      data["comment"] = (id > 0)? tr("Storno für Beleg Nr: %1-%2").arg(QDate::currentDate().year()).arg(id):tr("KASSABON");
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    DocumentPrinter *p = new DocumentPrinter(this, progressBar);
    p->printReceipt(data);
    delete p;
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    emit documentButton_clicked();
  }
}
