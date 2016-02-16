#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QWidget>
#include <QFileInfoList>

class QTimer;

class FileWatcher : public QWidget
{
    Q_OBJECT

public:
    FileWatcher(QWidget* parent=0);
    ~FileWatcher();

  signals:
    void addToQueue(QFileInfoList infoList);

  public slots:
    void showModified(const QString& str);
    void fileAdded();


private:
    void appendQueue(QString path);
    QTimer *timer;
    QFileInfoList list;
};

#endif // FILEWATCHER_H
