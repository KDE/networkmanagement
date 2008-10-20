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
#include "remoteconnection.h"


InterfaceGroup::InterfaceGroup(Solid::Control::NetworkInterface::Type type, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, QGraphicsWidget * parent)
: QGraphicsWidget(parent), m_type(type), m_userSettings(userSettings), m_systemSettings(systemSettings)
{
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, this);

    // create an interfaceItem for each interface of our type
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->type() == interfaceType()) {
            addInterfaceInternal(iface);
        }
    }
    // create a connectionItem for each appropriate connection
    addSettingsService(m_userSettings);
    addSettingsService(m_systemSettings);

    setLayout(m_layout);
    // hook up signals to allow us to change the connection list depending on APs present, etc
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            SLOT(interfaceAdded(const QString&)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            SLOT(interfaceRemoved(const QString&)));

    if (m_interfaces.count() == 0) {
        hide();
    }
}


InterfaceGroup::~InterfaceGroup()
{

}

void InterfaceGroup::addInterfaceInternal(Solid::Control::NetworkInterface* iface)
{
    Q_ASSERT(iface);
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * ii = new InterfaceItem(iface, m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
        m_layout->addItem(ii);
        m_interfaces.insert(iface->uni(), ii);
        m_layout->invalidate();
    }
    show();
}

void InterfaceGroup::addSettingsService(NetworkManagerSettings * service)
{
    connect(service, SIGNAL(connectionAdded(NetworkManagerSettings *, const QString&)), SLOT(connectionAddedToService(NetworkManagerSettings *, const QString&)));
    connect(service, SIGNAL(connectionRemoved(NetworkManagerSettings *, const QString&)), SLOT(connectionRemovedFromService(NetworkManagerSettings *, const QString&)));
    //connect(service, SIGNAL(connectionUpdated(NetworkManagerSettings *, const QString&);
    connect(service, SIGNAL(appeared(NetworkManagerSettings*)), SLOT(serviceAppeared(NetworkManagerSettings*)));
    connect(service, SIGNAL(disappeared(NetworkManagerSettings*)), SLOT(serviceDisappeared(NetworkManagerSettings*)));
    serviceAppeared(service);
}

void InterfaceGroup::serviceAppeared(NetworkManagerSettings * service)
{
    if (service->isValid()) {
        foreach (QString connectionPath, service->connections() ) {
            addConnectionInternal(service, connectionPath);
        }
    }
}

void InterfaceGroup::addConnectionInternal(NetworkManagerSettings * service, const QString& connectionPath)
{
    ConnectionInspectorFactory cif;
    RemoteConnection * connection = service->findConnection(connectionPath);
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->type() == m_type) {
            ConnectionInspector * inspector = cif.connectionInspector(iface);
            if (inspector->accept(connection)) {
                ConnectionItem * ci = new ConnectionItem(connection, this);
                connect(ci, SIGNAL(clicked(ConnectionItem*)), SLOT(activateConnection(ConnectionItem*)));
                m_connections.insert(QPair<QString,QString>(service->service(), connectionPath), ci);
                m_layout->addItem(ci);
            }
        }
    }
}

void InterfaceGroup::serviceDisappeared(NetworkManagerSettings* settings)
{
    //remove all connections from this service
    ServiceConnectionHash::iterator i = m_connections.begin();
    while (i != m_connections.end()) {
        if (i.key().first == settings->service()) {
            ConnectionItem * item = i.value();
            m_layout->removeItem(item);
            i = m_connections.erase(i);
            delete item;
        } else {
            ++i;
        }
    }
}

Solid::Control::NetworkInterface::Type InterfaceGroup::interfaceType() const
{
    return m_type;
}

void InterfaceGroup::interfaceAdded(const QString& uni)
{
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    addInterfaceInternal(iface);
    // KNotification
}

void InterfaceGroup::interfaceRemoved(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        InterfaceItem * item = m_interfaces.take(uni);
        m_layout->removeItem(item);
        delete item;
    }
}

void InterfaceGroup::activateConnection(ConnectionItem* item)
{
    // tell the manager to activate the connection
    // which device??
    // HACK - take the first one
    QHash<QString, InterfaceItem *>::const_iterator i = m_interfaces.constBegin();
    if ( i != m_interfaces.constEnd()) {
        QString firstDeviceUni = i.key();
        Solid::Control::NetworkManager::activateConnection(firstDeviceUni, item->connection()->service() + " " + item->connection()->path(), QVariantMap());
    }
    // if the manager updates the interface's state, we should then refresh the list of
    // connections(remove any active connections from the list
    // What about a connection that could be activated on 2 devices?
    // Ideally we should keep them around
}

void InterfaceGroup::connectionAddedToService(NetworkManagerSettings * service, const QString& connectionPath)
{
    addConnectionInternal(service, connectionPath);
}

void InterfaceGroup::connectionRemovedFromService(NetworkManagerSettings * service, const QString& connectionPath)
{
    // look up the ConnectionItem and remove it
    QPair<QString,QString> key(service->service(), connectionPath);
    if (m_connections.contains(key)) {
        ConnectionItem * item = m_connections.value(key);
        m_layout->removeItem(item);
        m_connections.remove(key);
        delete item;
    }
}

// vim: sw=4 sts=4 et tw=100
