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

#include "qrkregister.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

QRKRegister::QRKRegister(QProgressBar *progressBar, QWidget *parent)
  : QWidget(parent), ui(new Ui::QRKRegister), currentReceipt(0), totallyup(false), noPrinter(false)

{
  ui->setupUi(this);
  this->progressBar = progressBar;
  this->totallyup = totallyup;
  this->noPrinter = noPrinter;

  ui->totallyup->setVisible(false);
  ui->dateEdit->setDate(QDate::currentDate());
  ui->dateEdit->setDateRange(QDate::fromString(QString("%1-01-01").arg(QDate::currentDate().year()),"yyyy-MM-dd"), QDate::currentDate());

  connect(ui->totallyupButton, SIGNAL(clicked()), this, SLOT(totallyupSlot()));
  connect(ui->totallyupExitButton, SIGNAL(clicked()), this, SLOT(totallyupExitSlot()));
  connect(ui->receiptToInvoice, SIGNAL(clicked()), this, SLOT(receiptToInvoiceSlot()));

  connect(ui->plusButton, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui->minusButton, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui->cancelRegisterButton, SIGNAL(clicked()), this, SIGNAL(cancelRegisterButton_clicked()));

  QShortcut *plusButtonShortcut = new QShortcut(QKeySequence("Insert"), this);
  QShortcut *minusButtonShortcut = new QShortcut(QKeySequence("Delete"), this);

  connect(plusButtonShortcut, SIGNAL(activated()), this, SLOT(plusSlot()));
  connect(minusButtonShortcut, SIGNAL(activated()), this, SLOT(minusSlot()));

  ui->buttonGroup->setId(ui->cashReceipt, PAYED_BY_CASH);
  ui->buttonGroup->setId(ui->creditcardReceipt, PAYED_BY_CREDITCARD);
  ui->buttonGroup->setId(ui->debitcardReceipt, PAYED_BY_DEBITCARD);

  connect(ui->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroup_payNow_clicked(int)));

  //    connect(&productsMapper, SIGNAL(mapped(int)), this, SLOT(productSelected(int)));

  orderListModel = new ReceiptItemModel(this);

}

//--------------------------------------------------------------------------------

QString QRKRegister::getHeaderText()
{
  return ui->headerText->text();
}

//--------------------------------------------------------------------------------

void QRKRegister::init()
{
  QDate last = Database::getLastReceiptDate();
  if (last.toString(Qt::ISODate) == lastEOD.toString(Qt::ISODate))
    last = last.addDays(1);

  ui->dateEdit->setDateRange(last, QDate::currentDate());

  ui->orderList->setModel(orderListModel);
  connect(ui->orderList->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(itemChangedSlot(const QModelIndex&, const QModelIndex&)));
  //  stackedWidget->setCurrentIndex(1);

}

//--------------------------------------------------------------------------------

void QRKRegister::updateOrderSum()
{
  double sum = 0;

  for (int row = 0; row < orderListModel->rowCount(); row++)
  {
    QStringList temp = ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_TOTAL, QModelIndex())).toString().split(" ");

    double d4 = temp[0].toDouble();

    sum += d4;
  }

  //  sum = (long)(sum*100+0.5)/100.0;
  ui->sumLabel->setText(tr("%1 €").arg(QString::number(sum, 'f', 2)));

}

//--------------------------------------------------------------------------------

