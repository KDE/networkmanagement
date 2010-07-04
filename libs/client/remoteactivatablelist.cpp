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

#include "remoteactivatablelist.h"

#include <QHash>
#include <QStringList>
#include <QDBusInterface>

#include <KDebug>

#include "networkmanagementinterface.h"

#include "remoteactivatable.h"
#include "remoteinterfaceconnection.h"
#include "remoteunconfiguredinterface.h"
#include "remotevpninterfaceconnection.h"
#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessnetwork.h"
#include "remotegsminterfaceconnection.h"

class RemoteActivatableListPrivate
{
public:
    NetworkManagementInterface * iface;
    QHash<QString, RemoteActivatable *> activatables;
};


RemoteActivatableList::RemoteActivatableList(QObject * parent)
: QObject(parent), d_ptr(new RemoteActivatableListPrivate)
{
    Q_D(RemoteActivatableList);
    d->iface = new NetworkManagementInterface("org.kde.networkmanagement", "/org/kde/networkmanagement", QDBusConnection::sessionBus(), this);
    connect(d->iface, SIGNAL(ActivatableAdded(const QString&,uint)),
            this, SLOT(handleActivatableAdded(const QString&,uint)));
    connect(d->iface, SIGNAL(ActivatableRemoved(const QString&)),
            this, SLOT(handleActivatableRemoved(const QString &)));
    // clean our connections out if the service goes away
    connect(QDBusConnection::sessionBus().interface(),
            SIGNAL(serviceOwnerChanged(const QString&,const QString&,const QString&)),
            SLOT(serviceOwnerChanged(const QString&,const QString&,const QString&)));
}

void RemoteActivatableList::init()
{
    Q_D(RemoteActivatableList);
    if (d->iface->isValid()) {
        if (d->activatables.isEmpty()) {
            QDBusReply<QStringList> rv = d->iface->ListActivatables();
            if (!rv.error().isValid()) {
                foreach (const QString &activatable, rv.value()) {
                    // messy, I know, but making ListActivatables return a(si) is boring
                    QDBusInterface iface(QLatin1String("org.kde.networkmanagement"),
                            activatable, "org.kde.networkmanagement.Activatable", QDBusConnection::sessionBus());
                    QDBusReply<uint> type = iface.call("activatableType");
                    handleActivatableAdded(activatable, type.value());
                }
            }
        }
    }
}

void RemoteActivatableList::clear()
{
    Q_D(RemoteActivatableList);
    foreach (RemoteActivatable * activatable, d->activatables) {
        emit activatableRemoved(activatable);
        delete activatable;
    }
    d->activatables.clear();
}

RemoteActivatableList::~RemoteActivatableList()
{
    delete d_ptr;
}

QList<RemoteActivatable *> RemoteActivatableList::activatables() const
{
    Q_D(const RemoteActivatableList);
    return d->activatables.values();
}

void RemoteActivatableList::handleActivatableAdded(const QString &addedPath, uint type)
{
    if (!addedPath.startsWith('/')) {
        kDebug() << "Invalid path:" << addedPath << type;
        return;
    }
    Q_D(RemoteActivatableList);
    if (!d->activatables.contains(addedPath)) {
        RemoteActivatable * newActivatable = 0;
        switch (type) {
            case Knm::Activatable::InterfaceConnection:
                newActivatable = new RemoteInterfaceConnection(addedPath, this);
                //kDebug() << "interfaceconnection at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::WirelessInterfaceConnection:
                newActivatable = new RemoteWirelessInterfaceConnection(addedPath, this);
                //kDebug() << "wirelessconnection at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::WirelessNetwork:
                newActivatable = new RemoteWirelessNetwork(addedPath, this);
                //kDebug() << "wirelessnetwork at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::UnconfiguredInterface:
                newActivatable = new RemoteUnconfiguredInterface(addedPath, this);
                //kDebug() << "unconfiguredinterface at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::VpnInterfaceConnection:
                newActivatable = new RemoteVpnInterfaceConnection(addedPath, this);
                //kDebug() << "vpnconnection at" << addedPath << "with type" << newActivatable->activatableType();
                break;
            case Knm::Activatable::GsmInterfaceConnection:
                newActivatable = new RemoteGsmInterfaceConnection(addedPath, this);
                //kDebug() << "gsminterfaceconnection at" << addedPath << "with type" << newActivatable->activatableType();
                break;
        }
        if (newActivatable) {
            d->activatables.insert(addedPath, newActivatable);
            emit activatableAdded(newActivatable);
        }
    }
}

void RemoteActivatableList::handleActivatableRemoved(const QString &removed)
{
    Q_D(RemoteActivatableList);
    kDebug() << "removed" << removed;
    RemoteActivatable * removedActivatable = d->activatables.take(removed);
    if (removedActivatable) {
        delete removedActivatable;
        emit activatableRemoved(removedActivatable);
    }
}

void RemoteActivatableList::serviceOwnerChanged(const QString & changedService, const QString & oldOwner, const QString & newOwner)
{
    Q_D(RemoteActivatableList);
    //kDebug() << changedService << changedService << oldOwner << newOwner;
    if (changedService == d->iface->service()) {
        if (!oldOwner.isEmpty() && newOwner.isEmpty()) {
            emit disappeared();
            clear();
        } else if (oldOwner.isEmpty() && !newOwner.isEmpty()) {
            init();
            emit appeared();
        } else if (!oldOwner.isEmpty() && !newOwner.isEmpty()) {
            emit disappeared();
            clear();
            init();
            emit appeared();
        }
    }
}// vim: sw=4 sts=4 et tw=100
