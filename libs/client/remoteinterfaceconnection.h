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

#ifndef REMOTEINTERFACECONNECTION_H
#define REMOTEINTERFACECONNECTION_H

#include "remoteactivatable.h"

#include "interfaceconnection.h"

#include "connection.h"

#include "knmclient_export.h"

class RemoteInterfaceConnectionPrivate;
/**
 * Interface to an InterfaceConnection supplied by a remote service
 */
class KNMCLIENT_EXPORT RemoteInterfaceConnection : public RemoteActivatable
{
Q_OBJECT
Q_PROPERTY(uint type READ connectionType)
Q_PROPERTY(QString uuid READ connectionUuid)
Q_PROPERTY(QString name READ connectionName)
Q_PROPERTY(uint activationState READ activationState)
Q_PROPERTY(uint oldActivationState READ oldActivationState)
Q_PROPERTY(bool hasDefaultRoute READ hasDefaultRoute)

friend class RemoteActivatableList;

public:
    virtual ~RemoteInterfaceConnection();

    Knm::Connection::Type connectionType() const;

    QUuid connectionUuid() const;

    QString connectionName(const bool escaped = false) const;

    QString iconName() const;

    bool hasDefaultRoute() const;

    Knm::InterfaceConnection::ActivationState activationState() const;

    Knm::InterfaceConnection::ActivationState oldActivationState() const;

    void deactivate();
Q_SIGNALS:
    void activationStateChanged(Knm::InterfaceConnection::ActivationState, Knm::InterfaceConnection::ActivationState);
    void hasDefaultRouteChanged(bool);
protected Q_SLOTS:
    void icPropertiesChanged(const QVariantMap &properties);
protected:
    RemoteInterfaceConnection(const QVariantMap &properties, QObject * parent);
    RemoteInterfaceConnection(RemoteInterfaceConnectionPrivate &dd, const QVariantMap &properties, QObject * parent);
private:
    void init(const QVariantMap & properties);
    Q_DECLARE_PRIVATE(RemoteInterfaceConnection)
};

#endif // REMOTEINTERFACECONNECTION_H
