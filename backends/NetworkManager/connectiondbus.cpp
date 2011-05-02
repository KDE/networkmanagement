/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "connectiondbus.h"

#include <nm-setting-connection.h>
#include <nm-setting-wired.h>
#include <nm-setting-wireless.h>
#include <nm-setting-gsm.h>
#include <nm-setting-cdma.h>
#include <nm-setting-pppoe.h>
#include <nm-setting-vpn.h>

#include "internals/connection.h"
#include "internals/setting.h"

#include "settings/802-11-wireless.h"
#include "802-11-wirelessdbus.h"
#include "settings/802-11-wireless-security.h"
#include "802-11-wireless-securitydbus.h"
#include "settings/802-1x.h"
#include "802-1xdbus.h"
#include "settings/802-3-ethernet.h"
#include "802-3-ethernetdbus.h"
#include "settings/cdma.h"
#include "cdmadbus.h"
#include "settings/gsm.h"
#include "gsmdbus.h"
#include "settings/bluetooth.h"
#include "bluetoothdbus.h"
#include "settings/ipv4.h"
#include "ipv4dbus.h"
#include "settings/ppp.h"
#include "pppdbus.h"
#include "settings/pppoe.h"
#include "pppoedbus.h"
#include "settings/serial.h"
#include "serialdbus.h"
#include "settings/vpn.h"
#include "vpndbus.h"

using namespace Knm;

ConnectionDbus::ConnectionDbus(Knm::Connection * conn)
    : m_connection(conn)
{
    qDBusRegisterMetaType<QList<uint> >();
    qDBusRegisterMetaType<QVariantMapMap>();
    qDBusRegisterMetaType<QList<QList<uint> > >();
}

ConnectionDbus::~ConnectionDbus()
{
    qDeleteAll(m_dbus);
}

SettingDbus * ConnectionDbus::dbusFor(Setting * setting)
{
    SettingDbus * sd = m_dbus.value(setting);
    if (!sd) {
        switch (setting->type()) {
            case Setting::Cdma:
                sd = new CdmaDbus(static_cast<CdmaSetting*>(setting));
                break;
            case Setting::Gsm:
                sd = new GsmDbus(static_cast<GsmSetting*>(setting));
                break;
            case Setting::Bluetooth:
                sd = new BluetoothDbus(static_cast<BluetoothSetting*>(setting));
                break;
            case Setting::Ipv4:
                sd = new Ipv4Dbus(static_cast<Ipv4Setting*>(setting));
                break;
            case Setting::Ppp:
                sd = new PppDbus(static_cast<PppSetting*>(setting));
                break;
            case Setting::Pppoe:
                sd = new PppoeDbus(static_cast<PppoeSetting*>(setting));
                break;
            case Setting::Security8021x:
                sd = new Security8021xDbus(static_cast<Security8021xSetting*>(setting));
                break;
            case Setting::Serial:
                sd = new SerialDbus(static_cast<SerialSetting*>(setting));
                break;
            case Setting::Vpn:
                sd = new VpnDbus(static_cast<VpnSetting*>(setting));
                break;
            case Setting::Wired:
                sd = new WiredDbus(static_cast<WiredSetting*>(setting));
                break;
            case Setting::Wireless:
                sd = new WirelessDbus(static_cast<WirelessSetting*>(setting));
                break;
            case Setting::WirelessSecurity:
                sd = new WirelessSecurityDbus(static_cast<WirelessSecuritySetting*>(setting),
                        static_cast<WirelessSetting*>(m_connection->setting(Setting::Wireless))->ssid());
                break;
            case Setting::Ipv6:
                break;
        }
    }
    if (sd) {
        m_dbus.insert(setting, sd);
    }

    return sd;
}

QVariantMapMap ConnectionDbus::toDbusMap()
{
    QVariantMapMap mapMap;
    // connection settings
    QVariantMap connectionMap;

    // workaround Qt and NetworkManager master disagreeing on what makes a legal uuid
    // Dedicated to Alexander Sack and Kubuntu Karmic Koala, with love from the Geekos
    QString uuidString = m_connection->uuid().toString();
    if (uuidString.startsWith('{')) {
        uuidString = uuidString.mid(1, uuidString.length() - 2);
    }

    connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_ID), m_connection->name());
    connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_UUID), uuidString);
    QString dbusConnectionType;
    switch (m_connection->type()) {
        case Knm::Connection::Wired:
            dbusConnectionType = QLatin1String(NM_SETTING_WIRED_SETTING_NAME);
            break;
        case Knm::Connection::Wireless:
            dbusConnectionType = QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME);
            break;
        case Knm::Connection::Gsm:
            dbusConnectionType = QLatin1String(NM_SETTING_GSM_SETTING_NAME);
            break;
        case Knm::Connection::Bluetooth:
            dbusConnectionType = QLatin1String(NM_SETTING_BLUETOOTH_SETTING_NAME);
            break;
        case Knm::Connection::Cdma:
            dbusConnectionType = QLatin1String(NM_SETTING_CDMA_SETTING_NAME);
            break;
        case Knm::Connection::Vpn:
            dbusConnectionType = QLatin1String(NM_SETTING_VPN_SETTING_NAME);
            break;
        case Knm::Connection::Pppoe:
            dbusConnectionType = QLatin1String(NM_SETTING_PPPOE_SETTING_NAME);
            break;
        default:
            break;
    }
    connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_TYPE), dbusConnectionType);
    connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_AUTOCONNECT), m_connection->autoConnect());
    // as unix time
    if (m_connection->timestamp().isValid()) {
        connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_TIMESTAMP), m_connection->timestamp().toTime_t());
    }

    //kDebug() << "Printing connection map: ";
    //foreach(QString key, connectionMap.keys())
        //kDebug() << key << " : " << connectionMap.value(key);

    mapMap.insert(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME), connectionMap);

    // all other settings
    foreach (Setting * setting, m_connection->settings()) {
        SettingDbus * sd = dbusFor(setting);
        if (sd) {
            QVariantMap map = sd->toMap();
            // IN GENERAL, we don't serialise empty maps.
            // Except if they are the setting for the connection's specific type, NM requires this
            // And NM 0.7 requires that PPPoE connections always have a PPP setting
            // And NM 0.7 requires that serial connections always have a PPP setting
            if (!map.isEmpty()
                    || (setting->name() == dbusConnectionType)
                    || (m_connection->type() == Knm::Connection::Gsm && setting->type() == Knm::Setting::Ppp)
                    || (m_connection->type() == Knm::Connection::Cdma && setting->type() == Knm::Setting::Ppp)
                    || (m_connection->type() == Knm::Connection::Bluetooth && setting->type() == Knm::Setting::Ppp)
                    || (m_connection->type() == Knm::Connection::Pppoe && setting->type() == Knm::Setting::Ppp)) {
                mapMap.insert(setting->name(), map);
                //kDebug() << "  Settings: " << setting->name();
                //foreach(QString key, map.keys())
                    //kDebug() << "    " << key << " : " << map.value(key);
            }
        }
    }
    if (!mapMap.contains(dbusConnectionType)) {
        kWarning() << "The setting group for the specified connection type" << dbusConnectionType << "is missing, expect a bumpy ride!";
    }
    return mapMap;
}

