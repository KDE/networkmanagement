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

#include <nm-setting-connection.h>
#include <nm-setting-ip4-config.h>

#include <KDebug>
#include <KSharedConfig>
#include <KStandardDirs>

#include "configxml.h"
#include "datamappings.h"
#include "networksettings.h"
#include "knmserviceprefs.h"
#include "secretstoragehelper.h"
#include "knetworkmanagerserviceadaptor.h"

KConfigToService::KConfigToService(NetworkSettings * service, bool active)
    : m_service(service), m_error(!active)
{
    (void) new KNetworkManagerServiceAdaptor( this );
    QDBusConnection::sessionBus().registerService( "org.kde.knetworkmanagerd" ) ;
    QDBusConnection::sessionBus().registerObject( "/Configuration", this );

    KNetworkManagerServicePrefs::instance(KStandardDirs::locate("config",
                QLatin1String("knetworkmanagerrc")));
    m_dataMappings = new DataMappings;

    connect(m_service, SIGNAL(connectionActivated(const QString&)), SLOT(connectionActivated(const QString&)));
}

KConfigToService::~KConfigToService()
{
    delete m_dataMappings;
}

void KConfigToService::init()
{
    if (!m_error) {
        // 1) get the names of all the connections from the main config file
        // (this could also be just the connections in one profile, after removing all connections)
        QStringList connectionIds;
        connectionIds = KNetworkManagerServicePrefs::self()->connections();
        // 2) open each connection's file and create 1 or more ConfigXml for it
        foreach (QString connectionId, connectionIds) {
            QVariantMapMap connectionMap = restoreConnection(connectionId);
            m_connectionIdToObjectPath.insert(connectionId, m_service->addConnection(connectionMap));
        }
    }
}

 /*
 * how to know which ConfigXml are needed?
 * a) peek connection/type and use a static list of settings per connection type
 * ** b) use the config's groups list to instantiate multiple configxmls as needed
 * c) use the config's groups list to generate a synthetic configxml containing the needed sections
 *
 * for each group, create a QVariantMap containing its settings and store them in the master
 * connection map (maybe exclude any secrets)
 * check key for key name conversion
 *   check value for any value conversions needed
 */

QVariantMapMap KConfigToService::restoreConnection(const QString & connectionId)
{
    kDebug() << connectionId;
    m_configFile = KStandardDirs::locate("data",
                QLatin1String("knetworkmanager/connections/") + connectionId);
    QVariantMapMap connectionMap;
    KSharedConfig::Ptr config = KSharedConfig::openConfig(m_configFile, KConfig::NoGlobals);
    kDebug() << config->name() << " is at " << m_configFile;
    foreach (QString group, config->groupList()) {
        QVariantMap groupSettings = handleGroup(group);
        if (groupSettings.isEmpty()) {
            kDebug() << "Settings group '" << group << "' contains no settings!";
            connectionMap.insert(group, QVariantMap());
        } else {
            connectionMap.insert(group, groupSettings );
        }
    }
    kDebug() << connectionMap;
    // NM requires that a map exists for the connection's type.  If the settings are all defaults,
    // our config won't contain that group.  So create that map (empty) if it hasn't been created by
    // reading the config.
    if (!connectionMap.isEmpty() && !connectionMap.contains(m_currentConnectionType)) {
        connectionMap.insert(m_currentConnectionType, QVariantMap());
        //m_currentConnectionType = QString();
    }
    // Special case #2, NM requires that a setting group for "gsm" is accompannied by a "serial"
    // group
    QString serialSetting = QLatin1String("serial");
    if (!connectionMap.isEmpty() && !connectionMap.contains(serialSetting)) {
        connectionMap.insert(serialSetting, QVariantMap());
    }
    // Special case #3, NM requires that a setting group for "serial" is accompanied by a "ppp"
    // group
    QString pppSetting = QLatin1String("ppp");
    if (connectionMap.contains(serialSetting) && !connectionMap.contains(pppSetting)) {
        connectionMap.insert(pppSetting, QVariantMap());
    }
    // Special case #4, NM requires that a setting group for "pppoe" is accompanied by a "ppp"
    // group
    QString pppoeSetting = QLatin1String("pppoe");
    if (connectionMap.contains(pppoeSetting) && !connectionMap.contains(pppSetting)) {
        connectionMap.insert(pppSetting, QVariantMap());
    }
    return connectionMap;
}

