#include "backup.h"
#include "JlCompress.h"

#include <QSettings>
#include <QDebug>

void Backup::create()
{

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  QString backupDir = settings.value("backupDirectory", ".").toString();
  QString dataDir = settings.value("dataDirectory", "./data").toString();

  QString infile = QString("%1/%2-QRK.db").arg(dataDir).arg(QDate::currentDate().year());

  QString outfile = QString("%1/data_%2.zip").arg(backupDir).arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));

  bool ok = JlCompress::compressFile( outfile, infile );
  if (!ok)
    qDebug() << "JlCompress::compressFile:" << ok;

}