bool QRKRegister::finishReceipts(int payedBy, int id, bool isReport)
{

  QDateTime receiptTime;

  if (totallyup) {
    receiptTime.setDate(ui->dateEdit->date());
    if(QDate::currentDate() == ui->dateEdit->date())
      receiptTime.setTime(QTime::currentTime());
    else
      receiptTime.setTime(QTime::fromString("23:59:59"));
  }
  else
    receiptTime = QDateTime::currentDateTime();

  QSqlDatabase dbc = QSqlDatabase::database("CN");

  QSqlQuery query(dbc);
  query.exec("SELECT value FROM globals WHERE name='lastReceiptNum'");
  query.next();
  int receiptNum = query.value(0).toInt();

  receiptNum++;
  query.exec(QString("UPDATE globals SET value=%1 WHERE name='lastReceiptNum'").arg(receiptNum));

  double sum = 0.0;
  double net = 0.0;

  if (!isReport) {

    QSqlQuery orders(dbc);
    orders.prepare(QString("SELECT orders.count, orders.gross, orders.tax FROM orders WHERE orders.receiptId=%1")
                   .arg(receiptNum));

    orders.exec();

    while ( orders.next() )
    {
      int count = orders.value("count").toInt();
      double singlePrice = orders.value("gross").toDouble();
      int tax = orders.value("tax").toInt();

      double gross = singlePrice * count;
      sum += gross;
      net += gross / (1.0 + tax / 100.0);
    }
  }

  QString signature = Utils::getSignature(receiptTime, sum, net, receiptNum);
  query.exec(QString("UPDATE receipts SET timestamp='%1', receiptNum=%2, payedBy=%3, gross=%4, net=%5,signature='%6' WHERE id=%7")
             .arg(receiptTime.toString(Qt::ISODate))
             .arg(receiptNum)
             .arg(payedBy)
             .arg(sum)
             .arg(net)
             .arg(signature)
             .arg(receiptNum));

  if (isReport)
    return true;

  if (id)
    Database::setStornoId(receiptNum, id);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  QJsonObject data = compileData(id);
  DocumentPrinter *p = new DocumentPrinter(this, progressBar, noPrinter);
  p->printReceipt(data);

  delete p;

  DEP *dep = new DEP(this);
  dep->depInsertReceipt(data);
  delete dep;
  QApplication::setOverrideCursor(Qt::ArrowCursor);

  return true;

}

//--------------------------------------------------------------------------------

bool QRKRegister::createOrder(bool storno)
{

  bool ret = false;

  for (int row = 0; row < ui->orderList->model()->rowCount(); row++)
  {
    int count = ui->orderList->model()->data(ui->orderList->model()->index(row, 0, QModelIndex())).toInt();
    if (storno)
      count *= -1;

    QString product = ui->orderList->model()->data(ui->orderList->model()->index(row, 1, QModelIndex())).toString();
    int tax = ui->orderList->model()->data(ui->orderList->model()->index(row, 2, QModelIndex())).toDouble();
    double egross = ui->orderList->model()->data(ui->orderList->model()->index(row, 3, QModelIndex())).toDouble();

    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc) ;

    QString q = QString("INSERT INTO orders (receiptId, product, count, gross, tax) SELECT %1, id, %2, %3, %4 FROM products WHERE name='%5'")
        .arg(currentReceipt)
        .arg(count)
        .arg(egross)
        .arg(tax)
        .arg(product);

    ret = query.exec( q );

    if (!ret)
      qDebug()  << "QRK::createOrUpdateOrder " << query.lastError().text();

  }

  return ret;
}

//--------------------------------------------------------------------------------

int QRKRegister::createReceipts()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery query(dbc);

  try {
    query.exec(QString("INSERT INTO receipts (`timestamp`) VALUES('%1')")
               .arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
  } catch (QSqlError &e) {
    qDebug() << "QRK::createReceipts() " << e.text();
  }

  QString driverName = dbc.driverName();
  if ( driverName == "QMYSQL" ) {
    query.prepare("SELECT LAST_INSERT_ID()");
    query.exec();
  }
  else if ( driverName == "QSQLITE" ) {
    query.prepare("SELECT last_insert_rowid()");
    query.exec();
  }

  query.next();

  return query.value(0).toInt();
}

//--------------------------------------------------------------------------------