QVariantMap KConfigToService::handleGroup(const QString & groupName)
{
    kDebug() << groupName;

    QVariantMap map;
    QFile schemaFile(KStandardDirs::locate("data",
            QString::fromLatin1("knetworkmanager/schemas/%1.kcfg").arg( groupName)));
    if (!schemaFile.exists()) {
        kDebug() << groupName << " config file at " << schemaFile.fileName() << " not found!";
        return QVariantMap();
    }
    QFile configFile(m_configFile);
    if (!configFile.exists()) {
        kDebug() << "configuration file: " << m_configFile << " not found!";
        return QVariantMap();
    }
    ConfigXml * config = new ConfigXml(m_configFile, &schemaFile, false,
            new SecretStorageHelper(/*connection id*/QLatin1String("testconfigxml"), groupName));

    foreach (KConfigSkeletonItem * item, config->items()) {
        item->swapDefault();
        QVariant defaultV = item->property();
        item->swapDefault();
        kDebug() << item->key() << " : '" << item->property() << "' is a " << item->property().type() << ", and " << (defaultV == item->property() ? "IS" : "IS NOT") << " default";
        if (defaultV != item->property()) { // only deserialise non-default values
            KCoreConfigSkeleton::ItemEnum * itemEnum = 0;
            if ((itemEnum = dynamic_cast<KCoreConfigSkeleton::ItemEnum *>(item))) {
                // get the list of choices from the ItemEnum, look up the name corresponding to the 
                // int returned by property() and put that in the map instead.
                // KDE5: choices2 is going to go away..
                QList<KCoreConfigSkeleton::ItemEnum::Choice> choices = itemEnum->choices();
                KCoreConfigSkeleton::ItemEnum::Choice choice = choices[item->property().toUInt()];
                map.insert(m_dataMappings->convertKey(item->key()),
                        m_dataMappings->convertValue(item->key(), choice.name));
            } else {
                map.insert(m_dataMappings->convertKey(item->key()),
                        m_dataMappings->convertValue(item->key(), item->property()));
            }
        }
    }
    // special case for ipv4 "addresses" field, which isn't KConfigSkeletonItem-friendly
    // TODO put this somewhere else - not every special case can live in this function.
    if ( groupName == QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME)) {
        KSharedConfig::Ptr ipv4Config = KSharedConfig::openConfig(m_configFile, KConfig::NoGlobals);
        KConfigGroup ipv4Group(ipv4Config, groupName);
        uint addressCount = ipv4Group.readEntry("addressCount", 0 );
        kDebug() << "#addresses:" << addressCount;
        UintListList addresses;
        for (uint i = 0; i < addressCount; i++) {
            QList<uint> addressList = ipv4Group.readEntry(QString::fromLatin1("address%1").arg(i), QList<uint>());
            kDebug() << "address " << i << " " << addressList;
            // a valid address must have 3 values: ip, netmask, broadcast
            if ( addressList.count() == 3 ) {
                addresses.append(addressList);
            }
        }
        if (!addresses.isEmpty()) {
            map.insert(QLatin1String(NM_SETTING_IP4_CONFIG_ADDRESSES), QVariant::fromValue(addresses));
        }
    }
    // special case for connection's "type" field, for which a corresponding QVariantMap must exist
    // for NM to accept the connection
    if (groupName == QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME)) {
        KConfigSkeletonItem * item = config->findItem(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME), QLatin1String(NM_SETTING_CONNECTION_TYPE));
        Q_ASSERT(item);
        m_currentConnectionType = item->property().toString();
    }
    
    return map;
}

/* 
 * Update the NetworkService's connections
 * - determine which connectionids are new and call restoreConnection on them
 * - take the list of changed and update those, or do it automatically
 * -
 */
void KConfigToService::configure(const QStringList& changedConnections)
{
    kDebug();
    if (m_error) { // don't update if we don't own the service, we will do this later when we regain the service
        kDebug() << "kded4 does not own the NetworkManagerUserSettings service, so it ignored a request to reload configuration";
        return;
    }

    KNetworkManagerServicePrefs::self()->readConfig();
    QStringList addedConnections, deletedConnections;
    // figure out which connections were added
    QStringList existingConnections = m_connectionIdToObjectPath.keys();
    foreach (QString connectionId, KNetworkManagerServicePrefs::self()->connections()) {
        if (!existingConnections.contains(connectionId)) {
            addedConnections.append(connectionId);
        }
    }
    // figure out which connections were deleted
    foreach (QString connectionId, existingConnections) {
        if (!KNetworkManagerServicePrefs::self()->connections().contains(connectionId)) {
            deletedConnections.append(connectionId);
        }
    }
    // update the service
    foreach (QString connectionId, deletedConnections) {
        QString objectPath = m_connectionIdToObjectPath.take(connectionId);
        kDebug() << "removing connection with id: " << connectionId;
        m_service->removeConnection(objectPath);
    }
    foreach (QString connectionId, changedConnections) {
        if (m_connectionIdToObjectPath.contains(connectionId)) {
            QVariantMapMap changedConnection = restoreConnection(connectionId);
            if (!changedConnection.isEmpty()) {
                kDebug() << "updating connection with id: " << connectionId;
                m_service->updateConnection(m_connectionIdToObjectPath[connectionId], changedConnection);
            }
        }
    }
    foreach (QString connectionId, addedConnections) {
        QVariantMapMap changedConnection = restoreConnection(connectionId);
        kDebug() << "adding connection with id: " << connectionId;
        m_service->addConnection(restoreConnection(connectionId));
    }
}

void KConfigToService::connectionActivated(const QString & uuid)
{
    kDebug() << uuid;
    // write the connection file
    QString configFile = KStandardDirs::locate("data",
                QLatin1String("knetworkmanager/connections/") + uuid);
    QVariantMapMap connectionMap;
    KSharedConfig::Ptr config = KSharedConfig::openConfig(configFile, KConfig::NoGlobals);
    kDebug() << config->name() << " is at " << configFile;
    KConfigGroup connectionGroup(config, NM_SETTING_CONNECTION_SETTING_NAME);

    uint newtimestamp = QDateTime::currentDateTime().toTime_t();

    if (connectionGroup.exists()) {
        uint timestamp = connectionGroup.readEntry<uint>(NM_SETTING_CONNECTION_TIMESTAMP, 0);
        kDebug() << "uuid: " << uuid << "old timestamp: " << timestamp << " new timestamp: " << newtimestamp;
        connectionGroup.writeEntry(NM_SETTING_CONNECTION_TIMESTAMP, newtimestamp);
    }

    // write knetworkmanagerrc
    // TODO: don't save the timestamp to the connection file, just read it from knetworkmanagerrc
    // and initialise the connection using that
    KConfigGroup mainConnectionDetails(KNetworkManagerServicePrefs::self()->config(), QLatin1String("Connection_") + uuid);
    if (mainConnectionDetails.exists()) {
        mainConnectionDetails.writeEntry("LastUsed",  QDateTime::fromTime_t(newtimestamp));
    }
}

// vim: sw=4 sts=4 et tw=100
