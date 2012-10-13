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


#include <kdebug.h>
#include <solid/control/networkmodeminterface.h>

#include "connection.h"

#include "settings/802-11-wireless-security.h"
#include "settings/802-11-wireless.h"
#include "settings/802-1x.h"
#include "settings/802-3-ethernet.h"
#include "settings/cdma.h"
#include "settings/gsm.h"
#include "settings/bluetooth.h"
#include "settings/ipv4.h"
#include "settings/ipv6.h"
#include "settings/ppp.h"
#include "settings/pppoe.h"
#include "settings/serial.h"
#include "settings/vpn.h"

using namespace Knm;

QString Connection::typeAsString(Connection::Type type)
{
    QString typeString;
    switch (type) {
        case Wired:
            typeString = QLatin1String("802-3-ethernet");
            break;
        case Wireless:
            typeString = QLatin1String("802-11-wireless");
            break;
        case Gsm:
            typeString = QLatin1String("gsm");
            break;
        case Cdma:
            typeString = QLatin1String("cdma");
            break;
        case Bluetooth:
            typeString = QLatin1String("bluetooth");
            break;
        case Vpn:
            typeString = QLatin1String("vpn");
            break;
        case Pppoe:
            typeString = QLatin1String("pppoe");
            break;
        default:
            break;
    }
    return typeString;
}

Connection::Type Connection::typeFromString(const QString & typeString)
{
    Connection::Type type = Wired;
    if (typeString == QLatin1String("802-3-ethernet")) {
        type = Wired;
    } else if (typeString == QLatin1String("802-11-wireless")) {
        type = Wireless;
    } else if (typeString == QLatin1String("gsm")) {
        type = Gsm;
    } else if (typeString == QLatin1String("cdma")) {
        type = Cdma;
    } else if (typeString == QLatin1String("bluetooth")) {
        type = Bluetooth;
    } else if (typeString == QLatin1String("vpn")) {
        type = Vpn;
    } else if (typeString == QLatin1String("pppoe")) {
        type = Pppoe;
    }
    return type;
}

Connection::Type Connection::typeFromSolidType(const Solid::Control::NetworkInterfaceNm09 *iface)
{
    switch (iface->type()) {
        case Solid::Control::NetworkInterfaceNm09::Ethernet: return Knm::Connection::Wired;
        case Solid::Control::NetworkInterfaceNm09::Wifi: return Knm::Connection::Wireless;
        case Solid::Control::NetworkInterfaceNm09::Bluetooth: return Knm::Connection::Bluetooth;
        case Solid::Control::NetworkInterfaceNm09::Modem: {
             const Solid::Control::ModemNetworkInterfaceNm09 * nmModemIface = qobject_cast<const Solid::Control::ModemNetworkInterfaceNm09 *>(iface);
             if (nmModemIface) {
                 switch(nmModemIface->subType()) {
                     case Solid::Control::ModemNetworkInterfaceNm09::GsmUmts: return Knm::Connection::Gsm;
                     case Solid::Control::ModemNetworkInterfaceNm09::CdmaEvdo: return Knm::Connection::Cdma;
                     case Solid::Control::ModemNetworkInterfaceNm09::Pots: return Knm::Connection::Pppoe;
                     /* TODO: add Solid::Control::ModemNetworkInterfaceNm09::Lte */
                     case Solid::Control::ModemNetworkInterfaceNm09::Lte: kWarning() << "Unhandled modem sub type: LTE";
                          break;
                     case Solid::Control::ModemNetworkInterfaceNm09::None:
                          kWarning() << "Unhandled modem sub type: Solid::Control::ModemNetworkInterfaceNm09::None";
                          break;
                 }
             }
        }
        case Solid::Control::NetworkInterfaceNm09::UnknownType:
        case Solid::Control::NetworkInterfaceNm09::Unused1:
        case Solid::Control::NetworkInterfaceNm09::Unused2:
            return Knm::Connection::Unknown;
        default:
            kWarning() << "Unhandled type:" << iface->type();
            break;
    }
    return Knm::Connection::Wired;
}

