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
#include "database.h"
#include "qrkdocument.h"
#include "documentprinter.h"
#include "preferences/qrksettings.h"
#include "reports.h"
#include "qrkdelegate.h"
#include "utils/utils.h"

#include <QMessageBox>
#include <QDesktopWidget>
#include <QJsonObject>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>

QRKDocument::QRKDocument(QWidget *parent)
  : QWidget(parent), ui(new Ui::QRKDocument)

{

  ui->setupUi(this);

  ui->textBrowser->setHidden(true);

  if ( QApplication::desktop()->width() < 1200 )
  {
    ui->cancelDocumentButton->setMinimumWidth(0);
    ui->cancellationButton->setMinimumWidth(0);
    ui->invoiceCompanyPrintcopyButton->setMinimumWidth(0);
    ui->pushFreeButton->setMinimumWidth(0);
  }

  connect(ui->cancelDocumentButton, SIGNAL(clicked()), this, SIGNAL(cancelDocumentButton_clicked()));
  connect(ui->printcopyButton, SIGNAL(clicked()), this, SLOT(onPrintcopyButton_clicked()));
  connect(ui->invoiceCompanyPrintcopyButton, SIGNAL(clicked()), this, SLOT(onInvoiceCompanyButton_clicked()));
  connect(ui->cancellationButton, SIGNAL(clicked()), this, SLOT(onCancellationButton_clicked()));

}

//--------------------------------------------------------------------------------

