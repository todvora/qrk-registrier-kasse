#include "givendialog.h"
#include "database.h"
#include "ui_givendialog.h"

GivenDialog::GivenDialog(double &sum, QWidget *parent) :
    QDialog(parent), ui(new Ui::GivenDialog)
{
    ui->setupUi(this);
    ui->lcdNumber->setDigitCount(10);
    ui->givenEdit->setValidator(new QDoubleValidator(0.0, 9999999.99, 2, this));
    this->sum = sum;
    ui->toPayLabel->setText(tr("Zu bezahlen: %1 %2").arg(QString::number(sum,'f',2)).arg(Database::getCurrency()));

    connect (ui->givenEdit, SIGNAL(textChanged(QString)), SLOT(textChanged(QString)));
    connect (ui->pushButton, SIGNAL(clicked(bool)), SLOT(accept(bool)));

}

GivenDialog::~GivenDialog()
{
    delete ui;
}

void GivenDialog::accept(bool)
{

  QDialog::accept();
}

void GivenDialog::textChanged(QString given)
{

    given.replace(",",".");
    double retourMoney = given.toDouble() - sum;

    ui->lcdNumber->display(QString::number(retourMoney,'f',2));

}
