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

#include <KDebug>

#include "remoteactivatable.h"
#include "remoteactivatable_p.h"

RemoteActivatablePrivate::~RemoteActivatablePrivate()
{

}

RemoteActivatable::RemoteActivatable(RemoteActivatablePrivate & dd, const QString &dbusPath, QObject * parent)
: QObject(parent), d_ptr(&dd)
{
    Q_D(RemoteActivatable);
    d->activatableIface = new ActivatableInterface("org.kde.networkmanagement", dbusPath, QDBusConnection::sessionBus(), this);
    init();
}

RemoteActivatable::RemoteActivatable(const QString &dbusPath, QObject * parent)
: QObject(parent), d_ptr(new RemoteActivatablePrivate)
{
    Q_D(RemoteActivatable);
    d->activatableIface = new ActivatableInterface("org.kde.networkmanagement", dbusPath, QDBusConnection::sessionBus(), this);
    init();
}

RemoteActivatable::~RemoteActivatable()
{
    delete d_ptr;
}

void RemoteActivatable::init()
{
    Q_D(RemoteActivatable);
    //kDebug() << "ActivatableInterface is (" << d->activatableIface << ") on" << d_ptr;
    connect(d->activatableIface, SIGNAL(activated()),
            this, SIGNAL(activated()));
    connect(d->activatableIface, SIGNAL(changed()),
            this, SIGNAL(changed()));

    QDBusReply<QString> reply = d->activatableIface->deviceUni();
    if (reply.isValid()) {
        d->deviceUni = reply.value();
    } else {
        kDebug() << "deviceUni reply is invalid";
    }

    QDBusReply<uint> reply2 = d->activatableIface->activatableType();
    if (reply2.isValid()) {
        d->activatableType = (Knm::Activatable::ActivatableType)reply2.value();
    } else {
        d->activatableType = Knm::Activatable::InterfaceConnection;
        kDebug() << "activatableType reply is invalid";
    }
}

Knm::Activatable::ActivatableType RemoteActivatable::activatableType() const
{
    Q_D(const RemoteActivatable);
    return d->activatableType;
}

QString RemoteActivatable::deviceUni() const
{
    Q_D(const RemoteActivatable);
    return d->deviceUni;
}

bool RemoteActivatable::isShared() const
{
    Q_D(const RemoteActivatable);
    if (!d->activatableIface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = d->activatableIface->isShared();
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

void RemoteActivatable::activate()
{
    Q_D(RemoteActivatable);
    if (d->activatableIface) {
        d->activatableIface->activate();
    }
}

// vim: sw=4 sts=4 et tw=100
