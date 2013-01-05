/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#include "openconnectui.h"

#include <KPluginFactory>

#include "openconnectwidget.h"
#include "openconnectauth.h"
#include "connection.h"

K_PLUGIN_FACTORY( OpenconnectUiPluginFactory, registerPlugin<OpenconnectUiPlugin>(); )
K_EXPORT_PLUGIN( OpenconnectUiPluginFactory( "networkmanagement_openconnectui", "libknetworkmanager" ) )

OpenconnectUiPlugin::OpenconnectUiPlugin(QObject * parent, const QVariantList &) : VpnUiPlugin(parent)
{

}

OpenconnectUiPlugin::~OpenconnectUiPlugin()
{

}

SettingWidget * OpenconnectUiPlugin::widget(Knm::Connection * connection, QWidget * parent)
{
    return new OpenconnectSettingWidget(connection, parent);
}

SettingWidget * OpenconnectUiPlugin::askUser(Knm::Connection * connection, QWidget * parent)
{
    return new OpenconnectAuthWidget(connection, parent);
}

KDialog::ButtonCodes OpenconnectUiPlugin::suggestAuthDialogButtons()
{
    return KDialog::Cancel;
}

QString OpenconnectUiPlugin::suggestedFileName(Knm::Connection *connection) const
{
    // TODO : implement suggested file name
    return QString();
}

QString OpenconnectUiPlugin::supportedFileExtensions() const
{
    // TODO: return supported file extensions
    return QString();
}

QVariantList OpenconnectUiPlugin::importConnectionSettings(const QString &fileName)
{
    // TODO : import the Openconnect connection from file and return settings
    mError = VpnUiPlugin::NotImplemented;
    return QVariantList();
}

bool OpenconnectUiPlugin::exportConnectionSettings(Knm::Connection * connection, const QString &fileName)
{
    // TODO : export Openconnect connection to file
    mError = VpnUiPlugin::NotImplemented;
    return false;
}

