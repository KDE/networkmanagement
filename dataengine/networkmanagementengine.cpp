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
#include "remotewirelessnetwork.h"
#include "remotegsminterfaceconnection.h"

#include "../applet/wirelessstatus.h"


class NetworkManagementEnginePrivate
{
public:
    RemoteActivatableList* activatables;
    QHash<RemoteActivatable*, QString> sources;
    int i;
};


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


void NetworkManagementEngine::updateWireless()
{
    RemoteActivatable* remote = static_cast<RemoteActivatable*>(sender());
    if (!remote) {
        return; // wrong sender, shouldn't happen but better safe than sorry
    }
    RemoteWirelessNetwork* rwn = static_cast<RemoteWirelessNetwork*>(remote);
    if (!rwn) {

    }
    WirelessStatus* wirelessStatus = new WirelessStatus(rwn);
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
    QString _s = wirelessStatus->ssid();
    setData(_s, "connectionName", wirelessStatus->ssid());
    setData(_s, "connectionType", "Wireless");
    setData(_s, "ssid", wirelessStatus->ssid());
    setData(_s, "securityToolTip", wirelessStatus->securityTooltip());
    setData(_s, "securityIcon", wirelessStatus->securityIcon());
    setData(_s, "adhoc", wirelessStatus->isAdhoc());
    scheduleSourcesUpdated();
    /*
    RemoteInterfaceConnection *remoteconnection = interfaceConnection();
    if (remoteconnection) {
        connect(remoteconnection, SIGNAL(hasDefaultRouteChanged(bool)),
                SLOT(handleHasDefaultRouteChanged(bool)));
        connect(remoteconnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
                SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState)));
    }

    switch (state) {
        //Knm::InterfaceConnectihon::ActivationState
        case Knm::InterfaceConnection::Activated:
            kDebug() << "activated";
            //f.setBold(true);
            //f.setItalic(false);
            break;
        case Knm::InterfaceConnection::Unknown:
            kDebug() << "unknown";
            //f.setBold(false);
            //f.setItalic(false);
            break;
        case Knm::InterfaceConnection::Activating:
            kDebug() << "activatING....";
            //f.setBold(false);
            //f.setItalic(true);
    }
    */

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

#include "networkmanagementengine.moc"
