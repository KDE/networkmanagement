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
#include "nmextenderitem.h"

// Qt
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

// KDE
#include <KDebug>
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
#include "wiredinterfaceitem.h"
#include "activatablelistwidget.h"

NMExtenderItem::NMExtenderItem(RemoteActivatableList * activatableList, Plasma::Extender * ext)
: Plasma::ExtenderItem(ext),
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
    setName("nmextenderitem");
    setTitle(i18nc("Extender title", "Network Management"));
    widget();
    init();
}

NMExtenderItem::~NMExtenderItem()
{
}

void NMExtenderItem::init()
{
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

QGraphicsItem * NMExtenderItem::widget()
{
    if (!m_widget) {
        kDebug() << "Creating widget";
        m_widget = new QGraphicsWidget(this);
        m_widget->setMinimumWidth(600);
        m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_mainLayout = new QGraphicsGridLayout(m_widget);
        m_widget->setLayout(m_mainLayout);

        m_leftWidget = new Plasma::Frame(m_widget);
        m_leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_interfaceWidget = new QGraphicsWidget(m_leftWidget);
        m_leftLayout = new QGraphicsLinearLayout(m_leftWidget);
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

        m_leftWidget->setLayout(m_leftLayout);
        m_mainLayout->addItem(m_leftWidget, 0, 0);


        m_rightWidget = new Plasma::Frame(m_widget);
        m_rightWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_rightLayout = new QGraphicsLinearLayout(m_rightWidget);
        m_rightLayout->setOrientation(Qt::Vertical);
        // Tabs for activatables
        m_connectionTabs = new Plasma::TabBar(m_rightWidget);
        //m_connectionTabs->setTabBarShown(false);
        //m_connectionTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //m_connectionTabs->setPreferredSize(300, 240);
        m_connectionTabs->setMinimumSize(280, 240);

        m_rightLayout->addItem(m_connectionTabs);

        m_connectionsButton = new Plasma::IconWidget(m_rightWidget);
        m_connectionsButton->setIcon("networkmanager");
        m_connectionsButton->setOrientation(Qt::Horizontal);
        m_connectionsButton->setText(i18nc("button in general settings extender", "Manage Connections..."));
        m_connectionsButton->setMaximumHeight(KIconLoader::SizeSmallMedium);
        m_connectionsButton->setMinimumHeight(KIconLoader::SizeSmallMedium);
        m_connectionsButton->setDrawBackground(true);
#if KDE_IS_VERSION(4,2,60)
        m_connectionsButton->setTextBackgroundColor(QColor());
#endif

        connect(m_connectionsButton, SIGNAL(activated()), this, SLOT(manageConnections()));
        m_rightLayout->addItem(m_connectionsButton);

        m_mainLayout->addItem(m_rightWidget, 0, 1);
        setWidget(m_widget);
    }
    return m_widget;
}

// Interfaces
void NMExtenderItem::interfaceAdded(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        return;
    }
    kDebug() << "Interface Added.";
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    addInterfaceInternal(iface);
    switchToDefaultTab();
}

void NMExtenderItem::interfaceRemoved(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        InterfaceItem * item = m_interfaces.take(uni);
        m_interfaceLayout->removeItem(item);
        // TODO: remove tab
        delete item;
    }
    switchToDefaultTab();
}

Solid::Control::NetworkInterface* NMExtenderItem::defaultInterface()
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


