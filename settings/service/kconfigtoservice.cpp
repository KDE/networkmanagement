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

#include <nm-setting-wired.h>
#include <nm-setting-ip4-config.h>
#include <nm-setting-8021x.h>
#include <nm-setting-gsm.h>
#include <nm-setting-ppp.h>
#include <nm-setting-serial.h>
#include <nm-setting-vpn.h>
#include <nm-setting-wireless.h>
#include <nm-setting-wireless-security.h>

#include <KDebug>
#include <KSharedConfig>
#include <KStandardDirs>

#include "configxml.h"
#include "networksettings.h"
#include "knmserviceprefs.h"
#include "secretstoragehelper.h"
#include "knetworkmanagerserviceadaptor.h"

KConfigToService::KConfigToService(NetworkSettings * service)
    : m_service(service)
{
    (void) new KNetworkManagerServiceAdaptor( this );
    QDBusConnection::sessionBus().registerService( "org.kde.knetworkmanagerd" ) ;
    QDBusConnection::sessionBus().registerObject( "/Configuration", this );

    initKeyMappings();
    KNetworkManagerServicePrefs::instance(KStandardDirs::locate("config",
                QLatin1String("knetworkmanagerrc")));
}

KConfigToService::~KConfigToService()
{
}

void KConfigToService::init()
{
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
        } else {
            connectionMap.insert(group, groupSettings );
        }
    }
    kDebug() << connectionMap;
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
    ConfigXml * config = new ConfigXml(m_configFile, &schemaFile,
            new SecretStorageHelper(/*connection id*/QLatin1String("testconfigxml"), groupName));

    foreach (KConfigSkeletonItem * item, config->items()) {
        item->swapDefault();
        QVariant defaultV = item->property();
        item->swapDefault();
        if (defaultV != item->property()) { // only deserialise non-default values
            kDebug() << item->key() << " : '" << item->property() << "' is a " << item->property().type() << ", and " << (defaultV == item->property() ? "IS" : "IS NOT") << " default";
            map.insert(convertKey(item->key()), convertValue(item->key(), item->property()));
        }
    }
    // special case for ipv4 "addresses" field, which isn't KConfigSkeletonItem-friendly
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
        m_service->removeConnection(objectPath);
    }
    foreach (QString connectionId, changedConnections) {
        if (m_connectionIdToObjectPath.contains(connectionId)) {
            QVariantMapMap changedConnection = restoreConnection(connectionId);
            if (!changedConnection.isEmpty()) {
                m_service->updateConnection(m_connectionIdToObjectPath[connectionId], changedConnection);
            }
        }
    }
    foreach (QString connectionId, addedConnections) {
        QVariantMapMap changedConnection = restoreConnection(connectionId);
        m_service->addConnection(restoreConnection(connectionId));
    }
}

QString KConfigToService::convertKey(const QString & storedKey) const
{
    QString nmKey = storedKey;
    QMap<QString, QString>::const_iterator it = m_keyMappings.find(storedKey);
    if (it != m_keyMappings.end()) {
        nmKey = it.value();
        kDebug() << "converting " << storedKey << " to " << nmKey;
    }
    return nmKey;
}

QVariant KConfigToService::convertValue(const QString& key, const QVariant& value) const
{
    Q_UNUSED(key);
    return value;
}

