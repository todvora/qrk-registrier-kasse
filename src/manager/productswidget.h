#ifndef _PRODUCTSWIDGET_H_
#define _PRODUCTSWIDGET_H_

#include <QDialog>
class QSqlRelationalTableModel;
class QSortFilterProxyModel;

#include <ui_productswidget.h>
class ProductEdit;

//--------------------------------------------------------------------------------

namespace Ui {
  class ProductsWidget;
}

class ProductsWidget : public QWidget
{
  Q_OBJECT

  public:
    ProductsWidget(QWidget *parent);

  private slots:
    void filterProduct(const QString &filter);
    void plusSlot();
    void minusSlot();
    void editSlot();

  private:
    Ui::ProductsWidget *ui;
    QSqlRelationalTableModel *model;
    ProductEdit *newProductDialog;
    QSortFilterProxyModel *proxyModel;
};

#endif