void NMExtenderItem::addInterfaceInternal(Solid::Control::NetworkInterface* iface)
{
    Q_ASSERT(iface);
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * ifaceItem = 0;
        QString icon;
        switch (iface->type()) {
            case Solid::Control::NetworkInterface::Ieee80211:
            {
                // Create the wireless interface item
                WirelessInterfaceItem* wifiItem = 0;
                wifiItem = new WirelessInterfaceItem(static_cast<Solid::Control::WirelessNetworkInterface *>(iface), InterfaceItem::InterfaceName, this);
                ifaceItem = wifiItem;
                //connect(wirelessinterface, SIGNAL(stateChanged()), this, SLOT(updateNetworks()));
                wifiItem->setEnabled(Solid::Control::NetworkManager::isWirelessEnabled());
                //createTab(ifaceItem, iface, i18nc("title of the wireless tab", "Wireless"), "network-wireless");
                kDebug() << "WiFi added";
                connect(wifiItem, SIGNAL(disconnectInterface()), m_wirelessList, SLOT(deactivateConnection()));
                break;
            }
            case Solid::Control::NetworkInterface::Serial:
                icon = "phone";
            case Solid::Control::NetworkInterface::Gsm:
                icon = "phone";
            case Solid::Control::NetworkInterface::Cdma:
                icon = "phone";
            case Solid::Control::NetworkInterface::Ieee8023:
                icon = "network-wired";
            default:
            {
                // Create the interfaceitem
                WiredInterfaceItem* wiredItem = 0;
                ifaceItem = wiredItem = new WiredInterfaceItem(static_cast<Solid::Control::WiredNetworkInterface *>(iface), InterfaceItem::InterfaceName, this);

                connect(wiredItem, SIGNAL(disconnectInterface()), m_connectionList, SLOT(deactivateConnection()));
                // Add a wired tab
                //createTab(wiredItem, iface, i18nc("title of the wired tab", "Wired"), icon);
                break;
            }
        }
        // Catch connection changes
        connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        connect(iface, SIGNAL(linkUpChanged(bool)), this, SLOT(switchToDefaultTab()));
        m_interfaceLayout->addItem(ifaceItem);
        m_interfaces.insert(iface->uni(), ifaceItem);
    }
}

void NMExtenderItem::createTab(Knm::Activatable::ActivatableType type)
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
                icon = KIcon("network-wireless");
                m_tabIndex[Knm::Activatable::WirelessInterfaceConnection] = m_connectionTabs->addTab(icon, name, m_wirelessList);
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
                icon = KIcon("emblem-favorite");
                m_tabIndex[type] = m_connectionTabs->addTab(icon, name, m_connectionList);
            }
            break;
        }
    }
}

void NMExtenderItem::switchTab(int type)
{
    kDebug() << "Switching to ..." << type;
    switch (type) {
        case Solid::Control::NetworkInterface::Ieee80211:
        {
            m_connectionTabs->setCurrentIndex(m_tabIndex[Knm::Activatable::WirelessInterfaceConnection]);
            break;
        }
        case Solid::Control::NetworkInterface::Serial:
        case Solid::Control::NetworkInterface::Gsm:
        case Solid::Control::NetworkInterface::Cdma:
        case Solid::Control::NetworkInterface::Ieee8023:
        default:
        {
            m_connectionTabs->setCurrentIndex(m_tabIndex[Knm::Activatable::InterfaceConnection]);
            break;
        }
    }
}

void NMExtenderItem::switchToDefaultTab()
{
    if (m_interfaces.count()) {
        m_connectionTabs->setCurrentIndex(0);
        //switchTab(defaultInterface()->type());
    }
}



void NMExtenderItem::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Q_UNUSED( reason );
    // Switch to default tab if an interface has become available, or unavailable
    if (available(new_state) != available(old_state)) {
        switchToDefaultTab();
    }
}

bool NMExtenderItem::available(int state)
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

void NMExtenderItem::wirelessEnabledToggled(bool checked)
{
    kDebug() << "Applet wireless enable switch toggled" << checked;
    Solid::Control::NetworkManager::setWirelessEnabled(checked);
}

void NMExtenderItem::networkingEnabledToggled(bool checked)
{
    kDebug() << "Applet networking enable switch toggled" << checked;
    Solid::Control::NetworkManager::setNetworkingEnabled(checked);
}

void NMExtenderItem::managerWirelessEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wireless enable state" << enabled;
    // it might have changed because we toggled the switch,
    // but it might have been changed externally, so set it anyway
    m_rfCheckBox->setChecked(enabled);
    switchToDefaultTab();
}

void NMExtenderItem::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    m_rfCheckBox->setChecked(enabled && Solid::Control::NetworkManager::isWirelessEnabled());
    m_rfCheckBox->setEnabled(!enabled);
    switchToDefaultTab();
}

void NMExtenderItem::manageConnections()
{
    //kDebug() << "opening connection management dialog";
    QStringList args;
    args << "kcm_networkmanagement";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

// vim: sw=4 sts=4 et tw=100

