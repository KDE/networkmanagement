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

friend class RemoteActivatableList;

public:
    virtual ~RemoteInterfaceConnection();

    Knm::Connection::Type connectionType() const;

    QUuid connectionUuid() const;

    QString connectionName() const;

    Knm::InterfaceConnection::ActivationState activationState() const;

Q_SIGNALS:
    void activationStateChanged(Knm::InterfaceConnection::ActivationState);
protected Q_SLOTS:
    void handleActivationStateChange(uint);
protected:
    RemoteInterfaceConnection(const QString &dbusPath, QObject * parent);
    RemoteInterfaceConnection(RemoteInterfaceConnectionPrivate &dd, const QString &dbusPath, QObject * parent);
private:
    Q_DECLARE_PRIVATE(RemoteInterfaceConnection);
};

#endif // REMOTEINTERFACECONNECTION_H
