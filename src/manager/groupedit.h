#ifndef _GroupEdit_H_
#define _GroupEdit_H_

#include <QDialog>
#include <QSqlQueryModel>

#include <ui_groupedit.h>

class GroupEdit : public QDialog
{
  Q_OBJECT

  public:
    GroupEdit(QWidget *parent, int id = -1);  // -1 ... neu

  public slots:
    virtual void accept();

  private:
    Ui::GroupEdit ui;
    int id;
    QSqlQueryModel categoriesModel;
};

#endif
