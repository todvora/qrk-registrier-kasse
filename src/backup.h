#ifndef BACKUP_H
#define BACKUP_H

class QString;

class Backup
{
  public:
    static void create();
    static void cleanUp();

  private:
    static bool removeDir(const QString & dirName);


};

#endif // BACKUP_H
