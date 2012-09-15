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
#include <KStandardDirs>

// Plasma
#include <Plasma/Label>
#include <Plasma/IconWidget>

// networkmanagement lib
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"
#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessnetwork.h"
#include "remotegsminterfaceconnection.h"
#include "activatableitem.h"

// networkmanagement applet
#include "interfaceconnectionitem.h"
#include "wirelessnetworkitem.h"
#include "hiddenwirelessnetworkitem.h"
#include "gsminterfaceconnectionitem.h"
#include "../solidcontrolfuture/wirelessnetworkinterfaceenvironment.h"

ActivatableListWidget::ActivatableListWidget(RemoteActivatableList* activatables, QGraphicsWidget* parent) : Plasma::ScrollWidget(parent),
    m_hiddenItem(0),
    m_activatables(activatables),
    m_layout(0),
    m_showAllTypes(true),
    m_vpn(false),
    m_hasWireless(false)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_widget = new QGraphicsWidget(this);
    //m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout = new QGraphicsLinearLayout(m_widget);
    m_layout->setOrientation(Qt::Vertical);
    m_layout->setSpacing(0);
    setWidget(m_widget);
}

void ActivatableListWidget::init()
{
    listAppeared();
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*,int)),
            SLOT(activatableAdded(RemoteActivatable*,int)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)),
            SLOT(activatableRemoved(RemoteActivatable*)));

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

void ActivatableListWidget::addInterface(Solid::Control::NetworkInterfaceNm09* iface)
{
    kDebug() << "interface added";
    if (iface) {
        m_interfaces.insert(iface->uni(), iface->type());
        m_showAllTypes = true;
        filter();
    }
}

void ActivatableListWidget::clearInterfaces()
{
    m_interfaces.clear();
    m_vpn = false;
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
    if (!m_interfaces.isEmpty()) {
        // If interfaces are set, activatables for other interfaces are not shown
        if (!m_interfaces.contains(activatable->deviceUni())) {
            return false;
        }
    }
    if (!m_showAllTypes) {
        // when no filter is set, only show activatables of a certain type
        if (!(m_types.contains(activatable->activatableType()))) {
            return false;
        }
    }
    if (activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
        RemoteWirelessInterfaceConnection * wic = static_cast<RemoteWirelessInterfaceConnection*>(activatable);

        if (!Solid::Control::NetworkManagerNm09::isWirelessEnabled() ||
            (!m_showAllTypes && !wic->isShared() && wic->operationMode() == Solid::Control::WirelessNetworkInterfaceNm09::Adhoc &&
                                 wic->activationState() != Knm::InterfaceConnection::Activated)) {
            return false;
        }
    }
    return true;
}

