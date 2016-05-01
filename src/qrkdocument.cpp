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
    ui->invoiceCompanyPrintcopyButton->setMinimumWidth(0);
    ui->pushButton_free->setMinimumWidth(0);
  }

  connect(ui->cancelDocumentButton, SIGNAL(clicked()), this, SIGNAL(cancelDocumentButton_clicked()));
  connect(ui->printcopyButton, SIGNAL(clicked()), this, SLOT(onPrintcopyButton_clicked()));
  connect(ui->invoiceCompanyPrintcopyButton, SIGNAL(clicked(bool)), this, SLOT(onInvoiceCompanyButton_clicked()));
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

  ui->documentFilterLabel->setText("Filter " + documentListModel->getFilterColumnName());

  connect(documentListModel, SIGNAL(sortChanged()), this, SLOT(sortChanged()));
  connect(ui->documentFilterEdit, SIGNAL (textChanged(QString)), documentListModel, SLOT (filter(QString)));
  connect(ui->documentList->selectionModel(), SIGNAL (selectionChanged ( const QItemSelection &, const QItemSelection &)),this, SLOT (onDocumentSelectionChanged(const QItemSelection &, const QItemSelection &)));

  ui->cancellationButton->setEnabled(false);
  ui->printcopyButton->setEnabled(false);
  ui->invoiceCompanyPrintcopyButton->setEnabled(false);

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
    ui->customerTextLabel->setHidden(true);
    ui->documentContent->setHidden(true);
    ui->textBrowser->setHidden(false);
    ui->cancellationButton->setEnabled(false);
    ui->invoiceCompanyPrintcopyButton->setEnabled(false);
    ui->printcopyButton->setEnabled(true);
    ui->textBrowser->setHtml(Reports::getReport(receiptNum));

  } else {
    ui->customerTextLabel->setHidden(false);
    ui->documentContent->setHidden(false);
    ui->textBrowser->setHidden(true);
    ui->cancellationButton->setEnabled(true);;
    ui->invoiceCompanyPrintcopyButton->setEnabled(true);
    ui->printcopyButton->setEnabled(true);

    QString stornoText = "";
    if (Database::getStorno(receiptNum) == 1)
      stornoText = tr("(Stornierter Beleg, siehe Beleg Nr: %1)").arg(Database::getStornoId(receiptNum));
    else if (Database::getStorno(receiptNum) == 2)
      stornoText = tr("(Storno Beleg für Beleg Nr: %1)").arg(Database::getStornoId(receiptNum));

    ui->documentLabel->setText(tr("Beleg Nr: %1\t%2\t%3\t\t%4").arg(receiptNum).arg(payedByText).arg(QString::number(price, 'f', 2)).arg(stornoText));
    ui->customerTextLabel->setText(tr("Kunden Zusatztext: ") + Database::getCustomerText(receiptNum));

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    int id = ui->documentList->model()->data(documentListModel->index(row, REGISTER_COL_COUNT, QModelIndex())).toInt();

    documentContentModel->setQuery(QString("SELECT orders.count, products.name, orders.tax, orders.net, orders.gross, orders.count * orders.gross AS Price FROM orders INNER JOIN products ON products.id=orders.product WHERE orders.receiptId=%1").arg(id), dbc);
    documentContentModel->setHeaderData(REGISTER_COL_COUNT, Qt::Horizontal, tr("Anz."));
    documentContentModel->setHeaderData(REGISTER_COL_PRODUCT, Qt::Horizontal, tr("Artikel"));
    documentContentModel->setHeaderData(REGISTER_COL_TAX, Qt::Horizontal, tr("MwSt."));
    documentContentModel->setHeaderData(REGISTER_COL_NET, Qt::Horizontal, tr("E-Netto"));
    documentContentModel->setHeaderData(REGISTER_COL_SINGLE, Qt::Horizontal, tr("E-Preis"));
    documentContentModel->setHeaderData(REGISTER_COL_TOTAL, Qt::Horizontal, tr("Preis"));
    ui->documentContent->setItemDelegateForColumn(REGISTER_COL_NET, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));
    ui->documentContent->setItemDelegateForColumn(REGISTER_COL_SINGLE, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));
    ui->documentContent->setItemDelegateForColumn(REGISTER_COL_TOTAL, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));

    ui->documentContent->resizeColumnsToContents();
    ui->documentContent->horizontalHeader()->setSectionResizeMode(REGISTER_COL_PRODUCT, QHeaderView::Stretch);
    //ui->documentContent->setColumnHidden(REGISTER_COL_NET, true);

  }
}

