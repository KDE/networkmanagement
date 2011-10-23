/*
    Copyright 2011 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/


#ifndef NETWORKMANAGEMENTENGINE_H
#define NETWORKMANAGEMENTENGINE_H

#include <plasma/dataengine.h>
#include "remoteinterfaceconnection.h"


class RemoteActivatableList;

class RemoteActivatable;
class RemoteInterfaceConnection;
class RemoteInterfaceConnection;
class RemoteWirelessInterfaceConnection;
class RemoteWirelessNetwork;
class RemoteUnconfiguredInterface;
class RemoteVpnInterfaceConnection;
class RemoteHiddenWirelessInterfaceConnection;
class RemoteGsmInterfaceConnection;

class WirelessStatus;

class NetworkManagementEnginePrivate;

class NetworkManagementEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        NetworkManagementEngine(QObject* parent, const QVariantList& args);
        ~NetworkManagementEngine();
        QStringList sources() const;
        virtual void init();

    private Q_SLOTS:
        void activatableAdded(RemoteActivatable*);
        void activatableRemoved(RemoteActivatable*);
        void listDisappeared();
        void listAppeared();
        void activationStateChanged(Knm::InterfaceConnection::ActivationState, Knm::InterfaceConnection::ActivationState); // for debugging

        void addActivatable(RemoteActivatable* remote);
        void updateActivatable(RemoteActivatable* remote = 0);

        void addInterfaceConnection(RemoteActivatable* remote);
        void updateInterfaceConnection(RemoteActivatable* remote = 0);

        void addWirelessInterfaceConnection(RemoteActivatable* remote);
        void updateWirelessInterfaceConnection(RemoteActivatable* remote = 0);

        void addWirelessNetwork(RemoteActivatable* remote);
        void updateWirelessNetwork(RemoteActivatable* remote = 0);

        void addHiddenWirelessInterfaceConnection(RemoteActivatable* remote = 0);
        void updateHiddenWirelessInterfaceConnection(RemoteActivatable* remote);

        void updateWirelessStatus(const QString &source, WirelessStatus *wirelessStatus);

        void addUnconfiguredInterface(RemoteActivatable* remote);
        void updateUnconfiguredInterface(RemoteActivatable* remote = 0);

        void addVpnInterfaceConnection(RemoteActivatable* remote);
        void updateVpnInterfaceConnection(RemoteActivatable* remote = 0);

        void addGsmInterfaceConnection(RemoteActivatable* remote);
        void updateGsmInterfaceConnection(RemoteActivatable* remote = 0);

    protected:
        bool sourceRequestEvent(const QString &name);

    private:
        //QString sourceForActivatable(RemoteActivatable* remote);
        QString source(RemoteActivatable* remote);
        //void updateConnection(const QString &source, RemoteActivatable* remote);
        //void updateWireless(const QString &source, WirelessStatus *wirelessStatus);
        /*
        enum ActivatableType {
            InterfaceConnection = RemoteActivatable
            WirelessInterfaceConnection = RemoteInterfaceConnection, WirelessObject
            WirelessNetwork,
            UnconfiguredInterface = RemoteActivatable
            VpnInterfaceConnection = RemoteInterfaceConnection,
            HiddenWirelessInterfaceConnection,
            GsmInterfaceConnection = RemoteInterfaceConnection

            WirelessObject =
        };
        */

        NetworkManagementEnginePrivate* d;
//public slots:
    //void updateWirelessNetwork();
};

K_EXPORT_PLASMA_DATAENGINE(networkmanagementengine, NetworkManagementEngine)

#endif
