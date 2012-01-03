/*
Copyright 2008, 2009 Sebastian K?gler <sebas@kde.org>
Copyright 2011 Lamarque V. Souza <lamarque@kde.org>

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
#include <QtNetworkManager/manager.h>
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
#include "showmoreitem.h"
#include "gsminterfaceconnectionitem.h"
#include <QtNetworkManager/wirelessnetworkinterfaceenvironment.h>

ActivatableListWidget::ActivatableListWidget(RemoteActivatableList* activatables, QGraphicsWidget* parent) : Plasma::ScrollWidget(parent),
    m_hiddenItem(0),
    m_showMoreItem(0),
    m_activatables(activatables),
    m_layout(0),
    m_vpn(false),
    m_hasWireless(false),
    m_moreNetworks(0),
    m_filter(NormalConnections)
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

bool ActivatableListWidget::accept(RemoteActivatable * activatable)
{
    // Filter for this interface only. The interface is selected by clicking on the plasmoids's interface list.
    if (m_filter.testFlag(FilterDevice) && m_device && activatable->deviceUni() != m_device->uni()) {
        return false;
    }

    // Some as above but for VPN interfaces, which does not have a real NetworkManager::Device associated to them.
    if (m_vpn && activatable->activatableType() != Knm::Activatable::VpnInterfaceConnection) {
        return false;
    }

    // The code below is executed when no interface is selected.

    if (m_filter.testFlag(NormalConnections)) {
        if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection ||
            activatable->isShared()) {
            return false;
        } else if (activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection ||
                   activatable->activatableType() == Knm::Activatable::WirelessNetwork) {
            if (!NetworkManager::isWirelessEnabled()) {
                return false;
            } else if (m_filter.testFlag(SavedConnections)) {
                if (activatable->activatableType() == Knm::Activatable::WirelessNetwork) {
                    m_moreNetworks++; // number of suppressed networks in m_showMoreItem.
                    return false;
                }
            }
        }
    } else if (m_filter.testFlag(VPNConnections) && activatable->activatableType() != Knm::Activatable::VpnInterfaceConnection) {
        return false;
    } else if (m_filter.testFlag(SharedConnections) && !activatable->isShared()) {
        return false;
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
    m_layout->insertItem(index + 1, ai);
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

void ActivatableListWidget::updateShowMoreItem()
{
    /* m_showMoreItem only makes sense for NormalConnections, so disabled it for VPN and Shared connections.
     * Also disables it when there is no more connections to show and when the m_showMoreItem is not toggled.
     * When m_showMoreItem is toggled m_moreNetworks is zero, so do not hide m_showMoreItem in that case. */
    if (!m_filter.testFlag(NormalConnections) || (m_moreNetworks == 0 && m_filter.testFlag(SavedConnections))) {
        if (m_showMoreItem) {
            m_showMoreItem->disappear();
            m_showMoreItem = 0;
        }
        return;
    }
    if (m_showMoreItem) {
        m_layout->removeItem(m_showMoreItem);
        m_layout->insertItem(100, m_showMoreItem); // append to connection list.

        // If SavedConnection is set then m_showMoreItem is *not* toggled.
        m_showMoreItem->setChecked(!m_filter.testFlag(SavedConnections));
        m_showMoreItem->setNetworkCount(m_moreNetworks);
        return;
    }
    m_showMoreItem = new ShowMoreItem(m_moreNetworks, m_widget);
    Q_ASSERT(m_showMoreItem);
    m_showMoreItem->setupItem();
    m_layout->insertItem(100, m_showMoreItem);
    m_showMoreItem->setChecked(!m_filter.testFlag(SavedConnections));
    connect(m_showMoreItem, SIGNAL(disappearAnimationFinished()),
            this, SLOT(deleteItem()));
    connect(m_showMoreItem, SIGNAL(clicked()), this, SIGNAL(showMoreClicked()));
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
    } else {
        updateShowMoreItem();
    }
    if(added->activatableType() == Knm::Activatable::WirelessInterfaceConnection && static_cast<RemoteWirelessInterfaceConnection*>(added)->operationMode() == NetworkManager::WirelessDevice::Adhoc)
        connect(added,SIGNAL(changed()),SLOT(filter()));
}

