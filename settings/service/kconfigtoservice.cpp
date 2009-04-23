/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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
#include "networksettings.h"
#include "knmserviceprefs.h"
#include "knetworkmanagerserviceadaptor.h"

#include "connectionpersistence.h"

KConfigToService::KConfigToService(NetworkSettings * service, bool active)
    : QObject( service ), m_service(service), m_error(!active), m_init( false )
{
    (void) new KNetworkManagerServiceAdaptor( this );
    QDBusConnection::sessionBus().registerService( "org.kde.knetworkmanagerd" ) ;
    QDBusConnection::sessionBus().registerObject( "/modules/knetworkmanager", this );

    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
    // we have to force a reparse because
    // 1) knetworkmanager module was loaded
    // 2) config was parsed
    // 3) module was unloaded
    // 4) config was changed
    // 5) kded now holds a static ksharedconfig containing the stale configuration
    // 6) module reloaded
    // 7) instance only calls KConfig::reparseConfiguration() the first time it is called (static)
    KNetworkManagerServicePrefs::self()->config()->reparseConfiguration();

    connect(m_service, SIGNAL(connectionUpdated(Knm::Connection*)), SLOT(connectionUpdated(Knm::Connection*)));
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
        // 2) restore each connection
        foreach (QString connectionId, connectionIds) {
            Knm::Connection * connection = restoreConnection(connectionId);
            if (connection) {
                m_connectionIdToObjectPath.insert(connectionId, m_service->addConnection(connection));
            }
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
   Knm::ConnectionPersistence::setWalletWid( 0 );
}

Knm::Connection * KConfigToService::restoreConnection(const QString & connectionId)
{
    kDebug() << connectionId;
    m_config = connectionFileForUuid(connectionId);
    Knm::Connection * connection = 0;
    if (!m_config.isNull()) {
        // restore from disk
        Knm::ConnectionPersistence cp(m_config,
                (KNetworkManagerServicePrefs::self()->storeInWallet() ? Knm::ConnectionPersistence::Secure :
                 Knm::ConnectionPersistence::PlainText));
        cp.load();
        connection = cp.connection();
    } else {
        kError() << "Config file for connection" << connectionId << "not found!";
    }

    return connection;
}

// not called any more, but special cases must be moved out to connectionDbus
QVariantMap KConfigToService::handleGroup(const QString & groupName)
{
    kDebug() << groupName;

    QVariantMap map;
    // stuff left to port from configxml
#if 0
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
        Knm::Connection * connection = restoreConnection(connectionId);
        if (connection) {
            m_connectionIdToObjectPath.insert(connectionId, m_service->addConnection(connection));
        }
    }
}

KSharedConfig::Ptr KConfigToService::connectionFileForUuid(const QString & uuid)
{
    KSharedConfig::Ptr config;
    if (!uuid.isEmpty()) {
        QString configFile = KStandardDirs::locate("data",
                Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + uuid);
        if (!configFile.isEmpty()) {
            config = KSharedConfig::openConfig(configFile, KConfig::NoGlobals);
            kDebug() << config->name() << " is at " << configFile;
        }
    }
    return config;
}

void KConfigToService::connectionUpdated(Knm::Connection * connection)
{
    QString uuid = connection->uuid();
    Knm::ConnectionPersistence cp(connection, connectionFileForUuid(uuid),
            (KNetworkManagerServicePrefs::self()->storeInWallet() ? Knm::ConnectionPersistence::Secure :
             Knm::ConnectionPersistence::PlainText));
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

// vim: sw=4 sts=4 et tw=100
