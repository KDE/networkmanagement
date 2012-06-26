/*
    Copyright 2010 Sebastian Kügler <sebas@kde.org>

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
    QHash<RemoteActivatable*, WirelessStatus*> wirelessStatus;
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
    delete d;
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
                SLOT(activatableAdded(RemoteActivatable*)));
        connect(d->activatables, SIGNAL(activatableRemoved(RemoteActivatable*)),
                SLOT(activatableRemoved(RemoteActivatable*)));

        connect(d->activatables, SIGNAL(appeared()), SLOT(listAppeared()));
        connect(d->activatables, SIGNAL(disappeared()), SLOT(listDisappeared()));
        kDebug() << "connected...";
        listAppeared();
        return true;
    }

    return false;
}

QString NetworkManagementEngine::source(RemoteActivatable* remote)
{
    return d->sources[remote];
}

void NetworkManagementEngine::activatableAdded(RemoteActivatable* remote)
{
    //d->i++;
    kDebug() << "activatableAdded" << d->i << "/" << d->activatables->activatables().count();
    if (d->sources.keys().contains(remote)) {
        kDebug() << "not adding twice:" << source(remote);
    }
    addActivatable(remote);

    switch (remote->activatableType()) {
        case Knm::Activatable::WirelessNetwork:
        {
            addWirelessNetwork(remote);
            break;
        }
        case Knm::Activatable::WirelessInterfaceConnection:
        { // Wireless
            addWirelessInterfaceConnection(remote);
            break;
        }
        case Knm::Activatable::InterfaceConnection:
        {
            addInterfaceConnection(remote);
            break;
        }
        case Knm::Activatable::VpnInterfaceConnection:
        {
            addVpnInterfaceConnection(remote);
            break;
        }
        case Knm::Activatable::HiddenWirelessInterfaceConnection:
        {
            kWarning() << "Hidden wireless networks not supported :/";
            addHiddenWirelessInterfaceConnection(remote);
            break;
        }
        case Knm::Activatable::GsmInterfaceConnection:
        { // Gsm (2G, 3G, etc)
            addGsmInterfaceConnection(remote);
            break;
        }
        default:
        {
            addActivatable(remote);
            break;
        }
    }

    scheduleSourcesUpdated();
}

void NetworkManagementEngine::activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    Q_UNUSED(oldState);

    // FIXME: never trigged... ???
    kDebug() << "actstatechange";
    if (newState == Knm::InterfaceConnection::Activating) {
        kDebug() << "1ACTIVATING:";
    }
    RemoteInterfaceConnection* remote = static_cast<RemoteInterfaceConnection*>(sender());
    if (remote && newState == Knm::InterfaceConnection::Activating) {
        kDebug() << "2ACTIVATING:" << remote->connectionName();
    }
}

void NetworkManagementEngine::activatableRemoved(RemoteActivatable* remote)
{
    kDebug() << "activatableRemoved" << d->activatables->activatables().count() << d->sources[remote];
    //d->i--;
    removeSource(source(remote));
    d->sources.remove(remote);
    d->wirelessStatus.remove(remote);
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
    // create a unique-per-source uuid and use that as source string
    QString uuid = QUuid::createUuid();
    RemoteInterfaceConnection* remoteconnection = qobject_cast<RemoteInterfaceConnection*>(remote);
    if (remoteconnection) {
        // Use the connection's uuid if it's available,
        // while this is technically not really necessary
        // it seems like a good idea to prevent confusion
        // between data sources and connection ids
        uuid = remoteconnection->connectionUuid();
    }
    while(d->sources.values().contains(uuid)) {
        uuid = QUuid::createUuid();
    }
    d->sources[remote] = uuid;

    setData(source(remote), "activatableType", "Activatable");
}

void NetworkManagementEngine::updateActivatable(RemoteActivatable* remote)
{
    if (!remote) {
        // handling for SLOT usage
        remote = qobject_cast<RemoteActivatable*>(sender());
    }

    setData(source(remote), "deviceUni", remote->deviceUni());
    scheduleSourcesUpdated();
}


void NetworkManagementEngine::addInterfaceConnection(RemoteActivatable* remote)
{
    kDebug() << "Adding and connecting InterfaceConnection";
    RemoteInterfaceConnection* remoteconnection = qobject_cast<RemoteInterfaceConnection*>(remote);
    if (!remoteconnection) {
        kDebug() << "cast RemoteActivatable -> RemoteConnection failed";
        return;
    }

    // this one's just for debugging, seems to get never called
    connect(remoteconnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
            SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));

    // connect remoteinterface for updates
    connect(remoteconnection, SIGNAL(hasDefaultRouteChanged(bool)),
            SLOT(updateInterfaceConnection()));
    connect(remoteconnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
            SLOT(updateInterfaceConnection()));

    updateActivatable(remote);
}

void NetworkManagementEngine::updateInterfaceConnection(RemoteActivatable* remote)
{
    if (!remote) {
        // handling for SLOT usage
        remote = qobject_cast<RemoteActivatable*>(sender());
    }
    if (!remote) {
        // invalid caller
        kDebug() << "invalid caller.";
        return;
    }
    updateActivatable(remote);
    RemoteInterfaceConnection* remoteconnection = qobject_cast<RemoteInterfaceConnection*>(remote);
    if (!remoteconnection) {
        kDebug() << "cast RemoteActivatable -> RemoteConnection failed";
        return;
    }
    kDebug() << "updating interface connection for" << remoteconnection->connectionName();
    setData(source(remote), "activatableType", "InterfaceConnection");
    setData(source(remote), "connectionUuid", remoteconnection->connectionUuid().toString());
    setData(source(remote), "connectionName", remoteconnection->connectionName());
    setData(source(remote), "iconName", remoteconnection->iconName());

    QString _state;
    switch (remoteconnection->activationState()) {
        //Knm::InterfaceConnectihon::ActivationState
        case Knm::InterfaceConnection::Activated:
            _state = "Activated";
            break;
        case Knm::InterfaceConnection::Unknown:
            _state = "Unknown";
            break;
        case Knm::InterfaceConnection::Activating:
            kDebug() << "Activating ........";
            _state = "Activating";
            break;
    }

    QString _type;
    switch (remoteconnection->connectionType()) {
        //Knm::InterfaceConnectihon::ActivationState
        case Knm::Connection::Wired:
            _type = "Wired";
            break;
        case Knm::Connection::Wireless:
            _type = "Wireless";
            break;
        case Knm::Connection::Gsm:
            _type = "Gsm";
            break;
        case Knm::Connection::Cdma:
            _type = "Cdma";
            break;
        case Knm::Connection::Vpn:
            _type = "Vpn";
            break;
        case Knm::Connection::Pppoe:
            _type = "Pppoe";
            break;
        case Knm::Connection::Bluetooth:
            /* TODO */
            kWarning() << "Unhandled type: Bluetooth";
            break;
        default:
            kWarning() << "Unhandled type" << remoteconnection->connectionType();
            break;
    }

    setData(source(remote), "connectionType", _type);
    setData(source(remote), "activationState", _state);
    setData(source(remote), "hasDefaultRoute", remoteconnection->hasDefaultRoute());

    scheduleSourcesUpdated();
}

