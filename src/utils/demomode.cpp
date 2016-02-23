#include "demomode.h"
#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>

bool DemoMode::isDemoMode()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);
  q.prepare("SELECT value FROM globals WHERE name='demomode'");
  q.exec();
  if (q.next())
    return q.value("value").toBool();

  return !isModeNotSet();

}

bool DemoMode::isModeNotSet()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);
  q.prepare("SELECT value FROM globals WHERE name='demomode'");
  q.exec();
  if (q.next())
    return false;

  q.prepare(QString("INSERT INTO globals (name, value) VALUES('%1', %2)").arg("demomode").arg(true));
  q.exec();

  return true;

}

void DemoMode::leaveDemoMode()
{
  QSqlDatabase dbc = QSqlDatabase::database("CN");
  QSqlQuery q(dbc);
  q.prepare(QString("UPDATE globals SET value=%1 WHERE name='demomode'").arg(false));
  q.exec();

}
