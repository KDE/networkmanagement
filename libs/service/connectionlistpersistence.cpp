/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "connectionlistpersistence.h"

// remove NM dep, provide own defs
#if 0
#include <nm-setting-connection.h>
#include <nm-setting-vpn.h>
#include <nm-setting-ip4-config.h>
#include <nm-setting-8021x.h>
#endif

#include <KDebug>
#include <KSharedConfig>
#include <KStandardDirs>

#include <libnm-qt/generic-types.h>
// internals includes
#include "connectionpersistence.h"
#include "knmserviceprefs.h"

#include "connectionlist.h"

// move to sessionabstractedconnectionhandler
#if 0
#include "knetworkmanagerserviceadaptor.h"
#endif

class ConnectionListPersistencePrivate
{
public:
    ConnectionList * list;
    QString persistencePath;
    bool ignoreChangedConnections;
    bool init;
    // flag used while we are loading connections to prevent us handling our own changes
};

const QString ConnectionListPersistence::NETWORKMANAGEMENT_RCFILE = QLatin1String("networkmanagementrc");
const QString ConnectionListPersistence::CONNECTION_PERSISTENCE_PATH = QLatin1String("networkmanagement/connections/");

ConnectionListPersistence::ConnectionListPersistence(ConnectionList * list, const QString & rcfile, const QString & persistencePath)
: QObject(list), d_ptr(new ConnectionListPersistencePrivate)
{
    Q_D(ConnectionListPersistence);
    d->list = list;
    d->ignoreChangedConnections = false;
    d->init = false;
    d->persistencePath = persistencePath;

    KNetworkManagerServicePrefs::instance(rcfile.isEmpty() ? Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE : rcfile);
    // we have to force a reparse because this library may be used in a plugin (eg a KDEDModule), and:
    // 1) plugin was loaded
    // 2) config was parsed
    // 3) plugin was unloaded
    // 4) config was changed
    // 5) owning process now holds a static ksharedconfig containing the stale configuration
    // 6) plugin reloaded
    // 7) instance only calls KConfig::reparseConfiguration() the first time it is called (static)
    KNetworkManagerServicePrefs::self()->config()->reparseConfiguration();
}

ConnectionListPersistence::~ConnectionListPersistence()
{
}

void ConnectionListPersistence::init()
{
    Q_D(ConnectionListPersistence);
    if (!d->init) {
        // 1) get the names of all the connections from the main config file
        // (this could also be just the connections in one profile, after removing all connections)
        QStringList connectionIds;
        connectionIds = KNetworkManagerServicePrefs::self()->connections();
        // 2) restore each connection
        foreach (const QString &connectionId, connectionIds) {
            Knm::Connection * connection = restoreConnection(connectionId);
            if (connection ) {
                d->list->addConnection(connection);
            }
        }
        d->init = true;
    }
}

