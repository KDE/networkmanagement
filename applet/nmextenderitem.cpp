/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "nmextenderitem.h"

#include <QGraphicsLinearLayout>

#include <Plasma/Extender>
#include <Plasma/Label>

#include <KDebug>
#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>

#include "activatableitem.h"
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"

#include "interfaceitem.h"
#include "wirelessinterfaceitem.h"
#include "wiredinterfaceitem.h"
#include "activatablelistwidget.h"

NMExtenderItem::NMExtenderItem(RemoteActivatableList * activatableList, Plasma::Extender * ext)
: Plasma::ExtenderItem(ext),
    m_activatables(activatableList),
    m_widget(0),
    m_connectionTabs(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setName("nmextenderitem");
    widget();
    init();
}

NMExtenderItem::~NMExtenderItem()
{
}

void NMExtenderItem::init()
{
    kDebug() << "Adding interfaces initially";
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        addInterfaceInternal(iface);
        kDebug() << "Network Interface:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
    }
    // hook up signals to allow us to change the connection list depending on APs present, etc
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            SLOT(interfaceAdded(const QString&)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            SLOT(interfaceRemoved(const QString&)));

    /*
    m_widget = new QGraphicsWidget(this);
    m_layout = new QGraphicsLinearLayout(Qt::Vertical, m_widget);
    m_layout->addItem(m_connectionLayout);
    m_widget->setLayout(m_layout);
    setWidget(m_widget);

    // adds items from subclasses above our layout
    setupHeader();

    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*)),
            SLOT(activatableAdded(RemoteActivatable *)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)),
            SLOT(activatableRemoved(RemoteActivatable *)));

    connect(m_activatables, SIGNAL(appeared()), SLOT(getList()));
    connect(m_activatables, SIGNAL(disappeared()), SLOT(listDisappeared()));

    getList();

    // adds items from subclasses below our layout
    setupFooter();
    m_layout->addStretch(5);
    */
}

QGraphicsItem * NMExtenderItem::widget()
{
    if (!m_widget) {
        kDebug() << "Creating widget";
        m_widget = new QGraphicsWidget(this);
        m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setWidget(m_widget);

        m_mainLayout = new QGraphicsLinearLayout(m_widget);
        m_mainLayout->setOrientation(Qt::Horizontal);
        m_widget->setLayout(m_mainLayout);

        QGraphicsWidget* interfaceWidget = new QGraphicsWidget(m_widget);
        m_interfaceLayout = new QGraphicsLinearLayout(interfaceWidget);
        m_interfaceLayout->setOrientation(Qt::Vertical);
        interfaceWidget->setLayout(m_interfaceLayout);
        m_mainLayout->addItem(interfaceWidget);

        m_connectionTabs = new Plasma::TabBar(m_widget);
        m_connectionTabs->setMinimumSize(200, 300);
        //m_connectionTabs->addTab(KIcon("network-wireless"), i18n("Wireless Networking"));
        //m_connectionTabs->addTab(KIcon("network-wired"), i18n("Wired Networking"));
        //m_connectionTabs->addTab(i18n("Wireless Networking"));
        //m_connectionTabs->addTab(i18n("Wired Networking"));
        m_mainLayout->addItem(m_connectionTabs);

    } else {
        kDebug() << "widget non empty";
    }
    kDebug() << "widget() run";
    return m_widget;
}

void NMExtenderItem::listDisappeared()
{
    /*
    //remove all connections from this service
    QHash<RemoteActivatable*, ActivatableItem*>::iterator i = m_connections.begin();
    while (i != m_connections.end()) {
        ActivatableItem * item = i.value();
        m_connectionLayout->removeItem(item);
        i = m_connections.erase(i);
        delete item;
    }
    emit connectionListUpdated();
    */
}

/*
bool NMExtenderItem::registerActivatable(RemoteActivatable * activatable)
{

    bool changed = false;
    / *
    if (!m_connections.contains(activatable)) {
        // let subclass decide
        if (accept(activatable)) {
            kDebug() << "adding activatable";
            ActivatableItem * ci = createItem(activatable);

            m_connections.insert(activatable, ci);
            m_connectionLayout->addItem(ci);
            m_connectionLayout->invalidate();
            m_layout->invalidate();
            changed = true;
        }
    }
    * /
    return changed;
}
*/
void NMExtenderItem::activatableAdded(RemoteActivatable * added)
{
    /*
    if (registerActivatable(added)) {
        kDebug();
        emit connectionListUpdated();
    }
    */
}

