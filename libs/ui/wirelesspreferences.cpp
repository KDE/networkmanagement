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

#include "802_11_wirelesswidget.h"
#include "security/802_11_wireless_security_widget.h"
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
: ConnectionPreferences( KGlobal::mainComponent(), parent, args ), m_caps(0), m_wpa(0), m_rsn(0)
{
    // at least 1
    Q_ASSERT(args.count());

    QString connectionId = args[0].toString();
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Wireless);

    if (args.count() == 5) {
        m_ssid = args[1].toString();
        m_caps = args[2].toUInt();
        m_wpa = args[3].toUInt();
        m_rsn = args[4].toUInt();
        kDebug() << "SSID:" << m_ssid << "CAPS:" << m_caps << "WPA:" << m_wpa << "RSN:" << m_rsn;
    } else {
        kDebug() << args;
    }

    QVBoxLayout * layout = new QVBoxLayout(this);
    m_contents = new ConnectionWidget(m_connection, (m_ssid.isEmpty() ? i18n("New Wireless Connection") : m_ssid), this);
    layout->addWidget(m_contents);
    Wireless80211Widget* connectionTypeWidget = new Wireless80211Widget(m_connection, m_ssid, this);
    Wireless80211SecurityWidget * wirelessSecurityWidget = new Wireless80211SecurityWidget(setDefaults, m_connection, m_caps, m_wpa, m_rsn, this);
    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);

    // the wireless security widget also creates the wpa-eap widget which
    // manages 802.1x parameters. 
//    addSettingWidget(wirelessSecurityWidget->wpaEapWidget());

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
    return !( !m_ssid.isEmpty() && m_caps == 0 && m_wpa == 0 && m_rsn == 0 );
}
// vim: sw=4 sts=4 et tw=100