void NetworkManagementEngine::addWirelessInterfaceConnection(RemoteActivatable* remote)
{
    addInterfaceConnection(remote);
    updateWirelessInterfaceConnection(remote);
}

void NetworkManagementEngine::updateWirelessInterfaceConnection(RemoteActivatable* remote)
{
    if (!remote) {
        // handling for SLOT usage
        remote = qobject_cast<RemoteActivatable*>(sender());
    }
    if (!remote) {
        // invalid caller
        return;
    }
    updateInterfaceConnection(remote);
    RemoteWirelessInterfaceConnection* remoteconnection = qobject_cast<RemoteWirelessInterfaceConnection*>(remote);
    if (!remoteconnection) {
        kDebug() << "cast RemoteActivatable -> RemoteConnection failed";
        return;
    }

    setData(source(remote), "activatableType", "WirelessInterfaceConnection");
    scheduleSourcesUpdated();
}


void NetworkManagementEngine::addWirelessNetwork(RemoteActivatable* remote)
{
    RemoteWirelessNetwork* rwn = static_cast<RemoteWirelessNetwork*>(remote);
    if (!rwn) {
        return;
    }

    WirelessStatus* wirelessStatus = new WirelessStatus(rwn);
    wirelessStatus->setParent(remote); // make sure it goes away when the remote does
    d->wirelessStatus[remote] = wirelessStatus;
    updateWirelessStatus(source(remote), wirelessStatus);
    connect(wirelessStatus, SIGNAL(strengthChanged(int)), SLOT(updateWirelessNetwork()));
    connect(rwn, SIGNAL(changed()), SLOT(updateWirelessNetwork()));
}