void QRKRegister::newOrder()
{

  QStringList list = Database::getLastReceipt();

  ui->headerText->clear();

  if (!list.empty()) {
    QString date = list.takeAt(0);
    QString bonNr = list.takeAt(0);
    QString taxName = Database::getActionType(list.takeAt(0).toInt());
    double gross = list.takeAt(0).toDouble();

    if (bonNr.length()) {
      ui->lastReceiptLabel->setText(QString(tr("Letzter Barumsatz: BON Nr. %1, %2, %3: %4 %5"))
                                    .arg(bonNr)
                                    .arg(date)
                                    .arg(taxName)
                                    .arg(QString::number(gross, 'f', 2)).arg(tr("€")));
    }
  }

  ui->newReceiptLabel->setText(QString(tr("Neuer Beleg")));

  currentReceipt = 0;  // a new receipt not yet in the DB

  orderListModel->setColumnCount(5);
  orderListModel->setHeaderData(REGISTER_COL_COUNT, Qt::Horizontal, QObject::tr("Anzahl"));
  orderListModel->setHeaderData(REGISTER_COL_PRODUCT, Qt::Horizontal, QObject::tr("Produkt"));
  orderListModel->setHeaderData(REGISTER_COL_TAX, Qt::Horizontal, QObject::tr("MwSt."));
  orderListModel->setHeaderData(REGISTER_COL_SINGLE, Qt::Horizontal, QObject::tr("E-Preis"));
  orderListModel->setHeaderData(REGISTER_COL_TOTAL, Qt::Horizontal, QObject::tr("Preis"));

  //  ui->orderList->horizontalHeader()->saveGeometry();
  //    ui->orderList->setColumnWidth(1, 350);
  ui->orderList->setAutoScroll(true);
  ui->orderList->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->orderList->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

  ui->orderList->setItemDelegateForColumn(REGISTER_COL_COUNT, new QrkDelegate (QrkDelegate::SPINBOX, this));
  ui->orderList->setItemDelegateForColumn(REGISTER_COL_PRODUCT, new QrkDelegate (QrkDelegate::PRODUCTS, this));
  ui->orderList->setItemDelegateForColumn(REGISTER_COL_TAX, new QrkDelegate (QrkDelegate::COMBO_TAX, this));
  ui->orderList->setItemDelegateForColumn(REGISTER_COL_SINGLE, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));
  ui->orderList->setItemDelegateForColumn(REGISTER_COL_TOTAL, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));

  ui->orderList->horizontalHeader()->setSectionResizeMode(REGISTER_COL_PRODUCT, QHeaderView::Stretch);

  updateOrderSum();
  plusSlot();
}

//--------------------------------------------------------------------------------

QJsonObject QRKRegister::compileData(int id)
{

  QSqlDatabase dbc = QSqlDatabase::database("CN");

  QSqlQuery query(dbc);
  QString q;
  QJsonObject Root;//root object

  // receiptNum, ReceiptTime
  q = (QString("SELECT `receiptNum`,`timestamp`, `payedBy` FROM receipts WHERE id=%1").arg(currentReceipt));
  query.exec(q);
  query.next();
  int receiptNum = query.value(0).toInt();
  QDateTime receiptTime = query.value(1).toDateTime();
  int payedBy = query.value(2).toInt();

  // Positions
  q = QString("SELECT COUNT(*) FROM orders WHERE receiptId=%1").arg(currentReceipt);
  query.exec(q);
  query.next();
  int positions = query.value(0).toInt();

  // sum Year
  QString year = receiptTime.toString("yyyy");
  q = QString("SELECT SUM(gross) AS Total FROM receipts where timestamp like '%1%'").arg(year);
  query.exec(q);
  query.next();
  double sumYear = query.value(0).toDouble();

  // Header
  q = QString("SELECT strValue FROM globals WHERE name='printHeader'");
  query.exec(q);
  query.next();
  Root["printHeader"] = query.value(0).toString();

  // Footer
  q = QString("SELECT strValue FROM globals WHERE name='printFooter'");
  query.exec(q);
  query.next();
  Root["printFooter"] = query.value(0).toString();

  // TaxTypes

  QSqlQuery taxTypes(dbc);
  taxTypes.prepare(QString("SELECT tax, comment FROM taxTypes ORDER BY id"));
  taxTypes.exec();
  while(taxTypes.next())
  {
    Root[taxTypes.value(1).toString()] = 0.0;
  }

  // Orders
  QSqlQuery orders(dbc);
  orders.prepare(QString("SELECT orders.count, products.name, orders.gross, orders.tax FROM orders INNER JOIN products ON products.id=orders.product WHERE orders.receiptId=%1")
                 .arg(currentReceipt));

  orders.exec();

  // ZNr Programmversion Kassen-Id Beleg Belegtyp Bemerkung Nachbonierung
  // Belegnummer Datum Umsatz_Normal Umsatz_Ermaessigt1 Umsatz_Ermaessigt2
  // Umsatz_Null Umsatz_Besonders Jahresumsatz_bisher Erstellungsdatum

  Root["version"] = QString("%1.%2").arg(QRK_VERSION_MAJOR).arg(QRK_VERSION_MINOR);
  Root["action"] = DEP_RECEIPT;
  Root["kasse"] = Database::getCashRegisterId();
  Root["actionText"] = tr("Beleg");
  Root["typeText"] = Database::getActionType(payedBy);
  Root["shopName"] = Database::getShopName();
  Root["headerText"] = getHeaderText();
  Root["totallyup"] = (totallyup)? "Nachbonierung":"";
  Root["comment"] = (id > 0)? tr("Storno für Beleg Nr: %1-%2").arg(QDate::currentDate().year()).arg(id):tr("KASSABON");
  Root["receiptNum"] = receiptNum;
  Root["receiptTime"] = receiptTime.toString(Qt::ISODate);
  Root["currentRegisterYear"] = QDate::currentDate().year();

  QJsonArray Orders;

  double sum = 0;
  QMap<int, double> taxes; // <tax-percent, sum>

  while(orders.next())//load all data from the database
  {
    int count = orders.value(0).toInt();
    double singlePrice = orders.value(2).toDouble();
    double gross = singlePrice * count;
    int tax = orders.value(3).toInt();

    sum += gross;

    if ( taxes.contains(tax) )
      taxes[tax] += (gross * (tax / 100.0)) / (1.0 + tax / 100.0);
    else
      taxes[tax] = (gross * (tax / 100.0)) / (1.0 + tax / 100.0);

    QJsonObject order;
    order["count"] = count;
    order["product"] = orders.value(1).toString();
    order["gross"] = gross;
    order["singleprice"] = singlePrice;
    order["tax"] = tax;
    Orders.append(order);

    QString taxType = Database::getTaxType(tax);
    Root[taxType] = Root[taxType].toDouble() + gross;
  }

  QJsonArray Taxes;
  QList<int> keys = taxes.keys();
  for (int i = 0; i < keys.count(); i++)
  {
    QJsonObject tax;
    tax["t1"] = QString("%1%").arg( keys[i] );
    tax["t2"] = QString::number(taxes[keys[i]], 'f', 2);
    Taxes.append(tax);
  }

  Root["Orders"] = Orders;
  Root["sum"] = sum;
  Root["sumYear"] = sumYear;
  Root["positions"] = positions;
  //  Root["positionsText"] = tr("Anzahl verkaufter Produkte oder Leistungen: %1").arg(positions);
  Root["Taxes"] = Taxes;
  Root["taxesCount"] = taxes.count();

  return Root;

}

