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
#include <nm-setting-vpn.h>
#include <nm-setting-ip4-config.h>
#include <nm-setting-8021x.h>

#include <KDebug>
#include <KSharedConfig>
#include <KStandardDirs>

#include "types.h"
#include "configxml.h"
#include "networksettings.h"
#include "knmserviceprefs.h"
#include "secretstoragehelper.h"
#include "knetworkmanagerserviceadaptor.h"

#include "connectionpersistence.h"

KConfigToService::KConfigToService(NetworkSettings * service, bool active)
    : QObject( service ), m_service(service), m_error(!active), m_init( false )
{
    (void) new KNetworkManagerServiceAdaptor( this );
    QDBusConnection::sessionBus().registerService( "org.kde.knetworkmanagerd" ) ;
    QDBusConnection::sessionBus().registerObject( "/modules/knetworkmanager", this );

    KNetworkManagerServicePrefs::instance(KStandardDirs::locate("config",
                QLatin1String("knetworkmanagerrc")));

    connect(m_service, SIGNAL(connectionActivated(const QString&)), SLOT(connectionActivated(const QString&)));
}

KConfigToService::~KConfigToService()
{
    kDebug();
    QDBusConnection::sessionBus().unregisterService( "org.kde.knetworkmanagerd" ) ;
    QDBusConnection::sessionBus().unregisterObject( "/modules/knetworkmanager" );
}

void KConfigToService::init()
{
    if (!m_error && !m_init) {
        // 1) get the names of all the connections from the main config file
        // (this could also be just the connections in one profile, after removing all connections)
        QStringList connectionIds;
        connectionIds = KNetworkManagerServicePrefs::self()->connections();
        // 2) open each connection's file and create 1 or more ConfigXml for it
        foreach (QString connectionId, connectionIds) {
            Knm::Connection * connection = restoreConnection(connectionId);
            m_connectionIdToObjectPath.insert(connectionId, m_service->addConnection(connection));
        }
        m_init = true;
    }
}

void KConfigToService::start( WId wid )
{
    // for now we do nothing but trigger kded load-on-demand
    // TODO: it might be good to autostop if this function wasn't called
    // in say 2 minutes (plasma crash / communication failure)
    kDebug();
    Knm::ConnectionPersistence::setWalletWid( wid );
    init();
}