QVariantMapMap ConnectionDbus::toDbusSecretsMap()
{
    QVariantMapMap mapMap;
    // all other settings
    foreach (Setting * setting, m_connection->settings()) {
        SettingDbus * sd = dbusFor(setting);
        if (sd)
            mapMap.insert(setting->name(), sd->toSecretsMap());
    }

    return mapMap;
}

void ConnectionDbus::fromDbusMap(const QVariantMapMap &settings)
{
    // connection settings
    QVariantMap connectionSettings = settings.value(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME));

    kDebug() << "Settings map is " << settings;

    QString connName = connectionSettings.value(QLatin1String(NM_SETTING_CONNECTION_ID)).toString();
    QUuid uuid(connectionSettings.value(QLatin1String(NM_SETTING_CONNECTION_UUID)).toString());
    QString dbusConnectionType = connectionSettings.value(QLatin1String(NM_SETTING_CONNECTION_TYPE)).toString();
    bool autoconnect = true; //default value must be true according to NM settings spec

    if (connectionSettings.contains(QLatin1String(NM_SETTING_CONNECTION_AUTOCONNECT)))
        autoconnect = connectionSettings.value(QLatin1String(NM_SETTING_CONNECTION_AUTOCONNECT)).toBool();

    if (connectionSettings.contains(QLatin1String(NM_SETTING_CONNECTION_TIMESTAMP))) {
        int timestamp = connectionSettings.value(QLatin1String(NM_SETTING_CONNECTION_TIMESTAMP)).toInt();
	QDateTime dateTime;
	dateTime.setTime_t(timestamp);
	m_connection->setTimestamp(dateTime);
    }

    Connection::Type type = Connection::Wired;
    if (dbusConnectionType == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
        type = Connection::Wired;
    } else if (dbusConnectionType == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
        type = Connection::Wireless;
    } else if (dbusConnectionType == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
        type = Connection::Gsm;
    } else if (dbusConnectionType == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
        type = Connection::Cdma;
    } else if (dbusConnectionType == QLatin1String(NM_SETTING_BLUETOOTH_SETTING_NAME)) {
        type = Connection::Bluetooth;
    } else if (dbusConnectionType == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        type = Connection::Vpn;
    } else if (dbusConnectionType == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
        type = Connection::Pppoe;
    }

    m_connection->setName(connName);
    m_connection->setUuid(uuid);
    m_connection->setType(type);
    m_connection->setAutoConnect(autoconnect);

    // all other settings
    foreach (Setting * setting, m_connection->settings()) {
        if (settings.contains(setting->name())) {
            SettingDbus * sd = dbusFor(setting);
            sd->fromMap(settings.value(setting->name()));
        }
    }
}

//TODO: Write real fromDbusSecretsMap method, this one is fake, just uses toDbusMap, unite and fromDbusMap
//correct one must be the exact reverse of toDbusSecretsMap method

void ConnectionDbus::fromDbusSecretsMap(const QVariantMapMap &secrets)
{
    QVariantMapMap origs = toDbusMap();

    kDebug() << "Printing connection map: ";
    kDebug() << "Secrets:" << secrets;
    kDebug() << "Original settings:" << origs;

    foreach(QString secretName, secrets.keys())
    {
        //kDebug() << "Secret setting name " << secretName;
        QVariantMap secret = secrets.value(secretName); // Example secret is QMap(("wep-key0", QVariant(QString, "pardusman")))

        if (secret.count() == 0)
        {
            kDebug() << "Empty secret setting found '" << secretName << "', skipping...";
            continue;
        }

        if (origs.contains(secretName))
        {
            QVariantMap origSetting = origs.take(secretName);

            foreach(QString k, secret.keys())
            {
                if (origSetting.contains(k))
                    origSetting.remove(k); //override the map key given by GetSecrets method of NM

                origSetting.insert(k, secret.value(k));
                kDebug() << "Adding setting " << k << " with value " << secret.value(k) ;
            }

            origs.insert(secretName, origSetting);
        }
        else
        {
            origs.insert(secretName, secret);
            kDebug() << "Inserted setting " << secretName<< " " << secret;
        }
    }

    kDebug() << "New settings: " << origs;

    fromDbusMap(origs);
}

// vim: sw=4 sts=4 et tw=100
