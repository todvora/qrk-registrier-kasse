#include "salesinfo.h"
#include "qrkdelegate.h"
#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QSqlError>
#include "ui_salesinfo.h"

SalesInfo::SalesInfo(QString from, QString to, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SalesInfo)
{
    ui->setupUi(this);

    QSqlDatabase dbc = QSqlDatabase::database("CN");

    m_salesContentModel = new QSqlQueryModel;
    ui->label->setText(tr("Umsätze %1 bis %2").arg(from).arg(to));

    QSqlQuery query(dbc);
    query.prepare("SELECT actionTypes.actionText, orders.tax, SUM(ROUND(orders.count * orders.gross,2)) from orders "
                  " LEFT JOIN receipts on orders.receiptId=receipts.receiptNum"
                  " LEFT JOIN actionTypes on receipts.payedBy=actionTypes.actionId"
                  " WHERE receipts.timestamp between :fromDate AND :toDate AND receipts.payedBy < 3"
                  " GROUP BY orders.tax, receipts.payedBy ORDER BY receipts.payedBy, orders.tax");
    query.bindValue(":fromDate", from);
    query.bindValue(":toDate", to);

    query.exec();
    m_salesContentModel->setQuery(query);

    ui->salesView->setModel(m_salesContentModel);

    m_salesContentModel->setHeaderData(0, Qt::Horizontal, tr("Bezahlart"));
    m_salesContentModel->setHeaderData(1, Qt::Horizontal, tr("Steuersatz"));
    m_salesContentModel->setHeaderData(2, Qt::Horizontal, tr("Summen"));


    ui->salesView->setItemDelegateForColumn(1, new QrkDelegate (QrkDelegate::COMBO_TAX, this));
    ui->salesView->setItemDelegateForColumn(2, new QrkDelegate (QrkDelegate::NUMBERFORMAT_DOUBLE, this));

    ui->salesView->resizeColumnsToContents();
    ui->salesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->closePushButton, SIGNAL(clicked(bool)), this, SLOT(deleteLater()));
}

SalesInfo::~SalesInfo()
{
    delete ui;
}
