/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "ipv4widget.h"
#include "connectionwidget.h"
#include "vpnuiplugin.h"

#include "connection.h"
#include "settings/vpn.h"

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
        m_vpnPluginName = args[1].toString();
        m_uiPlugin = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( m_vpnPluginName ), this, QVariantList(), &error );
        if (error.isEmpty()) {
            SettingWidget * vpnWidget = m_uiPlugin->widget(m_connection, this);
            addToTabWidget(vpnWidget);
        } else {
            kDebug() << error;
        }
    }
    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);
    addToTabWidget(ipv4Widget);
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
            m_vpnPluginName = vpnSetting->pluginName();
            QString error;
            m_uiPlugin = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( m_vpnPluginName ), this, QVariantList(), &error );
            if (m_uiPlugin && error.isEmpty()) {
                SettingWidget * vpnWidget = m_uiPlugin->widget(m_connection, this);
                addToTabWidget(vpnWidget);
                // load this widget manually, as it was not present when ConnectionPreferences::load() ran
                vpnWidget->readConfig();
            }
        }
    } else { // we are loading a new connection's settings.  Set the plugin name after the load so this can be saved later
        vpnSetting->setPluginName(m_vpnPluginName);
    }
}

bool VpnPreferences::needsEdits() const
{
    return true;
}

// vim: sw=4 sts=4 et tw=100
