/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kconfigtoservice.h"

#include <KDebug>
#include <KSharedConfig>

#include "configxml.h"
#include "networksettings.h"

KConfigToService::KConfigToService(NetworkSettings * service)
    : m_service(service)
{

}

KConfigToService::~KConfigToService()
{
}

void KConfigToService::init()
{
    // 1) get the names of all the connections from the main config file
    // (this could also be just the connections in one profile, after removing all connections)
    QStringList connectionIds;
    connectionIds << "/tmp/testconfigxmlrc";
    // 2) open each connection's file and create 1 or more ConfigXml for it
    foreach (QString connectionId, connectionIds) {
        restoreConnection(connectionId);
    }

 /*
 * how to know which ConfigXml are needed?  
 * a) peek connection/type and use a static list of settings per connection type
 * b) use the config's groups list to instantiate multiple configxmls as needed
 * c) use the config's groups list to generate a synthetic configxml containing the needed sections
 *
 * for each group, create a QVariantMap containing its settings and store them in the master
 * connection map (maybe exclude any secrets)
 * check key for key name conversion
 *   check value for any value conversions needed
 */
}

void KConfigToService::restoreConnection(const QString & connectionId)
{
    // eventually, take the connectionId and look up a file in appData
    // for now, it is an absolute path
    QMap<QString, QVariantMap> connectionMap;
    KSharedConfig::Ptr config = KSharedConfig::openConfig(connectionId, KConfig::NoGlobals);
    foreach (QString group, config->groupList()) {
        connectionMap.insert(group, handleGroup(group, connectionId));
    }
    kDebug() << connectionMap;
}

QVariantMap KConfigToService::handleGroup(const QString & groupName, const QString & configFile)
{
    kDebug() << groupName;
    QVariantMap map;
    QFile * kcfgFile = new QFile(QString("settings/%1.kcfg").arg(groupName));
    ConfigXml * config = new ConfigXml(configFile, kcfgFile);
    foreach (KConfigSkeletonItem * item, config->items()) {
        item->swapDefault();
        QVariant defaultV = item->property();
        item->swapDefault();
        if (defaultV != item->property()) { // only serialise non-default values
            kDebug() << item->key() << item->property() << item->property().type() << (defaultV == item->property() ? "IS" : "IS NOT") << " default";
            map.insert(convertKey(item->key()), convertValue(item->key(), item->property()));
        }
    }
    return map;
}

QString KConfigToService::convertKey(const QString & storedKey) const
{
    //kDebug() << storedKey << "UNIMPLEMENTED";
    QString nmKey = storedKey;
    return nmKey;
}

QVariant KConfigToService::convertValue(const QString& key, const QVariant& value) const
{
    //kDebug() << key << "UNIMPLEMENTED";
    Q_UNUSED(key);
    return value;
}

// vim: sw=4 sts=4 et tw=100