Knm::Connection * ConnectionListPersistence::restoreConnection(const QString & connectionId)
{
    kDebug() << connectionId;

    Knm::Connection * connection = 0;
    KSharedConfig::Ptr config = connectionFileForUuid(connectionId);
    if (!config.isNull()) {
        // restore from disk
        kDebug() << config->name();
        kDebug() << config->groupList();

        Knm::ConnectionPersistence cp(config,
                (Knm::ConnectionPersistence::SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode()
                );
        cp.load();
        connection = cp.connection();
        Q_ASSERT(connection->uuid() == connectionId);
        // set the origin of this connection
        connection->setOrigin(QLatin1String("ConnectionListPersistence"));
    }

    return connection;
}

KSharedConfig::Ptr ConnectionListPersistence::connectionFileForUuid(const QString & uuid)
{
    Q_D(ConnectionListPersistence);
    KSharedConfig::Ptr config;
    if (!uuid.isEmpty()) {
        QString configFile;
        if (d->persistencePath.isEmpty()) {
            configFile = KStandardDirs::locate("data",
                    CONNECTION_PERSISTENCE_PATH + uuid);
        } else {
            configFile = d->persistencePath + uuid;
        }

        kDebug() << "configFile:" << configFile;
        if (!configFile.isEmpty()) {
            config = KSharedConfig::openConfig(configFile, KConfig::NoGlobals);
            if (config.isNull()) {
                kDebug() << "Config not found at" << configFile;
            }
        }
    }
    return config;
}

/*
 * Update the NetworkService's connections
 * - determine which connectionids are new and call restoreConnection on them
 * - take the list of changed and update those, or do it automatically
 */
void ConnectionListPersistence::configure(const QStringList& changedConnections)
{
    Q_D(ConnectionListPersistence);
    // start ignoring connection changes, so we don't write out things we have just read.
    d->ignoreChangedConnections = true;

    KNetworkManagerServicePrefs::self()->readConfig();
    QStringList addedConnections, deletedConnections;
    // figure out which connections were added
    QStringList allConnections = d->list->connections();
    QStringList localConnections;
    foreach (const QString &connectionId, allConnections) {
        Knm::Connection * connection = d->list->findConnection(connectionId);
        if (connection->origin() == QLatin1String("ConnectionListPersistence")) {
            localConnections.append(connectionId);
        }
    }

    QStringList onDiskConnections = KNetworkManagerServicePrefs::self()->connections();
    qSort(localConnections);
    qSort(onDiskConnections);
    kDebug() << "known local connections are:" << localConnections;
    kDebug() << "on-disk connections are:" << onDiskConnections;

    foreach (const QString &connectionId, onDiskConnections) {
        if (!localConnections.contains(connectionId)) {
            addedConnections.append(connectionId);
        }
    }
    // figure out which connections were deleted
    foreach (const QString &connectionId, localConnections) {
        if (!onDiskConnections.contains(connectionId)) {
            deletedConnections.append(connectionId);
        }
    }
    kDebug() << "added connections:" << addedConnections;
    kDebug() << "changed connections:" << changedConnections;
    kDebug() << "deleted connections:" << deletedConnections;

    // update the service
    // remove
    foreach (const QString &connectionId, deletedConnections) {
        d->list->removeConnection(connectionId);
    }

    // changed - we are going to get our own change signals back now, but since
    // d->ignoreChangedConnections is set, nothing happens
    foreach (const QString &connectionId, changedConnections) {
        d->list->replaceConnection(restoreConnection(connectionId));
    }
    // if this is accidentally removed, updated timestamps and seenBSSID lists will not be saved
    d->ignoreChangedConnections = false;

    // added
    foreach (const QString &connectionId, addedConnections) {
        d->list->addConnection(restoreConnection(connectionId));
    }
}

void ConnectionListPersistence::handleUpdate(Knm::Connection * connection)
{
    Q_D(ConnectionListPersistence);

    if (connection && !d->ignoreChangedConnections && connection->origin() == QLatin1String("ConnectionListPersistence")) {
        QString uuid = connection->uuid();

        Knm::ConnectionPersistence cp(connection, connectionFileForUuid(uuid),
                (Knm::ConnectionPersistence::SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode()
                );
        cp.save();

        // write knetworkmanagerrc
        // TODO: don't save the timestamp to the connection file, just read it from knetworkmanagerrc
        // and initialise the connection using that
        KConfigGroup mainConnectionDetails(KNetworkManagerServicePrefs::self()->config(), QLatin1String("Connection_") + uuid);
        if (mainConnectionDetails.exists()) {
            mainConnectionDetails.writeEntry("LastUsed",  QDateTime::currentDateTime());
            // start timer instead
            KNetworkManagerServicePrefs::self()->config()->sync();
        }
    }
}

void ConnectionListPersistence::handleAdd(Knm::Connection *)
{
    qDebug();
}

void ConnectionListPersistence::handleRemove(Knm::Connection *)
{
    qDebug();
}

//move to separate object
#if 0
void ConnectionListPersistence::start( WId wid )
{
    // for now we do nothing but trigger kded load-on-demand
    // TODO: it might be good to autostop if this function wasn't called
    // in say 2 minutes (plasma crash / communication failure)
    kDebug();
    Knm::ConnectionPersistence::setWalletWid( wid );
    init();
}

void ConnectionListPersistence::stop()
{
   QDBusInterface kded("org.kde.kded", "/kded", "org.kde.kded");
   kded.call( "unloadModule", "knetworkmanager" );
   Knm::ConnectionPersistence::setWalletWid( 0 );
}
#endif

// to setting?
// not called any more, but special cases must be moved out to connectionDbus
#if 0
QVariantMap ConnectionListPersistence::handleGroup(const QString & groupName)
{
    kDebug() << groupName;

    QVariantMap map;
    // stuff left to port from configxml
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
    return map;
}
#endif

// vim: sw=4 sts=4 et tw=100