void KConfigToService::initKeyMappings()
{
    // TODO: split this into separate per-group maps, in case 2 groups have the same key resolving
    // to different values (macaddress)
    // nm-setting-wired.h
    m_keyMappings.insert(QLatin1String("macaddress"), QLatin1String(NM_SETTING_WIRED_MAC_ADDRESS));
    m_keyMappings.insert(QLatin1String("autonegotiate"), QLatin1String(NM_SETTING_WIRED_AUTO_NEGOTIATE));
    // nm-setting-ip4-config.h
    m_keyMappings.insert(QLatin1String("dnssearch"), QLatin1String(NM_SETTING_IP4_CONFIG_DNS_SEARCH));
    m_keyMappings.insert(QLatin1String("ignoredhcpdns"), QLatin1String(NM_SETTING_IP4_CONFIG_IGNORE_DHCP_DNS));
    // nm-setting-8021x.h
    m_keyMappings.insert(QLatin1String("anonymousidentity"), QLatin1String(NM_SETTING_802_1X_ANONYMOUS_IDENTITY));
    m_keyMappings.insert(QLatin1String("cacert"), QLatin1String(NM_SETTING_802_1X_CA_CERT));
    m_keyMappings.insert(QLatin1String("capath"), QLatin1String(NM_SETTING_802_1X_CA_PATH));
    m_keyMappings.insert(QLatin1String("clientcert"), QLatin1String(NM_SETTING_802_1X_CLIENT_CERT));
    m_keyMappings.insert(QLatin1String("phase1peapver"), QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER));
    m_keyMappings.insert(QLatin1String("phase1peaplabel"), QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL));
    m_keyMappings.insert(QLatin1String("phase1fastprovisioning"), QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING));
    m_keyMappings.insert(QLatin1String("phase2auth"), QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH));
    m_keyMappings.insert(QLatin1String("phase2autheap"), QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP));
    m_keyMappings.insert(QLatin1String("phase2cacert"), QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT));
    m_keyMappings.insert(QLatin1String("phase2capath"), QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH));
    m_keyMappings.insert(QLatin1String("phase2clientcert"), QLatin1String(NM_SETTING_802_1X_PHASE2_CLIENT_CERT));
    m_keyMappings.insert(QLatin1String("privatekey"), QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY));
    m_keyMappings.insert(QLatin1String("phase2privatekey"), QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY));
    // nm-setting-cdma.h - has legal keys
    // nm-setting-gsm.h
    m_keyMappings.insert(QLatin1String("networkid"), QLatin1String(NM_SETTING_GSM_NETWORK_ID));
    m_keyMappings.insert(QLatin1String("networktype"), QLatin1String(NM_SETTING_GSM_NETWORK_TYPE));
    // nm-setting-ppp.h
    m_keyMappings.insert(QLatin1String("refuse-eap"), QLatin1String(NM_SETTING_PPP_REFUSE_EAP));
    m_keyMappings.insert(QLatin1String("refuse-pap"), QLatin1String(NM_SETTING_PPP_REFUSE_PAP));
    m_keyMappings.insert(QLatin1String("refuse-chap"), QLatin1String(NM_SETTING_PPP_REFUSE_CHAP));
    m_keyMappings.insert(QLatin1String("refuse-mschap"), QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAP));
    m_keyMappings.insert(QLatin1String("refuse-mschapv2"), QLatin1String(NM_SETTING_PPP_REFUSE_MSCHAPV2));
    m_keyMappings.insert(QLatin1String("novjcomp"), QLatin1String(NM_SETTING_PPP_NO_VJ_COMP));
    m_keyMappings.insert(QLatin1String("requiremppe"), QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE));
    m_keyMappings.insert(QLatin1String("requiremppe128"), QLatin1String(NM_SETTING_PPP_REQUIRE_MPPE_128));
    m_keyMappings.insert(QLatin1String("mpppestateful"), QLatin1String(NM_SETTING_PPP_MPPE_STATEFUL));
    m_keyMappings.insert(QLatin1String("lcpechofailure"), QLatin1String(NM_SETTING_PPP_LCP_ECHO_FAILURE));
    m_keyMappings.insert(QLatin1String("lcpechointerval"), QLatin1String(NM_SETTING_PPP_LCP_ECHO_INTERVAL));
    // nm-setting-pppoe.h is good
    // nm-setting-serial.h
    m_keyMappings.insert(QLatin1String("senddelay"), QLatin1String(NM_SETTING_SERIAL_SEND_DELAY));
    // nm-setting-vpn.h
    m_keyMappings.insert(QLatin1String("servicetype"), QLatin1String(NM_SETTING_VPN_SERVICE_TYPE));
    m_keyMappings.insert(QLatin1String("username"), QLatin1String(NM_SETTING_VPN_USER_NAME));
    // nm-setting-vpn-properties.h is good
    // nm-setting-wireless.h
    m_keyMappings.insert(QLatin1String("macaddress"), QLatin1String(NM_SETTING_WIRELESS_MAC_ADDRESS));
    m_keyMappings.insert(QLatin1String("txpower"), QLatin1String(NM_SETTING_WIRELESS_TX_POWER));
    m_keyMappings.insert(QLatin1String("seenbssids"), QLatin1String(NM_SETTING_WIRELESS_SEEN_BSSIDS));
    // nm-setting-wireless-security.h
    m_keyMappings.insert(QLatin1String("keymgmt"), QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT));
    m_keyMappings.insert(QLatin1String("weptxkeyidx"), QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX));
    m_keyMappings.insert(QLatin1String("authalg"), QLatin1String(NM_SETTING_WIRELESS_SECURITY_AUTH_ALG));
    m_keyMappings.insert(QLatin1String("leapusername"), QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME));
    m_keyMappings.insert(QLatin1String("wepkey0"), QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY0));
    m_keyMappings.insert(QLatin1String("wepkey1"), QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY1));
    m_keyMappings.insert(QLatin1String("wepkey2"), QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY2));
    m_keyMappings.insert(QLatin1String("wepkey3"), QLatin1String(NM_SETTING_WIRELESS_SECURITY_WEP_KEY3));
    m_keyMappings.insert(QLatin1String("leappassword"), QLatin1String(NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD));
}

// vim: sw=4 sts=4 et tw=100
