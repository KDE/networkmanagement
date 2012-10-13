/*
Copyright 2011 Lamarque V. Souza <lamarque@gmail.com>

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

namespace Knm {
    class Setting;
}

using namespace Knm;

#include <kstandarddirs.h>
#include <knmserviceprefs.h>

#include "../libs/internals/connection.h"
#include "settingsnm08/settingpersistence.cpp"
#include "settingsnm08/802-11-wirelesspersistence.cpp"
#include "settingsnm08/802-11-wireless-securitypersistence.cpp"
#include "settingsnm08/802-1xpersistence.cpp"
#include "settingsnm08/802-3-ethernetpersistence.cpp"
#include "settingsnm08/bluetoothpersistence.cpp"
#include "settingsnm08/cdmapersistence.cpp"
#include "settingsnm08/gsmpersistence.cpp"
#include "settingsnm08/ipv4persistence.cpp"
#include "settingsnm08/ipv6persistence.cpp"
#include "settingsnm08/pppoepersistence.cpp"
#include "settingsnm08/ppppersistence.cpp"
#include "settingsnm08/serialpersistence.cpp"
#include "settingsnm08/vpnpersistence.cpp"
#include "paths.h"
#include "nm08connections.h"

static const QString CONNECTION_PERSISTENCE_PATH = QLatin1String("networkmanagement/connections/");

Nm08Connections::Nm08Connections(SecretStorage * secretStorage, NMDBusSettingsConnectionProvider * nmDBusConnectionProvider, QObject * parent)
    : QObject(parent), m_secretStorage(secretStorage), m_nmDBusConnectionProvider(nmDBusConnectionProvider)
{
    KNetworkManagerServicePrefs::instance(Knm::NETWORKMANAGEMENT_RCFILE);
    KNetworkManagerServicePrefs::self()->config()->reparseConfiguration();
    m_storageMode = (SecretStorage::SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode();

    m_connectionsToImport = KNetworkManagerServicePrefs::self()->connections();
    kDebug() << "Connections to import:" << m_connectionsToImport;

    connect(m_secretStorage, SIGNAL(connectionRead(Knm::Connection*,QString,bool,bool)), SLOT(gotSecrets(Knm::Connection*,QString,bool)));
    connect(m_nmDBusConnectionProvider, SIGNAL(addConnectionCompleted(bool,QString)), SLOT(importNextNm08Connection()));
}

Nm08Connections::~Nm08Connections()
{
}

void Nm08Connections::importNextNm08Connection()
{
    if (m_connectionsToImport.isEmpty()) {
        if (!m_connectionsToDelete.isEmpty()) {
            disconnect(m_secretStorage, 0, this, 0);
            disconnect(m_nmDBusConnectionProvider, 0, this, 0);
            if (!m_persistences.isEmpty()) {
                qDeleteAll(m_persistences);
                m_persistences.clear();
            }
            if (!m_connectionsBeingAdded.isEmpty()) {
                KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
                foreach (const Connection * con, m_connectionsBeingAdded) {
                    //prefs->config()->deleteGroup(QLatin1String("Connection_") + con->uuid());
                    //QString configFile = KStandardDirs::locate("data", CONNECTION_PERSISTENCE_PATH + con->uuid());
                    //File::remove(configFile);
                    m_connectionsToImport.removeOne(con->uuid());
                }
                prefs->setConnections(m_connectionsToImport);
                prefs->writeConfig();
                m_connectionsBeingAdded.clear();
            }
            // child removes itself from list in its destructor, which causes crashes if we use qDeleteAll() of Qt >= 4.8,
            // so use this loop instead. See https://bugs.kde.org/show_bug.cgi?id=284989
            while (!m_connectionsToDelete.isEmpty()) {
                delete m_connectionsToDelete.takeFirst();
            }
        }
        deleteLater();
        return;
    }

    QString connectionId = m_connectionsToImport.takeFirst();
    QString configFile = KStandardDirs::locate("data", CONNECTION_PERSISTENCE_PATH + connectionId);
    Connection * connection = 0;
    QString uuid, type;
    KConfigGroup cg;
    KSharedConfig::Ptr config;
    QHash<QString, QString> permissions;

    if (!QFile::exists(configFile)) {
        goto END;
    }

    kWarning() << "Importing" << connectionId;
    config = KSharedConfig::openConfig(configFile, KConfig::NoGlobals);

    if (config.isNull()) {
        kWarning() << "Config not found at" << configFile;
        goto END;
    }

    cg = KConfigGroup(config, "connection");
    uuid = cg.readEntry("uuid");
    type = cg.readEntry("type");

    if (uuid.isEmpty() || type.isEmpty()) {
        kWarning() << "Ignoring connection because uuid == '" << uuid << "' type == '" << type << "'";
        goto END;
    } else {
        connection = new Connection(QUuid(uuid), Connection::typeFromString(type));
        m_connectionsToDelete.append(connection);
    }

    connection->setName(cg.readEntry("id"));
    connection->setAutoConnect(cg.readEntry<bool>("autoconnect", false));
    connection->setTimestamp(cg.readEntry<QDateTime>("timestamp", QDateTime()));
    connection->setIconName(cg.readEntry("icon"));

    // load each setting
    foreach (Setting * setting, connection->settings()) {
        SettingPersistence * sp = persistenceFor(setting, config);
        sp->load();

        // This is asynchronous, Nm08Connections::gotSecrets() is the callback.
        m_secretStorage->loadSecrets(connection, setting->name(), SecretsProvider::None);
    }
    return;

END:
    QTimer::singleShot(0, this, SLOT(importNextNm08Connection()));
}

void Nm08Connections::gotSecrets(Knm::Connection * connection, const QString & settingName, bool ok)
{
    Q_UNUSED(settingName);
    Q_UNUSED(ok);
 
    if (!m_connectionsToDelete.contains(connection) || m_connectionsBeingAdded.contains(connection)) {
        return;
    }
    m_connectionsBeingAdded.append(connection);

    // This is asynchronous, Nm08Connections::importNextNm08Connection() is the callback.
    m_nmDBusConnectionProvider->addConnection(connection);
    kWarning() << "Importing" << connection->uuid() << "finished.";
}

Knm::SettingPersistence * Nm08Connections::persistenceFor(Knm::Setting * setting, KSharedConfig::Ptr config)
{
    SettingPersistence * sp = m_persistences.value(setting);
    SettingPersistence::SecretStorageMode storageMode = (SettingPersistence::SecretStorageMode) m_storageMode;
    if (!sp)
        switch (setting->type()) {
            case Setting::Cdma:
                sp = new CdmaPersistence(static_cast<Knm::CdmaSetting*>(setting), config, storageMode);
                break;
            case Setting::Gsm:
                sp = new GsmPersistence(static_cast<GsmSetting*>(setting), config, storageMode);
                break;
            case Setting::Bluetooth:
                sp = new BluetoothPersistence(static_cast<BluetoothSetting*>(setting), config, storageMode);
                break;
            case Setting::Ipv4:
                sp = new Ipv4Persistence(static_cast<Ipv4Setting*>(setting), config, storageMode);
                break;
            case Setting::Ipv6:
                sp = new Ipv6Persistence(static_cast<Ipv6Setting*>(setting), config, storageMode);
                break;
            case Setting::Ppp:
                sp = new PppPersistence(static_cast<PppSetting*>(setting), config, storageMode);
                break;
            case Setting::Pppoe:
                sp = new PppoePersistence(static_cast<PppoeSetting*>(setting), config, storageMode);
                break;
            case Setting::Security8021x:
                sp = new Security8021xPersistence(static_cast<Security8021xSetting*>(setting), config, storageMode);
                break;
            case Setting::Serial:
                sp = new SerialPersistence(static_cast<SerialSetting*>(setting), config, storageMode);
                break;
            case Setting::Vpn:
                sp = new VpnPersistence(static_cast<VpnSetting*>(setting), config, storageMode);
                break;
            case Setting::Wired:
                sp = new WiredPersistence(static_cast<WiredSetting*>(setting), config, storageMode);
                break;
            case Setting::Wireless:
                sp = new WirelessPersistence(static_cast<WirelessSetting*>(setting), config, storageMode);
                break;
            case Setting::WirelessSecurity:
                sp = new WirelessSecurityPersistence(
                        static_cast<WirelessSecuritySetting*>(setting), config, storageMode
                        );
                break;
        }
    if (sp) {
        m_persistences.insert(setting, sp);
    }
    return sp;
}

// vim: sw=4 sts=4 et tw=100
