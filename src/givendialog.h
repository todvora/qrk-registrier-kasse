#ifndef GIVENDIALOG_H
#define GIVENDIALOG_H

#include <QDialog>

namespace Ui {
class GivenDialog;
}

class GivenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GivenDialog(double &sum, QWidget *parent = 0);
    ~GivenDialog();

private slots:
    void accept(bool);
    void textChanged(QString);

private:
    Ui::GivenDialog *ui;
    double sum;
};

#endif // GIVENDIALOG_H
