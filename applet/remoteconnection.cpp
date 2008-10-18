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

#include "remoteconnection.h"

#include <nm-setting-cdma.h>
#include <nm-setting-connection.h>
#include <nm-setting-gsm.h>
#include <nm-setting-pppoe.h>
#include <nm-setting-vpn.h>
#include <nm-setting-wired.h>
#include <nm-setting-wireless.h>

#include <QDBusConnection>

#include <KDebug>

RemoteConnection::RemoteConnection(const QString& service, const QString & path, QObject * parent)
: OrgFreedesktopNetworkManagerSettingsConnectionInterface(service, path, QDBusConnection::systemBus(), parent)
{
    qDBusRegisterMetaType<QMap<QString, QVariant> >();
    qDBusRegisterMetaType<QMap<QString, QMap<QString, QVariant> > >();

    QVariantMapMap connection = GetSettings();
    kDebug() << connection;

    if ( connection.contains(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME))) {
        QVariantMap connectionSetting = connection.value(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME));
        if (connectionSetting.contains(QLatin1String(NM_SETTING_CONNECTION_ID))) {
            m_id = connectionSetting.value(QLatin1String(NM_SETTING_CONNECTION_ID)).toString();
        }
        QString nmType;
        if (connectionSetting.contains(QLatin1String(NM_SETTING_CONNECTION_TYPE))) {
            nmType = connectionSetting.value(QLatin1String(NM_SETTING_CONNECTION_TYPE)).toString();
        }
        if (nmType == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterface::Cdma;
        } else if (nmType == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterface::Gsm;
        } else if (nmType == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterface::Serial;
        } else if (nmType == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterface::Ieee8023;
        } else if (nmType == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterface::Ieee80211;
        }
    }
}

RemoteConnection::~RemoteConnection()
{
}

QString RemoteConnection::id() const
{
    return m_id;
}

Solid::Control::NetworkInterface::Type RemoteConnection::type() const
{
    return m_type;
}

// vim: sw=4 sts=4 et tw=100
