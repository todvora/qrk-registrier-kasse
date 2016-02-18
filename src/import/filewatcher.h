#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QWidget>
#include <QFileInfoList>

class QTimer;
class Import;

class FileWatcher : public QWidget
{
    Q_OBJECT

public:
    FileWatcher(QWidget* parent=0);
    ~FileWatcher();

  signals:
    void addToQueue(QFileInfoList infoList);
    void importInfo(QString str);

  public slots:
    void directoryChanged(const QString& str);
    void fileAdded();


private:
    void appendQueue(QString path);
    QTimer *timer;
    QFileInfoList list;
    Import *import;
};

#endif // FILEWATCHER_H
