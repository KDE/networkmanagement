/*
Copyright 2012 Arthur de Souza Ribeiro <arthurdesribeiro@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONNECTION_ITEM_H
#define CONNECTION_ITEM_H

#include "remoteactivatable.h"
#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessnetwork.h"

class ConnectionItem : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionItem(RemoteActivatable *activatable, bool hidden = false, QObject *parent = 0);
    QString deviceUni();
    bool isShared();
    QString ssid();
    QString connectionName();
    bool hidden();
    int signalStrength();
    QString connectionUuid();
    RemoteInterfaceConnection* interfaceConnection() const;
    RemoteActivatable* activatable() const;
    bool equals(const ConnectionItem *item);
    void disconnect();
    void connectNetwork();
    QString activationState();
    QString protectedIcon();
    QString connectionType();
    QString connectionIcon();
    void hoverEnter();
    void hoverLeft();
    bool hover();
    bool defaultRoute();
    int signalQuality();
    QString accessTechnology();

protected Q_SLOTS:
    void handlePropertiesChanges();
    void activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState);
    void notifyNetworkingState();
    void handleHasDefaultRouteChanged(bool has);

Q_SIGNALS:
    void itemChanged();
    void showInterfaceDetails(QString);

private:
    RemoteActivatable *m_activatable;
    bool m_hoverEnter;
    bool m_hasDefaultRoute;
    QString m_activationState;
    QString m_type;
    bool m_hidden;
};

#endif
