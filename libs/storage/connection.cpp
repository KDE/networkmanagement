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

    enum Type { Wired, Wireless, Gsm, Cdma, Vpn, Pppoe };

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
    } else if (typeString == QLatin1String("vpn")) {
        type = Vpn;
    } else if (typeString == QLatin1String("pppoe")) {
        type = Pppoe;
    }
    return type;
}

Connection::Type typeFromString(const QString & type);

Connection::Connection(const QString & name, const Connection::Type type)
    : m_name(name), m_uuid(QUuid::createUuid()), m_type(type), m_autoConnect(false)
{

}

Connection::Connection(const QUuid & uuid, const Connection::Type type)
    : m_uuid(uuid), m_type(type), m_autoConnect(false)
{

}

Connection::~Connection()
{
    qDeleteAll(m_settings);
}

QString Connection::name() const
{
    return m_name;
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

bool Connection::addSetting(Setting * newSetting)
{
    bool acceptable = false;
    if (m_type == Connection::Wired) {
        if (newSetting->type() == Setting::Ipv4 ||
                newSetting->type() == Setting::Security8021x ||
                newSetting->type() == Setting::Wired) {
            acceptable = true;
        }
    } else if (m_type == Connection::Wireless) {
        if (newSetting->type() == Setting::Ipv4 ||
                newSetting->type() == Setting::Wireless ||
                newSetting->type() == Setting::WirelessSecurity) {
            acceptable = true;
        }
    } else if (m_type == Connection::Gsm) {
        if (newSetting->type() == Setting::Gsm ||
                newSetting->type() == Setting::Ppp) {
            acceptable = true;
        }
    } else if (m_type == Connection::Cdma) {
        if (newSetting->type() == Setting::Cdma ||
                newSetting->type() == Setting::Ppp) {
            acceptable = true;
        }
    } else if (m_type == Connection::Pppoe) {
        if (newSetting->type() == Setting::Pppoe ||
                newSetting->type() == Setting::Ppp ||
                newSetting->type() == Setting::Wired ||
                newSetting->type() == Setting::Ipv4) {
            acceptable = true;
        }
    } else if (m_type == Connection::Vpn) {
        if (newSetting->type() == Setting::Vpn ||
                newSetting->type() == Setting::Ppp) {
            acceptable = true;
        }
    }

    if (acceptable && !setting(newSetting->type())) {
        m_settings.append(newSetting);
        return true;
    }
    return false;
}

// vim: sw=4 sts=4 et tw=100
