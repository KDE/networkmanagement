/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
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
#include "nmpopup.h"

// Qt
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

// KDE
#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KToolInvocation>

// Plasma
#include <Plasma/Label>

// Solid::Control
#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>

// client lib
#include "activatableitem.h"
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"

// More own includes
#include "interfaceitem.h"
#include "wirelessinterfaceitem.h"
#include "activatablelistwidget.h"
#include "interfacedetailswidget.h"

NMPopup::NMPopup(RemoteActivatableList * activatableList, QGraphicsWidget* parent)
: QGraphicsWidget(parent),
    m_activatables(activatableList),
    m_connectionTabs(0),
    m_widget(0),
    m_mainLayout(0),
    m_leftWidget(0),
    m_interfaceWidget(0),
    m_leftLayout(0),
    m_interfaceLayout(0),
    m_connectionList(0),
    m_wirelessList(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    init();
}

NMPopup::~NMPopup()
{
}

void NMPopup::init()
{
    m_mainLayout = new QGraphicsGridLayout(this);

    m_leftWidget = new Plasma::TabBar(this);
    m_leftWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    m_interfaceWidget = new QGraphicsWidget(m_leftWidget);
    m_leftLayout = new QGraphicsLinearLayout;
    m_leftLayout->setOrientation(Qt::Vertical);

    m_interfaceLayout = new QGraphicsLinearLayout(m_interfaceWidget);
    m_interfaceLayout->setOrientation(Qt::Vertical);
    //m_interfaceWidget->setLayout(m_interfaceLayout);
    m_leftLayout->addItem(m_interfaceWidget);
    m_leftLayout->addStretch(5);

    // flight-mode checkbox
    m_networkingCheckBox = new Plasma::CheckBox(m_leftWidget);
    m_networkingCheckBox->setChecked(Solid::Control::NetworkManager::isNetworkingEnabled());
    m_networkingCheckBox->setText(i18nc("CheckBox to enable or disable networking completely", "Enable networking"));
    m_leftLayout->addItem(m_networkingCheckBox);
    connect(m_networkingCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(networkingEnabledToggled(bool)));

    // flight-mode checkbox
    m_rfCheckBox = new Plasma::CheckBox(m_leftWidget);
    m_rfCheckBox->setChecked(Solid::Control::NetworkManager::isWirelessEnabled());
    m_rfCheckBox->setEnabled(Solid::Control::NetworkManager::isWirelessHardwareEnabled());
    m_rfCheckBox->setText(i18nc("CheckBox to enable or disable wireless interface (rfkill)", "Enable wireless"));
    m_leftLayout->addItem(m_rfCheckBox);

    connect(m_rfCheckBox, SIGNAL(toggled(bool)), SLOT(wirelessEnabledToggled(bool)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));

    //m_leftWidget->setLayout(m_leftLayout);
    m_leftWidget->addTab(i18nc("tabbar on the left side", "Interfaces"), m_leftLayout);
    m_leftWidget->setTabBarShown(false); // TODO: enable


    m_interfaceDetailsWidget = new InterfaceDetailsWidget(m_leftWidget);
    connect(m_interfaceDetailsWidget, SIGNAL(back()), this, SLOT(toggleInterfaceTab()));
    m_leftWidget->addTab(i18nc("details for the interface", "Details"), m_interfaceDetailsWidget);
    m_leftWidget->setPreferredWidth(300);

    m_mainLayout->addItem(m_leftWidget, 0, 0);

    m_rightWidget = new QGraphicsWidget(this);
    m_rightWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    m_rightLayout = new QGraphicsLinearLayout(m_rightWidget);
    m_rightLayout->setOrientation(Qt::Vertical);
    // Tabs for activatables
    kDebug() << "Creating tABS";
    m_connectionTabs = new Plasma::TabBar(m_rightWidget);
    //m_connectionTabs->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_connectionTabs->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    m_connectionTabs->setPreferredHeight(320);

    m_connectionTabs->setPreferredWidth(320);

    m_rightLayout->addItem(m_connectionTabs);

    m_connectionsButton = new Plasma::PushButton(m_rightWidget);
    m_connectionsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_connectionsButton->setIcon(KIcon("networkmanager"));
    m_connectionsButton->setText(i18nc("manage connections button in the applet's popup", "Manage Connections..."));
    m_connectionsButton->setMaximumHeight(28);
    connect(m_connectionsButton, SIGNAL(clicked()), this, SLOT(manageConnections()));

    QGraphicsLinearLayout* connectionLayout = new QGraphicsLinearLayout;
    connectionLayout->addStretch();
    connectionLayout->addItem(m_connectionsButton);

    m_rightLayout->addItem(connectionLayout);

    m_mainLayout->addItem(m_rightWidget, 0, 1);

    createTab(Knm::Activatable::InterfaceConnection);
    createTab(Knm::Activatable::WirelessInterfaceConnection);
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

}