void ActivatableListWidget::createItem(RemoteActivatable * activatable, int index)
{
    ActivatableItem* ai = m_itemIndex.value(activatable, 0);
    if (ai) {
        m_layout->addItem(ai);
        ai->show();
        //kDebug() << "activatable already in the layout, not creating an item" << a;
        return;
    }

    switch (activatable->activatableType()) {
        case Knm::Activatable::WirelessNetwork:
        case Knm::Activatable::WirelessInterfaceConnection:
        { // Wireless
            ai = new WirelessNetworkItem(static_cast<RemoteWirelessNetwork*>(activatable), m_widget);
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
        case Knm::Activatable::GsmInterfaceConnection:
        { // Gsm (2G, 3G, etc)
            GsmInterfaceConnectionItem* gici = new GsmInterfaceConnectionItem(static_cast<RemoteGsmInterfaceConnection*>(activatable), m_widget);
            ai = gici;
            break;
        }
        default:
            break;
    }

    Q_ASSERT(ai);
    ai->setupItem();
    if (m_hiddenItem)
        m_layout->insertItem(index + 1, ai);
    else
        m_layout->insertItem(index, ai);
    m_itemIndex[activatable] = ai;
    connect(ai, SIGNAL(disappearAnimationFinished()),
            this, SLOT(deleteItem()));
    connect(ai, SIGNAL(showInterfaceDetails(QString)), SIGNAL(showInterfaceDetails(QString)));
}

void ActivatableListWidget::createHiddenItem()
{
    if (m_hiddenItem) {
        return;
    }
    //HiddenWirelessNetworkItem* ai = 0;
    m_hiddenItem = new HiddenWirelessNetworkItem(m_widget);
    Q_ASSERT(m_hiddenItem);
    m_hiddenItem->setupItem();
    m_layout->insertItem(0, m_hiddenItem);
    //m_itemIndex[activatable] = ai;
    connect(m_hiddenItem, SIGNAL(disappearAnimationFinished()),
            this, SLOT(deleteItem()));
    connect(m_hiddenItem, SIGNAL(connectToHiddenNetwork(QString)),
            this, SLOT(connectToHiddenNetwork(QString)));
}

void ActivatableListWidget::listAppeared()
{
    int i = 0;
    foreach (RemoteActivatable* remote, m_activatables->activatables()) {
        activatableAdded(remote, i);
        i++;
    }
    filter();
}

void ActivatableListWidget::deactivateConnection(const QString& deviceUni)
{
    foreach (ActivatableItem* item, m_itemIndex) {
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
    m_hiddenItem = 0;
}

void ActivatableListWidget::activatableAdded(RemoteActivatable * added, int index)
{
    kDebug();
    if (accept(added)) {
        createItem(added, index);
    }
    if(added->activatableType() == Knm::Activatable::WirelessInterfaceConnection && static_cast<RemoteWirelessInterfaceConnection*>(added)->operationMode() == Solid::Control::WirelessNetworkInterfaceNm09::Adhoc)
        connect(added,SIGNAL(changed()),SLOT(filter()));
}

void ActivatableListWidget::setHasWireless(bool hasWireless)
{
    kDebug() << "++++++++++++++" << hasWireless;
    m_hasWireless = hasWireless;
    filter();
}

void ActivatableListWidget::filter()
{
    // Clear connection list first, but do not delete the items.
    foreach (ActivatableItem* item, m_itemIndex) {
        // Hide them first to prevent glitches in GUI.
        item->hide();
        m_layout->removeItem(item);
    }

    int i = 0;
    foreach (RemoteActivatable *act, m_activatables->activatables()) {
        if (accept(act)) {
            // The "true" parameter means add the item to m_layout if it is already cached in m_itemIndex.
            createItem(act, i);
        } else {
            activatableRemoved(act);
        }
        i++;
    }

    if (!m_interfaces.isEmpty() && m_hasWireless) {
        bool found = false;
        if (Solid::Control::NetworkManagerNm09::isWirelessEnabled()) {
            foreach (const QString & uni, m_interfaces.keys()) {
                if (m_interfaces.value(uni) == Solid::Control::NetworkInterfaceNm09::Wifi) {
                    createHiddenItem();
                    found = true;
                    break;
                }
            }
        }
        if (!found && m_hiddenItem) {
            m_hiddenItem->disappear();
            m_hiddenItem = 0;
        }
    } else if (m_hasWireless && Solid::Control::NetworkManagerNm09::isWirelessEnabled() && !m_vpn) {
        createHiddenItem();
    } else if (m_hiddenItem) {
        m_hiddenItem->disappear();
        m_hiddenItem = 0;
    }
    m_layout->invalidate();
}

void ActivatableListWidget::activatableRemoved(RemoteActivatable * removed)
{
    ActivatableItem *it = m_itemIndex.value(removed, 0);
    if (!it) {
        return;
    }
    it->disappear();
}

void ActivatableListWidget::deleteItem()
{
    ActivatableItem* ai = dynamic_cast<ActivatableItem*>(sender());
    m_layout->removeItem(ai);
    if (m_itemIndex.key(ai, 0))
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
        }
    }
}

void ActivatableListWidget::vpnHoverEnter()
{
    foreach (ActivatableItem* item, m_itemIndex) {
        if (!item) { // the item might be gone here
            continue;
        }

        RemoteInterfaceConnection * conn = item->interfaceConnection();
        if (conn && conn->connectionType() == Knm::Connection::Vpn)
            item->hoverEnter();
    }
}

void ActivatableListWidget::vpnHoverLeave()
{
    foreach (ActivatableItem* item, m_itemIndex) {
        if (!item) { // the item might be gone here
            continue;
        }

        RemoteInterfaceConnection * conn = item->interfaceConnection();
        if (conn && conn->connectionType() == Knm::Connection::Vpn)
            item->hoverLeave();
    }
}

void ActivatableListWidget::connectToHiddenNetwork(const QString &ssid)
{
    Solid::Control::WirelessNetworkInterfaceNm09 * wiface = 0;
    foreach (Solid::Control::NetworkInterfaceNm09 * iface, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
        if (iface->type() == Solid::Control::NetworkInterfaceNm09::Wifi && iface->connectionState() > Solid::Control::NetworkInterfaceNm09::Unavailable) {
            wiface = qobject_cast<Solid::Control::WirelessNetworkInterfaceNm09 *>(iface);
            break;
        }
    }

    if (!wiface) {
        return;
    }

    QStringList args;
    QString moduleArgs;

    Solid::Control::WirelessNetworkInterfaceEnvironment envt(wiface);
    Solid::Control::WirelessNetwork * network = envt.findNetwork(ssid);

    if (network) {
        moduleArgs = QString::fromLatin1("%1 %2")
            .arg(wiface->uni())
            .arg(network->referenceAccessPoint());

    } else {
        moduleArgs = QString::fromLatin1("%1 %2")
            .arg(wiface->uni())
            .arg(ssid);
    }

    args << QLatin1String("create") << QLatin1String("--type") << QLatin1String("802-11-wireless") << QLatin1String("--specific-args") << moduleArgs << QLatin1String("wifi_pass");
    kDebug() << "invoking networkmanagement_configshell" << args;
    int ret = KToolInvocation::kdeinitExec(KGlobal::dirs()->findResource("exe", "networkmanagement_configshell"), args);
    kDebug() << ret << args;
}

// vim: sw=4 sts=4 et tw=100
