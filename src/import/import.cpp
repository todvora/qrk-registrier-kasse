#include "import.h"
#include "qrkregister.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

#include <QDebug>

Import::Import(QObject *parent) : QObject(parent)
{

}

void Import::loadJSonFile(QFileInfoList filenames)
{

  Q_FOREACH(QFileInfo fileinfo, filenames) {

    QString receiptInfo;
    qDebug() << "Import::loadJSonFile " << fileinfo.absoluteFilePath();
    QFile file;
    file.setFileName(fileinfo.absoluteFilePath());
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    receiptInfo = file.readAll();
    file.close();
    file.remove();

    QJsonDocument jd = QJsonDocument::fromJson(receiptInfo.toUtf8());
    QJsonObject data = jd.object();
    if (data.contains("r2b")) {
      qDebug() << "Import::loadJSonFile type = r2b";
      importR2B(data);
    } else if (data.contains("receipt")) {
      qDebug() << "Import::loadJSonFile type = receipt";

    }

  }

}

void Import::importR2B(QJsonObject data)
{
  QJsonArray r2bArray = data.value("r2b").toArray();
  bool ok = false;
  foreach (const QJsonValue & value, r2bArray) {
      QJsonObject obj = value.toObject();
      ok = obj.contains("gross") && obj.contains("net") && obj.contains("receiptNum"); // && obj.contains("payedBy");
      if (ok) {
        QProgressBar *bar = new QProgressBar(0);
        QRKRegister *reg = new QRKRegister(bar);
        reg->init();
        reg->setServerMode(true);
        reg->newOrder();
        reg->setR2BServerMode(obj);

        if (reg->checkEOAnyServerMode()) {
          if (int receiptNum = reg->createReceipts()) {
            reg->setCurrentReceiptNum(receiptNum);
            if (reg->createOrder())
              reg->finishReceipts(0);
          }
        }

      } else {
        /* TODO: emit ERROR INFO*/
      }

  }
}

void Import::importReceipt(QJsonObject data)
{
  QJsonArray r2bData = data.value("0").toArray();

}
