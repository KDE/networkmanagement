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

Connection::Type Connection::typeFromSolidType(const Solid::Control::NetworkInterface::Type type)
{
    switch (type) {
        case Solid::Control::NetworkInterface::Ieee8023: return Knm::Connection::Wired;
        case Solid::Control::NetworkInterface::Ieee80211: return Knm::Connection::Wireless;
        case Solid::Control::NetworkInterface::Gsm: return Knm::Connection::Gsm;
        case Solid::Control::NetworkInterface::Cdma: return Knm::Connection::Cdma;
#ifdef NM_0_8
        case Solid::Control::NetworkInterface::Bluetooth: return Knm::Connection::Bluetooth;
#endif
        case Solid::Control::NetworkInterface::Serial: return Knm::Connection::Pppoe;
        case Solid::Control::NetworkInterface::UnknownType: return Knm::Connection::Unknown;
    }
    return Knm::Connection::Wired;
}

QString Connection::scopeAsString(Connection::Scope scope)
{
    QString scopeString;
    switch (scope) {
        case User:
            scopeString = QLatin1String("User");
            break;
        case System:
            scopeString = QLatin1String("System");
            break;
        default:
            break;
    }
    return scopeString;
}

Connection::Scope Connection::scopeFromString(const QString & scopeString)
{
    if (scopeString == QLatin1String("User")) {
        return Connection::User;
    }
    return Connection::System;
}

Connection::Connection(const QString & name, const Connection::Type type, const Connection::Scope scope)
    : m_name(name), m_uuid(QUuid::createUuid()), m_type(type), m_scope(scope), m_autoConnect(true)
{
    init();
}

Connection::Connection(const QUuid & uuid, const Connection::Type type)
    : m_uuid(uuid), m_type(type), m_autoConnect(true)
{
    init();
}

Connection::~Connection()
{
    qDeleteAll(m_settings);
}

void Connection::init()
{
    m_settings.clear();

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
            addSetting(new GsmSetting());
            addSetting(new Ipv4Setting());
            addSetting(new PppSetting());
            addSetting(new SerialSetting());
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
            break;
        default:
            break;
    }
}

void Connection::saveCertificates()
{
    foreach (Setting * setting, m_settings) {
        setting->save((int)m_scope);
    }
}

void Connection::removeCertificates()
{
    foreach (Setting * setting, m_settings) {
        setting->remove();
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
            iconName = QLatin1String("network-server");

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

Connection::Scope Connection::scope() const
{
    return m_scope;
}

bool Connection::autoConnect() const
{
    return m_autoConnect;
}

bool Connection::originalAutoConnect() const
{
    return m_originalAutoConnect;
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

void Connection::setOriginalAutoConnect(bool autoConnect)
{
    m_originalAutoConnect = autoConnect;
}

void Connection::updateTimestamp()
{
    m_timestamp = QDateTime::currentDateTime();
}

void Connection::addSetting(Setting * newSetting)
{
    m_settings.append(newSetting);
}

bool Connection::hasSecrets() const
{
    bool connectionHasSecrets = false;
    foreach (Setting * setting, m_settings) {
        if (setting->hasSecrets()) {
            connectionHasSecrets = true;
            break;
        }
    }
    kDebug() << "These settings seems to provide secret info:" << secretSettings();

    return connectionHasSecrets;
}

QStringList Connection::secretSettings() const
{
    QStringList settings;
    foreach (Setting * setting, m_settings) {
        if (setting->hasSecrets()) {
            settings << setting->name();
        }
    }

    return settings;
}

bool Connection::secretsAvailable() const
{
    bool allSecretsAvailable = true;
    foreach (Setting * setting, m_settings) {
        if (setting->hasSecrets() && !setting->secretsAvailable()) {
            allSecretsAvailable = false;
            break;
        }
    }
    return allSecretsAvailable;
}

void Connection::setOrigin(const QString & origin)
{
    m_origin = origin;
}

QString Connection::origin() const
{
    return m_origin;
}

void Connection::setScope(Connection::Scope scope)
{
    m_scope = scope;
}

void Connection::setType(Connection::Type type)
{
    if (type == m_type)
        return;

    m_type = type;

    init();

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
