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

#include "connection.h"
#include "setting.h"

#include "settings/802-11-wireless.h"
#include "settings/802-11-wirelessdbus.h"
#include "settings/802-11-wireless-security.h"
#include "settings/802-11-wireless-securitydbus.h"
#include "settings/802-1x.h"
#include "settings/802-1xdbus.h"
#include "settings/802-3-ethernet.h"
#include "settings/802-3-ethernetdbus.h"
#include "settings/cdma.h"
#include "settings/cdmadbus.h"
#include "settings/gsm.h"
#include "settings/gsmdbus.h"
#include "settings/ipv4.h"
#include "settings/ipv4dbus.h"
#include "settings/ppp.h"
#include "settings/pppdbus.h"
#include "settings/pppoe.h"
#include "settings/pppoedbus.h"
#include "settings/serial.h"
#include "settings/serialdbus.h"
#include "settings/vpn.h"
#include "settings/vpndbus.h"

using namespace Knm;

ConnectionDbus::ConnectionDbus(Connection * conn)
    : m_connection(conn)
{
    qDBusRegisterMetaType<QList<uint> >();
    qDBusRegisterMetaType<QList<QList<uint> > >();
}

ConnectionDbus::~ConnectionDbus()
{
    qDeleteAll(m_dbus.values());
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
    connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_ID), m_connection->name());
    connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_UUID), m_connection->uuid().toString());
    QString dbusConnectionType;
    switch (m_connection->type()) {
        case Connection::Wired:
            dbusConnectionType = QLatin1String(NM_SETTING_WIRED_SETTING_NAME);
            break;
        case Connection::Wireless:
            dbusConnectionType = QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME);
            break;
        case Connection::Gsm:
            dbusConnectionType = QLatin1String(NM_SETTING_GSM_SETTING_NAME);
            break;
        case Connection::Cdma:
            dbusConnectionType = QLatin1String(NM_SETTING_CDMA_SETTING_NAME);
            break;
        case Connection::Vpn:
            dbusConnectionType = QLatin1String(NM_SETTING_VPN_SETTING_NAME);
            break;
        case Connection::Pppoe:
            dbusConnectionType = QLatin1String(NM_SETTING_PPPOE_SETTING_NAME);
            break;
    }
    connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_TYPE), dbusConnectionType);
    connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_AUTOCONNECT), m_connection->autoConnect());
    // as unix time
    if (m_connection->timestamp().isValid()) {
        connectionMap.insert(QLatin1String(NM_SETTING_CONNECTION_TIMESTAMP), m_connection->timestamp().toTime_t());
    }
    mapMap.insert(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME), connectionMap);

    // all other settings
    foreach (Setting * setting, m_connection->settings()) {
        SettingDbus * sd = dbusFor(setting);
        if (sd) {
            QVariantMap map = sd->toMap();
            if (!map.isEmpty()) {  // GENERAL RULE, type specific special cases will follow.
                mapMap.insert(setting->name(), map);
            }
        }
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

void ConnectionDbus::fromDbusMap(const QVariantMapMap &)
{
    kDebug() << "Unimplemented!";
    // connection settings
    // all other settings
}
// vim: sw=4 sts=4 et tw=100
