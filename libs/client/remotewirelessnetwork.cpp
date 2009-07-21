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

#include "remotewirelessnetwork.h"
#include "remotewirelessnetwork_p.h"

RemoteWirelessNetwork::RemoteWirelessNetwork(const QString & path, QObject * parent)
: RemoteActivatable(*new RemoteWirelessNetworkPrivate, path, parent)
{
    Q_D(RemoteWirelessNetwork);
    d->wirelessNetworkItemInterface = new WirelessNetworkInterface("org.kde.networkmanagement", path, QDBusConnection::sessionBus(), this);
    connect(d->wirelessNetworkItemInterface, SIGNAL(strengthChanged(int)), this, SIGNAL(strengthChanged(int)));
}

RemoteWirelessNetwork::~RemoteWirelessNetwork()
{

}

QString RemoteWirelessNetwork::ssid() const
{
    Q_D(const RemoteWirelessNetwork);
    return d->wirelessNetworkItemInterface->ssid();
}

int RemoteWirelessNetwork::strength() const
{
    Q_D(const RemoteWirelessNetwork);
    return d->wirelessNetworkItemInterface->strength();
}

Solid::Control::AccessPoint::Capabilities RemoteWirelessNetwork::capabilities() const
{
    Q_D(const RemoteWirelessNetwork);
    uint wpaflags = d->wirelessNetworkItemInterface->capabilities();
    return (Solid::Control::AccessPoint::Capabilities)wpaflags;

}

Solid::Control::AccessPoint::WpaFlags RemoteWirelessNetwork::wpaFlags() const
{
    Q_D(const RemoteWirelessNetwork);
    uint wpaflags = d->wirelessNetworkItemInterface->wpaFlags();
    return (Solid::Control::AccessPoint::WpaFlags)wpaflags;

}

Solid::Control::AccessPoint::WpaFlags RemoteWirelessNetwork::rsnFlags() const
{
    Q_D(const RemoteWirelessNetwork);
    uint rsnflags = d->wirelessNetworkItemInterface->rsnFlags();
    return (Solid::Control::AccessPoint::WpaFlags)rsnflags;
}

// vim: sw=4 sts=4 et tw=100
