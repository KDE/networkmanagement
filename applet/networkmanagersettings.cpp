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

#include "networkmanagersettings.h"

#include <NetworkManager.h>

#include <KDebug>

NetworkManagerSettings::NetworkManagerSettings(const QString & service, QObject * parent)
    : OrgFreedesktopNetworkManagerSettingsInterface(service, QLatin1String(NM_DBUS_PATH_SETTINGS), QDBusConnection::systemBus(), parent), m_service(service)
{
    if (isValid())
    {
    QList<QDBusObjectPath> userConnections = ListConnections();
    foreach (QDBusObjectPath op, userConnections) {
        OrgFreedesktopNetworkManagerSettingsConnectionInterface * connectionIface = new OrgFreedesktopNetworkManagerSettingsConnectionInterface(m_service, op.path(), QDBusConnection::systemBus(), this);
        m_connections.insert(op.path(), connectionIface);
        connect( connectionIface, SIGNAL(Removed()), this, SLOT(onConnectionRemoved()));
        connect( connectionIface, SIGNAL(Updated(const QVariantMapMap&)), this, SLOT(onConnectionUpdated(const QVariantMapMap&)));
    }
    connect(this, SIGNAL(NewConnection(const QDBusObjectPath&)),
            this, SLOT(onConnectionAdded(const QDBusObjectPath&)));
    }
}

NetworkManagerSettings::~NetworkManagerSettings()
{

}

QStringList NetworkManagerSettings::connections() const
{
    return m_connections.keys();
}

OrgFreedesktopNetworkManagerSettingsConnectionInterface * NetworkManagerSettings::findConnection(const QString& op) const
{
    return m_connections.value(op);
}

void NetworkManagerSettings::onConnectionAdded(const QDBusObjectPath& op)
{
    kDebug() << op.path();
    OrgFreedesktopNetworkManagerSettingsConnectionInterface * connectionIface = new OrgFreedesktopNetworkManagerSettingsConnectionInterface(m_service, op.path(), QDBusConnection::systemBus());
    m_connections.insert(op.path(), connectionIface);
    connect( connectionIface, SIGNAL(Removed()), this, SLOT(onConnectionRemoved()));
    connect( connectionIface, SIGNAL(Updated(const QVariantMapMap&)), this, SLOT(onConnectionUpdated(const QVariantMapMap&)));
    emit connectionAdded(op.path());
}

void NetworkManagerSettings::onConnectionRemoved()
{
    OrgFreedesktopNetworkManagerSettingsConnectionInterface * connection = static_cast<OrgFreedesktopNetworkManagerSettingsConnectionInterface*>(sender());
    kDebug() << connection->path();
    emit connectionRemoved(connection->path());
}

void NetworkManagerSettings::onConnectionUpdated(const QVariantMapMap&)
{
    OrgFreedesktopNetworkManagerSettingsConnectionInterface * connection = static_cast<OrgFreedesktopNetworkManagerSettingsConnectionInterface*>(sender());
    kDebug() << connection->path();
    emit connectionUpdated(connection->path());
}

#include "networkmanagersettings.moc"

// vim: sw=4 sts=4 et tw=100