Connection::Connection(const QString & name, const Connection::Type type, NMBluetoothCapabilities bt_cap)
    : m_name(name), m_uuid(QUuid::createUuid()), m_type(type), m_autoConnect(false), m_zone("")
{
    addToPermissions(KUser().loginName(),QString());
    init(bt_cap);
}

Connection::Connection(const QUuid & uuid, const Connection::Type type, NMBluetoothCapabilities bt_cap)
    : m_uuid(uuid), m_type(type), m_autoConnect(false), m_zone("")
{
    addToPermissions(KUser().loginName(),QString());
    init(bt_cap);
}

Connection::Connection(Connection *con)
{
    setUuid(con->uuid());
    setType(con->type());
    setAutoConnect(con->autoConnect());
    setZone(con->zone());
    setPermissions(con->permissions());
    setTimestamp(con->timestamp());
    setName(con->name());
    setOrigin(con->origin());
    setIconName(con->iconName());
    init(con);
}

Connection::~Connection()
{
    clearSettings();
}

void Connection::clearSettings()
{
    // child removes itself from list in its destructor, which causes crashes if we use qDeleteAll() of Qt >= 4.8,
    // so use this loop instead. See https://bugs.kde.org/show_bug.cgi?id=284989
    while (!m_settings.isEmpty()) {
        delete m_settings.takeFirst();
    }
    m_settings.clear();
}

void Connection::init(NMBluetoothCapabilities bt_cap)
{
    clearSettings();

    switch (m_type) {
        case Cdma:
            addSetting(new CdmaSetting());
            addSetting(new Ipv4Setting());
            addSetting(new Ipv6Setting());
            addSetting(new PppSetting());
            addSetting(new SerialSetting());
            break;
        case Gsm:
            addSetting(new GsmSetting());
            addSetting(new Ipv4Setting());
            addSetting(new Ipv6Setting());
            addSetting(new PppSetting());
            addSetting(new SerialSetting());
            break;
        case Bluetooth:
            addSetting(new BluetoothSetting());
            addSetting(new Ipv4Setting());
            // Gsm, Ppp and Serial settings are not needed for PAN
            if (bt_cap == NM_BT_CAPABILITY_DUN) {
                addSetting(new GsmSetting());
                addSetting(new PppSetting());
                addSetting(new SerialSetting());
            }
            break;
        case Pppoe:
            addSetting(new Ipv4Setting());
            addSetting(new Ipv6Setting());
            addSetting(new PppSetting());
            addSetting(new PppoeSetting());
            addSetting(new WiredSetting());
            break;
        case Vpn:
            addSetting(new VpnSetting());
            addSetting(new Ipv4Setting());
            addSetting(new Ipv6Setting());
            break;
        case Wired:
            addSetting(new Ipv4Setting());
            addSetting(new Ipv6Setting());
            addSetting(new Security8021xSetting());
            addSetting(new WiredSetting());
            break;
        case Wireless:
            addSetting(new Ipv4Setting());
            addSetting(new Ipv6Setting());
            addSetting(new Security8021xSetting());
            addSetting(new WirelessSetting());
            addSetting(new WirelessSecuritySetting());
            setAutoConnect(true);
            break;
        default:
            break;
    }
}

