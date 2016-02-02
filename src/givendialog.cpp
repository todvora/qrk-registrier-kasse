#include "givendialog.h"
#include "ui_givendialog.h"

GivenDialog::GivenDialog(double &sum, QWidget *parent) :
    QDialog(parent), ui(new Ui::GivenDialog)
{
    ui->setupUi(this);
    this->sum = sum;

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

    double retourMoney = given.toDouble() - sum;

    ui->lcdNumber->display(retourMoney);

}
