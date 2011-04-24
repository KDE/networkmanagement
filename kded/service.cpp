/*
Copyright 2009 Dario Freddi <drf54321@gmail.com>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include <KCModuleInfo>

#include <connectionlist.h>
#include <connectionlistpersistence.h>
#include <connectionlistpersistencedbus.h>
#include <activatablelist.h>
#include <activatabledebug.h>
#include <connectionusagemonitor.h>
#include <configurationlauncher.h>
#include <networkinterfacemonitor.h>
#include <sortedactivatablelist.h>
#include <vpninterfaceconnectionprovider.h>
#include <notificationmanager.h>

#include <nmdbussettingsservice.h>
#include <nmdbusactiveconnectionmonitor.h>
#include <nmdbussettingsconnectionprovider.h>

#include <sessionabstractedservice.h>

K_PLUGIN_FACTORY(NetworkManagementServiceFactory,
                 registerPlugin<NetworkManagementService>();
    )
K_EXPORT_PLUGIN(NetworkManagementServiceFactory("networkmanagement"))

class NetworkManagementServicePrivate
{
public:
    // the most basic object
    ConnectionList * connectionList;
    // its loader/saver
    ConnectionListPersistence * listPersistence;
    // its dbus presence
    ConnectionListPersistenceDBus * sessionDbusConfigureInterface;
    // list of things to show in the UI
    ActivatableList * activatableList;
    // creates Activatables based on the state of network interfaces
    NetworkInterfaceMonitor * networkInterfaceMonitor;
    // NetworkManager settings service
    // also calls NetworkManager via Solid when connections clicked
    NMDBusSettingsService * nmSettingsService;
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
};

NetworkManagementService::NetworkManagementService(QObject * parent, const QVariantList&)
        : KDEDModule(parent), d_ptr(new NetworkManagementServicePrivate)
{
    Q_D(NetworkManagementService);
    d->connectionList = new ConnectionList(this);
    d->listPersistence = new ConnectionListPersistence(d->connectionList);

    d->nmSettingsService = new NMDBusSettingsService(d->connectionList);

    d->connectionList->registerConnectionHandler(d->listPersistence);
    d->connectionList->registerConnectionHandler(d->nmSettingsService);


    d->activatableList = new ActivatableList(d->connectionList);

    d->sessionDbusConfigureInterface = new ConnectionListPersistenceDBus(d->listPersistence, d->listPersistence);

    d->configurationLauncher = new ConfigurationLauncher(this);
    d->connectionUsageMonitor = new ConnectionUsageMonitor(d->connectionList, d->activatableList, d->activatableList);

    d->vpnInterfaceConnectionProvider = new VpnInterfaceConnectionProvider(d->connectionList, d->activatableList, d->activatableList);
    d->connectionList->registerConnectionHandler(d->vpnInterfaceConnectionProvider);

    // watches events and creates KNotifications
    d->notificationManager = new NotificationManager(this);

    d->nmDBusConnectionProvider = new NMDBusSettingsConnectionProvider(d->connectionList, NMDBusSettingsService::SERVICE_SYSTEM_SETTINGS, d->connectionList);

    // generic observers
    d->activatableList->registerObserver(d->configurationLauncher);
    d->activatableList->registerObserver(d->connectionUsageMonitor);

    d->activatableList->registerObserver(d->nmSettingsService);
    d->activatableList->registerObserver(d->nmDBusConnectionProvider);
    d->activatableList->registerObserver(d->notificationManager);

    // create ActiveConnectionMonitor after construction of NMDBusSettingsConnectionProvider and observer registrations 
    // because, activatableList is filled in NetworkInterfaceMonitor and updated in registerObservers above. This is why "Auto eth0" connection created automatically by NM has 
    // Unknown activationState in its /org/kde/networkmanagement/Activatable interface
    d->nmActiveConnectionMonitor = new NMDBusActiveConnectionMonitor(d->activatableList, d->nmSettingsService);

    // register after nmSettingsService and nmDBusConnectionProvider because it relies on changes they
    // make to interfaceconnections
    d->activatableList->registerObserver(d->nmActiveConnectionMonitor);

    // debug activatable changes
    //ActivatableDebug debug;
    //activatableList->registerObserver(&debug);
    
    Solid::Control::NetworkInterface::Types types =
        (Solid::Control::NetworkInterface::Ieee8023
         | Solid::Control::NetworkInterface::Ieee80211
         | Solid::Control::NetworkInterface::Serial
         | Solid::Control::NetworkInterface::Gsm
         | Solid::Control::NetworkInterface::Cdma
#ifdef NM_0_8
         | Solid::Control::NetworkInterface::Bluetooth
#endif
	 );

    d->sortedList = new SortedActivatableList(types, this);
    d->activatableList->registerObserver(d->sortedList);

    d->sessionAbstractedService = new SessionAbstractedService(d->sortedList, this);
    d->sortedList->registerObserver(d->sessionAbstractedService);

    // load our local connections
    d->listPersistence->init();

    // there is a problem setting this as a child of connectionList or of activatableList since it has
    // references to both and NetworkInterfaceActivatableProvider touches the activatableList
    // in its dtor (needed so it cleans up when removed by the monitor)
    // ideally this will always be deleted before the other list
    d->networkInterfaceMonitor = new NetworkInterfaceMonitor(d->connectionList, d->activatableList, d->activatableList);
}


NetworkManagementService::~NetworkManagementService()
{
}
