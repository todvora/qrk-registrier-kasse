/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015 Christian Kvasny <chris@ckvsoft.at>
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
  : ui(new Ui::MainWindow), menu(0), currentReceipt(0), noPrinter(false), totallyup(false)

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

  orderListModel = new ReceiptItemModel(this);

  setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint);

  if ( QApplication::desktop()->width() < 1200 )
  {
    ui->plusButton->setMinimumWidth(0);
    ui->minusButton->setMinimumWidth(0);
  }

  // page 1
  connect(ui->registerButton, SIGNAL(clicked()), this, SLOT(onRegisterButton_clicked()));
  connect(ui->documentButton, SIGNAL(clicked()), this, SLOT(onDocumentButton_clicked()));

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
    connect(b, SIGNAL(clicked()), this, SLOT(exitSlot()));
    vbox->addWidget(b);

    b = new QPushButton(QIcon(":/icons/view-fullscreen.png"), tr("Vollbild"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    connect(b, SIGNAL(clicked()), this, SLOT(fullScreenSlot()));
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
    connect(b, SIGNAL(clicked()), this, SLOT(endOfDaySlot()));
    vbox->addWidget(b);

    b = new QPushButton(QIcon(":/icons/month.png"), tr("Monatsabschluss"));
    b->setFixedHeight(MIN_HEIGHT);
    b->setIconSize(QSize(32, 32));
    connect(b, SIGNAL(clicked()), this, SLOT(endOfMonthSlot()));
    vbox->addWidget(b);

    connect(ui->taskButton, SIGNAL(clicked()), this, SLOT(taskSlot()));

  }

  // page 2

  ui->tityup->setVisible(false);
  ui->dateEdit->setDate(QDate::currentDate());
  ui->dateEdit->setDateRange(QDate::fromString(QString("%1-01-01").arg(currentRegisterYear),"yyyy-MM-dd"), QDate::currentDate());

  connect(ui->totallyupButton, SIGNAL(clicked()), this, SLOT(totallyupSlot()));
  connect(ui->totallyupExitButton, SIGNAL(clicked()), this, SLOT(totallyupExitSlot()));
  connect(ui->receiptToInvoice, SIGNAL(clicked()), this, SLOT(receiptToInvoiceSlot()));

  connect(ui->plusButton, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui->minusButton, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui->cancelRegisterButton, SIGNAL(clicked()), this, SLOT(onCancelRegisterButton_clicked()));

  QShortcut *plusButtonShortcut = new QShortcut(QKeySequence("Insert"), this);
  QShortcut *minusButtonShortcut = new QShortcut(QKeySequence("Delete"), this);

  connect(plusButtonShortcut, SIGNAL(activated()), this, SLOT(plusSlot()));
  connect(minusButtonShortcut, SIGNAL(activated()), this, SLOT(minusSlot()));

  ui->buttonGroup->setId(ui->cashReceipt, PAYED_BY_CASH);
  ui->buttonGroup->setId(ui->creditcardReceipt, PAYED_BY_CREDITCARD);
  ui->buttonGroup->setId(ui->debitcardReceipt, PAYED_BY_DEBITCARD);

  connect(ui->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onButtonGroup_payNow_clicked(int)));

  //    connect(&productsMapper, SIGNAL(mapped(int)), this, SLOT(productSelected(int)));

  // page 3

  ui->textBrowser->setHidden(true);

  connect(ui->cancelDocumentButton, SIGNAL(clicked()), this, SLOT(onCancelDocumentButton_clicked()));
  connect(ui->printcopyButton, SIGNAL(clicked()), this, SLOT(onPrintcopyButton_clicked()));
  connect(ui->cancellationButton, SIGNAL(clicked()), this, SLOT(onCancellationButton_clicked()));


  connect(ui->actionDEP_Exportieren, SIGNAL(triggered()), this, SLOT(actionDEP_Export()));
  connect(ui->actionEOM, SIGNAL(triggered()), this, SLOT(endOfMonthSlot()));
  connect(ui->actionEOD, SIGNAL(triggered()), this, SLOT(endOfDaySlot()));

  // ----------------------------------------------------------------------------

  ui->stackedWidget->setCurrentIndex(0);
  timer->start(1000);
  cashRegisterId = 1;

}

//--------------------------------------------------------------------------------

QRK::~QRK()
{
  delete ui;
}

//--------------------------------------------------------------------------------

