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
#include <KLocale>
#include <KTabWidget>

#include <solid/control/networkmanager.h>
#include <solid/control/wirelessaccesspoint.h>


#include "802_11_wirelesswidget.h"
#include "security/wirelesssecuritysettingwidget.h"
#include "ipv4widget.h"
#include "connectionwidget.h"

#include "connection.h"
#include "settings/802-11-wireless-security.h"
#include "settings/802-11-wireless.h"

#include <nm-setting-connection.h>
#include <nm-setting-wireless.h>

//K_PLUGIN_FACTORY( WirelessPreferencesFactory, registerPlugin<WirelessPreferences>();)
//K_EXPORT_PLUGIN( WirelessPreferencesFactory( "kcm_knetworkmanager_wireless" ) )

WirelessPreferences::WirelessPreferences(bool setDefaults, QWidget *parent, const QVariantList &args)
: ConnectionPreferences( KGlobal::mainComponent(), parent, args )
{
    // at least 1
    Q_ASSERT(args.count());

    QString connectionId = args[0].toString();
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Wireless);

    QString ssid;
    QString deviceUni;
    QString apUni;

    if (args.count() == 3) {
        deviceUni = args[1].toString();
        apUni = args[2].toString();
        kDebug() << "DeviceUni" << deviceUni << "AP UNI" << apUni;
    } else {
        kDebug() << args;
    }

    Solid::Control::AccessPoint * ap = 0;
    Solid::Control::WirelessNetworkInterface * iface = 0;

    if (!deviceUni.isEmpty() && deviceUni != QLatin1String("/")) {
        iface = qobject_cast<Solid::Control::WirelessNetworkInterface*>(Solid::Control::NetworkManager::findNetworkInterface(deviceUni));
        if (iface) {
            if ( !apUni.isEmpty() && apUni != QLatin1String("/")) {
                ap = iface->findAccessPoint(apUni);
                if (ap) {
                    ssid = ap->ssid();
                }
            }
        }
    }

    QVBoxLayout * layout = new QVBoxLayout(this);
    m_contents = new ConnectionWidget(m_connection, (ssid.isEmpty() ? i18n("New Wireless Connection") : ssid), this);
    layout->addWidget(m_contents);
    Wireless80211Widget* connectionTypeWidget = new Wireless80211Widget(m_connection, ssid, this);
    WirelessSecuritySettingWidget * wirelessSecurityWidget = new WirelessSecuritySettingWidget(m_connection, iface, ap, this);
    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);

    addToTabWidget(connectionTypeWidget);
    addToTabWidget(wirelessSecurityWidget);
    addToTabWidget(ipv4Widget);

    if ( setDefaults )
    {
        // for defaults the security is most interesting
        m_contents->connectionSettingsWidget()->setCurrentIndex( 1 );
    }
}

WirelessPreferences::~WirelessPreferences()
{
}


bool WirelessPreferences::needsEdits() const
{
    return false;
}

// vim: sw=4 sts=4 et tw=100