void Connection::init(Connection *con)
{
    clearSettings();

    switch (m_type) {
        case Cdma:
            addSetting(new CdmaSetting(static_cast<CdmaSetting*>(con->setting(Setting::Cdma))));
            addSetting(new Ipv4Setting(static_cast<Ipv4Setting*>(con->setting(Setting::Ipv4))));
            addSetting(new Ipv6Setting(static_cast<Ipv6Setting*>(con->setting(Setting::Ipv6))));
            addSetting(new PppSetting(static_cast<PppSetting*>(con->setting(Setting::Ppp))));
            addSetting(new SerialSetting(static_cast<SerialSetting*>(con->setting(Setting::Serial))));
            break;
        case Gsm:
            addSetting(new GsmSetting(static_cast<GsmSetting*>(con->setting(Setting::Gsm))));
            addSetting(new Ipv4Setting(static_cast<Ipv4Setting*>(con->setting(Setting::Ipv4))));
            addSetting(new Ipv6Setting(static_cast<Ipv6Setting*>(con->setting(Setting::Ipv6))));
            addSetting(new PppSetting(static_cast<PppSetting*>(con->setting(Setting::Ppp))));
            addSetting(new SerialSetting(static_cast<SerialSetting*>(con->setting(Setting::Serial))));
            break;
        case Bluetooth:
            addSetting(new BluetoothSetting(static_cast<BluetoothSetting*>(con->setting(Setting::Bluetooth))));
            addSetting(new Ipv4Setting(static_cast<Ipv4Setting*>(con->setting(Setting::Ipv4))));
            // Gsm, Ppp and Serial settings are not present for PAN, only for DUN
            if (con->setting(Setting::Gsm) && con->setting(Setting::Ppp) && con->setting(Setting::Serial)) {
                addSetting(new GsmSetting(static_cast<GsmSetting*>(con->setting(Setting::Gsm))));
                addSetting(new PppSetting(static_cast<PppSetting*>(con->setting(Setting::Ppp))));
                addSetting(new SerialSetting(static_cast<SerialSetting*>(con->setting(Setting::Serial))));
            }
            break;
        case Pppoe:
            addSetting(new Ipv4Setting(static_cast<Ipv4Setting*>(con->setting(Setting::Ipv4))));
            addSetting(new Ipv6Setting(static_cast<Ipv6Setting*>(con->setting(Setting::Ipv6))));
            addSetting(new PppSetting(static_cast<PppSetting*>(con->setting(Setting::Ppp))));
            addSetting(new PppoeSetting(static_cast<PppoeSetting*>(con->setting(Setting::Pppoe))));
            addSetting(new WiredSetting(static_cast<WiredSetting*>(con->setting(Setting::Wired))));
            break;
        case Vpn:
            addSetting(new VpnSetting(static_cast<VpnSetting*>(con->setting(Setting::Vpn))));
            addSetting(new Ipv4Setting(static_cast<Ipv4Setting*>(con->setting(Setting::Ipv4))));
            addSetting(new Ipv6Setting(static_cast<Ipv6Setting*>(con->setting(Setting::Ipv6))));
            break;
        case Wired:
            addSetting(new Ipv4Setting(static_cast<Ipv4Setting*>(con->setting(Setting::Ipv4))));
            addSetting(new Ipv6Setting(static_cast<Ipv6Setting*>(con->setting(Setting::Ipv6))));
            addSetting(new Security8021xSetting(static_cast<Security8021xSetting*>(con->setting(Setting::Security8021x))));
            addSetting(new WiredSetting(static_cast<WiredSetting*>(con->setting(Setting::Wired))));
            break;
        case Wireless:
            addSetting(new Ipv4Setting(static_cast<Ipv4Setting*>(con->setting(Setting::Ipv4))));
            addSetting(new Ipv6Setting(static_cast<Ipv6Setting*>(con->setting(Setting::Ipv6))));
            addSetting(new Security8021xSetting(static_cast<Security8021xSetting*>(con->setting(Setting::Security8021x))));
            addSetting(new WirelessSetting(static_cast<WirelessSetting*>(con->setting(Setting::Wireless))));
            addSetting(new WirelessSecuritySetting(static_cast<WirelessSecuritySetting*>(con->setting(Setting::WirelessSecurity))));
            break;
        default:
            break;
    }
}

QString Connection::name() const
{
    return m_name;
}