void ActivatableListWidget::setHasWireless(bool hasWireless)
{
    kDebug() << "++++++++++++++" << hasWireless;
    if (m_hasWireless != hasWireless) {
        m_hasWireless = hasWireless;
        filter();
    }
}

void ActivatableListWidget::setFilter(FilterTypes f)
{
//    if (f != m_filter) {
        m_filter = f;
        filter();
//    }
}

void ActivatableListWidget::setDeviceToFilter(NetworkManager::Device* device, const bool vpn)
{
    m_device = device;

    // VpnInterfaceItems do not have NetworkManager::Device associated to them.
    if (m_device || vpn) {
        m_filter |= ActivatableListWidget::FilterDevice;
    } else {
        m_filter &= ~ActivatableListWidget::FilterDevice;
    }
    m_vpn = vpn;
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
    m_moreNetworks = 0;
    foreach (RemoteActivatable *act, m_activatables->activatables()) {
        if (accept(act)) {
            createItem(act, i);
        } else {
            activatableRemoved(act);
        }
        i++;
    }

    if (m_filter.testFlag(NormalConnections)) {
        if (m_filter.testFlag(FilterDevice) && m_hasWireless) {
            if (NetworkManager::isWirelessEnabled() && m_device && m_device->type() == NetworkManager::Device::Wifi) {
                createHiddenItem();
            } else if (m_hiddenItem) {
                m_hiddenItem->hide();
                m_layout->removeItem(m_hiddenItem);
                m_hiddenItem->disappear();
                m_hiddenItem = 0;
            }
        } else if (m_hasWireless && NetworkManager::isWirelessEnabled() && !m_vpn) {
            createHiddenItem();
        } else if (m_hiddenItem) {
            m_hiddenItem->hide();
            m_layout->removeItem(m_hiddenItem);
            m_hiddenItem->disappear();
            m_hiddenItem = 0;
        }
    } else if (m_hiddenItem) {
        m_hiddenItem->hide();
        m_layout->removeItem(m_hiddenItem);
        m_hiddenItem->disappear();
        m_hiddenItem = 0;
    }
    updateShowMoreItem();
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
        RemoteInterfaceConnection *conn = item->interfaceConnection();
        if (conn && conn->deviceUni() == uni) {
            item->hoverEnter();
        }
    }
}

void ActivatableListWidget::hoverLeave(const QString& uni)
{
    foreach (ActivatableItem* item, m_itemIndex) {
        RemoteInterfaceConnection *conn = item->interfaceConnection();
        if (conn && conn->deviceUni() == uni) {
            item->hoverLeave();
        }
    }
}

void ActivatableListWidget::vpnHoverEnter()
{
    foreach (ActivatableItem* item, m_itemIndex) {
        RemoteInterfaceConnection * conn = item->interfaceConnection();
        if (conn && conn->connectionType() == Knm::Connection::Vpn)
            item->hoverEnter();
    }
}

void ActivatableListWidget::vpnHoverLeave()
{
    foreach (ActivatableItem* item, m_itemIndex) {
        RemoteInterfaceConnection * conn = item->interfaceConnection();
        if (conn && conn->connectionType() == Knm::Connection::Vpn)
            item->hoverLeave();
    }
}

void ActivatableListWidget::connectToHiddenNetwork(const QString &ssid)
{
    NetworkManager::WirelessDevice * wiface = 0;
    foreach (NetworkManager::Device * iface, NetworkManager::networkInterfaces()) {
        if (iface->type() == NetworkManager::Device::Wifi && iface->state() > NetworkManager::Device::Unavailable) {
            wiface = qobject_cast<NetworkManager::WirelessDevice *>(iface);
            break;
        }
    }

    if (!wiface) {
        return;
    }

    QStringList args;
    QString moduleArgs;

    NetworkManager::WirelessNetworkInterfaceEnvironment envt(wiface);
    NetworkManager::WirelessNetwork * network = envt.findNetwork(ssid);

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
