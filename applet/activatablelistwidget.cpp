/*
Copyright 2008, 2009 Sebastian K?gler <sebas@kde.org>

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

// Own
#include "activatablelistwidget.h"
#include "wirelessstatus.h"

// Qt
#include <QGraphicsLinearLayout>

// KDE
#include <KDebug>
#include <solid/control/networkmanager.h>
#include <KToolInvocation>

// Plasma
#include <Plasma/Label>
#include <Plasma/IconWidget>

// networkmanagement lib
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"
#include "remotewirelessnetwork.h"
#include "remotegsminterfaceconnection.h"
#include "activatableitem.h"

// networkmanagement applet
#include "interfaceconnectionitem.h"
#include "wirelessnetworkitem.h"
#include "hiddenwirelessnetworkitem.h"
#include "gsminterfaceconnectionitem.h"

ActivatableListWidget::ActivatableListWidget(RemoteActivatableList* activatables, QGraphicsWidget* parent) : Plasma::ScrollWidget(parent),
    m_hiddenItem(0),
    m_activatables(activatables),
    m_layout(0),
    m_vpn(false)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_widget = new QGraphicsWidget(this);
    //m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout = new QGraphicsLinearLayout(m_widget);
    m_layout->setOrientation(Qt::Vertical);
    m_layout->setSpacing(1);
    setWidget(m_widget);
}

void ActivatableListWidget::init()
{
    listAppeared();
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*)),
            SLOT(activatableAdded(RemoteActivatable *)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)),
            SLOT(activatableRemoved(RemoteActivatable *)));

    connect(m_activatables, SIGNAL(appeared()), SLOT(listAppeared()));
    connect(m_activatables, SIGNAL(disappeared()), SLOT(listDisappeared()));
}

ActivatableListWidget::~ActivatableListWidget()
{
}

void ActivatableListWidget::addType(Knm::Activatable::ActivatableType type)
{
    if (!(m_types.contains(type))) {
        m_types.append(type);
    }
    filter();
}

void ActivatableListWidget::removeType(Knm::Activatable::ActivatableType type)
{
    if (m_types.contains(type)) {
        m_types.removeAll(type);
    }
    filter();
}

void ActivatableListWidget::addInterface(Solid::Control::NetworkInterface* iface)
{
    kDebug() << "interface added";
    if (iface) {
        m_interfaces << iface->uni();
        m_showAllTypes = true;
        filter();
    }
}

void ActivatableListWidget::clearInterfaces()
{
    m_interfaces = QStringList();
    m_showAllTypes = false;
    filter();
}

void ActivatableListWidget::setShowAllTypes(bool show, bool refresh)
{
    m_showAllTypes = show;
    if (show) {
        m_vpn = false;
    }
    if (refresh) {
        filter();
    }
}

void ActivatableListWidget::toggleVpn()
{
    kDebug() << "VPN toggled";
    m_vpn = true;
    filter();
}

bool ActivatableListWidget::accept(RemoteActivatable * activatable) const
{
    if (m_vpn) {
        if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
            return true;
        } else {
            return false;
        }
    }
    // Policy whether an activatable should be shown or not.
    if (m_interfaces.count()) {
        // If interfaces are set, activatables for other interfaces are not shown
        if (m_interfaces.contains(activatable->deviceUni())) {
        } else {
            return false;
        }
    }
    if (!m_showAllTypes) {
    // when no filter is set, only show activatables of a certain type
        if (!(m_types.contains(activatable->activatableType()))) {
            return false;
        }
    }
    return true;
}

void ActivatableListWidget::createItem(RemoteActivatable * activatable)
{
    // FIXME: m_itemIndex.contains()?
    foreach (RemoteActivatable* a, m_itemIndex.keys()) {
        if (activatable == a) {
            //kDebug() << "activatable already in the layout, not creating an item" << a;
            return;
        }
    }

    ActivatableItem* ai = 0;
    switch (activatable->activatableType()) {
        case Knm::Activatable::WirelessNetwork:
        case Knm::Activatable::WirelessInterfaceConnection:
        { // Wireless
            WirelessNetworkItem* wni = new WirelessNetworkItem(static_cast<RemoteWirelessNetwork*>(activatable), m_widget);
            ai = wni;
            QString ssid = wni->ssid();
            if (m_hiddenConnectionInProgress.contains(ssid)) {
                kDebug() << "hidden network" << ssid << "appeared, connecting ...";
                activatable->activate();
            }
            m_hiddenConnectionInProgress.removeAll(ssid);

            break;
        }
        case Knm::Activatable::InterfaceConnection:
        case Knm::Activatable::VpnInterfaceConnection:
        {
            ai = new InterfaceConnectionItem(static_cast<RemoteInterfaceConnection*>(activatable), m_widget);
            break;
        }
        case Knm::Activatable::HiddenWirelessInterfaceConnection:
        {
            kWarning() << "This is handled differently, this codepath should be disabled.";
            //ai = new HiddenWirelessNetworkItem(static_cast<RemoteInterfaceConnection*>(activatable), m_widget);
            break;
        }
#ifdef COMPILE_MODEM_MANAGER_SUPPORT
        case Knm::Activatable::GsmInterfaceConnection:
        { // Gsm (2G, 3G, etc)
            GsmInterfaceConnectionItem* gici = new GsmInterfaceConnectionItem(static_cast<RemoteGsmInterfaceConnection*>(activatable), m_widget);
            ai = gici;
            break;
        }
#endif
        default:
            break;
    }

    Q_ASSERT(ai);
    ai->setupItem();
    m_layout->addItem(ai);
    m_itemIndex[activatable] = ai;
    connect(ai, SIGNAL(disappearAnimationFinished()),
            this, SLOT(deleteItem()));

}

void ActivatableListWidget::createHiddenItem()
{
    //HiddenWirelessNetworkItem* ai = 0;
    m_hiddenItem = new HiddenWirelessNetworkItem(m_widget);
    Q_ASSERT(m_hiddenItem);
    m_hiddenItem->setupItem();
    m_layout->insertItem(1337, m_hiddenItem);
    //m_itemIndex[activatable] = ai;
    connect(m_hiddenItem, SIGNAL(disappearAnimationFinished()),
            this, SLOT(deleteItem()));
    connect(m_hiddenItem, SIGNAL(connectToHiddenNetwork(const QString&)),
            this, SLOT(connectToHiddenNetwork(const QString&)));
}

void ActivatableListWidget::listAppeared()
{
    createHiddenItem(); // TODO: move to end

    foreach (RemoteActivatable* remote, m_activatables->activatables()) {
        if (accept(remote)) {
            createItem(remote);
        }
    }
}

void ActivatableListWidget::deactivateConnection(const QString& deviceUni)
{
    foreach (ActivatableItem* item, m_itemIndex) {
        if (!item) { // the item might be gone here
            continue;
        }
        RemoteInterfaceConnection *conn = item->interfaceConnection();
        if (conn && conn->deviceUni() == deviceUni) {
            //kDebug() << "deactivating" << conn->connectionName();
            conn->deactivate();
        }
    }
}

void ActivatableListWidget::listDisappeared()
{
    foreach (ActivatableItem* item, m_itemIndex) {
        m_layout->removeItem(item);
        delete item;
    }
    m_itemIndex.clear();

    delete m_hiddenItem;
}

void ActivatableListWidget::activatableAdded(RemoteActivatable * added)
{
    //kDebug();
    if (accept(added)) {
        createItem(added);
    }
}

void ActivatableListWidget::setHasWireless(bool hasWireless)
{
    kDebug() << "++++++++++++++" << hasWireless;
    m_hasWireless = hasWireless;
    if (hasWireless) {
        if (!m_hiddenItem) {
            createHiddenItem();
        }
    } else {
        delete m_hiddenItem;
        m_hiddenItem = 0;
    }
}

void ActivatableListWidget::filter()
{
    foreach (RemoteActivatable *act, m_activatables->activatables()) {
        if (accept(act)) {
            createItem(act);
        } else {
            if (m_itemIndex.keys().contains(act)) {
                activatableRemoved(act);
            }
        }
    }
    m_layout->invalidate();
}

void ActivatableListWidget::activatableRemoved(RemoteActivatable * removed)
{
    ActivatableItem *it = m_itemIndex[removed];
    if (!it) {
        return;
    }
    it->disappear();
}

void ActivatableListWidget::deleteItem()
{
    ActivatableItem* ai = dynamic_cast<ActivatableItem*>(sender());
    m_layout->removeItem(ai);
    m_itemIndex.remove(m_itemIndex.key(ai));
    delete ai;
}

void ActivatableListWidget::hoverEnter(const QString& uni)
{
    foreach (ActivatableItem* item, m_itemIndex) {
        if (!item) { // the item might be gone here
            continue;
        }

        RemoteInterfaceConnection *conn = item->interfaceConnection();
        if (conn && conn->deviceUni() == uni) {
            item->hoverEnter();
            break;
        }
    }
}

void ActivatableListWidget::hoverLeave(const QString& uni)
{
    foreach (ActivatableItem* item, m_itemIndex) {
        if (!item) { // the item might be gone here
            continue;
        }

        RemoteInterfaceConnection *conn = item->interfaceConnection();
        if (conn && conn->deviceUni() == uni) {
            item->hoverLeave();
            break;
        }
    }
}

void ActivatableListWidget::connectToHiddenNetwork(const QString &ssid)
{
    m_hiddenConnectionInProgress << ssid;
    QStringList args = QStringList(ssid) << "create";
    kDebug() << "invoking networkmanagement_configshell" << args;
    KToolInvocation::kdeinitExec("networkmanagement_configshell", args);
}
// vim: sw=4 sts=4 et tw=100
