#include "backup.h"
#include "JlCompress.h"

#include <QSettings>
#include <QApplication>
#include <QDebug>

void Backup::create()
{

  QSettings settings(QSettings::IniFormat, QSettings::UserScope, "QRK", "QRK");
  QString backupDir = settings.value("backupDirectory", qApp->applicationDirPath()).toString();
  QString dataDir = qApp->applicationDirPath() + "/data";

  QString infile = QString("%1/%2-QRK.db").arg(dataDir).arg(QDate::currentDate().year());

  QString outfile = QString("%1/data_%2.zip").arg(backupDir).arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));

  bool ok = JlCompress::compressFile( outfile, infile );
  if (!ok)
    qDebug() << "JlCompress::compressFile:" << ok;

}

void Backup::cleanUp()
{
  removeDir("./imageformats");
  removeDir("./platforms");
  removeDir("./printsupport");
  removeDir("./sqldrivers");
  QFile::remove("./*.dll");

}

bool Backup::removeDir(const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}
