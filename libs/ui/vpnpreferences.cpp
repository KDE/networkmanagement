/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2012 Rajeesh K Nambiar <rajeeshknambiar@gmail.com>

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

#include "vpnpreferences.h"

#include <QFile>

#include <KDebug>
#include <KPluginFactory>
#include <KServiceTypeTrader>
#include <KTabWidget>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include <nm-setting-connection.h>
#include <nm-setting-vpn.h>
#include <nm-setting-ip4-config.h>

#include "ipv4widget.h"
#include "ipv6widget.h"
#include "connectionwidget.h"
#include "vpnuiplugin.h"

#include "connection.h"
#include "settings/vpn.h"
#include "settings/ipv4.h"

VpnPreferences::VpnPreferences(const QVariantList &args, QWidget *parent)
: ConnectionPreferences(args, parent ), m_uiPlugin(0)
{
    QString connectionId = args[0].toString();
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Vpn);
    m_contents->setConnection(m_connection);
    m_contents->setDefaultName(i18n("New VPN Connection"));

    // load the plugin in m_vpnType, get its SettingWidget and add it
    QString error;
    if (args.count() > 1) {  // if we have a vpn type in the args, we are creating a new connection
        QString serviceType = args[1].toString();
        m_uiPlugin = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-NetworkManager-Services]=='%1'" ).arg( serviceType ), this, QVariantList(), &error );
        if (error.isEmpty()) {
            SettingWidget * vpnWidget = m_uiPlugin->widget(m_connection, this);
            addToTabWidget(vpnWidget);
        } else {
            kDebug() << error;
        }
    }
    // IpV4Setting
    if (args.count() > 5 ) {
        Knm::Ipv4Setting * ipv4Setting = static_cast<Knm::Ipv4Setting*>(m_connection->setting(Knm::Setting::Ipv4));
        QList <Solid::Control::IPv4RouteNm09> routes;
        QStringMap ipv4Data = Knm::VpnSetting::stringMapFromStringList(Knm::VpnSetting::variantMapToStringList(args[5].toMap()));
        if (ipv4Data.contains(QLatin1String(NM_SETTING_IP4_CONFIG_NEVER_DEFAULT))) {
            ipv4Setting->setNeverdefault( static_cast<bool>(ipv4Data[QLatin1String(NM_SETTING_IP4_CONFIG_NEVER_DEFAULT)].toUInt()) );
        }
        if (ipv4Data.contains(QLatin1String(NM_SETTING_IP4_CONFIG_ROUTES))) {
            foreach(const QString &oneRoute, ipv4Data[NM_SETTING_IP4_CONFIG_ROUTES].split(' ')) { // Split at whitespace
                QStringList routeData = oneRoute.split('/');    // Host + Prefix (e.g: 192.168.2.0/24)
                if (routeData.count() == 2) {
                    Solid::Control::IPv4RouteNm09 route(QHostAddress(routeData[0]).toIPv4Address(), routeData[1].toUInt(),0,0);
                    routes.append(route);
                }
            }
            if (!routes.isEmpty()) {
                ipv4Setting->setRoutes(routes);
            }
        }
    }

    // If additional settings are passed, populate them : data, secretsData, connection name
    // Connection Name
    if (args.count() > 4) {
        m_contents->setDefaultName(args[4].toString());
    }
    Knm::VpnSetting * conSetting = static_cast<Knm::VpnSetting*>(m_connection->setting(Knm::Setting::Vpn));
    if (args.count() > 3) {
        // VPN Secrets
	conSetting->setVpnSecrets(Knm::VpnSetting::stringMapFromStringList(Knm::VpnSetting::variantMapToStringList(args[3].toMap())));
    }
    if (args.count() > 2) {
        // VPN connection data
	conSetting->setData(Knm::VpnSetting::stringMapFromStringList(Knm::VpnSetting::variantMapToStringList(args[2].toMap())));
    }

    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);
    IpV6Widget * ipv6Widget = new IpV6Widget(m_connection, this);
    addToTabWidget(ipv4Widget);
    addToTabWidget(ipv6Widget);
}

VpnPreferences::VpnPreferences(Knm::Connection *con, QWidget *parent)
: ConnectionPreferences(QVariantList(), parent ), m_uiPlugin(0)
{
    if (!con)
    {
        kDebug() << "Connection pointer is NULL, creating a new connection.";
        m_connection = new Knm::Connection(QUuid::createUuid(), Knm::Connection::Vpn);
    }
    else
        m_connection = con;

    QString connectionId = m_connection->uuid().toString();

    m_contents->setConnection(m_connection);
    m_contents->setDefaultName(i18n("New VPN Connection"));

    // load the plugin in m_vpnType, get its SettingWidget and add it
    QString error;
    Knm::VpnSetting *vpnSetting = static_cast<Knm::VpnSetting*>(m_connection->setting(Knm::Setting::Vpn));
    if (!vpnSetting) {
        kDebug() << "Missing VPN setting!";
    } else {
        QString serviceType = vpnSetting->serviceType();
        kDebug() << serviceType;
        m_uiPlugin = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-NetworkManager-Services]=='%1'" ).arg( serviceType ), this, QVariantList(), &error );
        if (error.isEmpty()) {
            SettingWidget * vpnWidget = m_uiPlugin->widget(m_connection, this);
            addToTabWidget(vpnWidget);
        } else {
            kDebug() << error << ". serviceType == " << serviceType;
        }
    }
    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);
    IpV6Widget * ipv6Widget = new IpV6Widget(m_connection, this);
    addToTabWidget(ipv4Widget);
    addToTabWidget(ipv6Widget);
}


VpnPreferences::~VpnPreferences()
{
}

void VpnPreferences::load()
{
    ConnectionPreferences::load();
    Knm::VpnSetting * vpnSetting = static_cast<Knm::VpnSetting*>(m_connection->setting(Knm::Setting::Vpn));
    if (!m_uiPlugin) { // if this is not set yet, we are restoring a connection.  Look in the vpn setting for the plugin
        if (vpnSetting) {
            QString serviceType = vpnSetting->serviceType();
            QString error;
            m_uiPlugin = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-NetworkManager-Services]=='%1'" ).arg( serviceType ), this, QVariantList(), &error );
            if (m_uiPlugin && error.isEmpty()) {
                SettingWidget * vpnWidget = m_uiPlugin->widget(m_connection, this);
                addToTabWidget(vpnWidget);
                // load this widget manually, as it was not present when ConnectionPreferences::load() ran
                vpnWidget->readConfig();
            }
        }
    }
}

bool VpnPreferences::needsEdits() const
{
    return true;
}

// vim: sw=4 sts=4 et tw=100