void QRK::timerDone()
{
  QDateTime t = QDateTime::currentDateTime();
  dateLcd->display (t.toString("dd-MM-yyyy  hh:mm:ss"));
}

//--------------------------------------------------------------------------------

QJsonObject QRK::compileData(int id)
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
  Root["kasse"] = getCashRegisterId();
  Root["actionText"] = tr("Beleg");
  Root["typeText"] = Database::getActionType(payedBy);
  Root["shopName"] = shopName;
  Root["headerText"] = ui->headerText->text();
  Root["totallyup"] = (totallyup)? "Nachbonierung":"";
  Root["comment"] = (id > 0)? tr("Storno für Beleg Nr: %1-%2").arg(currentRegisterYear).arg(id):tr("KASSABON");
  Root["receiptNum"] = receiptNum;
  Root["receiptTime"] = receiptTime.toString(Qt::ISODate);
  Root["currentRegisterYear"] = currentRegisterYear;

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
    /*
    switch (tax)
    {
      case 20:
        Root["Satz-Normal"] = Root["Satz-Normal"].toDouble() + gross;
        break;
      case 10:
        Root["Satz-Ermaessigt-1"] = Root["Satz-Ermaessigt-1"].toDouble() + gross;
        break;
      case 12:
        Root["Satz-Ermaessigt-2"] = Root["Satz-Ermaessigt-2"].toDouble() + gross;
        break;
      case 13:
        Root["Satz-Besonders"] = Root["Satz-Besonders"].toDouble() + gross;
        break;
      case 0:
        Root["Satz-Null"] = Root["Satz-Null"].toDouble() + gross;
        break;
    }
    */
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
  Root["positionsText"] = tr("Anzahl verkaufter Produkte oder Leistungen: %1").arg(positions);
  Root["Taxes"] = Taxes;
  Root["taxesCount"] = taxes.count();

  return Root;

}

//--------------------------------------------------------------------------------

void QRK::init()
{
  setShopName();
  if (shopName.length() == 0){
    QMessageBox::warning(this, "Shopname", tr("Shopname darf nicht leer sein.\nBitte machen Sie Ihre Angaben."));
    emit this->settingsSlot();
  }

}

//--------------------------------------------------------------------------------

void QRK::setShopName()
{
  shopName = Database::getShopName();
}

// page 1 Main
//--------------------------------------------------------------------------------

void QRK::menuSlot()
{
  QPoint p(ui->menuButton->x() + ui->menuButton->width() - menu->sizeHint().width(),
           ui->menuButton->y() - menu->sizeHint().height());

  menu->move(mapToGlobal(p));
  menu->show();
}

//--------------------------------------------------------------------------------