void KConfigToService::stop()
{
   QDBusInterface kded("org.kde.kded", "/kded", "org.kde.kded");
   kded.call( "unloadModule", "knetworkmanager" );
    SecretStorageHelper::setWalletWid( 0 );
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

Knm::Connection * KConfigToService::restoreConnection(const QString & connectionId)
{
    kDebug() << connectionId;
    QString configFile = KStandardDirs::locate("data",
            QLatin1String("knetworkmanager/connections/") + connectionId);
    Knm::Connection * connection;
    if (!configFile.isEmpty())
    {
        m_config = KSharedConfig::openConfig(configFile, KConfig::NoGlobals);
        m_config->reparseConfiguration();
        // restore from disk
        Knm::ConnectionPersistence cp(m_config,
                (KNetworkManagerServicePrefs::self()->storeInWallet() ? Knm::ConnectionPersistence::Secure :
                 Knm::ConnectionPersistence::PlainText));
        cp.load();
        connection = cp.connection();

#if 0 // probably redundant anyway now - otherwise fix in connectiondbus
        // NM requires that a map exists for the connection's type.  If the settings are all defaults,
        // our config won't contain that group.  So create that map (empty) if it hasn't been created by
        // reading the config.
        if (!connectionMap.isEmpty() && !connectionMap.contains(m_currentConnectionType)) {
            connectionMap.insert(m_currentConnectionType, QVariantMap());
            //m_currentConnectionType = QString();
        }
#endif
#if 0
        // Special case #2, NM requires that a setting group for "gsm" is accompannied by a "serial"
        // group
        QString serialSetting = QLatin1String("serial");
        if (connectionMap.contains(QLatin1String("gsm") ) && !connectionMap.contains(serialSetting)) {
            connectionMap.insert(serialSetting, QVariantMap());
        }
#endif
#if 0
        // Special case #3, NM requires that a setting group for "serial" is accompanied by a "ppp"
        // group
        QString pppSetting = QLatin1String("ppp");
        if (connectionMap.contains(serialSetting) && !connectionMap.contains(pppSetting)) {
            connectionMap.insert(pppSetting, QVariantMap());
        }
#endif
#if 0
        // Special case #4, NM requires that a setting group for "pppoe" is accompanied by a "ppp"
        // group
        QString pppoeSetting = QLatin1String("pppoe");
        if (connectionMap.contains(pppoeSetting) && !connectionMap.contains(pppSetting)) {
            connectionMap.insert(pppSetting, QVariantMap());
        }
        kDebug() << connectionMap;
#endif
    }
    return connection;
}

// not called any more, but special cases must be moved out to connectionDbus
QVariantMap KConfigToService::handleGroup(const QString & groupName)
{
    kDebug() << groupName;

    QVariantMap map;
#if 0
    QFile schemaFile(KStandardDirs::locate("data",
            QString::fromLatin1("knetworkmanager/schemas/%1.kcfg").arg( groupName)));
    if (!schemaFile.exists()) {
        kDebug() << groupName << " config file at " << schemaFile.fileName() << " not found!";
        return QVariantMap();
    }
    ConfigXml * config = new ConfigXml(m_config, &schemaFile, false,
            new SecretStorageHelper(QString(), groupName));

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
                QList<KCoreConfigSkeleton::ItemEnum::Choice2> choices = itemEnum->choices2();
                KCoreConfigSkeleton::ItemEnum::Choice2 choice = choices[item->property().toUInt()];
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
        KConfigGroup ipv4Group(m_config, groupName);
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
    // special case for vpn data - which is not a simple type
    // TODO put this somewhere else - not every special case can live in this function.
    if ( groupName == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        KConfigGroup vpnGroup(m_config, groupName);
        QStringList data = vpnGroup.readEntry( "data", QStringList() );
        QStringMap dataMap;

        for ( int i = 0; i < data.count(); i += 2 )
            dataMap.insert( data[i], data[i+1] );

        map.insert(QLatin1String(NM_SETTING_VPN_DATA), QVariant::fromValue(dataMap));

        if ( !map.contains( NM_SETTING_VPN_SECRETS ) )
            map.insert( QLatin1String(NM_SETTING_VPN_SECRETS ), QVariant::fromValue( QStringMap() ) );
    }

    // special case for connection's "type" field, for which a corresponding QVariantMap must exist
    // for NM to accept the connection
    if (groupName == QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME)) {
        KConfigSkeletonItem * item = config->findItem(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME),
                                                      QLatin1String(NM_SETTING_CONNECTION_TYPE));
        Q_ASSERT(item);
        m_currentConnectionType = item->property().toString();
    }

    // special case for 8021x settings ca_cert -> have to pass it as blob
    if (groupName == QLatin1String(NM_SETTING_802_1X_SETTING_NAME)) {
        KConfigGroup group8021x(m_config, groupName);
        QString ca_path = group8021x.readEntry("capath");
        QFile ca_cert(ca_path);

        if (ca_cert.open(QIODevice::ReadOnly)) {
           QByteArray bytes = ca_cert.readAll();

           // FIXME: verify that the ca_cert is a X509 cert
           // (see libnm-util/nm-setting-8021x.c function nm_setting_802_1x_set_ca_cert_from_file)

           map.insert( QLatin1String(NM_SETTING_802_1X_CA_CERT), QVariant::fromValue(bytes));
        }
    }
#endif
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
    if (m_error) { // don't update if we don't own the service, we will do this later when we regain the service
        kDebug() << "kded4 does not own the NetworkManagerUserSettings service, so it ignored a request to reload configuration";
        return;
    }

    KNetworkManagerServicePrefs::self()->readConfig();
    QStringList addedConnections, deletedConnections;
    // figure out which connections were added
    QStringList existingConnections = m_connectionIdToObjectPath.keys();
    QStringList onDiskConnections = KNetworkManagerServicePrefs::self()->connections();
    qSort(existingConnections);
    qSort(onDiskConnections);
    kDebug() << "existing connections are:" << existingConnections;
    kDebug() << "on-disk connections are:" << onDiskConnections;

    foreach (QString connectionId, onDiskConnections) {
        if (!existingConnections.contains(connectionId)) {
            addedConnections.append(connectionId);
        }
    }
    // figure out which connections were deleted
    foreach (QString connectionId, existingConnections) {
        if (!onDiskConnections.contains(connectionId)) {
            deletedConnections.append(connectionId);
        }
    }
    kDebug() << "added connections:" << addedConnections;
    kDebug() << "changed connections:" << changedConnections;
    kDebug() << "deleted connections:" << deletedConnections;

    // update the service
    foreach (QString connectionId, deletedConnections) {
        QString objectPath = m_connectionIdToObjectPath.take(connectionId);
        kDebug() << "removing connection with id: " << connectionId;
        m_service->removeConnection(objectPath);
    }
    foreach (const QString connectionId, changedConnections) {
        if (m_connectionIdToObjectPath.contains(connectionId)) {
            Knm::Connection * changedConnection = restoreConnection(connectionId);
            if (changedConnection) {
                kDebug() << "updating connection with id:" << connectionId;
                QString objPath = m_connectionIdToObjectPath.value(connectionId);
                kDebug() << "at objectpath:" << objPath;
                m_service->updateConnection(objPath, changedConnection);
            }
        }
    }
    foreach (QString connectionId, addedConnections) {
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
