#ifndef _PRODUCTEDIT_H_
#define _PRODUCTEDIT_H_

#include <QDialog>
#include <QSqlQueryModel>

#include <ui_productedit.h>

namespace Ui {
  class ProductEdit;
}

class ProductEdit : public QDialog
{
  Q_OBJECT

  public:
    ProductEdit(QWidget *parent, int theId = -1);

  public slots:
    virtual void accept();

  private:
    Ui::ProductEdit *ui;
    int id;
    QSqlQueryModel *groupsModel;
};

#endif
