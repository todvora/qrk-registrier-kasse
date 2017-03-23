/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015-2017 Christian Kvasny <chris@ckvsoft.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Button Design, and Idea for the Layout are lean out from LillePOS, Copyright 2010, Martin Koller, kollix@aon.at
 *
*/

#include "Interfaces/plugininterface.h"
#include "pluginmanager.h"

#include <QtCore>
#include <QtDebug>

class PluginManagerPrivate
{
public:
    bool check(const QString& path);

public:
    QHash<QString, QVariant> names;
    QHash<QString, QVariant> versions;
    QHash<QString, QVariant> author;
    QHash<QString, QVariantList> dependencies;

public:
    QHash<QString, QPluginLoader *> loaders;
};

bool PluginManagerPrivate::check(const QString& path)
{
    bool status = true;

    foreach(QVariant item, this->dependencies.value(path)) {

        QVariantMap mitem = item.toMap();
        QVariant na_mitem = mitem.value("name");
        QVariant ve_mitem = mitem.value("version");
        QString key = this->names.key(na_mitem);

        if(!this->names.values().contains(na_mitem)) {
            qWarning() << Q_FUNC_INFO << "  Missing dependency:" << na_mitem.toString() << "for plugin" << path;
            status = false;
            continue;
        }

        if (this->versions.value(key) != ve_mitem) {
            qWarning() << Q_FUNC_INFO << "    Version mismatch:" << na_mitem.toString() << "version" << this->versions.value(this->names.key(na_mitem)).toString() << "but" << ve_mitem.toString() << "required for plugin" << path;
            status = false;
            continue;
        }

        if(!check(key)) {
            qWarning() << Q_FUNC_INFO << "Corrupted dependency:" << na_mitem.toString() << "for plugin" << path;
            status = false;
            continue;
        }
    }

    return status;
}

PluginManager *PluginManager::instance(void)
{
    if(!s_instance)
        s_instance = new PluginManager;

    return s_instance;
}

void PluginManager::initialize(void)
{
    QDir path = QDir(qApp->applicationDirPath());
    path.cdUp();
    path.cd("plugins/bin");

    QStringList paths;
    paths << path.absolutePath() << "/usr/share/ckvsoft/qrk/plugins";
    path.setPath(qApp->applicationDirPath());
    path.cd("plugins");
    paths << path.absolutePath();
    path.setPath(qApp->applicationDirPath());
    path.cdUp();
    path.cd("plugins");
    paths << path.absolutePath();

    foreach (const QString &str, paths) {
        qInfo() << "Plugin search Path=" << str;
        path = QDir(str);
        foreach(QFileInfo info, path.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
            this->scan(info.absoluteFilePath().trimmed());

        foreach(QFileInfo info, path.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
            this->load(info.absoluteFilePath().trimmed());
    }
}

void PluginManager::uninitialize(void)
{
    foreach(const QString &path, d->loaders.keys())
        this->unload(path);
}

void PluginManager::scan(const QString& path)
{
    if(!QLibrary::isLibrary(path))
        return;

    QPluginLoader *loader = new QPluginLoader(path);

    QVariant name = loader->metaData().value("MetaData").toObject().value("name").toVariant();
    QList<QVariant> values = d->names.values();

    if (!values.contains(name)) {
        d->names.insert(path, loader->metaData().value("MetaData").toObject().value("name").toVariant());
        d->versions.insert(path, loader->metaData().value("MetaData").toObject().value("version").toVariant());
        d->author.insert(path, loader->metaData().value("MetaData").toObject().value("author").toVariant());
        d->dependencies.insert(path, loader->metaData().value("MetaData").toObject().value("dependencies").toArray().toVariantList());
    }

    delete loader;
}

void PluginManager::load(const QString& path)
{
    if(!QLibrary::isLibrary(path))
        return;

    if(!d->check(path))
        return;

    QPluginLoader *loader = new QPluginLoader(path);
    if (d->names.value(path) == loader->metaData().value("MetaData").toObject().value("name").toVariant()) {
        if(qobject_cast<PluginInterface *>(loader->instance()))
            d->loaders.insert(path, loader);
        else {
            qWarning() << "can't load:" << path << " Error: " << loader->errorString();
            delete loader;
        }
    } else {
        delete loader;
    }

}

void PluginManager::unload(const QString& path)
{
    QPluginLoader *loader = d->loaders.value(path);

    if(loader->unload()) {
        d->loaders.remove(path);
        delete loader;
    }
}

QStringList PluginManager::plugins(void)
{
    return d->loaders.keys();
}

QObject* PluginManager::getObjectByName(QString name)
{
    if (!d->names.key((QVariant)name).isNull()) {
        if (d->loaders.value(d->names.key((QVariant)name))) {
            return d->loaders.value(d->names.key((QVariant)name))->instance();
        }
    }
    return NULL;
}

PluginManager::PluginManager(void) : d(new PluginManagerPrivate)
{

}

PluginManager::~PluginManager(void)
{
    delete d;

    d = NULL;
}

PluginManager *PluginManager::s_instance = NULL;
