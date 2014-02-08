/*
    Copyright 2013 Jan Grulich <jgrulich@redhat.com>

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

#include "openswan.h"

#include <KPluginFactory>

#include "openswanwidget.h"
#include "openswanauth.h"
#include "connection.h"


K_PLUGIN_FACTORY( OpenswanUiPluginFactory, registerPlugin<OpenswanUiPlugin>(); )
K_EXPORT_PLUGIN( OpenswanUiPluginFactory( "networkmanagement_openswanui", "libknetworkmanager" ) )

OpenswanUiPlugin::OpenswanUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{

}

OpenswanUiPlugin::~OpenswanUiPlugin()
{

}

SettingWidget *OpenswanUiPlugin::widget(Knm::Connection *connection, QWidget *parent)
{
    return new OpenswanWidget(connection, parent);
}

SettingWidget *OpenswanUiPlugin::askUser(Knm::Connection *connection, QWidget *parent)
{
    return new OpenswanAuthDialog(connection, parent);
}

QString OpenswanUiPlugin::suggestedFileName(Knm::Connection *connection) const
{
    Q_UNUSED(connection);

    // TODO : implement suggested file name
    return QString();
}

QString OpenswanUiPlugin::supportedFileExtensions() const
{
    // TODO : return supported file extensions
    return QString();
}

QVariantList OpenswanUiPlugin::importConnectionSettings(const QString &fileName)
{
    Q_UNUSED(fileName);

    // TODO : import the OpenSwan connection from file and return settings
    mError = VpnUiPlugin::NotImplemented;
    return QVariantList();
}

bool OpenswanUiPlugin::exportConnectionSettings(Knm::Connection *connection, const QString &fileName)
{
    Q_UNUSED(connection);
    Q_UNUSED(fileName);

    // TODO : export OpenSwan connection to file
    mError = VpnUiPlugin::NotImplemented;
    return false;
}

// vim: sw=4 sts=4 et tw=100
