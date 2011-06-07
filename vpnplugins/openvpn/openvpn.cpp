/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "openvpn.h"

#include <KPluginFactory>

#include "openvpnwidget.h"

#include "connection.h"

K_PLUGIN_FACTORY( OpenVpnUiPluginFactory, registerPlugin<OpenVpnUiPlugin>(); )
K_EXPORT_PLUGIN( OpenVpnUiPluginFactory( "networkmanagement_openvpnui", "libknetworkmanager" ) )

OpenVpnUiPlugin::OpenVpnUiPlugin(QObject * parent, const QVariantList &) : VpnUiPlugin(parent)
{
}

OpenVpnUiPlugin::~OpenVpnUiPlugin()
{

}

SettingWidget * OpenVpnUiPlugin::widget(Knm::Connection * connection, QWidget * parent)
{
    OpenVpnSettingWidget * wid = new OpenVpnSettingWidget(connection, parent);
    wid->init();
    return wid;
}

QString OpenVpnUiPlugin::suggestedFileName(Knm::Connection *connection) const
{
    // TODO : implement suggested file name
    return QString();
}

QVariantList OpenVpnUiPlugin::importConnectionSettings(const QString &fileName)
{
    // TODO : import the OpenVPN connection from file and return settings
    return QVariantList();
}

void OpenVpnUiPlugin::exportConnectionSettings(Knm::Connection * connection, const QString &fileName)
{
    // TODO : export the OpenVPN connection to file
    return;
}

// vim: sw=4 sts=4 et tw=100
