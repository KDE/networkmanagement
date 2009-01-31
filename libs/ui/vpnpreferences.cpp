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

#include <QVBoxLayout>
#include <QFile>

#include <KDebug>
#include <KPluginFactory>
#include <KServiceTypeTrader>
#include <KTabWidget>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include <nm-setting-connection.h>
#include <nm-setting-vpn.h>

//#include "pppwidget.h"
#include "connectionwidget.h"
#include "vpnuiplugin.h"

#include "connection.h"

VpnPreferences::VpnPreferences(QWidget *parent, const QVariantList &args)
: ConnectionPreferences( KGlobal::mainComponent(), parent, args )
{
    QString connectionId = args[0].toString();
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Vpn);

    if (args.count() > 1)
        m_vpnType = args[1].toString();
    else
        kDebug() << args;

    QVBoxLayout * layout = new QVBoxLayout(this);
    m_contents = new ConnectionWidget(m_connection, i18n("New VPN Connection"), this);
    layout->addWidget(m_contents);
    //PppWidget * pppWidget = new PppWidget(m_connection, this);
    // load the plugin in m_vpnType, get its SettingWidget and add it
    QString error;
    m_uiPlugin = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "KNetworkManager/VpnUiPlugin" ), QString::fromLatin1( "[X-KDE-PluginInfo-Name]=='%1'" ).arg( m_vpnType ), this, QVariantList(), &error );
    if (error.isEmpty()) {
        SettingWidget * vpnWidget = m_uiPlugin->widget(m_connection, this);
        addToTabWidget(vpnWidget);
    } else {
        kDebug() << error;
    }
    //addToTabWidget(pppWidget);
}

VpnPreferences::~VpnPreferences()
{
}

void VpnPreferences::load()
{
    ConnectionPreferences::load();
//    KConfigGroup group(m_contents->configXml()->config(), NM_SETTING_CONNECTION_SETTING_NAME);
    //m_vpnType = group.readEntry( NM_SETTING_VPN_SERVICE_TYPE, m_vpnType );
}

void VpnPreferences::save()
{
    //KConfigGroup group(m_contents->configXml()->config(), NM_SETTING_CONNECTION_SETTING_NAME);
    //group.writeEntry( NM_SETTING_VPN_SERVICE_TYPE, m_vpnType );
    ConnectionPreferences::save();
}

// vim: sw=4 sts=4 et tw=100
