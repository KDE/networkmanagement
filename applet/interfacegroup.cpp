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
#include <KNotification>
#include <solid/control/networkserialinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "events.h"
#include "interfaceitem.h"
#include "connectionitem.h"
#include "wirelessconnectionitem.h"
#include "connectioninspector.h"
#include "networkmanagersettings.h"
#include "remoteconnection.h"
#include "serialinterfaceitem.h"
#include "wirelessinterfaceitem.h"
#include "wirelessenvironment.h"

InterfaceGroup::InterfaceGroup(Solid::Control::NetworkInterface::Type type, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, QGraphicsWidget * parent)
: ConnectionList(userSettings, systemSettings, parent), m_type(type), m_wirelessEnvironment(0), m_interfaceLayout(new QGraphicsLinearLayout(Qt::Vertical))
{
}

void InterfaceGroup::setupHeader()
{
    m_layout->addItem(m_interfaceLayout);
    // create an interfaceItem for each interface of our type
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->type() == interfaceType()) {
            addInterfaceInternal(iface);
            kDebug() << "Network Interface:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
        }
    }
    // hook up signals to allow us to change the connection list depending on APs present, etc
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            SLOT(interfaceAdded(const QString&)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            SLOT(interfaceRemoved(const QString&)));

    if (m_interfaces.count() == 0) {
        hide();
    }
    setLayout(m_layout);
}


InterfaceGroup::~InterfaceGroup()
{

}

void InterfaceGroup::addInterfaceInternal(Solid::Control::NetworkInterface* iface)
{
    Q_ASSERT(iface);
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * ii = 0;
        WirelessInterfaceItem * wi = 0;
        SerialInterfaceItem * si = 0;
        ConnectionInspector * inspector = 0;
        switch (iface->type()) {
            case Solid::Control::NetworkInterface::Ieee80211:
                wi = new WirelessInterfaceItem(static_cast<Solid::Control::WirelessNetworkInterface *>(iface), m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                m_wirelessEnvironment = wi->wirelessEnvironment();
                ii = wi;
                inspector = new WirelessConnectionInspector(static_cast<Solid::Control::WirelessNetworkInterface*>(iface), wi->wirelessEnvironment());
                connect(wi, SIGNAL(wirelessNetworksChanged()), SLOT(reassess()));
                break;
            case Solid::Control::NetworkInterface::Serial:
                ii = si = new SerialInterfaceItem(static_cast<Solid::Control::SerialNetworkInterface *>(iface),
                        m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                inspector = new PppoeConnectionInspector;
                break;
            case Solid::Control::NetworkInterface::Gsm:
                ii = si = new SerialInterfaceItem(static_cast<Solid::Control::SerialNetworkInterface *>(iface),
                        m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                // TODO: When ModemManager support is added, connect signals from the SII to
                // reassesConnectionList
                inspector = new GsmConnectionInspector;
                break;
            case Solid::Control::NetworkInterface::Cdma:
                ii = si = new SerialInterfaceItem(static_cast<Solid::Control::SerialNetworkInterface *>(iface),
                        m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                inspector = new CdmaConnectionInspector;
                // TODO: When ModemManager support is added, connect signals from the SII to
                // reassesConnectionList
                break;
            default:
            case Solid::Control::NetworkInterface::Ieee8023:
                ii = new InterfaceItem(iface, m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                inspector = new WiredConnectionInspector(static_cast<Solid::Control::WiredNetworkInterface*>(iface));
                break;
        }
        ii->setConnectionInspector(inspector);

        m_interfaceLayout->addItem(ii);
        m_interfaces.insert(iface->uni(), ii);
        m_interfaceLayout->invalidate();
    }
    show();
}

bool InterfaceGroup::accept(RemoteConnection * conn) const
{
    // there was some code I don't remember the purpose of in addConnectionInternal before it was refactored here
    // that cast to WirelessConnectionItem, then did the same interface accept() loop.  ...?
    bool accepted = false;
    foreach (InterfaceItem * iface, m_interfaces) {
        if (iface->connectionInspector()->accept(conn)) {
            accepted = true;
            break;
        }
    }
    return accepted;
}

ConnectionItem * InterfaceGroup::createItem(RemoteConnection* connection)
{
    ConnectionItem * ci;
    if (m_type == Solid::Control::NetworkInterface::Ieee80211) {
        WirelessConnectionItem * wi = new WirelessConnectionItem(connection, this);
        ci = wi;
        if (m_wirelessEnvironment) {
            wi->setNetwork(m_wirelessEnvironment->findWirelessNetwork(wi->ssid()));
        }
    } else {
        ci = new ConnectionItem(connection, this);
    }
    ci->setupItem();
    return ci;
}

Solid::Control::NetworkInterface::Type InterfaceGroup::interfaceType() const
{
    return m_type;
}

void InterfaceGroup::interfaceAdded(const QString& uni)
{
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    addInterfaceInternal(iface);
    KNotification::event(Event::HwAdded, i18nc("Notification for hardware added", "Network interface %1 attached", iface->interfaceName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
}

void InterfaceGroup::interfaceRemoved(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        InterfaceItem * item = m_interfaces.take(uni);
        m_interfaceLayout->removeItem(item);
        KNotification::event(Event::HwRemoved, i18nc("Notification for hardware removed", "Network interface removed"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
        delete item;
        reassess();
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
        KNotification::event(Event::Connecting, i18nc("Notification text when activating a connection","Connecting %1", item->connection()->id()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    }
    // if the manager updates the interface's state, we should then refresh the list of
    // connections(remove any active connections from the list
    // What about a connection that could be activated on 2 devices?
    // Ideally we should keep them around
}


// vim: sw=4 sts=4 et tw=100
