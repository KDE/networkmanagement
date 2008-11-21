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
#include <KDebug>
#include <KTabWidget>

#include "configxml.h"
#include "secretstoragehelper.h"
#include "802_11_wirelesswidget.h"
#include "security/802_11_wireless_security_widget.h"
#include "ipv4widget.h"
#include "connectionwidget.h"

//K_PLUGIN_FACTORY( WirelessPreferencesFactory, registerPlugin<WirelessPreferences>();)
//K_EXPORT_PLUGIN( WirelessPreferencesFactory( "kcm_knetworkmanager_wireless" ) )

WirelessPreferences::WirelessPreferences(QWidget *parent, const QVariantList &args)
: ConnectionPreferences( KGlobal::mainComponent(), parent, args )
{
    // at least 1
    Q_ASSERT(args.count());

    QString connectionId = args[0].toString();
    m_connectionType = "Wireless";

    QString ssid;
    uint caps = 0, wpa = 0, rsn = 0;
    if (args.count() == 5) {
        ssid = args[1].toString();
        caps = args[2].toUInt();
        wpa = args[3].toUInt();
        rsn = args[4].toUInt();
        kDebug() << "SSID:" << ssid << "CAPS:" << caps << "WPA:" << wpa << "RSN:" << rsn;
    } else {
        kDebug() << args;
    }

    QVBoxLayout * layout = new QVBoxLayout(this);
    m_contents = new ConnectionWidget(connectionId, this);
    layout->addWidget(m_contents);
    m_connectionTypeWidget = new Wireless80211Widget(connectionId, ssid, caps, wpa, rsn, this);
    Wireless80211SecurityWidget * wirelessSecurityWidget = new Wireless80211SecurityWidget(connectionId, this);
    IpV4Widget * ipv4Widget = new IpV4Widget(connectionId, this);
    // Must setup initial widget first
    addConfig(m_contents->configXml(), m_contents);

    addToTabWidget(m_connectionTypeWidget);
    addToTabWidget(wirelessSecurityWidget);
    addToTabWidget(ipv4Widget);
    kDebug() << "Setting connection name to " << ssid;
    m_contents->setConnectionName(ssid);
}

WirelessPreferences::~WirelessPreferences()
{
}

void WirelessPreferences::load()
{
    ConnectionPreferences::load();
}

void WirelessPreferences::save()
{
    ConnectionPreferences::save();
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
