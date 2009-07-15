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

#include <QtCore>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KDebug>
#include <KDialog>
#include <KLocale>

#include <solid/control/networkinterface.h>

#include <connectionlist.h>
#include <connectionlistpersistence.h>
#include <connectionlistpersistencedbus.h>
#include <activatablelist.h>
#include <activatabledebug.h>
#include <connectionusagemonitor.h>
#include <configurationlauncher.h>
#include <networkinterfacemonitor.h>

#include <nmdbussettingsservice.h>
#include <nmdbusactiveconnectionmonitor.h>
#include <nmdbussettingsconnectionprovider.h>

#include <sessionabstractedservice.h>

#include "knetworkmanagertrayicon.h"

static const char description[] =
    I18N_NOOP("KNetworkManager, the KDE 4 NetworkManager client");

static const char version[] = "v0.8";

int main( int argc, char** argv )
{
    KAboutData about("KNetworkManager", 0, ki18n("knetworkmanager"), version, ki18n(description), KAboutData::License_GPL, ki18n("(C) 2009 Will Stephenson"), KLocalizedString(), 0, "wstephenson@kde.org");
    about.addAuthor( ki18n("Will Stephenson"), KLocalizedString(), "wstephenson@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

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

    connectionList = new ConnectionList(&app);
    listPersistence = new ConnectionListPersistence(connectionList);

    nmSettingsService = new NMDBusSettingsService(connectionList);

    connectionList->registerConnectionHandler(listPersistence);
    connectionList->registerConnectionHandler(nmSettingsService);

    activatableList = new ActivatableList(connectionList);

    sessionDbusConfigureInterface = new ConnectionListPersistenceDBus(listPersistence, listPersistence);

    configurationLauncher = new ConfigurationLauncher(&app);
    connectionUsageMonitor = new ConnectionUsageMonitor(connectionList, activatableList, activatableList);

    nmDBusConnectionProvider = new NMDBusSettingsConnectionProvider(connectionList, NMDBusSettingsService::SERVICE_SYSTEM_SETTINGS, connectionList);
    nmActiveConnectionMonitor = new NMDBusActiveConnectionMonitor(activatableList, nmSettingsService);

    // there is a problem setting this as a child of connectionList or of activatableList since it has
    // references to both and NetworkInterfaceActivatableProvider touches the activatableList
    // in its dtor (needed so it cleans up when removed by the monitor)
    // ideally this will always be deleted before the other list
    networkInterfaceMonitor = new NetworkInterfaceMonitor(connectionList, activatableList, activatableList);

    // generic observers
    activatableList->registerObserver(configurationLauncher);
    activatableList->registerObserver(connectionUsageMonitor);

    activatableList->registerObserver(nmSettingsService);
    activatableList->registerObserver(nmDBusConnectionProvider);

    // register after nmSettingsService and nmDBusConnectionProvider because it relies on changes they
    // make to interfaceconnections
    activatableList->registerObserver(nmActiveConnectionMonitor);

    // debug activatable changes
    ActivatableDebug debug;
    activatableList->registerObserver(&debug);

    // really simple UI
    // register after everything except debug
    Solid::Control::NetworkInterface::Types types =
        (Solid::Control::NetworkInterface::Ieee8023
         | Solid::Control::NetworkInterface::Ieee80211
         | Solid::Control::NetworkInterface::Serial
         | Solid::Control::NetworkInterface::Gsm
         | Solid::Control::NetworkInterface::Cdma);

    //Solid::Control::NetworkInterface::Types types = (Solid::Control::NetworkInterface::Ieee8023);
    KNetworkManagerTrayIcon simpleUi(types, QString::number(types), activatableList, 0);
    activatableList->registerObserver(&simpleUi);

    //Solid::Control::NetworkInterface::Types secondTypes = (Solid::Control::NetworkInterface::Ieee80211);

    //KNetworkManagerTrayIcon secondTray(secondTypes, QString::number(secondTypes), activatableList, 0);
    //activatableList->registerObserver(&secondTray);


    // put the activatables on the session bus for external applets
    SessionAbstractedService * sessionAbstractedService = new SessionAbstractedService(&app);
    activatableList->registerObserver(sessionAbstractedService);

    // load our local connections
    listPersistence->init();

    int i = app.exec();
    // workaround KNotificationItem crashing when deleted from QCoreApp dtor
    return i;
}


