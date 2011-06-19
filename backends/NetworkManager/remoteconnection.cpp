/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "remoteconnection.h"

#include <NetworkManager.h>
#include <nm-setting-cdma.h>
#include <nm-setting-connection.h>
#include <nm-setting-gsm.h>
#include <nm-setting-bluetooth.h>
#include <nm-setting-pppoe.h>
#include <nm-setting-vpn.h>
#include <nm-setting-wired.h>
#include <nm-setting-wireless.h>

#include <QDBusConnection>

#include <KDebug>

#include <solid/control/networkmanager.h>

#include "nm-active-connectioninterface.h"

RemoteConnection::RemoteConnection(const QString& service, const QString & path, QObject * parent)
: OrgFreedesktopNetworkManagerSettingsConnectionInterface(service, path, QDBusConnection::systemBus(), parent)
{
    qDBusRegisterMetaType<QMap<QString, QVariant> >();
    qDBusRegisterMetaType<QMap<QString, QMap<QString, QVariant> > >();

    m_connection = GetSettings();
    m_path = path;
    m_type = Solid::Control::NetworkInterfaceNm09::UnknownType;

    //kDebug() << m_connection;

    if ( m_connection.contains(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME))) {
        QVariantMap connectionSetting = m_connection.value(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME));
        if (connectionSetting.contains(QLatin1String(NM_SETTING_CONNECTION_ID))) {
            m_id = connectionSetting.value(QLatin1String(NM_SETTING_CONNECTION_ID)).toString();
        }
        QString nmType;
        if (connectionSetting.contains(QLatin1String(NM_SETTING_CONNECTION_TYPE))) {
            nmType = connectionSetting.value(QLatin1String(NM_SETTING_CONNECTION_TYPE)).toString();
        }
        //kDebug() << nmType;
        if (nmType == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterfaceNm09::Modem;
        } else if (nmType == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterfaceNm09::Modem;
        } else if (nmType == QLatin1String(NM_SETTING_BLUETOOTH_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterfaceNm09::Bluetooth;
        } else if (nmType == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterfaceNm09::Modem;
        } else if (nmType == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterfaceNm09::Ethernet;
        } else if (nmType == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
            m_type = Solid::Control::NetworkInterfaceNm09::Wifi;
        }
        /* TODO: add NM_SETTING_OLPC_MESH_SETTING_NAME, NM_SETTING_WIMAX_SETTING_NAME */
    }
}

RemoteConnection::~RemoteConnection()
{
}

QString RemoteConnection::id() const
{
    return m_id;
}

Solid::Control::NetworkInterfaceNm09::Type RemoteConnection::type() const
{
    return m_type;
}

QVariantMapMap RemoteConnection::settings() const
{
    return m_connection;
}

bool RemoteConnection::active() const
{
    QStringList activeConnections = Solid::Control::NetworkManagerNm09::activeConnections();
    foreach (const QString &conn, activeConnections) {
        OrgFreedesktopNetworkManagerConnectionActiveInterface candidate(NM_DBUS_SERVICE,
                conn, QDBusConnection::systemBus(), 0);
        if (candidate.connection().path() == path()) {
            return true;
        }
    }
    return false;
}

QString RemoteConnection::path() const
{
    return m_path;
}
// vim: sw=4 sts=4 et tw=100