//-------------SLOTS--------------------------------------------------------------

void QRKRegister::itemChangedSlot( const QModelIndex& i, const QModelIndex&)
{

  int row = i.row();
  int col = i.column();
  double sum(0.0);

  QString s = ui->orderList->model()->data(orderListModel->index(row, col, QModelIndex())).toString();

  //  QString s2;
  QStringList temp = ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_SINGLE, QModelIndex())).toString().split(" ");
  double d2 = temp[0].replace(",", ".").toDouble();

  /* initialize QSqlDatabase dbc & QSqlQuery query(dbc)
   * will not work in a switch block
   */

  if (col == REGISTER_COL_PRODUCT) {
    QSqlDatabase dbc = QSqlDatabase::database("CN");
    QSqlQuery query(dbc);
    query.prepare(QString("SELECT gross, tax FROM products WHERE name='%1'").arg(s));
    query.exec();
    if (query.next()) {
      orderListModel->item(row, REGISTER_COL_SINGLE)->setText(query.value(0).toString());
      orderListModel->item(row, REGISTER_COL_TAX)->setText(query.value(1).toString());
    }
  }

  switch( col )
  {
    case REGISTER_COL_COUNT:
      sum = s.toDouble() * d2;
      //        sum = (long)(sum*100+0.5)/100.0;

      s = QString("%1").arg(sum);
      orderListModel->item(row, REGISTER_COL_TOTAL)->setText( s );
      break ;
    case REGISTER_COL_SINGLE:
      s.replace(",", ".");
      orderListModel->blockSignals(true);
      orderListModel->item(row, REGISTER_COL_SINGLE)->setText( s );
      sum = s.toDouble() * ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_COUNT, QModelIndex())).toDouble();
      //        sum = (long)(sum*100+0.5)/100.0;
      sum = QString::number(sum, 'f', 2).toDouble();

      s = QString("%1").arg(sum);
      orderListModel->item(row, REGISTER_COL_TOTAL)->setText( s );
      orderListModel->blockSignals(false);
      break ;
    case REGISTER_COL_TOTAL:
      s.replace(",", ".");
      s = QString("%1").arg(QString::number(s.toDouble(), 'f', 2));
      orderListModel->blockSignals(true);
      orderListModel->item(row, REGISTER_COL_TOTAL)->setText( s );
      sum = s.toDouble() / ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_COUNT, QModelIndex())).toDouble();

      s = QString("%1").arg(sum);
      orderListModel->item(row, REGISTER_COL_SINGLE)->setText( s );
      orderListModel->blockSignals(false);
      // ui->orderList->edit(ui->orderList->model()->index(row, 0));

      break ;

  }

  ui->orderList->resizeColumnsToContents();
  ui->orderList->horizontalHeader()->setSectionResizeMode(REGISTER_COL_PRODUCT, QHeaderView::Stretch);

  updateOrderSum();

}

