#ifndef IMPORT_H
#define IMPORT_H

#include <QObject>
#include <QFileInfoList>

class Import : public QObject
{
    Q_OBJECT
  public:
    explicit Import(QObject *parent = 0);
    void loadJSonFile(QFileInfoList filename);

  signals:
    void updateProgressBar(int value);
    void importInfo(QString str);

  public slots:

  private:
    bool importR2B(QJsonObject data);
    void importReceipt(QJsonObject data);
    QFileInfoList list;

};

#endif // IMPORT_H
