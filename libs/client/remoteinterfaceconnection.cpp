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

#include "remoteinterfaceconnection.h"
#include "remoteinterfaceconnection_p.h"

RemoteInterfaceConnection::RemoteInterfaceConnection(RemoteInterfaceConnectionPrivate &dd, const QString &dbusPath, QObject * parent)
: RemoteActivatable(dd, dbusPath, parent)
{
    Q_D(RemoteInterfaceConnection);
    d->interfaceConnectionIface = new InterfaceConnectionInterface("org.kde.networkmanagement", dbusPath, QDBusConnection::sessionBus(), this);
    connect(d->interfaceConnectionIface, SIGNAL(activationStateChanged(uint)),
            this, SLOT(handleActivationStateChange(uint)));
}

RemoteInterfaceConnection::RemoteInterfaceConnection(const QString &dbusPath, QObject * parent)
: RemoteActivatable(*new RemoteInterfaceConnectionPrivate, dbusPath, parent)
{
    Q_D(RemoteInterfaceConnection);
    d->interfaceConnectionIface = new InterfaceConnectionInterface("org.kde.networkmanagement", dbusPath, QDBusConnection::sessionBus(), this);
    connect(d->interfaceConnectionIface, SIGNAL(activationStateChanged(uint)),
            this, SLOT(handleActivationStateChange(uint)));
}

RemoteInterfaceConnection::~RemoteInterfaceConnection()
{
}

Knm::Connection::Type RemoteInterfaceConnection::connectionType() const
{
    Q_D(const RemoteInterfaceConnection);
    uint cType = d->interfaceConnectionIface->connectionType();
    return (Knm::Connection::Type)cType;
}

QUuid RemoteInterfaceConnection::connectionUuid() const
{
    Q_D(const RemoteInterfaceConnection);
    QString uuid = d->interfaceConnectionIface->connectionUuid();
    return QUuid(uuid);
}

QString RemoteInterfaceConnection::connectionName() const
{
    Q_D(const RemoteInterfaceConnection);
    return d->interfaceConnectionIface->connectionName();
}

QString RemoteInterfaceConnection::iconName() const
{
    Q_D(const RemoteInterfaceConnection);
    return d->interfaceConnectionIface->iconName();
}

Knm::InterfaceConnection::ActivationState RemoteInterfaceConnection::activationState() const
{
    Q_D(const RemoteInterfaceConnection);
    uint aState = d->interfaceConnectionIface->activationState();
    return (Knm::InterfaceConnection::ActivationState)aState;
}

void RemoteInterfaceConnection::handleActivationStateChange(uint state)
{
    emit activationStateChanged((Knm::InterfaceConnection::ActivationState)state);
}


void RemoteInterfaceConnection::deactivate()
{
    Q_D(RemoteInterfaceConnection);
    d->interfaceConnectionIface->deactivate();
}

// vim: sw=4 sts=4 et tw=100