//--------------------------------------------------------------------------------

void QRKRegister::plusSlot()
{

  int row;

  if (orderListModel->rowCount() > 0)
  {

    if (! ui->plusButton->isEnabled())
      return;

    row = orderListModel->rowCount() -1;

    QList<QVariant> list;

    list << ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_PRODUCT, QModelIndex())).toString()
         << ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_TAX, QModelIndex())).toString()
         << ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_SINGLE, QModelIndex())).toString();

    Database::addProduct(list);
  }

  row = orderListModel->rowCount();
  orderListModel->insertRow(row);
  ui->orderList->model()->blockSignals(true);

  orderListModel->setColumnCount(5);
  orderListModel->setItem(row, REGISTER_COL_COUNT, new QStandardItem(QString("1")));
  orderListModel->setItem(row, REGISTER_COL_PRODUCT, new QStandardItem(QString(tr("Artikelname"))));
  orderListModel->setItem(row, REGISTER_COL_TAX, new QStandardItem(QString("20")));
  orderListModel->setItem(row, REGISTER_COL_SINGLE, new QStandardItem(QString("0")));
  orderListModel->setItem(row, REGISTER_COL_TOTAL, new QStandardItem(QString("0")));

  orderListModel->item(row ,REGISTER_COL_COUNT)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
  orderListModel->item(row ,REGISTER_COL_TAX)->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);

  QModelIndex idx = orderListModel->index(row, REGISTER_COL_COUNT);

  ui->orderList->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::NoUpdate);
  ui->orderList->edit(idx);

  ui->orderList->selectRow(row);

  ui->orderList->model()->blockSignals(false);

  ui->orderList->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->orderList->setSelectionBehavior(QAbstractItemView::SelectRows);

  /* TODO: Workaround ... resize set only once will not work
   * but here col 1 will lost QHeaderView::Stretch
  */
  ui->orderList->resizeColumnsToContents();
  ui->orderList->horizontalHeader()->setSectionResizeMode(REGISTER_COL_PRODUCT, QHeaderView::Stretch);

  if (orderListModel->rowCount() > 1)
    ui->receiptToInvoice->setEnabled(false);

  ui->plusButton->setEnabled(true);

  updateOrderSum();

}

//--------------------------------------------------------------------------------

void QRKRegister::minusSlot()
{

  //get selections
  QItemSelection selection = ui->orderList->selectionModel()->selection();

  //find out selected rows
  QList<int> removeRows;
  foreach(QModelIndex index, selection.indexes()) {
    if(!removeRows.contains(index.row())) {
      removeRows.append(index.row());
    }
  }

  //loop through all selected rows
  for(int i=0;i<removeRows.count();++i)
  {
    //decrement all rows after the current - as the row-number will change if we remove the current
    for(int j=i;j<removeRows.count();++j) {
      if(removeRows.at(j) > removeRows.at(i)) {
        removeRows[j]--;
      }
    }
    //remove the selected row
    orderListModel->removeRow(removeRows.at(i));
  }

  if (orderListModel->rowCount() < 2)
    ui->receiptToInvoice->setEnabled(true);

  if (orderListModel->rowCount() < 1)
    ui->plusButton->setEnabled(true);

  updateOrderSum();
}

//--------------------------------------------------------------------------------

void QRKRegister::onButtonGroup_payNow_clicked(int payedBy)
{

  if (! checkEOAny())
    return;

  int row;

  if (orderListModel->rowCount() > 0)
  {
    row = orderListModel->rowCount() -1;

    QList<QVariant> list;

    list << ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_PRODUCT, QModelIndex())).toString()
         << ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_TAX, QModelIndex())).toString()
         << ui->orderList->model()->data(orderListModel->index(row, REGISTER_COL_SINGLE, QModelIndex())).toString();

    Database::addProduct(list);
  }

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  dbc.transaction();

  currentReceipt = createReceipts();
  if ( currentReceipt )
  {
    if ( createOrder() )
    {
      if ( finishReceipts(payedBy) )
      {
        emit cancelRegisterButton_clicked();
        emit registerButton_clicked();
      }
    }
  }

  if (!dbc.commit()) {
    dbc.rollback();
    // qDebug() << "QRK::onButtonGroup_payNow_clicked";
    QMessageBox::warning(this, "QRK::onButtonGroup_payNow_click", tr("Datenbank Fehler: Aktueller BON kann nicht erstellt werden."));
  }

}

