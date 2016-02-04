#include "productedit.h"
#include "database.h"


#include <QDoubleValidator>
#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

//--------------------------------------------------------------------------------
ProductEdit::ProductEdit(QWidget *parent, int theId)
    : QDialog(parent), ui(new Ui::ProductEdit), id(theId)
{
    ui->setupUi(this);
    ui->net->setValidator(new QDoubleValidator(0.0, 9999999.99, 2, this));
    ui->gross->setValidator(new QDoubleValidator(0.0, 9999999.99, 2, this));

    QSqlDatabase dbc = QSqlDatabase::database("CN");

    groupsModel = new QSqlRelationalTableModel(this, dbc);
    groupsModel->setQuery("SELECT id, name FROM groups WHERE id > 1", dbc);
    ui->groupComboBox->setModel(groupsModel);
    ui->groupComboBox->setModelColumn(1);  // show name

    taxModel = new QSqlRelationalTableModel(this, dbc);
    QString q = QString("SELECT id, tax FROM taxTypes WHERE taxlocation='%1'").arg(Database::getTaxLocation());
    taxModel->setQuery(q, dbc);
    ui->taxComboBox->setModel(taxModel);
    ui->taxComboBox->setModelColumn(1);  // show tax

    if ( id != -1 )
    {
        QSqlQuery query(QString("SELECT `name`,`group`,`visible`,`net`,`gross`,`tax` FROM products WHERE id=%1").arg(id), dbc);
        query.next();

        ui->name->setText(query.value(0).toString());
        ui->visibleCheckBox->setChecked(query.value(2).toBool());
        ui->net->setText(QString::number(query.value(3).toDouble(), 'f', 2));
        ui->gross->setText(QString::number(query.value(4).toDouble(), 'f', 2));

        int i;
        for (i = 0; i < groupsModel->rowCount(); i++)
            if ( query.value(1).toInt() == groupsModel->data(groupsModel->index(i, 0), Qt::DisplayRole).toInt() )
                break;

        ui->groupComboBox->setCurrentIndex(i);

        for (i = 0; i < taxModel->rowCount(); i++)
            if ( query.value(5).toDouble() == taxModel->data(taxModel->index(i, 1), Qt::DisplayRole).toDouble() )
                break;

        ui->taxComboBox->setCurrentIndex(i);

    }
    connect (ui->taxComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(taxComboChanged(int)));
    connect (ui->net, SIGNAL(editingFinished()), this, SLOT(netChanged()));
    connect (ui->gross, SIGNAL(editingFinished()), this, SLOT(grossChanged()));

}

//--------------------------------------------------------------------------------

void ProductEdit::taxComboChanged(int)
{
    double tax = taxModel->data(taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double net = ui->net->text().replace(",",".").toDouble();
    double gross = net * (1.0 + tax / 100.0);

    ui->gross->setText(QString::number(gross, 'f', 2));

}

void ProductEdit::netChanged()
{
    double tax = taxModel->data(taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double net = ui->net->text().replace(",",".").toDouble();
    double gross = net * (1.0 + tax / 100.0);

    ui->gross->setText(QString::number(gross, 'f', 2));

}

void ProductEdit::grossChanged()
{
    double tax = taxModel->data(taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double gross = ui->gross->text().replace(",",".").toDouble();
    double net = gross / (1.0 + tax / 100.0);

    ui->net->setText(QString::number(net, 'f', 2));

}

void ProductEdit::accept()
{
    QSqlDatabase dbc = QSqlDatabase::database("CN");

    QSqlQuery query(dbc);

    double tax = taxModel->data(taxModel->index(ui->taxComboBox->currentIndex(), 1)).toDouble();
    double net = ui->gross->text().toDouble() / (1.0 + tax / 100.0);

    if ( id == -1 )  // new entry
    {
        bool ok = query.exec(QString("INSERT INTO products (name, `group`, visible, net, gross, tax) VALUES('%1', %2, %3, %4, %5, %6)")
                             .arg(ui->name->text())
                             .arg(groupsModel->data(groupsModel->index(ui->groupComboBox->currentIndex(), 0)).toInt())
                             .arg(ui->visibleCheckBox->isChecked())
                             .arg(net)
                             .arg(ui->gross->text().toDouble())
                             .arg(tax));

        if (!ok) {
            qDebug() << "ProductEdit::accept() error: " << query.lastError().text();
            qDebug() << "ProductEdit::accept() query: " << query.lastQuery();

        }
    }
    else
    {
        bool ok = query.exec(QString("UPDATE products SET name='%1', `group`=%2,visible=%3,net=%4,gross=%5,tax=%6 WHERE id=%7")
                             .arg(ui->name->text())
                             .arg(groupsModel->data(groupsModel->index(ui->groupComboBox->currentIndex(), 0)).toInt())
                             .arg(ui->visibleCheckBox->isChecked())
                             .arg(net)
                             .arg(ui->gross->text().toDouble())
                             .arg(tax)
                             .arg(id));
        if (!ok) {
            qDebug() << "ProductEdit::accept() error: " << query.lastError().text();
            qDebug() << "ProductEdit::accept() query: " << query.lastQuery();

        }

    }

    QDialog::accept();
}

//--------------------------------------------------------------------------------
