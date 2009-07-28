/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "monolithic.h"

#include <KDebug>
#include <KLocale>

#include <solid/control/networkinterface.h>

#include <knmserviceprefs.h>
#include <connectionlist.h>
#include <connectionlistpersistence.h>
#include <connectionlistpersistencedbus.h>
#include <activatablelist.h>
#include <activatabledebug.h>
#include <connectionusagemonitor.h>
#include <configurationlauncher.h>
#include <networkinterfacemonitor.h>
#include <notificationmanager.h>
#include <vpninterfaceconnectionprovider.h>

#include <nmdbussettingsservice.h>
#include <nmdbusactiveconnectionmonitor.h>
#include <nmdbussettingsconnectionprovider.h>

#include <sortedactivatablelist.h>

#include "knetworkmanagertrayicon.h"

class MonolithicPrivate
{
public:
    QList<SortedActivatableList*> trayIconLists;
    ActivatableList * activatableList;
    NMDBusSettingsService * nmSettingsService;
    bool autostart;
};

Monolithic::Monolithic()
: KUniqueApplication(), d_ptr(new MonolithicPrivate)
{
}

Monolithic::~Monolithic()
{
    delete d_ptr;
}

void Monolithic::init()
{
    Q_D(Monolithic);

    disableSessionManagement();

    // the most basic object
    ConnectionList * connectionList;
    // its loader/saver
    ConnectionListPersistence * listPersistence;
    // its dbus presence
    ConnectionListPersistenceDBus * sessionDbusConfigureInterface;
    // list of things to show in the UI
    // creates Activatables based on the state of network interfaces
    NetworkInterfaceMonitor * networkInterfaceMonitor;
    // NetworkManager settings service
    // also calls NetworkManager via Solid when connections clicked
    // update interfaceconnections with status info from NetworkManager
    NMDBusActiveConnectionMonitor * nmActiveConnectionMonitor;
    // get connections from NM's service
    NMDBusSettingsConnectionProvider * nmDBusConnectionProvider;
    // sets up wireless networks on click and unconfigured devices of any type
    ConfigurationLauncher * configurationLauncher;
    // update connections as they are used
    ConnectionUsageMonitor * connectionUsageMonitor;
    // watches events and creates KNotifications
    NotificationManager * notificationManager;
    // create Activatables for VPN connections
    VpnInterfaceConnectionProvider * vpnInterfaceConnectionProvider;

    connectionList = new ConnectionList(this);
    listPersistence = new ConnectionListPersistence(connectionList);

    d->nmSettingsService = new NMDBusSettingsService(connectionList);

    connectionList->registerConnectionHandler(listPersistence);
    connectionList->registerConnectionHandler(d->nmSettingsService);
    connectionList->registerConnectionHandler(d->nmSettingsService);


    d->activatableList = new ActivatableList(connectionList);

    sessionDbusConfigureInterface = new ConnectionListPersistenceDBus(listPersistence, listPersistence);

    configurationLauncher = new ConfigurationLauncher(this);
    connectionUsageMonitor = new ConnectionUsageMonitor(connectionList, d->activatableList, d->activatableList);

    vpnInterfaceConnectionProvider = new VpnInterfaceConnectionProvider(connectionList, d->activatableList, d->activatableList);
    connectionList->registerConnectionHandler(vpnInterfaceConnectionProvider);

    notificationManager = new NotificationManager(this);

    nmDBusConnectionProvider = new NMDBusSettingsConnectionProvider(connectionList, NMDBusSettingsService::SERVICE_SYSTEM_SETTINGS, connectionList);

    // there is a problem setting this as a child of connectionList or of activatableList since it has
    // references to both and NetworkInterfaceActivatableProvider touches the activatableList
    // in its dtor (needed so it cleans up when removed by the monitor)
    // ideally this will always be deleted before the other list
    networkInterfaceMonitor = new NetworkInterfaceMonitor(connectionList, d->activatableList, d->activatableList);

    // generic observers
    d->activatableList->registerObserver(configurationLauncher);
    d->activatableList->registerObserver(connectionUsageMonitor);
    d->activatableList->registerObserver(notificationManager);

    d->activatableList->registerObserver(d->nmSettingsService);
    d->activatableList->registerObserver(nmDBusConnectionProvider);


    // instantiate and register after d->nmSettingsService and nmDBusConnectionProvider because it relies on changes they
    // make to interfaceconnections
    nmActiveConnectionMonitor = new NMDBusActiveConnectionMonitor(d->activatableList, d->nmSettingsService);
    d->activatableList->registerObserver(nmActiveConnectionMonitor);

    // debug activatable changes
    ActivatableDebug * debug = new ActivatableDebug;
    debug->setObjectName("unsorted");
    d->activatableList->registerObserver(debug);

    // load our local connections
    listPersistence->init();

    QDBusConnection::sessionBus().registerService("org.kde.knetworkmanager");
    QDBusConnection::sessionBus().registerObject("/tray", this, QDBusConnection::ExportScriptableSlots);

    d->autostart = KNetworkManagerServicePrefs::self()->autostart();

    createTrayIcons();
}

void Monolithic::createTrayIcons()
{
    Q_D(Monolithic);
    // get rid of any sorted lists and the icons they control
    foreach (SortedActivatableList * list, d->trayIconLists) {
        kDebug() << "unregistering sorted list observer";
        d->activatableList->unregisterObserver(list);
    }
    qDeleteAll(d->trayIconLists);
    d->trayIconLists.clear();

    KNetworkManagerServicePrefs::self()->readConfig();

    for (uint i = 0; i < KNetworkManagerServicePrefs::self()->iconCount(); ++i) {
        Solid::Control::NetworkInterface::Types types(KNetworkManagerServicePrefs::self()->iconTypes(i));

        SortedActivatableList * sortedList = new SortedActivatableList(types, 0);
        d->activatableList->registerObserver(sortedList);
#if 0
        // for debugging the sorted list
        ActivatableDebug * debug = new ActivatableDebug;

        debug->setObjectName("SORTED");
        sortedList->registerObserver(debug);
        kDebug() << "REGISTERED NEW SORTED LIST";
#endif
        KNetworkManagerTrayIcon * simpleUi = new KNetworkManagerTrayIcon(types, QString::number(types),
                sortedList, d->nmSettingsService->isServiceAvailable(), sortedList);

        QObject::connect(d->nmSettingsService, SIGNAL(serviceAvailable(bool)), simpleUi, SLOT(setActive(bool)));
        sortedList->registerObserver(simpleUi);
        d->trayIconLists.append(sortedList);
    }
}


void Monolithic::reloadConfig()
{
    Q_D(Monolithic);
    kDebug();
    createTrayIcons();
    if (d->autostart && !KNetworkManagerServicePrefs::self()->autostart()) {
        kapp->quit();
    }
    d->autostart = KNetworkManagerServicePrefs::self()->autostart();
}

