#include "filewatcher.h"
#include "import.h"

#include <QMessageBox>
#include <QDir>
#include <QTimer>
#include <QDebug>

FileWatcher::FileWatcher (QWidget* parent)
    : QWidget(parent)
{
    this->timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(fileAdded()));

}

FileWatcher::~FileWatcher ()
{
}

void FileWatcher::directoryChanged(const QString &str)
{
    timer->start(1);
    // Q_UNUSED(str)
    // QMessageBox::information(this,"Directory Modified", "Your Directory is modified: " + str);
    appendQueue(str);
}

void FileWatcher::appendQueue(QString path)
{
    QDir directory;

    directory.setPath(path);
    QStringList filter;
    filter.append("*.json");
    list = directory.entryInfoList(filter, QDir::Files, QDir::Time | QDir::Reversed);

}

void FileWatcher::fileAdded()
{
    Q_FOREACH(QFileInfo fileinfo, list)
        qDebug() << "FileWatcher::appendQueue: " << fileinfo.absoluteFilePath();

    // emit addToQueue(list);
    import = new Import(this);

    connect(import,SIGNAL(importInfo(QString)),this,SIGNAL(importInfo(QString)));

    import->loadJSonFile(&list);
    list.clear();
}
