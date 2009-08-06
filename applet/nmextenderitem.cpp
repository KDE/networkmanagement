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
    m_wiredList(0),
    m_wirelessList(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setName("nmextenderitem");
    setTitle(i18nc("Extender title", "Network Management"));
    widget();
    init();

    m_showWired = config().readEntry("showWired", true);
    m_showWireless = config().readEntry("showWireless", true);
    m_showCellular = config().readEntry("showCellular", true);
    m_showVpn = config().readEntry("showVpn", true);
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
    if (m_showWired) {
        createTab(Knm::Activatable::InterfaceConnection);
    }
    if (m_showWireless) {
        createTab(Knm::Activatable::WirelessInterfaceConnection);
    }
    if (m_showVpn) {
        createTab(Knm::Activatable::VpnInterfaceConnection);
    }
    /*
    // TODO: doesn't exist in Activatable
    if (m_showCellular) {
        createTab(Knm::Activatable::
    }
    */
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
        m_widget->setMinimumWidth(500);
        m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_mainLayout = new QGraphicsGridLayout(m_widget);
        //m_mainLayout = new QGraphicsLinearLayout(m_widget);
        //m_mainLayout->setOrientation(Qt::Horizontal);
        m_mainLayout->setColumnMinimumWidth(0, 200);
        m_mainLayout->setColumnMinimumWidth(1, 340);
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

        // Manage connections and flight-mode buttons
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
        m_connectionTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_connectionTabs->setPreferredSize(300, 240);
        m_connectionTabs->setMinimumSize(280, 240);

        //m_mainLayout->addItem(m_connectionTabs, 0, 1, 1, 1);
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
    } else {
        //kDebug() << "widget non empty";
    }
    //kDebug() << "widget() run";
    return m_widget;
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

                // Add a wired tab
                //createTab(wiredItem, iface, i18nc("title of the wired tab", "Wired"), icon);
                break;
            }
        }
        // Connect tab switching
        connect(ifaceItem, SIGNAL(clicked(int)), this, SLOT(switchTab(int)));
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
        case Knm::Activatable::InterfaceConnection:
        {
            if (!m_wiredList) {
                m_wiredList = new ActivatableListWidget(m_activatables, m_connectionTabs);
                m_wiredList->addType(Knm::Activatable::InterfaceConnection);
                m_wiredList->init();
                name = i18nc("title of the wired tab", "Wired");
                icon = KIcon("network-wired");
                m_tabIndex[type] = m_connectionTabs->addTab(icon, name, m_wiredList);
            }
            break;
        }
        case Knm::Activatable::WirelessInterfaceConnection:
        case Knm::Activatable::WirelessNetwork:
        {
            if (!m_wirelessList) {
                m_wirelessList = new ActivatableListWidget(m_activatables, m_connectionTabs);
                m_wirelessList->addType(Knm::Activatable::WirelessInterfaceConnection);
                m_wirelessList->addType(Knm::Activatable::WirelessNetwork);
                m_wirelessList->init();
                name = i18nc("title of the wireless tab", "Wireless");
                icon = KIcon("network-wireless");
                // All wireless stuff goes into one tab, marked as WirelessInterfaceConnection
                // (no separation between those connections and WirelessNetworks)
                m_tabIndex[Knm::Activatable::WirelessInterfaceConnection] = m_connectionTabs->addTab(icon, name, m_wirelessList);
            }
            break;
        }
        case Knm::Activatable::UnconfiguredInterface:
        case Knm::Activatable::VpnInterfaceConnection:
        {
            m_vpnList = new ActivatableListWidget(m_activatables, m_connectionTabs);
            m_vpnList->addType(Knm::Activatable::VpnInterfaceConnection);
            m_vpnList->init();
            name = i18nc("VPN connections tab", "VPN");
            icon = KIcon("network-wired"); // FIXME: icon
            kDebug() << "New VPN:" << name;
            m_tabIndex[Knm::Activatable::VpnInterfaceConnection] = m_connectionTabs->addTab(icon, name, m_vpnList);
            break;
        }
    }
}

void NMExtenderItem::switchTab(int type)
{
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
        switchTab(defaultInterface()->type());
    }
}

void NMExtenderItem::showWired(bool show)
{
    if (m_showWired == show) {
        return;
    }
    kDebug() << "Show wired?" << show;
    m_showWired = show;
    if (!show) {
        if (m_wiredList) {
            kDebug() << "deleting the tab" << Knm::Activatable::InterfaceConnection;
            m_connectionTabs->removeTab(m_tabIndex[Knm::Activatable::InterfaceConnection]);
            //delete m_wiredList;
            //m_wiredList = 0; this crashes at some point, but why?
            m_tabIndex.remove(Knm::Activatable::InterfaceConnection);
        }
    } else {
        createTab(Knm::Activatable::InterfaceConnection);
    }
    config().writeEntry("showWired", show);
    emit configNeedsSaving();
}

void NMExtenderItem::showWireless(bool show)
{
    if (m_showWireless == show) {
        return;
    }
    m_showWireless = show;
    config().writeEntry("showWireless", show);
    emit configNeedsSaving();
    // TODO
}

void NMExtenderItem::showVpn(bool show)
{
    if (m_showVpn == show) {
        return;
    }
    m_showVpn = show;
    config().writeEntry("showVpn", show);
    emit configNeedsSaving();
    // TODO
}

void NMExtenderItem::showCellular(bool show)
{
    if (m_showCellular == show) {
        return;
    }
    m_showCellular = show;
    config().writeEntry("showCellular", show);
    emit configNeedsSaving();
    // TODO
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

