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

#include "sessionabstractedservice.h"

#include <QHash>
#include <QDBusConnection>

#include <KDebug>

#include "activatablelist.h"

#include "activatableadaptor.h"
#include "interfaceconnectionadaptor.h"
#include "wirelessinterfaceconnectionadaptor.h"
#include "wirelessnetworkadaptor.h"

class SessionAbstractedServicePrivate
{
public:
    ActivatableList * list;
    QHash<Knm::Activatable *, QString> adaptors;
    uint nextConnectionId;
};

const QString SessionAbstractedService::SESSION_SERVICE_DBUS_PATH = QLatin1String("/org/kde/networkmanagement/Activatable");

SessionAbstractedService::SessionAbstractedService(ActivatableList * list, QObject *parent)
: QObject(parent), d_ptr(new SessionAbstractedServicePrivate)
{
    Q_D(SessionAbstractedService);
    d->list = list;
    d->nextConnectionId = 1;

    QDBusConnection::sessionBus().registerService("org.kde.networkmanagement");
    QDBusConnection::sessionBus().registerObject("/org/kde/networkmanagement", this, QDBusConnection::ExportScriptableContents);
}

SessionAbstractedService::~SessionAbstractedService()
{
}

void SessionAbstractedService::handleAdd(Knm::Activatable * added)
{
    Q_D(SessionAbstractedService);
    if (added) {
        if (added->activatableType() == Knm::Activatable::InterfaceConnection ) {
            Knm::InterfaceConnection * realObj = static_cast<Knm::InterfaceConnection*>(added);
            new InterfaceConnectionAdaptor(realObj);
            new ActivatableAdaptor(realObj);
        } else if (added->activatableType() == Knm::Activatable::WirelessInterfaceConnection ) {
            Knm::WirelessInterfaceConnection * realObj
                = static_cast<Knm::WirelessInterfaceConnection*>(added);
            new WirelessInterfaceConnectionAdaptor(realObj);
            new InterfaceConnectionAdaptor(realObj);
            new ActivatableAdaptor(realObj);
        } else { // WirelessNetwork
            Knm::WirelessNetwork * realObj
                = static_cast<Knm::WirelessNetwork*>(added);
            new WirelessNetworkAdaptor(realObj);
            new ActivatableAdaptor(realObj);
        }
        QString path = nextObjectPath();
        d->adaptors.insert(added, path);
        QDBusConnection::sessionBus().registerObject(path, added);
        //kDebug() << "registering object at " << path;
        emit ActivatableAdded(path, added->activatableType());
    }
}

void SessionAbstractedService::handleUpdate(Knm::Activatable *)
{
}

void SessionAbstractedService::handleRemove(Knm::Activatable * removed)
{
    // the adaptors themselves were children of the activatable and will be deleted
    Q_D(SessionAbstractedService);
    if (d->adaptors.contains(removed)) {
        QString path = d->adaptors.take(removed);
//debug
#if 0
        if (removed->activatableType() == Knm::Activatable::InterfaceConnection ) {
            Knm::InterfaceConnection * realObj = static_cast<Knm::InterfaceConnection*>(removed);
            kDebug() << path << realObj->connectionUuid();
        } else if (removed->activatableType() == Knm::Activatable::WirelessNetwork ) {
            Knm::WirelessNetwork * realObj
                = static_cast<Knm::WirelessNetwork*>(removed);
            kDebug() << path << realObj->ssid();
        }
#endif
        emit ActivatableRemoved(path);
    }
}

QStringList SessionAbstractedService::ListActivatables() const
{
    Q_D(const SessionAbstractedService);
    QStringList sortedPaths;
    foreach (Knm::Activatable * a, d->list->activatables()) {
        if (d->adaptors.contains(a)) {
            sortedPaths.append(d->adaptors[a]);
        }
    }
    return sortedPaths;
}

QString SessionAbstractedService::nextObjectPath()
{
    Q_D(SessionAbstractedService);
    return QString::fromLatin1("%1/%2").arg(SESSION_SERVICE_DBUS_PATH).arg(d->nextConnectionId++);
}

// vim: sw=4 sts=4 et tw=100

