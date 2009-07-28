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
#include <Plasma/Extender>
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
    m_widget(0),
    m_mainLayout(0),
    m_leftWidget(0),
    m_interfaceWidget(0),
    m_leftLayout(0),
    m_interfaceLayout(0),
    m_connectionTabs(0)
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
        m_widget->setMinimumWidth(600);
        m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        //m_mainLayout = new QGraphicsGridLayout(m_widget);
        m_mainLayout = new QGraphicsLinearLayout(m_widget);
        m_mainLayout->setOrientation(Qt::Horizontal);
        //m_mainLayout->setColumnFixedWidth(0, 200);
        //m_mainLayout->setColumnFixedWidth(1, 260);
        m_widget->setLayout(m_mainLayout);


        m_leftWidget = new QGraphicsWidget(m_widget);
        m_interfaceWidget = new QGraphicsWidget(m_leftWidget);
        m_leftLayout = new QGraphicsLinearLayout(m_leftWidget);
        m_leftLayout->setOrientation(Qt::Vertical);

        m_interfaceLayout = new QGraphicsLinearLayout(m_interfaceWidget);
        m_interfaceLayout->setOrientation(Qt::Vertical);
        //m_interfaceWidget->setLayout(m_interfaceLayout);
        m_leftLayout->addItem(m_interfaceWidget);
        m_leftWidget->setLayout(m_leftLayout);
        m_mainLayout->addItem(m_leftWidget);

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

        m_connectionsButton = new Plasma::IconWidget(m_leftWidget);
        m_connectionsButton->setIcon("networkmanager");
        m_connectionsButton->setOrientation(Qt::Horizontal);
        m_connectionsButton->setText(i18nc("button in general settings extender", "Manage Connections..."));
        m_connectionsButton->setMaximumHeight(KIconLoader::SizeMedium);
        m_connectionsButton->setMinimumHeight(KIconLoader::SizeMedium);
        m_connectionsButton->setDrawBackground(true);
#if KDE_IS_VERSION(4,2,60)
        m_connectionsButton->setTextBackgroundColor(QColor());
#endif

        connect(m_connectionsButton, SIGNAL(activated()), this, SLOT(manageConnections()));
        m_leftLayout->addItem(m_connectionsButton);
        m_mainLayout->addItem(m_leftWidget);

        // Tabs for activatables
        m_connectionTabs = new Plasma::TabBar(m_widget);
        //m_connectionTabs->setTabBarShown(false);
        m_connectionTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_connectionTabs->setPreferredSize(260, 240);
        m_connectionTabs->setMinimumSize(260, 240);

        //m_mainLayout->addItem(m_connectionTabs, 0, 1, 1, 1);
        m_mainLayout->addItem(m_connectionTabs);
        setWidget(m_widget);
    } else {
        kDebug() << "widget non empty";
    }
    kDebug() << "widget() run";
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

void NMExtenderItem::switchTab(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        m_connectionTabs->setCurrentIndex(m_tabIndex[uni]);
    }
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
                WirelessInterfaceItem * wifiItem = 0;
                wifiItem = new WirelessInterfaceItem(static_cast<Solid::Control::WirelessNetworkInterface *>(iface), InterfaceItem::InterfaceName, this);
                ifaceItem = wifiItem;
                //connect(wirelessinterface, SIGNAL(stateChanged()), this, SLOT(updateNetworks()));
                wifiItem->setEnabled(Solid::Control::NetworkManager::isWirelessEnabled());
                createTab(ifaceItem, iface, i18nc("title of the wireless tab", "Wireless"), "network-wireless");
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
                WiredInterfaceItem * wiredItem = 0;
                ifaceItem = wiredItem = new WiredInterfaceItem(static_cast<Solid::Control::WiredNetworkInterface *>(iface), InterfaceItem::InterfaceName, this);

                // Add a wired tab
                createTab(wiredItem, iface, i18nc("title of the wired tab", "Wired"), icon);
                break;
            }
        }
        //interface->setEnabled(m_enabled);
        m_interfaceLayout->addItem(ifaceItem);
        m_interfaces.insert(iface->uni(), ifaceItem);
        //m_interfaceLayout->invalidate();
        //m_interfaceLayout->updateGeometry();
        //updateNetworks();
    }
    //show();
    //emit updateLayout();
}

void NMExtenderItem::createTab(InterfaceItem * item, Solid::Control::NetworkInterface* iface, const QString &name, const QString &icon)
{
    // Add it to the list of connectables
    ActivatableListWidget* aList = new ActivatableListWidget(m_activatables, iface, m_connectionTabs);
    m_tabIndex[iface->uni()] = m_connectionTabs->addTab(KIcon(icon), name, aList);
    connect(item, SIGNAL(clicked(const QString&)), this, SLOT(switchTab(const QString&)));
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
}

void NMExtenderItem::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    m_rfCheckBox->setChecked(enabled && Solid::Control::NetworkManager::isWirelessEnabled());
    m_rfCheckBox->setEnabled(!enabled);
}

void NMExtenderItem::manageConnections()
{
    //kDebug() << "opening connection management dialog";
    QStringList args;
    args << "kcm_networkmanagement";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}


// vim: sw=4 sts=4 et tw=100