// Interfaces
void NMPopup::interfaceAdded(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        return;
    }
    kDebug() << "Interface Added.";
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    addInterfaceInternal(iface);
    switchToDefaultTab();
}

void NMPopup::interfaceRemoved(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        InterfaceItem * item = m_interfaces.take(uni);
        m_interfaceLayout->removeItem(item);
        delete item;
    }
    switchToDefaultTab();
}

Solid::Control::NetworkInterface* NMPopup::defaultInterface()
{
    // In fact we're returning the first available interface,
    // and if there is none available just the first one we have
    // and if we don't have one, 0. Make sure you check though.
    if (!Solid::Control::NetworkManager::networkInterfaces().count()) {
        return 0;
    }
    Solid::Control::NetworkInterface* iface = Solid::Control::NetworkManager::networkInterfaces().first();
    foreach (Solid::Control::NetworkInterface * _iface, Solid::Control::NetworkManager::networkInterfaces()) {
        switch (_iface->connectionState()) {
            case Solid::Control::NetworkInterface::Disconnected:
            case Solid::Control::NetworkInterface::Failed:
            case Solid::Control::NetworkInterface::Preparing:
            case Solid::Control::NetworkInterface::Configuring:
            case Solid::Control::NetworkInterface::NeedAuth:
            case Solid::Control::NetworkInterface::IPConfig:
            case Solid::Control::NetworkInterface::Activated:
                return _iface;
                break;
            case Solid::Control::NetworkInterface::Unavailable:
            case Solid::Control::NetworkInterface::Unmanaged:
            case Solid::Control::NetworkInterface::UnknownState:
            default:
                break;
        }
    }
    return iface;
}


void NMPopup::addInterfaceInternal(Solid::Control::NetworkInterface* iface)
{
    Q_ASSERT(iface);
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * ifaceItem = 0;
        if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            // Create the wireless interface item
            WirelessInterfaceItem* wifiItem = 0;
            wifiItem = new WirelessInterfaceItem(static_cast<Solid::Control::WirelessNetworkInterface *>(iface), InterfaceItem::InterfaceName, this);
            ifaceItem = wifiItem;
            //connect(wirelessinterface, SIGNAL(stateChanged()), this, SLOT(updateNetworks()));
            wifiItem->setEnabled(Solid::Control::NetworkManager::isWirelessEnabled());
            //createTab(ifaceItem, iface, i18nc("title of the wireless tab", "Wireless"), "network-wireless");
            kDebug() << "WiFi added";
            connect(wifiItem, SIGNAL(disconnectInterfaceRequested(const QString&)), m_wirelessList, SLOT(deactivateConnection(const QString&)));
        } else {
            // Create the interfaceitem
            ifaceItem = new InterfaceItem(static_cast<Solid::Control::WiredNetworkInterface *>(iface), InterfaceItem::InterfaceName, this);
            connect(ifaceItem, SIGNAL(disconnectInterfaceRequested(const QString&)), m_connectionList, SLOT(deactivateConnection(const QString&)));
        }
        connect(ifaceItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));

        // Catch connection changes
        connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        connect(iface, SIGNAL(linkUpChanged(bool)), this, SLOT(switchToDefaultTab()));
        m_interfaceLayout->addItem(ifaceItem);
        m_interfaces.insert(iface->uni(), ifaceItem);
    }
}