void NetworkManagementEngine::updateWirelessNetwork(RemoteActivatable* remote)
{
    if (!remote) {
        // handling for SLOT usage
        remote = qobject_cast<RemoteActivatable*>(sender());
    }
    if (!remote) {
        WirelessStatus* wirelessStatus = qobject_cast<WirelessStatus*>(sender());
        remote = wirelessStatus->activatable();
    }
    if (!remote) {
        kDebug() << "something wrong";
        return;
    }
    updateActivatable(remote);
    RemoteWirelessNetwork* remoteconnection = qobject_cast<RemoteWirelessNetwork*>(remote);
    if (!remoteconnection) {
        kDebug() << "cast RemoteActivatable -> RemoteWirelessNetwork failed";
        return;
    }
    updateWirelessStatus(source(remote), d->wirelessStatus[remote]);
    setData(source(remote), "activatableType", "WirelessNetwork");
    scheduleSourcesUpdated();
}

void NetworkManagementEngine::addHiddenWirelessInterfaceConnection(RemoteActivatable* remote)
{
    addWirelessInterfaceConnection(remote);
    updateHiddenWirelessInterfaceConnection(remote);
}

void NetworkManagementEngine::updateHiddenWirelessInterfaceConnection(RemoteActivatable* remote)
{
    if (!remote) {
        // handling for SLOT usage
        remote = qobject_cast<RemoteActivatable*>(sender());
    }
    if (!remote) {
        // invalid caller
        return;
    }
    updateActivatable(remote);
    setData(source(remote), "activatableType", "HiddenWirelessInterfaceConnection");
    scheduleSourcesUpdated();
}

void NetworkManagementEngine::updateWirelessStatus(const QString &source, WirelessStatus *wirelessStatus)
{
    if (!wirelessStatus) {
        kDebug() << "invalid wirelessStatus for " << source;
        return;
    }
    setData(source, "signalStrength", wirelessStatus->strength());
    setData(source, "ssid", wirelessStatus->ssid());
    setData(source, "securityToolTip", wirelessStatus->securityTooltip());
    setData(source, "securityIcon", wirelessStatus->securityIcon());
    setData(source, "adhoc", wirelessStatus->isAdhoc());
    scheduleSourcesUpdated();
}



void NetworkManagementEngine::addUnconfiguredInterface(RemoteActivatable* remote)
{

    updateUnconfiguredInterface(remote);
}

void NetworkManagementEngine::updateUnconfiguredInterface(RemoteActivatable* remote)
{
    if (!remote) {
        // handling for SLOT usage
        remote = qobject_cast<RemoteActivatable*>(sender());
    }
    if (!remote) {
        // invalid caller
        return;
    }
    updateActivatable(remote);
    setData(source(remote), "activatableType", "UnconfiguredInterface");

    scheduleSourcesUpdated();
}


void NetworkManagementEngine::addVpnInterfaceConnection(RemoteActivatable* remote)
{
    addInterfaceConnection(remote);
    updateVpnInterfaceConnection(remote);
}

void NetworkManagementEngine::updateVpnInterfaceConnection(RemoteActivatable* remote)
{
    if (!remote) {
        // handling for SLOT usage
        remote = qobject_cast<RemoteActivatable*>(sender());
    }
    if (!remote) {
        // invalid caller
        return;
    }
    updateInterfaceConnection(remote);
    setData(source(remote), "activatableType", "VpnInterfaceConnection");

    scheduleSourcesUpdated();
}


void NetworkManagementEngine::addGsmInterfaceConnection(RemoteActivatable* remote)
{
    addInterfaceConnection(remote);
    updateGsmInterfaceConnection(remote);
}

void NetworkManagementEngine::updateGsmInterfaceConnection(RemoteActivatable* remote)
{
    if (!remote) {
        // handling for SLOT usage
        remote = qobject_cast<RemoteActivatable*>(sender());
    }
    if (!remote) {
        // invalid caller
        return;
    }
    updateInterfaceConnection(remote);
    setData(source(remote), "activatableType", "GsmInterfaceConnection");

    scheduleSourcesUpdated();
}

#include "networkmanagementengine.moc"