void QRKDocument::documentList()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");

  ui->documentLabel->setText("");
  m_documentContentModel = new QSqlQueryModel;

  m_documentListModel = new QSortFilterSqlQueryModel;

  QString driverName = dbc.driverName();
  if ( driverName == "QMYSQL" ) {
      m_documentListModel->setQuery("SELECT receipts.receiptNum, actionTypes.actionText, DATE_FORMAT(receipts.infodate, '%Y-%m-%d'),ROUND(receipts.gross,2), receipts.timestamp FROM receipts INNER JOIN actionTypes ON receipts.payedBy=actionTypes.actionId", dbc);
  }
  else if ( driverName == "QSQLITE" ) {
      m_documentListModel->setQuery("SELECT receipts.receiptNum, actionTypes.actionText, strftime('%Y-%m-%d',receipts.infodate),ROUND(receipts.gross,2), receipts.timestamp FROM receipts INNER JOIN actionTypes ON receipts.payedBy=actionTypes.actionId", dbc);
  }

  m_documentListModel->setFilterColumn("receiptNum");
  m_documentListModel->setFilterFlags(Qt::MatchStartsWith);
  m_documentListModel->setFilter("");
  m_documentListModel->select();
  m_documentListModel->sort(DOCUMENT_COL_RECEIPT, Qt::DescendingOrder);

  m_documentListModel->setHeaderData(DOCUMENT_COL_RECEIPT, Qt::Horizontal, tr("Beleg"));
  m_documentListModel->setHeaderData(DOCUMENT_COL_TYPE, Qt::Horizontal, tr("Type"));
  m_documentListModel->setHeaderData(DOCUMENT_COL_INFO, Qt::Horizontal, tr("Info"));
  m_documentListModel->setHeaderData(DOCUMENT_COL_TOTAL, Qt::Horizontal, tr("Summe"));
  m_documentListModel->setHeaderData(DOCUMENT_COL_DATE, Qt::Horizontal, tr("Erstellungsdatum"));

  if (m_documentListModel->lastError().isValid())
      qWarning() << "Function Name: " << Q_FUNC_INFO << " Error: " << m_documentListModel->lastError();

  ui->documentList->setModel(m_documentListModel);
  ui->documentList->setItemDelegateForColumn(DOCUMENT_COL_TOTAL, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));

  ui->documentContent->setModel(m_documentContentModel);
  ui->documentContent->setShowGrid(false);

  ui->documentList->horizontalHeader()->setStretchLastSection(false);
  ui->documentList->resizeColumnsToContents();
  ui->documentContent->resizeColumnsToContents();
  ui->documentList->horizontalHeader()->setStretchLastSection(true);

  ui->documentFilterLabel->setText("Filter " + m_documentListModel->getFilterColumnName());

  connect(m_documentListModel, SIGNAL(sortChanged()), this, SLOT(sortChanged()));
  connect(ui->documentFilterEdit, SIGNAL (textChanged(QString)), m_documentListModel, SLOT (filter(QString)));
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

  int receiptNum = m_documentListModel->data(m_documentListModel->index(row, DOCUMENT_COL_RECEIPT, QModelIndex())).toInt();
  QString payedByText = m_documentListModel->data(m_documentListModel->index(row, DOCUMENT_COL_TYPE, QModelIndex())).toString();
  double price = m_documentListModel->data(m_documentListModel->index(row, DOCUMENT_COL_TOTAL, QModelIndex())).toDouble();
  int type = Database::getActionTypeByName(payedByText);

  bool isDamaged = false;
  ui->pixmapLabel->setPixmap(Utils::getQRCode(receiptNum, isDamaged));
  ui->pixmapLabel->show();
  if (isDamaged)
      ui->qrcodeTextLabel->setText(tr("Sicherheitseinrichtung ausgefallen"));
  else
      ui->qrcodeTextLabel->setText("");

  ui->cancellationButton->setEnabled(!Database::isCashRegisterInAktive() && type < PAYED_BY_REPORT_EOD);
  ui->invoiceCompanyPrintcopyButton->setEnabled(type < PAYED_BY_REPORT_EOD);

  if (type == PAYED_BY_REPORT_EOD || type == PAYED_BY_REPORT_EOM) { /* actionType Tagesbeleg*/
    ui->customerTextLabel->setHidden(true);
    ui->documentContent->setHidden(true);
    ui->textBrowser->setHidden(false);
    ui->printcopyButton->setEnabled(true);
    ui->textBrowser->setHtml(Reports::getReport(receiptNum));

  } else {
    ui->customerTextLabel->setHidden(false);
    ui->documentContent->setHidden(false);
    ui->textBrowser->setHidden(true);
    ui->printcopyButton->setEnabled(true);

    QString stornoText = "";
    if (Database::getStorno(receiptNum) == 1)
      stornoText = tr("(Stornierter Beleg, siehe Beleg Nr: %1)").arg(Database::getStornoId(receiptNum));
    else if (Database::getStorno(receiptNum) == 2)
      stornoText = tr("(Storno Beleg für Beleg Nr: %1)").arg(Database::getStornoId(receiptNum));

    ui->documentLabel->setText(tr("Beleg Nr: %1\t%2\t%3\t\t%4").arg(receiptNum).arg(payedByText).arg(QString::number(price, 'f', 2)).arg(stornoText));
    ui->customerTextLabel->setText(tr("Kunden Zusatztext: ") + Database::getCustomerText(receiptNum));

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    int id = ui->documentList->model()->data(m_documentListModel->index(row, REGISTER_COL_COUNT, QModelIndex())).toInt();

    m_documentContentModel->setQuery(QString("SELECT orders.count, products.name, round(orders.net,2), orders.tax, orders.gross, orders.discount * (-1), ROUND((orders.count * orders.gross) - ((orders.count * orders.gross / 100) * orders.discount),2) AS Price FROM orders INNER JOIN products ON products.id=orders.product WHERE orders.receiptId=%1").arg(id), dbc);
    m_documentContentModel->setHeaderData(REGISTER_COL_COUNT, Qt::Horizontal, tr("Anz."));
    m_documentContentModel->setHeaderData(REGISTER_COL_PRODUCT, Qt::Horizontal, tr("Artikel"));
    m_documentContentModel->setHeaderData(REGISTER_COL_NET, Qt::Horizontal, tr("E-Netto"));
    m_documentContentModel->setHeaderData(REGISTER_COL_TAX, Qt::Horizontal, tr("MwSt."));
    m_documentContentModel->setHeaderData(REGISTER_COL_SINGLE, Qt::Horizontal, tr("E-Preis"));
    m_documentContentModel->setHeaderData(REGISTER_COL_DISCOUNT, Qt::Horizontal, tr("Rabatt %"));
    m_documentContentModel->setHeaderData(REGISTER_COL_TOTAL, Qt::Horizontal, tr("Preis"));
    ui->documentContent->setItemDelegateForColumn(REGISTER_COL_NET, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));
    ui->documentContent->setItemDelegateForColumn(REGISTER_COL_TAX, new QrkDelegate (QrkDelegate::COMBO_TAX, this));
    ui->documentContent->setItemDelegateForColumn(REGISTER_COL_SINGLE, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));
    ui->documentContent->setItemDelegateForColumn(REGISTER_COL_DISCOUNT, new QrkDelegate (QrkDelegate::DISCOUNT, this));
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

    int id = m_documentListModel->data(m_documentListModel->index(row, DOCUMENT_COL_RECEIPT, QModelIndex())).toInt();

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

    int payedBy = Database::getPayedBy(id);
    Reports *rep = new Reports(this,true);
    bool ret = rep->checkEOAnyServerMode();
    delete rep;
    if (! ret) {
        emit documentButton_clicked();
        return;
    }

    ReceiptItemModel *reg = new ReceiptItemModel(this);
    reg->newOrder();
    reg->storno(id);

    m_currentReceipt = reg->createReceipts();
    if ( m_currentReceipt ) {
        reg->setCurrentReceiptNum(m_currentReceipt);
        if ( reg->createOrder(true) ) {
            if ( reg->finishReceipts(payedBy, id) ) {
                emit documentButton_clicked();
                delete reg;
                return;
            }
        }
    }
    delete reg;
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
  int id = m_documentListModel->data(m_documentListModel->index(row, DOCUMENT_COL_RECEIPT, QModelIndex())).toInt();

  if (!id)
    return;


  QString payedByText = m_documentListModel->data(m_documentListModel->index(row, DOCUMENT_COL_TYPE, QModelIndex())).toString();
  int type = Database::getActionTypeByName(payedByText);

  if (type == PAYED_BY_REPORT_EOD || type == PAYED_BY_REPORT_EOM) { /* actionType Tagesbeleg*/
    QString DocumentTitle = QString("BELEG_%1_%2").arg(id).arg(payedByText);
    QTextDocument doc;
    doc.setHtml(Reports::getReport(id));

    DocumentPrinter *p = new DocumentPrinter(this);
    p->printDocument(&doc, DocumentTitle);
    delete p;

    QMessageBox::information(0, QObject::tr("Drucker"), QObject::tr("%1 wurde gedruckt.").arg(payedByText));

  } else {

    m_currentReceipt = id;

    ReceiptItemModel *reg = new ReceiptItemModel(this);
    reg->setCurrentReceiptNum(id);

    QrkSettings settings;
    QJsonObject data = reg->compileData();

    data["isCopy"] = true;
    int storno = Database::getStorno(id);
    if (storno == 2) {
      id = Database::getStornoId(id);
      data["comment"] = (id > 0)? tr("Storno für Beleg Nr: %1").arg(id):settings.value("receiptPrinterHeading", "KASSABON").toString();
    }

    data["isInvoiceCompany"] = isInvoiceCompany;
    data["headerText"] = Database::getCustomerText(id);

    DocumentPrinter *p = new DocumentPrinter(this);
    p->printReceipt(data);
    delete p;
    QMessageBox::information(0, QObject::tr("Drucker"), QObject::tr("%1 %2 ( Kopie) wurde gedruckt.").arg(data.value("comment").toString()).arg(m_currentReceipt));

    emit documentButton_clicked();
  }
}

void QRKDocument::sortChanged()
{
  ui->documentFilterLabel->setText("Filter " + m_documentListModel->getFilterColumnName());
}
