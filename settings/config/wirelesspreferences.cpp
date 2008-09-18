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

#include "wirelesspreferences.h"

#include <QVBoxLayout>
#include <QFile>

#include <KPluginFactory>
#include <KTabWidget>

#include "configxml.h"
#include "secretstoragehelper.h"
#include "802_11_wirelesswidget.h"
#include "802_11_wireless_security_widget.h"
#include "ipv4widget.h"
#include "connectionwidget.h"

//K_PLUGIN_FACTORY( WirelessPreferencesFactory, registerPlugin<WirelessPreferences>();)
//K_EXPORT_PLUGIN( WirelessPreferencesFactory( "kcm_knetworkmanager_wireless" ) )

WirelessPreferences::WirelessPreferences(QWidget *parent, const QVariantList &args)
: ConnectionPreferences( KGlobal::mainComponent(), parent, args )
{
    Q_ASSERT(args.count() == 1);
    QString connectionId = args[0].toString();
    QVBoxLayout * layout = new QVBoxLayout(this);
    ConnectionWidget * contents = new ConnectionWidget(connectionId, this);
    layout->addWidget(contents);
    Wireless80211Widget * wirelessWidget = new Wireless80211Widget(connectionId, this);
    Wireless80211SecurityWidget * wirelessSecurityWidget = new Wireless80211SecurityWidget(connectionId, this);
    IpV4Widget * ipv4Widget = new IpV4Widget(connectionId, this);
    // Must setup initial widget 
    addConfig(contents->configXml(), contents);

    contents->connectionSettingsWidget()->addTab(wirelessWidget,wirelessWidget->label());
    contents->connectionSettingsWidget()->addTab(wirelessSecurityWidget,wirelessSecurityWidget->label());
    contents->connectionSettingsWidget()->addTab(ipv4Widget,ipv4Widget->label());
    addConfig(wirelessWidget->configXml(), wirelessWidget);
    addConfig(wirelessSecurityWidget->configXml(), wirelessSecurityWidget);
    addConfig(ipv4Widget->configXml(), ipv4Widget);
}

WirelessPreferences::~WirelessPreferences()
{
}

void WirelessPreferences::load()
{
    KCModule::load();
}

void WirelessPreferences::save()
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