//--------------------------------------------------------------------------------

void QRKDocument::onCancellationButton_clicked()
{

  ui->documentLabel->setText("");

  QModelIndex idx = ui->documentList->currentIndex();
  int row = idx.row();
  if (row < 0)
      return;

  int id = documentListModel->data(documentListModel->index(row, DOCUMENT_COL_RECEIPT, QModelIndex())).toInt();

  int storno = Database::getStorno(id);
  if (storno)
  {
    QString stornoText = "";
    if (storno == 1)
      stornoText = tr("Beleg mit der Nummer %1 wurde bereits storniert. Siehe Beleg Nr: %2").arg(id).arg(Database::getStornoId(id));
    else
      stornoText = tr("Beleg mit der Nummer %1 ist ein Stornobeleg von Beleg Nummer %2 und kann nicht storniert werden. Erstellen Sie einen neuen Kassebon").arg(id).arg(Database::getStornoId(id));

    QMessageBox::warning( this, tr("Storno"), stornoText);
    emit documentButton_clicked();
    return;
  }

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQueryModel *model = new QSqlQueryModel;
  model->setQuery(QString("SELECT orders.count, products.name, orders.tax, orders.net, orders.gross FROM orders INNER JOIN products ON products.id=orders.product WHERE orders.receiptId=%1").arg(id), dbc);

  int payedBy = Database::getPayedBy(id);

  QRKRegister *reg = new QRKRegister(progressBar);
  reg->setItemModel(model);
  if (! reg->checkEOAny()) {
    emit documentButton_clicked();
//    return;
  }
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

void QRKDocument::onInvoiceCompanyButton_clicked()
{
  onPrintcopyButton_clicked(true);
}

//--------------------------------------------------------------------------------

void QRKDocument::onPrintcopyButton_clicked(bool isInvoiceCompany)
{

  QModelIndex idx = ui->documentList->currentIndex();
  int row = idx.row();
  int id = documentListModel->data(documentListModel->index(row, DOCUMENT_COL_RECEIPT, QModelIndex())).toInt();

  if (!id)
    return;

  QString payedByText = documentListModel->data(documentListModel->index(row, DOCUMENT_COL_TYPE, QModelIndex())).toString();
  int type = Database::getActionTypeByName(payedByText);

  if (type == PAYED_BY_REPORT_EOD || type == PAYED_BY_REPORT_EOM) { /* actionType Tagesbeleg*/
    /* do fix for Month 01*/
    DEP *dep = new DEP(this);
    Reports *rep = new Reports(dep, progressBar);
    rep->fixMonth(id);

    QString DocumentTitle = QString("BELEG_%1_%2").arg(id).arg(payedByText);
    QTextDocument doc;
    doc.setHtml(Reports::getReport(id));

    QApplication::setOverrideCursor(Qt::WaitCursor);
    DocumentPrinter *p = new DocumentPrinter(this, progressBar);
    p->printDocument(&doc, DocumentTitle);
    delete p;
    QApplication::setOverrideCursor(Qt::ArrowCursor);

    QMessageBox::information(0, QObject::tr("Drucker"), QObject::tr("%1 wurde gedruckt.").arg(payedByText));

  } else {

    currentReceipt = id;
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");

    QRKRegister *reg = new QRKRegister(progressBar);
    reg->setCurrentReceiptNum(id);

    QJsonObject data = reg->compileData();

    data["isCopy"] = true;
    int storno = Database::getStorno(id);
    if (storno == 2) {
      id = Database::getStornoId(id);
      data["comment"] = (id > 0)? tr("Storno für Beleg Nr: %1").arg(id):settings.value("receiptPrinterHeading", "KASSABON").toString();
    }

    data["isInvoiceCompany"] = isInvoiceCompany;
    data["headerText"] = Database::getCustomerText(id);

    QApplication::setOverrideCursor(Qt::WaitCursor);
    DocumentPrinter *p = new DocumentPrinter(this, progressBar);
    p->printReceipt(data);
    delete p;
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    QMessageBox::information(0, QObject::tr("Drucker"), QObject::tr("%1 %2 ( Kopie) wurde gedruckt.").arg(data.value("comment").toString()).arg(currentReceipt));

    emit documentButton_clicked();
  }
}

void QRKDocument::sortChanged()
{
  ui->documentFilterLabel->setText("Filter " + documentListModel->getFilterColumnName());
}
