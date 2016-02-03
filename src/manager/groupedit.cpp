#include "groupedit.h"

#include <QSqlQuery>

//--------------------------------------------------------------------------------

GroupEdit::GroupEdit(QWidget *parent, int theId)
  : QDialog(parent), id(theId)
{
  ui.setupUi(this);

  if ( id != -1 )
  {
    QSqlDatabase dbc = QSqlDatabase::database("CN");

    QSqlQuery query(QString("SELECT name,visible FROM groups WHERE id=%1").arg(id), dbc);
    query.next();

    ui.name->setText(query.value(0).toString());
    ui.visibleCheckBox->setChecked(query.value(1).toBool());

  }
}

//--------------------------------------------------------------------------------

void GroupEdit::accept()
{
  QSqlQuery query;

  if ( id == -1 )  // new entry
  {
    query.exec(QString("INSERT INTO groups (name, visible) VALUES('%1', %2)")
                       .arg(ui.name->text())
                       .arg(ui.visibleCheckBox->isChecked()));
  }
  else
  {
    query.exec(QString("UPDATE groups SET name='%1',visible=%2 WHERE id=%3")
                       .arg(ui.name->text())
                       .arg(ui.visibleCheckBox->isChecked())
                       .arg(id));
  }

  QDialog::accept();
}

//--------------------------------------------------------------------------------