//--------------------------------------------------------------------------------

void QRKRegister::totallyupSlot()
{
  ui->totallyup->setVisible(true);
  this->totallyup = true;
}

//--------------------------------------------------------------------------------

void QRKRegister::totallyupExitSlot()
{
  ui->totallyup->setVisible(false);
  this->totallyup = false;
  ui->dateEdit->setDate(QDate::currentDate());
}

//--------------------------------------------------------------------------------

void QRKRegister::receiptToInvoiceSlot()
{

  R2BDialog r2b(this);
  if ( r2b.exec() == QDialog::Accepted )
  {
    int rc = orderListModel->rowCount();
    if (rc == 0)
      plusSlot();

    if (rc == 1) {
      orderListModel->item(0, REGISTER_COL_COUNT)->setText( "1" );
      orderListModel->item(0, REGISTER_COL_PRODUCT)->setText( r2b.getInvoiceNum() );
      orderListModel->item(0, REGISTER_COL_TAX)->setText( "0" );
      orderListModel->item(0, REGISTER_COL_SINGLE)->setText( r2b.getInvoiceSum() );

      orderListModel->item(0, REGISTER_COL_COUNT)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      orderListModel->item(0, REGISTER_COL_PRODUCT)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      orderListModel->item(0, REGISTER_COL_TAX)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      orderListModel->item(0, REGISTER_COL_SINGLE)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      orderListModel->item(0, REGISTER_COL_TOTAL)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

      ui->plusButton->setEnabled(false);

    }
  }
}

//--------------------------------------------------------------------------------

void QRKRegister::clearModel()
{
  orderListModel->clear();
  orderListModel->setRowCount(0);
}

//--------------------------------------------------------------------------------

void QRKRegister::setItemModel(QSqlQueryModel *model)
{
  ui->orderList->setModel(model);
}

//--------------------------------------------------------------------------------

void QRKRegister::setCurrentReceiptNum(int id)
{
  currentReceipt = id;
}

//--------------------------------------------------------------------------------

bool QRKRegister::checkEOAny()
{

  QDate date = Database::getLastReceiptDate();
  QDate checkDate;
  if (totallyup)
    checkDate = ui->dateEdit->date();
  else
    checkDate = QDate::currentDate();

  // ist letzter bon am gleichen tag
  bool needDay(false);
  bool needMonth(false);
  int type = Reports::getReportId();
  lastEOD = Reports::getLastEOD();

  if (! (type == -1)) {
    if (! (date == QDate::currentDate()) ) {
      // ist letzter Bon ein Tages oder Monatsabschluss

      // Monats wechsel?
      if (! (date.month() == checkDate.month()) )
        needMonth = !(type ==  4);
      else
        needDay = !(type ==  3);

    }
  }

  if (lastEOD.isValid() && lastEOD == checkDate) {
    // qDebug() << "Tagesabschluss von HEUTE schon erstellt. Neue Eingabe erst morgen möglich";
    QMessageBox::information(this,
                             "Tagesabschluss",
                             tr("Tagesabschluss vom %1 wurde schon erstellt.\nNeue Eingabe erst wieder am %2 möglich")
                             .arg(checkDate.toString())
                             .arg(checkDate.addDays(1).toString()));

    return false;
  } else if (needDay && checkDate != date) {
    // qDebug() << "Tagesabschluss vom %1 muß erstellt werden.";
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Tagesabschluss"));

    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(tr("Tagesabschluss vom %1 muß erstellt werden.").arg(date.toString()));
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, tr("Erstellen"));
    msgBox.setButtonText(QMessageBox::No, tr("Abbrechen"));
    msgBox.setDefaultButton(QMessageBox::No);

    if(msgBox.exec() == QMessageBox::Yes){
      DEP *dep = new DEP();
      Reports *rep = new Reports(dep, progressBar);
      if (! rep->endOfDay() ) {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        return true;
      }

    } else {
      return false;
    }
  } else if (needMonth && checkDate != date) {
    DEP *dep = new DEP();
    Reports *rep = new Reports(dep, progressBar);
    if (! rep->endOfMonth()) {
      QApplication::setOverrideCursor(Qt::ArrowCursor);
      return false;
    }
  }

  return true;

}
