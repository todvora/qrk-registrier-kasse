#ifndef SALESINFO_H
#define SALESINFO_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class SalesInfo;
}

class SalesInfo : public QDialog
{
    Q_OBJECT

public:
    explicit SalesInfo(QString from, QString to, QWidget *parent = 0);
    ~SalesInfo();

private:
    Ui::SalesInfo *ui;
    QSqlQueryModel *m_salesContentModel;
};

#endif // SALESINFO_H
