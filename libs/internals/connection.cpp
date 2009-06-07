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

#include "connection.h"

#include "settings/802-11-wireless-security.h"
#include "settings/802-11-wireless.h"
#include "settings/802-1x.h"
#include "settings/802-3-ethernet.h"
#include "settings/cdma.h"
#include "settings/gsm.h"
#include "settings/ipv4.h"
#include "settings/ppp.h"
#include "settings/pppoe.h"
#include "settings/serial.h"
#include "settings/vpn.h"

using namespace KnmInternals;

QString Connection::typeAsString(Knm::Externals::Connection::Type type)
{
    QString typeString;
    switch (type) {
        case Knm::Externals::Connection::Wired:
            typeString = QLatin1String("802-3-ethernet");
            break;
        case Knm::Externals::Connection::Wireless:
            typeString = QLatin1String("802-11-wireless");
            break;
        case Knm::Externals::Connection::Gsm:
            typeString = QLatin1String("gsm");
            break;
        case Knm::Externals::Connection::Cdma:
            typeString = QLatin1String("cdma");
            break;
        case Knm::Externals::Connection::Vpn:
            typeString = QLatin1String("vpn");
            break;
        case Knm::Externals::Connection::Pppoe:
            typeString = QLatin1String("pppoe");
            break;
        default:
            break;
    }
    return typeString;
}

Knm::Externals::Connection::Type Connection::typeFromString(const QString & typeString)
{
    Knm::Externals::Connection::Type type = Knm::Externals::Connection::Wired;
    if (typeString == QLatin1String("802-3-ethernet")) {
        type = Knm::Externals::Connection::Wired;
    } else if (typeString == QLatin1String("802-11-wireless")) {
        type = Knm::Externals::Connection::Wireless;
    } else if (typeString == QLatin1String("gsm")) {
        type = Knm::Externals::Connection::Gsm;
    } else if (typeString == QLatin1String("cdma")) {
        type = Knm::Externals::Connection::Cdma;
    } else if (typeString == QLatin1String("vpn")) {
        type = Knm::Externals::Connection::Vpn;
    } else if (typeString == QLatin1String("pppoe")) {
        type = Knm::Externals::Connection::Pppoe;
    }
    return type;
}

Knm::Externals::Connection::Type typeFromString(const QString & type);

Connection::Connection(const QString & name, const Knm::Externals::Connection::Type type)
    : m_name(name), m_uuid(QUuid::createUuid()), m_type(type), m_autoConnect(false)
{
    init();
}

Connection::Connection(const QUuid & uuid, const Knm::Externals::Connection::Type type)
    : m_uuid(uuid), m_type(type), m_autoConnect(false)
{
    init();
}

Connection::~Connection()
{
    qDeleteAll(m_settings);
}

void Connection::init()
{
    switch (m_type) {
        case Knm::Externals::Connection::Cdma:
            addSetting(new CdmaSetting());
            addSetting(new Ipv4Setting());
            addSetting(new PppSetting());
            addSetting(new SerialSetting());
            break;
        case Knm::Externals::Connection::Gsm:
            addSetting(new GsmSetting());
            addSetting(new Ipv4Setting());
            addSetting(new PppSetting());
            addSetting(new SerialSetting());
            break;
        case Knm::Externals::Connection::Pppoe:
            addSetting(new Ipv4Setting());
            addSetting(new PppSetting());
            addSetting(new PppoeSetting());
            addSetting(new WiredSetting());
            break;
        case Knm::Externals::Connection::Vpn:
            addSetting(new VpnSetting());
            break;
        case Knm::Externals::Connection::Wired:
            addSetting(new Ipv4Setting());
            addSetting(new Security8021xSetting());
            addSetting(new WiredSetting());
            break;
        case Knm::Externals::Connection::Wireless:
            addSetting(new Ipv4Setting());
            addSetting(new Security8021xSetting());
            addSetting(new WirelessSetting());
            addSetting(new WirelessSecuritySetting());
            break;
    }
}

QString Connection::name() const
{
    return m_name;
}

QUuid Connection::uuid() const
{
    return m_uuid;
}

Knm::Externals::Connection::Type Connection::type() const
{
    return m_type;
}

bool Connection::autoConnect() const
{
    return m_autoConnect;
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
    return connectionHasSecrets;
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

// vim: sw=4 sts=4 et tw=100