void NMPopup::createTab(Knm::Activatable::ActivatableType type)
{
    /*
    enum ActivatableType {
        InterfaceConnection,
        WirelessInterfaceConnection,
        WirelessNetwork,
        UnconfiguredInterface,
        VpnInterfaceConnection
    };
    */
    QString name;
    KIcon icon;
    switch(type) {
        case Knm::Activatable::WirelessInterfaceConnection:
        case Knm::Activatable::WirelessNetwork:
        {
            /*
                The Wireless Networks tab shows all wireless networks, preconfigured
                and new.
            */
            if (!m_wirelessList) {
                m_wirelessList = new ActivatableListWidget(m_activatables, m_connectionTabs);
                m_wirelessList->addType(Knm::Activatable::WirelessNetwork);
                m_wirelessList->addType(Knm::Activatable::WirelessInterfaceConnection);
                m_wirelessList->init();
                name = i18nc("title of the wireless tab", "Wireless Networks");
                //icon = KIcon("network-wireless");
                m_tabIndex[Knm::Activatable::WirelessInterfaceConnection] = m_connectionTabs->addTab(QIcon(), name, m_wirelessList);
            }
            break;
        }
        default:
        {
            /*
                The Connections tab shows all known available connections, wired,
                wireless and VPN.
            */
            if (!m_connectionList) {
                m_connectionList = new ActivatableListWidget(m_activatables, m_connectionTabs);
                m_connectionList->addType(Knm::Activatable::InterfaceConnection);
                m_connectionList->addType(Knm::Activatable::WirelessInterfaceConnection);
                m_connectionList->addType(Knm::Activatable::VpnInterfaceConnection);
                m_connectionList->init();
                name = i18nc("title of the connections tab", "Connections");
                //icon = KIcon("emblem-favorite");
                if (!m_connectionTabs)
                    kDebug() << "see, it's no there! :P";
                m_tabIndex[type] = m_connectionTabs->addTab(QIcon(), name, m_connectionList);
            }
            break;
        }
    }
}

void NMPopup::switchToDefaultTab()
{
    if (m_interfaces.count()) {
        m_connectionTabs->setCurrentIndex(0);
    }
}

void NMPopup::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Q_UNUSED( reason );
    // Switch to default tab if an interface has become available, or unavailable
    if (available(new_state) != available(old_state)) {
        switchToDefaultTab();
    }
}

bool NMPopup::available(int state)
{
    // Can an interface be used?
    switch (state) {
        case Solid::Control::NetworkInterface::Disconnected:
        case Solid::Control::NetworkInterface::Failed:
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
        case Solid::Control::NetworkInterface::Activated:
            return true;
            break;
        case Solid::Control::NetworkInterface::Unavailable:
        case Solid::Control::NetworkInterface::Unmanaged:
        case Solid::Control::NetworkInterface::UnknownState:
        default:
            return false;
            break;
    }
    return false;
}

void NMPopup::wirelessEnabledToggled(bool checked)
{
    kDebug() << "Applet wireless enable switch toggled" << checked;
    Solid::Control::NetworkManager::setWirelessEnabled(checked);
}

void NMPopup::networkingEnabledToggled(bool checked)
{
    kDebug() << "Applet networking enable switch toggled" << checked;
    Solid::Control::NetworkManager::setNetworkingEnabled(checked);
}

void NMPopup::managerWirelessEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wireless enable state" << enabled;
    // it might have changed because we toggled the switch,
    // but it might have been changed externally, so set it anyway
    m_rfCheckBox->setChecked(enabled);
    switchToDefaultTab();
}

void NMPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    m_rfCheckBox->setChecked(enabled && Solid::Control::NetworkManager::isWirelessEnabled());
    m_rfCheckBox->setEnabled(!enabled);
    switchToDefaultTab();
}

void NMPopup::manageConnections()
{
    //kDebug() << "opening connection management dialog";
    QStringList args;
    args << "--icon" << "networkmanager" << "kcm_networkmanagement" << "kcm_networkmanagement_tray";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

void NMPopup::toggleInterfaceTab()
{
    InterfaceItem* item = qobject_cast<InterfaceItem*>(sender());
    if (item) {
        m_interfaceDetailsWidget->setInterface(item->interface());
    }
    if (m_leftWidget->currentIndex() == 0) {
        m_leftWidget->setCurrentIndex(1);
    } else {
        m_leftWidget->setCurrentIndex(0);
    }
}
// vim: sw=4 sts=4 et tw=100

