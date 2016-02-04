#include "productswidget.h"
#include "productedit.h"

#include <QSqlRelationalTableModel>
#include <QSqlRelation>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QHeaderView>

//--------------------------------------------------------------------------------

ProductsWidget::ProductsWidget(QWidget *parent)
  : QDialog(parent), ui(new Ui::ProductsWidget), newProductDialog(0)
{
  ui->setupUi(this);

  connect(ui->plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui->minus, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui->edit, SIGNAL(clicked()), this, SLOT(editSlot()));
  connect(ui->productFilter, SIGNAL(textChanged(const QString &)), this, SLOT(filterProduct(const QString &)));

  QSqlDatabase dbc = QSqlDatabase::database("CN");

  model = new QSqlRelationalTableModel(this, dbc);
  model->setTable("products");
  model->setRelation(model->fieldIndex("group"), QSqlRelation("groups", "id", "name"));
  model->setFilter("\"group\" > 1");
  model->setEditStrategy(QSqlTableModel::OnFieldChange);
  model->select();
  model->fetchMore();  // else the list is not filled with all possible rows

  model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Produkt"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("price"), Qt::Horizontal, tr("Preis"), Qt::DisplayRole);
  model->setHeaderData(3, Qt::Horizontal, tr("Gruppe"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("visible"), Qt::Horizontal, tr("sichtbar"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("tax"), Qt::Horizontal, tr("MWSt"), Qt::DisplayRole);

  proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(model);
  proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  proxyModel->setFilterKeyColumn(model->fieldIndex("name"));

  ui->tableView->setModel(proxyModel);
  ui->tableView->setSortingEnabled(true);
  ui->tableView->setColumnHidden(model->fieldIndex("id"), true);
//  ui->tableView->horizontalHeader()->setResizeMode(model->fieldIndex("name"), QHeaderView::ResizeToContents);
  ui->tableView->resizeColumnToContents(model->fieldIndex("visible"));
  ui->tableView->resizeColumnToContents(model->fieldIndex("tax"));
  ui->tableView->resizeColumnToContents(3);  // related groups-name
  ui->tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

void ProductsWidget::filterProduct(const QString &filter)
{
  // show only matching items

  proxyModel->setFilterWildcard("*" + filter + "*");

  model->fetchMore();  // else the list is not filled with all possible rows
                       // e.g. when using mouse wheel it would fetch more items
                       // but on the WeTab we have no mouse
}

//--------------------------------------------------------------------------------

void ProductsWidget::plusSlot()
{
  // reuse the "new" dialog so that the next call has already the previous
  // settings defined; makes input of a lot of products of a given group simpler
  if ( !newProductDialog )
    newProductDialog = new ProductEdit(this);

  newProductDialog->exec();

  model->select();
  model->fetchMore();  // else the list is not filled with all possible rows
}

//--------------------------------------------------------------------------------

void ProductsWidget::minusSlot()
{
  int row = proxyModel->mapToSource(ui->tableView->currentIndex()).row();
  if ( row == -1 )
    return;

  if ( QMessageBox::question(this, tr("Produkt löschen"),
         tr("Möchten sie das Produkt '%1' wirklich löschen ?")
            .arg(model->data(model->index(row, 1)).toString()),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  if ( !model->removeRow(row) )
  {
    QMessageBox::information(this, tr("Löschen nicht möglich"),
        tr("Produkt '%1' kann nicht gelöscht werden, da es schon in Verwendung ist")
           .arg(model->data(model->index(row, 1)).toString()));
  }
}

//--------------------------------------------------------------------------------

void ProductsWidget::editSlot()
{
  QModelIndex current(proxyModel->mapToSource(ui->tableView->currentIndex()));
  int row = current.row();
  if ( row == -1 )
    return;

  ProductEdit dialog(this, model->data(model->index(row, model->fieldIndex("id"))).toInt());
  if ( dialog.exec() == QDialog::Accepted )
  {
    model->select();
    model->fetchMore();  // else the list is not filled with all possible rows
    ui->tableView->resizeRowsToContents();
    ui->tableView->setCurrentIndex(proxyModel->mapFromSource(current));
  }
}

//--------------------------------------------------------------------------------