QString Connection::iconName(const Connection::Type type)
{
    QString iconName;
    switch (type) {
        case Connection::Wired:
            iconName = QLatin1String("network-wired");
            break;
        case Connection::Wireless:
            iconName = QLatin1String("network-wireless");
            break;
        case Connection::Pppoe:
            iconName = QLatin1String("modem");
            break;
        case Connection::Gsm:
        case Connection::Cdma:
        case Connection::Bluetooth:
            iconName = QLatin1String("phone");
            break;
        case Connection::Vpn:
            iconName = QLatin1String("secure-card");

        default:
            break;
    }
    return iconName;
}

QString Connection::iconName() const
{
    if (m_iconName.isEmpty()) {
        return iconName(m_type);
    }
    return m_iconName;
}

QUuid Connection::uuid() const
{
    return m_uuid;
}

Connection::Type Connection::type() const
{
    return m_type;
}

bool Connection::autoConnect() const
{
    return m_autoConnect;
}

QString Connection::zone() const
{
    return m_zone;
}

bool Connection::isShared() const
{
    Ipv4Setting * ipv4 = static_cast<Ipv4Setting *>(setting(Setting::Ipv4));
    return (ipv4 && ipv4->method() == Ipv4Setting::EnumMethod::Shared);
}

QDateTime Connection::timestamp() const
{
    return m_timestamp;
}

QList<Setting *> Connection::settings() const
{
    return m_settings;
}

Setting * Connection::setting(Setting::Type settingType) const
{
    Setting * foundSetting = 0;
    foreach (Setting * setting, m_settings) {
        if (!setting) {
            kWarning() << "Setting is null. That should not happen.";
            continue;
        }
        if (setting->type() == settingType) {
            foundSetting = setting;
            break;
        }
    }
    return foundSetting;
}

void Connection::setName(const QString & name)
{
    m_name = name;
}

void Connection::setIconName(const QString & iconName)
{
    m_iconName = iconName;
}

void Connection::setUuid(const QUuid & uuid)
{
    m_uuid = uuid;
}

void Connection::setTimestamp(const QDateTime & timestamp)
{
    m_timestamp = timestamp;
}

void Connection::setAutoConnect(bool autoConnect)
{
    m_autoConnect = autoConnect;
}

void Connection::setZone(const QString & zone)
{
    m_zone = zone;
}

void Connection::updateTimestamp()
{
    m_timestamp = QDateTime::currentDateTime();
}

void Connection::addSetting(Setting * newSetting)
{
    m_settings.append(newSetting);
}

bool Connection::hasPersistentSecrets() const
{
    bool connectionHasSecrets = false;
    foreach (Setting * setting, m_settings) {
        if (setting->hasPersistentSecrets()) {
            connectionHasSecrets = true;
            break;
        }
    }
    return connectionHasSecrets;
}

QStringList Connection::hasPersistentSecretsSettings() const
{
    QStringList settings;
    foreach (Setting * setting, m_settings) {
        if (setting->hasPersistentSecrets()) {
            settings << setting->name();
        }
    }

    return settings;
}

void Connection::setOrigin(const QString & origin)
{
    m_origin = origin;
}

QString Connection::origin() const
{
    return m_origin;
}

void Connection::addToPermissions(const QString &user, const QString &tags)
{
    if (!m_permissions.contains(user))
        m_permissions.insert(user,tags);
}

void Connection::removeFromPermissions(const QString &user)
{
    m_permissions.remove(user);
}

void Connection::setPermissions(const QHash<QString,QString> &permissions)
{
    m_permissions = permissions;
}

QHash<QString,QString> Connection::permissions() const
{
    return m_permissions;
}

void Connection::setType(Connection::Type type, NMBluetoothCapabilities bt_cap)
{
    if (type == m_type)
        return;

    m_type = type;

    init(bt_cap);     // Ensure we add only relevant settings back

    kDebug() << "Connection type is set as " << typeAsString(type) << ". Settings of the connection removed since its type has been changed.";
}

bool Connection::hasVolatileSecrets() const
{
    foreach (Setting * setting, m_settings) {
        if (setting->hasVolatileSecrets()) {
            return true;
        }
    }
    return false;
}

// vim: sw=4 sts=4 et tw=100
