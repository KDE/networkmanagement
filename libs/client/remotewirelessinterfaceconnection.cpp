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

#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessinterfaceconnection_p.h"

RemoteWirelessInterfaceConnection::RemoteWirelessInterfaceConnection(const QString &dbusPath, QObject * parent)
: RemoteInterfaceConnection(*new RemoteWirelessInterfaceConnectionPrivate, dbusPath, parent)
{
    Q_D(RemoteWirelessInterfaceConnection);
    d->wirelessInterfaceConnectionIface = new WirelessInterfaceConnectionInterface("org.kde.networkmanagement", dbusPath, QDBusConnection::sessionBus(), this);
    connect(d->wirelessInterfaceConnectionIface, SIGNAL(strengthChanged(int)), this, SIGNAL(strengthChanged(int)));
}

RemoteWirelessInterfaceConnection::~RemoteWirelessInterfaceConnection()
{

}

QString RemoteWirelessInterfaceConnection::ssid() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    return d->wirelessInterfaceConnectionIface->ssid();
}

int RemoteWirelessInterfaceConnection::strength() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    return d->wirelessInterfaceConnectionIface->strength();
}

Solid::Control::AccessPoint::Capabilities RemoteWirelessInterfaceConnection::capabilities() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    uint caps = d->wirelessInterfaceConnectionIface->capabilities();
    return (Solid::Control::AccessPoint::Capabilities)caps;
}

Solid::Control::AccessPoint::WpaFlags RemoteWirelessInterfaceConnection::wpaFlags() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    uint wpaflags = d->wirelessInterfaceConnectionIface->wpaFlags();
    return (Solid::Control::AccessPoint::WpaFlags)wpaflags;
}

Solid::Control::AccessPoint::WpaFlags RemoteWirelessInterfaceConnection::rsnFlags() const
{
    Q_D(const RemoteWirelessInterfaceConnection);
    uint rsnflags = d->wirelessInterfaceConnectionIface->rsnFlags();
    return (Solid::Control::AccessPoint::WpaFlags)rsnflags;
}

// vim: sw=4 sts=4 et tw=100