void QRK::exitSlot()
{
  if ( QMessageBox::question(this, tr("Beenden"), tr("Möchten sie wirklich beenden ?"),
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
  {
    QApplication::exit();
  }
}

//--------------------------------------------------------------------------------

void QRK::settingsSlot()
{
  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  SettingsDialog dialog(this, settings);
  if ( dialog.exec() == QDialog::Accepted )
  {
    init();
  }
}

//--------------------------------------------------------------------------------

void QRK::taskSlot()
{
  QPoint p(ui->taskButton->x() + ui->taskButton->width() - task->sizeHint().width(),
           ui->taskButton->y() - task->sizeHint().height());

  task->move(mapToGlobal(p));
  task->show();
}

//--------------------------------------------------------------------------------

bool QRK::endOfDaySlot()
{
  QMessageBox msgBox;
  msgBox.setWindowTitle(tr("Tagesabschluss"));

  QDate date = Database::getLastReceiptDate();
  bool create = Reports::canCreateEOD(date);
  if (create) {
    if (date == QDate::currentDate()) {
      msgBox.setText(tr("Nach dem Erstellen des Tagesabschlusses ist eine Bonierung für den heutigen Tag nicht mehr möglich."));
    } else {
      msgBox.setText(tr("Nach dem Erstellen des Tagesabschlusses für %1 ist eine Nachbonierung erst ab %2 möglich.").arg(date.toString()).arg(QDateTime::currentDateTime().toString()));
    }
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, tr("Erstellen"));
    msgBox.setButtonText(QMessageBox::No, tr("Abbrechen"));
    msgBox.setDefaultButton(QMessageBox::No);
    if(msgBox.exec() == QMessageBox::Yes){
      this->setCursor(Qt::WaitCursor);
      DEP *dep = new DEP(this);
      Reports *rep = new Reports(dep, this->progressBar, this);
      currentReceipt = this->createReceipts();
      finishReceipts(3, currentReceipt, true);
      rep->createEOD(currentReceipt, date);
      lastEOD = Reports::getLastEOD();
      delete dep;
      delete rep;
      this->progressBar->reset();
      this->setCursor(Qt::ArrowCursor);
      return true;
    } else {
      return false;
    }
  } else {
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(tr("Tagesabschluss wurde bereits erstellt."));
    msgBox.setInformativeText(tr("Erstellungsdatum %1").arg(date.toString()));
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.setButtonText(QMessageBox::Yes, tr("OK"));
    msgBox.exec();
    return false;
  }

  return false;
}

//--------------------------------------------------------------------------------

bool QRK::endOfMonthSlot()
{

  // check if current month is newer than last Receipt Month

  QDate rDate = Database::getLastReceiptDate();
  int receiptMonth = (rDate.year() * 100) + rDate.month();
  int currMonth = (QDate::currentDate().year() * 100) + QDate::currentDate().month();

  bool ok = (rDate.isValid() && receiptMonth < currMonth);

  if (ok) {
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("Monatsabschluss"));
    QDateTime checkdate = QDateTime::currentDateTime();
    checkdate.setDate(QDate::fromString(QString("%1-%2-31").arg(QDate::currentDate().year()).arg(QDate::currentDate().month()),"yyyy-MM-dd").addMonths(-1));
    checkdate.setTime(QTime::fromString("23:59:59"));

    QDateTime dateTime = Database::getLastReceiptDateTime();

    bool create = Reports::canCreateEOD(dateTime.date());
    if (dateTime <= checkdate && create) {
      msgBox.setText(tr("Der Tagesabschlusses für %1 muß zuerst erstellt werden.").arg(dateTime.date().toString()));
      msgBox.setStandardButtons(QMessageBox::Yes);
      msgBox.addButton(QMessageBox::No);
      msgBox.setButtonText(QMessageBox::Yes, tr("Erstellen"));
      msgBox.setButtonText(QMessageBox::No, tr("Abbrechen"));
      msgBox.setDefaultButton(QMessageBox::No);
      if(msgBox.exec() == QMessageBox::Yes){
        if (! endOfDaySlot())
          return false;
      } else {
        return false;
      }
    }
    if (create) {
      this->setCursor(Qt::WaitCursor);
      DEP *dep = new DEP(this);
      Reports *rep = new Reports(dep, this->progressBar, this);
      currentReceipt = this->createReceipts();
      finishReceipts(4, currentReceipt, true);
      rep->createEOM(currentReceipt, checkdate.date());
      lastEOD = Reports::getLastEOD();
      delete dep;
      delete rep;
      this->progressBar->reset();
      this->setCursor(Qt::ArrowCursor);
    } else {
      QDate next = QDate::currentDate();
      next.setDate(next.year(), next.addMonths(1).month(), 1);
      QMessageBox::information(this, "Montatsabschluss", tr("Der Monatsabschluss kann erst ab %1 gemacht werden.").arg(next.toString()));
    }
  } else {
    QDate next = QDate::currentDate();
    next.setDate(next.year(), next.addMonths(1).month(), 1);
    QMessageBox::information(this, "Montatsabschluss", tr("Der Monatsabschluss kann erst ab %1 gemacht werden.").arg(next.toString()));
  }

  return true;
}

//--------------------------------------------------------------------------------

void QRK::fullScreenSlot()
{
  menu->hide();

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

//--------------------------------------------------------------------------------

void QRK::onRegisterButton_clicked()
{
  QDate last = Database::getLastReceiptDate();
  if (last.toString(Qt::ISODate) == lastEOD.toString(Qt::ISODate))
    last = last.addDays(1);

  ui->dateEdit->setDateRange(last, QDate::currentDate());

  ui->orderList->setModel(orderListModel);
  connect(ui->orderList->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(itemChangedSlot(const QModelIndex&, const QModelIndex&)));
  ui->stackedWidget->setCurrentIndex(1);

  newOrder();
}

//--------------------------------------------------------------------------------

void QRK::onDocumentButton_clicked()
{
  ui->stackedWidget->setCurrentIndex(2);
  documentList();
}

//--------------------------------------------------------------------------------

void QRK::updateOrderSum()
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

void QRK::itemChangedSlot( const QModelIndex& i, const QModelIndex&)
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

//  page 2    ListOrder
//--------------------------------------------------------------------------------

void QRK::plusSlot()
{

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

  updateOrderSum();

}

//--------------------------------------------------------------------------------

void QRK::minusSlot()
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

  updateOrderSum();
}

