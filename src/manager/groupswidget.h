#ifndef _GROUPS_WIDGET_H_
#define _GROUPS_WIDGET_H_

#include <QDialog>
class QSqlTableModel;
class QSortFilterProxyModel;

#include "ui_groupwidget.h"

namespace Ui {
  class GroupsWidget;
}

class GroupsWidget : public QWidget
{
  Q_OBJECT

  public:
    GroupsWidget(QWidget *parent);

  private slots:
    void filterGroup(const QString &filter);
    void plusSlot();
    void minusSlot();
    void editSlot();

  private:
    Ui::GroupsWidget *ui;
    QSqlTableModel *model;
    QSortFilterProxyModel *proxyModel;

};

#endif
