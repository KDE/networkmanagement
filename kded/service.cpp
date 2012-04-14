/*
Copyright 2009 Dario Freddi <drf54321@gmail.com>
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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

#include "service.h"
#include "paths.h"
#include "knmserviceprefs.h"

#include <KCModuleInfo>
#include <KDebug>

#include <connectionlist.h>
#include <secretstorage.h>
#include <activatablelist.h>
#include <activatabledebug.h>
#include <connectionusagemonitor.h>
#include <configurationlauncher.h>
#include <networkinterfacemonitor.h>
#include <sortedactivatablelist.h>
#include <vpninterfaceconnectionprovider.h>
#include <notificationmanager.h>

#include <nmdbussecretagent.h>
#include <nmdbusactiveconnectionmonitor.h>
#include <nmdbussettingsconnectionprovider.h>
#include <nm08connections.h>
#include <sessionabstractedservice.h>

K_PLUGIN_FACTORY(NetworkManagementServiceFactory,
                 registerPlugin<NetworkManagementService>();
    )
K_EXPORT_PLUGIN(NetworkManagementServiceFactory("networkmanagement", "libknetworkmanager"))

class NetworkManagementServicePrivate
{
public:
    // the most basic object
    ConnectionList * connectionList;
    // secrets storage
    SecretStorage * secretStorage;
    // list of things to show in the UI
    ActivatableList * activatableList;
    // creates Activatables based on the state of network interfaces
    NetworkInterfaceMonitor * networkInterfaceMonitor;
    // NetworkManager secrets agent
    NMDBusSecretAgent * nmDBusSecretAgent;
    // update interfaceconnections with status info from NetworkManager
    NMDBusActiveConnectionMonitor * nmActiveConnectionMonitor;
    // get connections from NM's service
    NMDBusSettingsConnectionProvider * nmDBusConnectionProvider;
    // sets up wireless networks on click and unconfigured devices of any type
    ConfigurationLauncher * configurationLauncher;
    // update connections as they are used
    ConnectionUsageMonitor * connectionUsageMonitor;
    // create Activatables for VPN connections
    VpnInterfaceConnectionProvider * vpnInterfaceConnectionProvider;

    // sort it
    SortedActivatableList * sortedList;

    SessionAbstractedService * sessionAbstractedService;

    NotificationManager * notificationManager;

    Nm08Connections * nm08Connections;
};

NetworkManagementService::NetworkManagementService(QObject * parent, const QVariantList&)
        : KDEDModule(parent), d_ptr(new NetworkManagementServicePrivate)
{
    Q_D(NetworkManagementService);
    KNetworkManagerServicePrefs::instance(Knm::NETWORKMANAGEMENT_RCFILE);

    Solid::Control::NetworkInterfaceNm09::Types types =
        (Solid::Control::NetworkInterfaceNm09::Ethernet
         | Solid::Control::NetworkInterfaceNm09::Wifi
         | Solid::Control::NetworkInterfaceNm09::Modem
         | Solid::Control::NetworkInterfaceNm09::Bluetooth
         );

    d->sortedList = new SortedActivatableList(types, this);

    d->sessionAbstractedService = new SessionAbstractedService(d->sortedList, this);
    d->sortedList->registerObserver(d->sessionAbstractedService);

    d->notificationManager = 0;
    connect(d->sessionAbstractedService, SIGNAL(DoFinishInitialization()), SLOT(finishInitialization()));
    QTimer::singleShot(1000, d->sessionAbstractedService, SIGNAL(ModuleReady()));
}

NetworkManagementService::~NetworkManagementService()
{
}

void NetworkManagementService::finishInitialization()
{
    Q_D(NetworkManagementService);

    if (d->notificationManager) {
        return;
    }

    QObject::disconnect(d->sessionAbstractedService, SIGNAL(DoFinishInitialization()), this, 0);

    d->connectionList = new ConnectionList(this);
 
    // watches events and creates KNotifications
    d->notificationManager = new NotificationManager(d->connectionList, this);
    d->activatableList = new ActivatableList(d->connectionList);

    d->activatableList->registerObserver(d->notificationManager);
    d->activatableList->registerObserver(d->sortedList);

    // debug activatable changes
    //ActivatableDebug debug;
    //activatableList->registerObserver(&debug);

    d->secretStorage = new SecretStorage(this);
    d->configurationLauncher = new ConfigurationLauncher(this);
    d->connectionUsageMonitor = new ConnectionUsageMonitor(d->connectionList, d->activatableList, d->activatableList);

    d->vpnInterfaceConnectionProvider = new VpnInterfaceConnectionProvider(d->connectionList, d->activatableList, d->activatableList);
    d->connectionList->registerConnectionHandler(d->vpnInterfaceConnectionProvider);

    d->nmDBusConnectionProvider = new NMDBusSettingsConnectionProvider(d->connectionList, d->connectionList);
    d->nmDBusSecretAgent = new NMDBusSecretAgent(this);
    d->nmDBusSecretAgent->registerSecretsProvider(d->secretStorage);

    // generic observers
    d->activatableList->registerObserver(d->configurationLauncher);
    d->activatableList->registerObserver(d->connectionUsageMonitor);

    d->activatableList->registerObserver(d->nmDBusConnectionProvider);

    // there is a problem setting this as a child of connectionList or of activatableList since it has
    // references to both and NetworkInterfaceActivatableProvider touches the activatableList
    // in its dtor (needed so it cleans up when removed by the monitor)
    // ideally this will always be deleted before the other list
    d->networkInterfaceMonitor = new NetworkInterfaceMonitor(d->connectionList, d->activatableList, d->activatableList);

    d->nm08Connections = new Nm08Connections(d->secretStorage, d->nmDBusConnectionProvider);
    d->nm08Connections->importNextNm08Connection();

    // give some time for the ActivatableAdded signal be processed before we emit the activationStateChanged
    // and hasDefaultRouteChanged signals from d->nmActiveConnectionMonitor.
    QTimer::singleShot(1000, this, SLOT(delayedRegisterObservers()));
}

void NetworkManagementService::delayedRegisterObservers()
{
    Q_D(NetworkManagementService);

    // create ActiveConnectionMonitor after construction of NMDBusSettingsConnectionProvider and observer registrations
    // because, activatableList is filled in NetworkManager::DeviceMonitor and updated in registerObservers above. This is why "Auto eth0" connection created automatically by NM has
    // Unknown activationState in its /org/kde/networkmanagement/Activatable interface
    d->nmActiveConnectionMonitor = new NMDBusActiveConnectionMonitor(d->activatableList, d->nmDBusConnectionProvider);

    // register after nmSettingsService and nmDBusConnectionProvider because it relies on changes they
    // make to interfaceconnections
    d->activatableList->registerObserver(d->nmActiveConnectionMonitor);
}