void NMExtenderItem::activatableRemoved(RemoteActivatable * removed)
{
    /*
    // look up the ActivatableItem and remove it
    if (m_connections.contains(removed)) {
        ActivatableItem * item = m_connections.value(removed);
        m_connectionLayout->removeItem(item);
        m_connections.remove(removed);
        delete item;
        kDebug();
        emit connectionListUpdated();
    }
    */
}


// Interfaces
void NMExtenderItem::interfaceAdded(const QString& uni)
{
    if (m_interfaces.keys().contains(uni)) {
        return;
    }
    kDebug() << "Interface Added.";
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    addInterfaceInternal(iface);
}

void NMExtenderItem::interfaceRemoved(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        InterfaceItem * item = m_interfaces.take(uni);
        m_interfaceLayout->removeItem(item);
        // TODO: remove tab
        delete item;
    }
}

void NMExtenderItem::addInterfaceInternal(Solid::Control::NetworkInterface* iface)
{
    Q_ASSERT(iface);
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * interface = 0;
        switch (iface->type()) {
            case Solid::Control::NetworkInterface::Ieee80211:
            {
                WirelessInterfaceItem * wirelessinterface = 0;
                wirelessinterface = new WirelessInterfaceItem(static_cast<Solid::Control::WirelessNetworkInterface *>(iface), InterfaceItem::InterfaceName, this);
                //connect(wirelessinterface, SIGNAL(stateChanged()), this, SLOT(updateNetworks()));
                wirelessinterface->setEnabled(Solid::Control::NetworkManager::isWirelessEnabled());
                ActivatableListWidget* aList = new ActivatableListWidget(m_activatables, iface, this);
                m_tabIndex[iface->uni()] = m_connectionTabs->addTab(KIcon("network-wireless"), "", aList);
                interface = wirelessinterface;
                kDebug() << "WiFi added";
                break;
            }
            case Solid::Control::NetworkInterface::Serial:
                #if 0
                {
                    interface = new SerialInterfaceItem(static_cast<Solid::Control::SerialNetworkInterface *>(iface),
                    m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                    inspector = new PppoeConnectionInspector;
                    break;
                }
                #endif
            case Solid::Control::NetworkInterface::Gsm:
                #if 0
                {
                    interface = new SerialInterfaceItem(static_cast<Solid::Control::SerialNetworkInterface *>(iface),
                    m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                    // TODO: When ModemManager support is added, connect signals from the SII to
                    // reassesConnectionList
                    inspector = new GsmConnectionInspector;
                    break;
                }
                #endif
            case Solid::Control::NetworkInterface::Cdma:
                #if 0
                {
                    interface = new SerialInterfaceItem(static_cast<Solid::Control::SerialNetworkInterface *>(iface),
                    m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                    inspector = new CdmaConnectionInspector;
                    // TODO: When ModemManager support is added, connect signals from the SII to
                    // reassesConnectionList
                    break;
                }
                #endif
            default:
            case Solid::Control::NetworkInterface::Ieee8023:
            {
                WiredInterfaceItem * wiredinterface = 0;
                interface = wiredinterface = new WiredInterfaceItem(static_cast<Solid::Control::WiredNetworkInterface *>(iface), InterfaceItem::InterfaceName, this);
                ActivatableListWidget* aList = new ActivatableListWidget(m_activatables, iface, this);
                m_tabIndex[iface->uni()] = m_connectionTabs->addTab(KIcon("network-wired"), "", aList);
                break;
            }
        }
        //interface->setEnabled(m_enabled);
        m_interfaceLayout->addItem(interface);
        m_interfaces.insert(iface->uni(), interface);
        //m_interfaceLayout->invalidate();
        //m_interfaceLayout->updateGeometry();
        //updateNetworks();
    }
    //show();
    //emit updateLayout();
}


// vim: sw=4 sts=4 et tw=100

