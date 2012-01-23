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

#ifndef REMOTEACTIVATABLELIST_H
#define REMOTEACTIVATABLELIST_H

#include <QObject>

#include "knmclient_export.h"
#include "remoteinterfaceconnection.h"

class RemoteActivatable;
class RemoteActivatableListPrivate;

class KNMCLIENT_EXPORT RemoteActivatableList : public QObject
{
Q_OBJECT
public:
    RemoteActivatableList(QObject * parent = 0);
    ~RemoteActivatableList();
    QList<RemoteActivatable *> activatables() const;
    void init();

    /**
     * @return the RemoteInterfaceConnection for a given network interface
     * @param interface the Solid::Control::NetworkInterface state
     * @param activatables the RemoteActivatableList of all connections
     */
    RemoteInterfaceConnection* connectionForInterface(Solid::Control::NetworkInterfaceNm09 *interface);

    static bool isConnectionForInterface(RemoteActivatable *activatable, Solid::Control::NetworkInterfaceNm09 *interface);

Q_SIGNALS:
    void activatableAdded(RemoteActivatable*, int);
    void activatableRemoved(RemoteActivatable*);
    void appeared();
    void disappeared();
protected Q_SLOTS:
    void handleActivatableAdded(const QVariantMap &properties);
    void handleActivatableRemoved(const QString &);
    void serviceOwnerChanged(const QString&, const QString&, const QString&);
private:
    void clear();
    Q_DECLARE_PRIVATE(RemoteActivatableList)
    RemoteActivatableListPrivate * d_ptr;
};

#endif // REMOTEACTIVATABLELIST_H
