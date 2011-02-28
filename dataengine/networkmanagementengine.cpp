/*
    Copyright 210 Sebastian Kügler <sebas@kde.org>

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


#include "networkmanagementengine.h"

#include <solid/control/networkmanager.h>

// networkmanagement lib
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"
#include "remoteunconfiguredinterface.h"
#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessnetwork.h"
#include "remotegsminterfaceconnection.h"
#include "remotevpninterfaceconnection.h"

#include "../applet/wirelessstatus.h"


class NetworkManagementEnginePrivate
{
public:
    RemoteActivatableList* activatables;
    QHash<RemoteActivatable*, QString> sources;
    int i;
};
/*
NetworkManagementEngine::NetworkManagementEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
}
*/
NetworkManagementEngine::NetworkManagementEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent)
{
    Q_UNUSED(args);
    d = new NetworkManagementEnginePrivate;

    d->i = 0;
    setMaxSourceCount(64); // Guard against loading too many connections

    //init();
}

void NetworkManagementEngine::init()
{
    kDebug() << "init.";
    d->activatables = new RemoteActivatableList(this);
    d->activatables->init();
}

NetworkManagementEngine::~NetworkManagementEngine()
{
}

QStringList NetworkManagementEngine::sources() const
{
    return QStringList() << "networkStatus" << "connections";
}

bool NetworkManagementEngine::sourceRequestEvent(const QString &name)
{
    kDebug() << "Source requested:" << name << sources();
    setData(name, DataEngine::Data());
    //setData("networkStatus", "isConnected", true);
    //scheduleSourcesUpdated();

    if (name == "connections") {
        connect(d->activatables, SIGNAL(activatableAdded(RemoteActivatable*)),
                SLOT(activatableAdded(RemoteActivatable *)));
        connect(d->activatables, SIGNAL(activatableRemoved(RemoteActivatable*)),
                SLOT(activatableRemoved(RemoteActivatable *)));

        connect(d->activatables, SIGNAL(appeared()), SLOT(listAppeared()));
        connect(d->activatables, SIGNAL(disappeared()), SLOT(listDisappeared()));
        kDebug() << "connected...";
        listAppeared();
        return true;
    }

    return false;
}

QString NetworkManagementEngine::sourceForActivatable(RemoteActivatable* remote)
{
    return d->sources[remote];
}

void NetworkManagementEngine::activatableAdded(RemoteActivatable* remote)
{
    d->i++;
    kDebug() << "activatableAdded" << d->i << "/" << d->activatables->activatables().count();
    QString source = sourceForActivatable(remote);

    //setData(source, "someRandomString", "thrilleeeeeeeerrrr!");
    d->sources[remote] = sourceForActivatable(remote);

    switch (remote->activatableType()) {
        case Knm::Activatable::WirelessNetwork:
        case Knm::Activatable::WirelessInterfaceConnection:
        { // Wireless
            RemoteWirelessNetwork* rwn = static_cast<RemoteWirelessNetwork*>(remote);
            WirelessStatus* wirelessStatus = new WirelessStatus(rwn);
            wirelessStatus->setParent(remote);
            d->sources[remote] = wirelessStatus->ssid();
            connect(wirelessStatus, SIGNAL(strengthChanged(int)), this, SLOT(updateWirelessStrength(int)));
            connect(rwn, SIGNAL(changed()), SLOT(updateWireless()));
            QString source = sourceForActivatable(rwn);
            updateConnection(sourceForActivatable(remote), remote);
            updateWireless(sourceForActivatable(remote), wirelessStatus);
            break;
        }
        case Knm::Activatable::InterfaceConnection:
        case Knm::Activatable::VpnInterfaceConnection:
        {
            //ai = new InterfaceConnectionItem(static_cast<RemoteInterfaceConnection*>(activatable), m_widget);
            break;
        }
        case Knm::Activatable::HiddenWirelessInterfaceConnection:
        {
            kWarning() << "This is handled differently, this codepath should be disabled.";
            //ai = new HiddenWirelessNetworkItem(static_cast<RemoteInterfaceConnection*>(activatable), m_widget);
            break;
        }
#ifdef COMPILE_MODEM_MANAGER_SUPPORT
        case Knm::Activatable::GsmInterfaceConnection:
        { // Gsm (2G, 3G, etc)
            //GsmInterfaceConnectionItem* gici = new GsmInterfaceConnectionItem(static_cast<RemoteGsmInterfaceConnection*>(activatable), m_widget);
            //ai = gici;
            break;
        }
#endif
        default:
            break;
    }

    scheduleSourcesUpdated();
}

