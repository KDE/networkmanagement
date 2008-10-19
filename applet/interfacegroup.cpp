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

#include "interfacegroup.h"

#include <NetworkManager.h>

#include <QGraphicsLinearLayout>
#include <KDebug>

#include "interfaceitem.h"
#include "connectionitem.h"
#include "connectioninspector.h"
#include "networkmanagersettings.h"

InterfaceGroup::InterfaceGroup(Solid::Control::NetworkInterface::Type type, QGraphicsWidget * parent)
: QGraphicsWidget(parent), m_type(type)

{
    //m_connections;
    //m_interfaces;
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);

    // create an interfaceItem for each interface of our type
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->type() == type) {
            InterfaceItem * ii = new InterfaceItem(iface, InterfaceItem::InterfaceName, this);
            m_layout->addItem(ii);
        }
    }
    // create a connectionItem for each appropriate connection
    m_userSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS), this);
    m_systemSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_SYSTEM_SETTINGS), this);
    populateConnectionList(m_userSettings);
    populateConnectionList(m_systemSettings);

    setLayout(m_layout);
    // hook up signals to allow us to change the connection list depending on APs present, etc
}

void InterfaceGroup::populateConnectionList(NetworkManagerSettings * service) {
    ConnectionInspectorFactory cif;
    foreach (QString connectionPath, service->connections() ) {
        RemoteConnection * connection = service->findConnection(connectionPath);
        foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
            if (iface->type() == m_type) {
                ConnectionInspector * inspector = cif.connectionInspector(iface);
                if (inspector->accept(connection)) {
                    ConnectionItem * ci = new ConnectionItem(connection, this);
                    m_layout->addItem(ci);
                }
            }
        }
    }
}

InterfaceGroup::~InterfaceGroup()
{

}

Solid::Control::NetworkInterface::Type InterfaceGroup::interfaceType() const
{
    return m_type;
}

void InterfaceGroup::interfaceAdded(const QString&)
{
}

void InterfaceGroup::interfaceRemoved(const QString&)
{

}

// vim: sw=4 sts=4 et tw=100
