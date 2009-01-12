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

#include "remoteconnection.h"

NetworkManagerSettings::NetworkManagerSettings(const QString & service, QObject * parent)
    : OrgFreedesktopNetworkManagerSettingsInterface(service,
                                                    QLatin1String(NM_DBUS_PATH_SETTINGS),
                                                    QDBusConnection::systemBus(), parent)
{
    kDebug() << service << isValid();
    if (isValid())
    {
        initConnections();
        // signal is from parent class
        connect(this, SIGNAL(NewConnection(const QDBusObjectPath&)),
                this, SLOT(onConnectionAdded(const QDBusObjectPath&)));
    }
    connect(QDBusConnection::systemBus().interface(),
            SIGNAL(serviceOwnerChanged(const QString&,const QString&,const QString&)),
            SLOT(serviceOwnerChanged(const QString&,const QString&,const QString&)));
}

void NetworkManagerSettings::initConnections()
{
    QList<QDBusObjectPath> userConnections = ListConnections();
    foreach (QDBusObjectPath op, userConnections) {
        kDebug() << op.path();
        if (m_connections.contains(op.path())) {
            kDebug() << "Service" << service() << "is reporting the same connection more than once in ListConnections!";
        } else {
            RemoteConnection * connectionIface = new RemoteConnection(service(), op.path(), this);
            makeConnections(connectionIface);
            m_connections.insert(op.path(), connectionIface);
        }
    }
}

NetworkManagerSettings::~NetworkManagerSettings()
{

}

void NetworkManagerSettings::makeConnections(RemoteConnection * connectionIface)
{
    connect( connectionIface, SIGNAL(Removed()), this, SLOT(onConnectionRemoved()));
    connect( connectionIface, SIGNAL(Updated(const QVariantMapMap&)),
            this, SLOT(onConnectionUpdated(const QVariantMapMap&)));
}

QStringList NetworkManagerSettings::connections() const
{
    return m_connections.keys();
}

RemoteConnection * NetworkManagerSettings::findConnection(const QString& op) const
{
    return m_connections.value(op);
}

void NetworkManagerSettings::onConnectionAdded(const QDBusObjectPath& op)
{
    kDebug() << op.path();
    if (m_connections.contains(op.path())) {
        kDebug() << "Service" << service() << "is reporting the same connection more than once in ListConnections!";
    } else {
        RemoteConnection * connectionIface = new RemoteConnection(service(), op.path(), this);
        makeConnections(connectionIface);
        m_connections.insert(op.path(), connectionIface);
        emit connectionAdded(this, op.path());
    }
}

void NetworkManagerSettings::onConnectionRemoved()
{
    RemoteConnection * connection = static_cast<RemoteConnection*>(sender());
    QString removedPath = connection->path();
    kDebug() << removedPath;
    delete m_connections.take(removedPath);
    emit connectionRemoved(this, removedPath);
}

void NetworkManagerSettings::onConnectionUpdated(const QVariantMapMap&)
{
    RemoteConnection * connection = static_cast<RemoteConnection*>(sender());
    kDebug() << connection->path();
    emit connectionUpdated(this, connection->path());
}

void NetworkManagerSettings::serviceOwnerChanged(const QString & changedService, const QString & oldOwner, const QString & newOwner)
{
    //kDebug() << changedService << service() << oldOwner << newOwner;
    if (changedService == service()) {
        if (!oldOwner.isEmpty() && newOwner.isEmpty()) {
            clearConnections();
            emit disappeared(this);
        } else if (oldOwner.isEmpty() && !newOwner.isEmpty()) {
            initConnections();
            emit appeared(this);
        } else if (!oldOwner.isEmpty() && !newOwner.isEmpty()) {
            clearConnections();
            emit disappeared(this);
            initConnections();
            emit appeared(this);
        }
    }
}

void NetworkManagerSettings::clearConnections()
{
    qDeleteAll(m_connections.begin(), m_connections.end());
    m_connections.clear();
}

#include "networkmanagersettings.moc"

// vim: sw=4 sts=4 et tw=100