void NetworkManagementEngine::updateConnection(const QString &source, RemoteActivatable* remote)
{
    RemoteInterfaceConnection* remoteconnection = dynamic_cast<RemoteInterfaceConnection*>(remote);
    if (!remoteconnection) {
        kDebug() << "cast RemoteActivatable -> RemoteConnection failed";
        return;
    }

    // set data
    // see libs/client/interfaceconnectioninterface.h

    //RemoteInterfaceConnection *remoteconnection = interfaceConnection();
    connect(remoteconnection, SIGNAL(hasDefaultRouteChanged(bool)),
            SLOT(hasDefaultRouteChanged(bool)));
    connect(remoteconnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
            SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState)));
    QString _state("Empty");
    switch (remoteconnection->activationState()) {
        //Knm::InterfaceConnectihon::ActivationState
        case Knm::InterfaceConnection::Activated:
            _state = "Activated";
            break;
        case Knm::InterfaceConnection::Unknown:
            _state = "Unknown";
            break;
        case Knm::InterfaceConnection::Activating:
            _state = "Activating";
            break;
    }
    setData(source, "activationState", _state);
    setData(source, "hasDefaultRoute", remoteconnection->hasDefaultRoute());


    scheduleSourcesUpdated();
}

void NetworkManagementEngine::activationStateChanged(Knm::InterfaceConnection::ActivationState)
{
    kDebug() << "activation state changed";
    RemoteActivatable* remote = static_cast<RemoteActivatable*>(sender());
    if (remote) {
        updateConnection(sourceForActivatable(remote), remote);
    }
}

void NetworkManagementEngine::hasDefaultRouteChanged(bool)
{
    //kDebug() << "hasdefaultreoute changed";
    RemoteActivatable* remote = static_cast<RemoteActivatable*>(sender());
    if (remote) {
        updateConnection(sourceForActivatable(remote), remote);
    }
}

//SLOT(handleHasDefaultRouteChanged(bool))

void NetworkManagementEngine::updateWireless()
{
    RemoteActivatable* remote = static_cast<RemoteActivatable*>(sender());
    if (!remote) {
        return; // wrong sender, shouldn't happen but better safe than sorry
    }
    RemoteWirelessNetwork* rwn = static_cast<RemoteWirelessNetwork*>(remote);
    if (!rwn) {

    }
    WirelessStatus* wirelessStatus = new WirelessStatus(rwn); // leak!!!
    wirelessStatus->setParent(remote);
    updateWireless(sourceForActivatable(remote), wirelessStatus);
}

void NetworkManagementEngine::updateWirelessStrength(int s)
{
    setData("networkStatus", "signalStrength", s);
    scheduleSourcesUpdated();
}

void NetworkManagementEngine::updateWireless(const QString &source, WirelessStatus *wirelessStatus)
{
    setData(source, "connectionName", wirelessStatus->ssid());
    setData(source, "connectionType", "Wireless");
    setData(source, "ssid", wirelessStatus->ssid());
    setData(source, "securityToolTip", wirelessStatus->securityTooltip());
    setData(source, "securityIcon", wirelessStatus->securityIcon());
    setData(source, "adhoc", wirelessStatus->isAdhoc());
}

void NetworkManagementEngine::activatableRemoved(RemoteActivatable* remote)
{
    kDebug() << "activatableRemoved" << d->activatables->activatables().count() << d->sources[remote];
    d->i--;
    removeSource(d->sources[remote]);
    d->sources.remove(remote);
    scheduleSourcesUpdated();
}

void NetworkManagementEngine::listDisappeared()
{
    kDebug() << "list disappeared" << d->activatables->activatables().count();
}

void NetworkManagementEngine::listAppeared()
{
    kDebug() << "list appeared" << d->activatables->activatables().count();
    foreach (RemoteActivatable* remote, d->activatables->activatables()) {
        activatableAdded(remote);
    }
    //scheduleSourcesUpdated();
}

void NetworkManagementEngine::addActivatable(RemoteActivatable* remote)
{

}

void NetworkManagementEngine::updateActivatable(RemoteActivatable* remote)
{

}


void NetworkManagementEngine::addInterfaceConnection(RemoteActivatable* remote)
{

}

void NetworkManagementEngine::updateInterfaceConnection(RemoteActivatable* remote)
{

}

void NetworkManagementEngine::addWirelessInterfaceConnection(RemoteActivatable* remote)
{

}

void NetworkManagementEngine::updateWirelessInterfaceConnection(RemoteActivatable* remote)
{

}


void NetworkManagementEngine::addRemoteWirelessNetwork(RemoteActivatable* remote)
{

}

void NetworkManagementEngine::updateRemoteWirelessNetwork(RemoteActivatable* remote)
{

}


//void addHiddenWirelessInterfaceConnection(RemoteHiddenWirelessInterfaceConnection* remote);
//void updateHiddenWirelessInterfaceConnection(RemoteHiddenWirelessInterfaceConnection* remote);

void NetworkManagementEngine::addUnconfiguredInterface(RemoteActivatable* remote)
{

}

void NetworkManagementEngine::updateUnconfiguredInterface(RemoteActivatable* remote)
{

}


void NetworkManagementEngine::addVpnInterfaceConnection(RemoteActivatable* remote)
{

}

void NetworkManagementEngine::updateVpnInterfaceConnection(RemoteActivatable* remote)
{

}


#ifdef COMPILE_MODEM_MANAGER_SUPPORT

void NetworkManagementEngine::addGsmInterfaceConnection(RemoteActivatable* remote)
{

}

void NetworkManagementEngine::updateGsmInterfaceConnection(RemoteActivatable* remote)
{

}

#endif



#include "networkmanagementengine.moc"
