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

#include "remoteinterfaceconnection.h"
#include "remoteinterfaceconnection_p.h"

#include <KDebug>

RemoteInterfaceConnection::RemoteInterfaceConnection(RemoteInterfaceConnectionPrivate &dd, const QVariantMap &properties, QObject * parent)
: RemoteActivatable(dd, properties, parent)
{
    init(properties);
}

RemoteInterfaceConnection::RemoteInterfaceConnection(const QVariantMap &properties, QObject * parent)
: RemoteActivatable(*new RemoteInterfaceConnectionPrivate, properties, parent)
{
    init(properties);
}

void RemoteInterfaceConnection::init(const QVariantMap &properties)
{
    Q_D(RemoteInterfaceConnection);
    d->interfaceConnectionIface = new InterfaceConnectionInterface("org.kde.networkmanagement", properties["path"].toString(), QDBusConnection::sessionBus(), this);
    connect(d->interfaceConnectionIface, SIGNAL(icPropertiesChanged(QVariantMap)), SLOT(icPropertiesChanged(QVariantMap)));

    d->activationState = Knm::InterfaceConnection::Unknown;
    icPropertiesChanged(properties);
}

RemoteInterfaceConnection::~RemoteInterfaceConnection()
{
}

void RemoteInterfaceConnection::icPropertiesChanged(const QVariantMap &changedProperties)
{
    Q_D(RemoteInterfaceConnection);
    QStringList propKeys = changedProperties.keys();
    QLatin1String connectionTypeKey("connectionType"),
                  uuidKey("uuid"),
                  nameKey("name"),
                  iconNameKey("iconName"),
                  activationStateKey("activationState"),
                  hasDefaultRouteKey("hasDefaultRoute");
    QVariantMap::const_iterator it = changedProperties.find(connectionTypeKey);
    if (it != changedProperties.end()) {
        d->connectionType = (Knm::Connection::Type)it->toUInt();
        propKeys.removeOne(connectionTypeKey);
    }
    it = changedProperties.find(uuidKey);
    if (it != changedProperties.end()) {
        d->uuid = it->toString();
        propKeys.removeOne(uuidKey);
    }
    it = changedProperties.find(nameKey);
    if (it != changedProperties.end()) {
        d->name = it->toString();
        propKeys.removeOne(nameKey);
    }
    it = changedProperties.find(iconNameKey);
    if (it != changedProperties.end()) {
        d->iconName = it->toString();
        propKeys.removeOne(iconNameKey);
    }
    it = changedProperties.find(activationStateKey);
    if (it != changedProperties.end()) {
        d->oldActivationState = d->activationState;
        d->activationState = (Knm::InterfaceConnection::ActivationState)it->toUInt();
        emit activationStateChanged(d->oldActivationState, d->activationState);
        propKeys.removeOne(activationStateKey);
    }
    it = changedProperties.find(hasDefaultRouteKey);
    if (it != changedProperties.end()) {
        d->hasDefaultRoute = it->toBool();
        emit hasDefaultRouteChanged(d->hasDefaultRoute);
        propKeys.removeOne(hasDefaultRouteKey);
    }
    /*if (propKeys.count()) {
        kDebug() << "Unhandled properties: " << propKeys;
    }*/
    emit changed();
}

Knm::Connection::Type RemoteInterfaceConnection::connectionType() const
{
    Q_D(const RemoteInterfaceConnection);
    return d->connectionType;
}

QUuid RemoteInterfaceConnection::connectionUuid() const
{
    Q_D(const RemoteInterfaceConnection);
    return d->uuid;
}

QString RemoteInterfaceConnection::connectionName(const bool escaped) const
{
    Q_D(const RemoteInterfaceConnection);
    if (escaped) {
        QString temp = d->name;
        temp.replace(QLatin1Char('&'), QLatin1String("&&"));
        return temp;
    }
    return d->name;
}

QString RemoteInterfaceConnection::iconName() const
{
    Q_D(const RemoteInterfaceConnection);
    return d->iconName;
}

Knm::InterfaceConnection::ActivationState RemoteInterfaceConnection::activationState() const
{
    Q_D(const RemoteInterfaceConnection);
    return d->activationState;
}

Knm::InterfaceConnection::ActivationState RemoteInterfaceConnection::oldActivationState() const
{
    Q_D(const RemoteInterfaceConnection);
    return d->oldActivationState;
}

bool RemoteInterfaceConnection::hasDefaultRoute() const
{
    Q_D(const RemoteInterfaceConnection);
    return d->hasDefaultRoute;
}

void RemoteInterfaceConnection::deactivate()
{
    Q_D(RemoteInterfaceConnection);
    d->interfaceConnectionIface->deactivate();
}

// vim: sw=4 sts=4 et tw=100
