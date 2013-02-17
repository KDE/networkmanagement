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

#include "setting.h"

#include <nm-setting-cdma.h>
#include <nm-setting-connection.h>
#include <nm-setting-wired.h>
#include <nm-setting-ip4-config.h>
#include <nm-setting-ip6-config.h>
#include <nm-setting-8021x.h>
#include <nm-setting-gsm.h>
#include <nm-setting-bluetooth.h>
#include <nm-setting-ppp.h>
#include <nm-setting-pppoe.h>
#include <nm-setting-serial.h>
#include <nm-setting-vpn.h>
#include <nm-setting-wireless.h>
#include <nm-setting-wireless-security.h>

#include <QString>
#include <QStringList>

using namespace Knm;

QString Setting::typeAsString(Setting::Type type)
{
    QString typeString;
    switch (type) {
        case Cdma:
            typeString = QLatin1String(NM_SETTING_CDMA_SETTING_NAME);
            break;
        case Gsm:
            typeString = QLatin1String(NM_SETTING_GSM_SETTING_NAME);
            break;
        case Bluetooth:
            typeString = QLatin1String(NM_SETTING_BLUETOOTH_SETTING_NAME);
            break;
        case Ipv4:
            typeString = QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME);
            break;
        case Ipv6:
            typeString = QLatin1String(NM_SETTING_IP6_CONFIG_SETTING_NAME);
            break;
        case Ppp:
            typeString = QLatin1String(NM_SETTING_PPP_SETTING_NAME);
            break;
        case Pppoe:
            typeString = QLatin1String(NM_SETTING_PPPOE_SETTING_NAME);
            break;
        case Security8021x:
            typeString = QLatin1String(NM_SETTING_802_1X_SETTING_NAME);
            break;
        case Serial:
            typeString = QLatin1String(NM_SETTING_SERIAL_SETTING_NAME);
            break;
        case Vpn:
            typeString = QLatin1String(NM_SETTING_VPN_SETTING_NAME);
            break;
        case Wired:
            typeString = QLatin1String(NM_SETTING_WIRED_SETTING_NAME);
            break;
        case Wireless:
            typeString = QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME);
            break;
        case WirelessSecurity:
            typeString = QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
            break;
        default:
            break;
    }
    return typeString;
}

Setting::Type Setting::typeFromString(const QString & typeString)
{
    Setting::Type type = Wired;
    if (typeString == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
        type = Cdma;
    } else if (typeString == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
        type = Gsm;
    } else if (typeString == QLatin1String(NM_SETTING_BLUETOOTH_SETTING_NAME)) {
        type = Bluetooth;
    } else if (typeString == QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME)) {
        type = Ipv4;
    } else if (typeString == QLatin1String(NM_SETTING_IP6_CONFIG_SETTING_NAME)) {
        type = Ipv6;
    } else if (typeString == QLatin1String(NM_SETTING_PPP_SETTING_NAME)) {
        type = Ppp;
    } else if (typeString == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
        type = Pppoe;
    } else if (typeString == QLatin1String(NM_SETTING_SERIAL_SETTING_NAME)) {
        type = Serial;
    } else if (typeString == QLatin1String(NM_SETTING_802_1X_SETTING_NAME)) {
        type = Security8021x;
    } else if (typeString == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        type = Vpn;
    } else if (typeString == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
        type = Wired;
    } else if (typeString == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
        type = Wireless;
    } else if (typeString == QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)) {
        type = WirelessSecurity;
    }
    return type;
}

Setting::Setting(Setting::Type type)
    : m_initialized(false), m_type(type), m_secretsAvailable(false)
{
}

Setting::Setting(Setting *setting)
{
    if (!setting->isNull())
        setInitialized(true);
    m_type = setting->type();
    setSecretsAvailable(setting->secretsAvailable());
}

Setting::~Setting()
{
}

bool Setting::isNull() const
{
    return !m_initialized;
}

void Setting::setInitialized(bool initialized)
{
    m_initialized = initialized;
}

Setting::Type Setting::type() const
{
    return m_type;
}

bool Setting::secretsAvailable() const
{
    return m_secretsAvailable;
}

void Setting::setSecretsAvailable(bool available)
{
    m_secretsAvailable = available;
}

QMap<QString,QString> Setting::secretsToMap() const
{
    return QMap<QString,QString>();
}

void Setting::secretsFromMap(QMap<QString,QString> secrets)
{
    Q_UNUSED(secrets);
}

bool Setting::hasPersistentSecrets() const
{
    return false;
}

QStringList Setting::needSecrets(const bool /*requestNew*/) const
{
    return QStringList();
}

// vim: sw=4 sts=4 et tw=100
