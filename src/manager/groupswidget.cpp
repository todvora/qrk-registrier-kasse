#include "groupedit.h"
#include "groupswidget.h"

#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRelation>
#include <QSqlError>
#include <QMessageBox>
#include <QHeaderView>

//--------------------------------------------------------------------------------

GroupsWidget::GroupsWidget(QWidget *parent)
  : QDialog(parent), ui(new Ui::GroupsWidget)

{
  ui->setupUi(this);

  QSqlDatabase dbc = QSqlDatabase::database("CN");

  connect(ui->plus, SIGNAL(clicked()), this, SLOT(plusSlot()));
  connect(ui->minus, SIGNAL(clicked()), this, SLOT(minusSlot()));
  connect(ui->edit, SIGNAL(clicked()), this, SLOT(editSlot()));

  model = new QSqlRelationalTableModel(this, dbc);
  model->setTable("groups");
//  model->setRelation(model->fieldIndex("name"), QSqlRelation("groups", "id", "name"));
  model->setEditStrategy(QSqlTableModel::OnFieldChange);
  model->select();

  model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, tr("Gruppe"), Qt::DisplayRole);
  model->setHeaderData(model->fieldIndex("visible"), Qt::Horizontal, tr("sichtbar"), Qt::DisplayRole);
//  model->setHeaderData(3, Qt::Horizontal, tr("Kategorie"), Qt::DisplayRole);

//  ui.tableView->setItemDelegate(new TouchDelegate(ui.tableView));
  ui->tableView->setModel(model);
  ui->tableView->setSortingEnabled(true);
  ui->tableView->setColumnHidden(model->fieldIndex("id"), true);
  ui->tableView->setColumnWidth(model->fieldIndex("name"), 250);
//  ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

  ui->tableView->resizeRowsToContents();
}

//--------------------------------------------------------------------------------

void GroupsWidget::plusSlot()
{
  GroupEdit dialog(this);
  dialog.exec();

  model->select();
}

//--------------------------------------------------------------------------------

void GroupsWidget::minusSlot()
{
  /*  ... this is for multi selection
  QModelIndexList selected = ui.tableView->selectionModel()->selectedIndexes();
  QMap<int, bool> rows;  // we need a row only once
  for (int i = 0; i < selected.count(); i++)
    rows[selected[i].row()] = true;

  QList<int> selectedRows = rows.keys();

  for (int i = 0; i < selectedRows.count(); i++)
  {
    if ( !model->removeRow(selectedRows[i]) )
    {
      QMessageBox::information(this, tr("Löschen nicht möglich"),
          tr("Gruppe %1 kann nicht gelöscht werden, da sie noch in Verwendung ist")
             .arg(model->data(model->index(selectedRows[i], 1)).toString()));
    }
  }
  */
  int row = ui->tableView->currentIndex().row();
  if ( row == -1 )
    return;

  if ( QMessageBox::question(this, tr("Gruppe löschen"),
         tr("Möchten sie die Gruppe '%1' wirklich löschen ?")
            .arg(model->data(model->index(row, 1)).toString()),
         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
    return;

  if ( !model->removeRow(row) )
  {
    QMessageBox::information(this, tr("Löschen nicht möglich"),
        tr("Gruppe '%1' kann nicht gelöscht werden, da sie noch in Verwendung ist")
           .arg(model->data(model->index(row, 1)).toString()));
  }
}

//--------------------------------------------------------------------------------

void GroupsWidget::editSlot()
{
  QModelIndex current(ui->tableView->currentIndex());
  int row = ui->tableView->currentIndex().row();
  if ( row == -1 )
    return;

  GroupEdit dialog(this, model->data(model->index(row, model->fieldIndex("id"))).toInt());
  if ( dialog.exec() == QDialog::Accepted )
  {
    model->select();
    ui->tableView->resizeRowsToContents();
    ui->tableView->setCurrentIndex(current);
  }
}

//--------------------------------------------------------------------------------
