/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

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

#include <QDBusReply>

#include "remotegsminterfaceconnection.h"
#include "remotegsminterfaceconnection_p.h"

RemoteGsmInterfaceConnection::RemoteGsmInterfaceConnection(const QString &dbusPath, QObject * parent)
: RemoteInterfaceConnection(*new RemoteGsmInterfaceConnectionPrivate, dbusPath, parent)
{
    Q_D(RemoteGsmInterfaceConnection);
    d->gsmInterfaceConnectionIface = new GsmInterfaceConnectionInterface("org.kde.networkmanagement", dbusPath, QDBusConnection::sessionBus(), this);
    connect(d->gsmInterfaceConnectionIface, SIGNAL(signalQualityChanged(int)), this, SIGNAL(signalQualityChanged(int)));

    connect(d->gsmInterfaceConnectionIface, SIGNAL(accessTechnologyChanged(const int)), this, SIGNAL(accessTechnologyChanged(const int)));
}

RemoteGsmInterfaceConnection::~RemoteGsmInterfaceConnection()
{

}

int RemoteGsmInterfaceConnection::getAccessTechnology() const
{
    Q_D(const RemoteGsmInterfaceConnection);
    QDBusReply<uint> reply = d->gsmInterfaceConnectionIface->getAccessTechnology();

    if (reply.isValid()) {
        return reply.value();
    } else {
        return Solid::Control::ModemInterface::UnknownTechnology;
    }
}

int RemoteGsmInterfaceConnection::getSignalQuality() const
{
    Q_D(const RemoteGsmInterfaceConnection);
    return d->gsmInterfaceConnectionIface->getSignalQuality();
}

// vim: sw=4 sts=4 et tw=100