//--------------------------------------------------------------------------------

void QRK::onButtonGroup_payNow_clicked(int payedBy)
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

    return;
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
      if (!endOfDaySlot())
        return;
    } else {
      return;
    }
  } else if (needMonth && checkDate != date) {
    if (! endOfMonthSlot())
      return;
  }

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
        emit ui->cancelRegisterButton->clicked();
        emit ui->registerButton->clicked();
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

bool QRK::finishReceipts(int payedBy, int id, bool isReport)
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
                   .arg(currentReceipt));

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
             .arg(currentReceipt));

  if (isReport)
    return true;

  if (id)
    Database::setStornoId(currentReceipt, id);

  this->setCursor(Qt::WaitCursor);
  QJsonObject data = compileData(id);
  DocumentPrinter *p = new DocumentPrinter(this, progressBar, noPrinter);
  p->printReceipt(data);

  delete p;

  DEP *dep = new DEP(this);
  dep->depInsertReceipt(data);
  delete dep;

  this->setCursor(Qt::ArrowCursor);

  return true;

}

//--------------------------------------------------------------------------------

bool QRK::createOrder(bool storno)
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

int QRK::createReceipts()
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

void QRK::newOrder()
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

void QRK::onCancelRegisterButton_clicked()
{
  currentReceipt = 0;
  orderListModel->clear();
  orderListModel->setRowCount(0);
  ui->stackedWidget->setCurrentIndex(0);
}

//  page 3    DocumentList
//--------------------------------------------------------------------------------

void QRK::documentList()
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

//--------------------------------------------------------------------------------

void QRK::onDocumentSelectionChanged(const QItemSelection &, const QItemSelection & )
{
  QModelIndexList indexList = ui->documentList->selectionModel()->selectedIndexes();
  int row;
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

void QRK::onCancelDocumentButton_clicked()
{
  ui->stackedWidget->setCurrentIndex(0);

}

void QRK::onCancellationButton_clicked()
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
    emit ui->documentButton->clicked();
    return;
  }

  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQueryModel *model = new QSqlQueryModel;

  model->setQuery(QString("SELECT orders.count, products.name, orders.tax, orders.gross FROM orders INNER JOIN products ON products.id=orders.product WHERE orders.receiptId=%1").arg(id), dbc);

  ui->orderList->setModel(model);

  int payedBy = Database::getPayedBy(id);

  currentReceipt = createReceipts();
  if ( currentReceipt )
  {
    if ( createOrder(true) )
    {
      if ( finishReceipts(payedBy, id) )
      {
        emit ui->documentButton->clicked();
        return;
      }
    }
  }
  // qDebug() << "QRK::onCancellationButton_clicked()";
}

void QRK::onPrintcopyButton_clicked()
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

        this->setCursor(Qt::WaitCursor);
        DocumentPrinter *p = new DocumentPrinter(this, progressBar);
        p->printDocument(&doc);
        delete p;
        this->setCursor(Qt::ArrowCursor);

    } else {
        currentReceipt = id;
        QJsonObject data = compileData();
        data["isCopy"] = true;
        int storno = Database::getStorno(id);
        if (storno == 2) {
            id = Database::getStornoId(id);
            data["comment"] = (id > 0)? tr("Storno für Beleg Nr: %1-%2").arg(currentRegisterYear).arg(id):tr("KASSABON");
        }

        this->setCursor(Qt::WaitCursor);
        DocumentPrinter *p = new DocumentPrinter(this, progressBar);
        p->printReceipt(data);
        delete p;
        this->setCursor(Qt::ArrowCursor);
        emit onDocumentButton_clicked();
    }
}

void QRK::actionDEP_Export()
{
  DepExportDialog *dlg = new DepExportDialog(this, progressBar);

  if ( dlg->exec() == QDialog::Accepted )
  {
    init();
  }

  delete dlg;
}

void QRK::totallyupSlot()
{
  ui->tityup->setVisible(true);
  this->totallyup = true;
}

void QRK::totallyupExitSlot()
{
  ui->tityup->setVisible(false);
  this->totallyup = false;
  ui->dateEdit->setDate(QDate::currentDate());
}

void QRK::receiptToInvoiceSlot()
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
    }
  }
}
