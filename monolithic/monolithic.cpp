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

#include "connectionlist.h"
#include "connectionlistpersistence.h"
#include "connectionlistpersistencedbus.h"
#include "activatabledebug.h"
#include "wirelessnetworkconfigurer.h"

#include "activatablelist.h"
#include "networkinterfacemonitor.h"

#include "nmdbussettingsservice.h"
#include "nmdbusactiveconnectionmonitor.h"

#include "simpleui.h"

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
    ConnectionListPersistenceDBus * dbus;
    // list of things to show in the UI
    ActivatableList * activatableList;
    // NetworkManager settings service
    NMDBusSettingsService * settingsService;
    NMDBusActiveConnectionMonitor * connectionMonitor;

    connectionList = new ConnectionList(&app);
    listPersistence = new ConnectionListPersistence(connectionList);
    settingsService = new NMDBusSettingsService(connectionList);

    connectionList->registerConnectionHandler(listPersistence);
    connectionList->registerConnectionHandler(settingsService);

    dbus = new ConnectionListPersistenceDBus(listPersistence, listPersistence);

    activatableList = new ActivatableList(connectionList);

    connectionMonitor = new NMDBusActiveConnectionMonitor(activatableList, settingsService);

    // debug activatable changes
    ActivatableDebug * debug = new ActivatableDebug(&app);
    activatableList->connectObserver(debug);

    // really simple UI
    SimpleUi * simpleUi = new SimpleUi(activatableList, &app);
    activatableList->connectObserver(simpleUi);

    // sets up wireless networks on click
    WirelessNetworkConfigurer * wirelessConfigurer = new WirelessNetworkConfigurer(&app);
    activatableList->connectObserver(wirelessConfigurer);

    // call NetworkManager when connections clicked
    activatableList->connectObserver(settingsService);

    listPersistence->init();
    new NetworkInterfaceMonitor(connectionList, activatableList, connectionList);
    return app.exec();
}


