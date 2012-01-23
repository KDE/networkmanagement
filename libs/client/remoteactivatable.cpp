/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2012 Lamarque V. Souza <lamarque@kde.org>

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

RemoteActivatable::RemoteActivatable(RemoteActivatablePrivate & dd, const QVariantMap &properties, QObject * parent)
: QObject(parent), d_ptr(&dd)
{
    init(properties);
}

RemoteActivatable::RemoteActivatable(const QVariantMap &properties, QObject * parent)
: QObject(parent), d_ptr(new RemoteActivatablePrivate)
{
    init(properties);
}

RemoteActivatable::~RemoteActivatable()
{
    delete d_ptr;
}

void RemoteActivatable::init(const QVariantMap & properties)
{
    Q_D(RemoteActivatable);
    d->activatableIface = new ActivatableInterface("org.kde.networkmanagement", properties["path"].toString(), QDBusConnection::sessionBus(), this);
    //kDebug() << "ActivatableInterface is (" << d->activatableIface << ") on" << d_ptr;
    connect(d->activatableIface, SIGNAL(activated()),
            this, SIGNAL(activated()));
    connect(d->activatableIface, SIGNAL(changed()),
            this, SIGNAL(changed()));
    connect(d->activatableIface, SIGNAL(propertiesChanged(QVariantMap)), SLOT(propertiesChanged(QVariantMap)));

    propertiesChanged(properties);
}

void RemoteActivatable::propertiesChanged(const QVariantMap &changedProperties)
{
    Q_D(RemoteActivatable);
    QStringList propKeys = changedProperties.keys();
    QLatin1String deviceUniKey("deviceUni"),
                  activatableTypeKey("activatableType"),
                  sharedKey("shared");
    QVariantMap::const_iterator it = changedProperties.find(deviceUniKey);
    if (it != changedProperties.end()) {
        d->deviceUni = it->toString();
        propKeys.removeOne(deviceUniKey);
    }
    it = changedProperties.find(activatableTypeKey);
    if (it != changedProperties.end()) {
        d->activatableType = (Knm::Activatable::ActivatableType)it->toUInt();
        propKeys.removeOne(activatableTypeKey);
    }
    it = changedProperties.find(sharedKey);
    if (it != changedProperties.end()) {
        d->shared = it->toBool();
        propKeys.removeOne(sharedKey);
    }
    /*if (propKeys.count()) {
        kDebug() << "Unhandled properties: " << propKeys;
    }*/
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
    return d->shared;
}

void RemoteActivatable::activate()
{
    Q_D(RemoteActivatable);
    if (d->activatableIface) {
        d->activatableIface->activate();
    }
}

// vim: sw=4 sts=4 et tw=100
