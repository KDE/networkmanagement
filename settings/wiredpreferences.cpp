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

#include "wiredpreferences.h"

#include <QFile>
#include <KPluginFactory>
#include "networksettings.h"
#include "ui_connection.h"
#include "ui_wired.h"
#include "ui_ipv4.h"
#include "settings/configxml.h"
#include "../secretstoragehelper.h"

K_PLUGIN_FACTORY( WiredPreferencesFactory, registerPlugin<WiredPreferences>();)
K_EXPORT_PLUGIN( WiredPreferencesFactory( "kcm_knetworkmanager_wired" ) )

WiredPreferences::WiredPreferences(QWidget *parent, const QVariantList &args)
: KCModule( WiredPreferencesFactory::componentData(), parent, args )
{
    QWidget * contents = new QWidget(this);
    QWidget * wiredWidget = new QWidget;
    QWidget * ipv4Widget = new QWidget;
    Ui_ConnectionSettings connUi;
    Ui_Settings8023Ethernet wiredUi;
    Ui_SettingsIp4Config ipv4Ui;
    connUi.setupUi(contents);
    wiredUi.setupUi(wiredWidget);
    ipv4Ui.setupUi(ipv4Widget);
    // need to do this BEFORE adding the tabs or KConfigDialogManager tries to handle child widgets
    // managed independently
    QFile * connectionXml = new QFile("settings/connection.kcfg");
    ConfigXml * config = new ConfigXml("/tmp/testconfigxmlrc", connectionXml, new SecretStorageHelper("testconfigxml", "connection"));
    addConfig(config, contents);

    connUi.tabwidget->addTab(wiredWidget,QLatin1String("Ethernet"));
    connUi.tabwidget->addTab(ipv4Widget,QLatin1String("IP"));

    QFile * ethernetXml = new QFile("settings/802-3-ethernet.kcfg");
    QFile * ipv4Xml = new QFile("settings/ipv4.kcfg");
    config = new ConfigXml("/tmp/testconfigxmlrc", ethernetXml, new SecretStorageHelper("testconfigxml", "802-3-ethernet"));
    addConfig(config, wiredWidget);
    config = new ConfigXml("/tmp/testconfigxmlrc", ipv4Xml, new SecretStorageHelper("testconfigxml", "ipv4"));
    addConfig(config, ipv4Widget);
}

WiredPreferences::~WiredPreferences()
{
}

void WiredPreferences::load()
{
    KCModule::load();
}

void WiredPreferences::save()
{
    KCModule::save();
    // this is where tab specific stuff should happen?
    // that should be in the shared config widget code not connection code, as groups are shared.
    // editing existing connections
    // creating new connection
    // popup to prompt for single missing secret
    // interaction between tray and kcm
    //   tray: new connection: launch kcm
    //   tray: Edit connections?
    //   Enable connection - does this need to go through UserSettingsService
    //   Enable wireless
    // interaction between kcm and service
    // interaction between tray and service
    // location of service (in-tray, in plasma)
    //
}

// vim: sw=4 sts=4 et tw=100
