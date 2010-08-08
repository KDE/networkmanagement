/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008, 2009 Sebastian Kügler <sebas@kde.org>

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
#include <Plasma/Separator>

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
#include "vpninterfaceitem.h"
#include "activatablelistwidget.h"
#include "interfacedetailswidget.h"
#include "uiutils.h"

NMPopup::NMPopup(RemoteActivatableList * activatableList, QGraphicsWidget* parent)
: QGraphicsWidget(parent),
    m_activatables(activatableList),
    m_widget(0),
    m_mainLayout(0),
    m_leftWidget(0),
    m_leftLayout(0),
    m_interfaceLayout(0),
    m_connectionList(0),
    m_vpnItem(0)
{
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    init();
}

NMPopup::~NMPopup()
{
}

void NMPopup::init()
{
    m_mainLayout = new QGraphicsGridLayout(this);

    m_leftLabel = new Plasma::Label(this);
    //m_leftLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_leftLabel->setMaximumHeight(24);
    m_leftLabel->setMinimumHeight(24);
    m_leftLabel->setText(i18nc("title on the LHS of the plasmoid", "<h3>Interfaces</h3>"));
    m_mainLayout->addItem(m_leftLabel, 0, 0);

    m_rightLabel = new Plasma::Label(this);
    //m_rightLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_leftLabel->setMaximumHeight(24);
    m_leftLabel->setMinimumHeight(24);
    m_rightLabel->setText(i18nc("title on the RHS of the plasmoid", "<h3>Connections</h3>"));
    m_mainLayout->addItem(m_rightLabel, 0, 2);

    Plasma::Separator* sep = new Plasma::Separator(this);
    sep->setOrientation(Qt::Vertical);
    m_mainLayout->addItem(sep, 0, 1, 2, 1);
    m_mainLayout->setRowFixedHeight(0, 24);

    m_leftWidget = new Plasma::TabBar(this);
    m_leftWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_leftLayout = new QGraphicsLinearLayout;
    m_leftLayout->setOrientation(Qt::Vertical);

    m_interfaceLayout = new QGraphicsLinearLayout;
    m_interfaceLayout->setOrientation(Qt::Vertical);
    m_interfaceLayout->setSpacing(0);

    m_leftLayout->addItem(m_interfaceLayout);
    m_leftLayout->addStretch(5);

    Plasma::Separator* sep2 = new Plasma::Separator(this);
    sep->setOrientation(Qt::Vertical);
    m_leftLayout->addItem(sep2);
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

    m_mainLayout->addItem(m_leftWidget, 1, 0);

    m_rightWidget = new QGraphicsWidget(this);
    m_rightWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    m_rightLayout = new QGraphicsLinearLayout(m_rightWidget);
    m_rightLayout->setOrientation(Qt::Vertical);

    // List with activatables
    m_connectionList = new ActivatableListWidget(m_activatables, m_rightWidget);
    m_connectionList->addType(Knm::Activatable::InterfaceConnection);
    m_connectionList->addType(Knm::Activatable::WirelessInterfaceConnection);
    m_connectionList->addType(Knm::Activatable::VpnInterfaceConnection);
    m_connectionList->addType(Knm::Activatable::GsmInterfaceConnection);
    m_connectionList->init();
    connect(m_interfaceDetailsWidget, SIGNAL(back()), m_connectionList, SLOT(clearInterfaces()));

    m_connectionList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //m_connectionList->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    m_connectionList->setPreferredHeight(240);

    m_connectionList->setMinimumWidth(320);
    m_connectionList->setShowAllTypes(false);

    m_rightLayout->addItem(m_connectionList);

    m_connectionsButton = new Plasma::PushButton(m_rightWidget);
    m_connectionsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_connectionsButton->setIcon(KIcon("configure"));
    m_connectionsButton->setText(i18nc("manage connections button in the applet's popup", "Manage Connections..."));
    m_connectionsButton->setMinimumHeight(28);
    m_connectionsButton->setMaximumHeight(28);
    connect(m_connectionsButton, SIGNAL(clicked()), this, SLOT(manageConnections()));

    m_showMoreButton = new Plasma::PushButton(m_rightWidget);
    m_showMoreButton->setCheckable(true);
    m_showMoreButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_showMoreButton->setIcon(KIcon("list-add"));
    m_showMoreButton->setText(i18nc("show more button in the applet's popup", "Show More..."));
    m_showMoreButton->setMinimumHeight(28);
    m_showMoreButton->setMaximumHeight(28);
    connect(m_showMoreButton, SIGNAL(clicked()), this, SLOT(showMore()));

    QGraphicsLinearLayout* connectionLayout = new QGraphicsLinearLayout;
    //connectionLayout->addStretch();
    connectionLayout->addItem(m_showMoreButton);
    connectionLayout->addItem(m_connectionsButton);

    m_rightLayout->addItem(connectionLayout);

    m_mainLayout->addItem(m_rightWidget, 1, 2);

    //createTab(Knm::Activatable::WirelessInterfaceConnection);
    kDebug() << "Adding interfaces initially";
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        addInterfaceInternal(iface);
        kDebug() << "Network Interface:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
    }
    addVpnInterface();
    // hook up signals to allow us to change the connection list depending on APs present, etc
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            SLOT(interfaceAdded(const QString&)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            SLOT(interfaceRemoved(const QString&)));

    //setPreferredSize(640, 400);


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
}

void NMPopup::interfaceRemoved(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        // To prevent crashes when the interface removed is the one in interfaceDetailsWidget.
        // the m_iface pointer in interfaceDetailsWidget become invalid in this case.
        if (uni == m_interfaceDetailsWidget->getLastIfaceUni()) {
            m_interfaceDetailsWidget->setInterface(0);
            // Since it is invalid go back to "main" window. 
            m_leftWidget->setCurrentIndex(0);
        }

        InterfaceItem* item = m_interfaces.take(uni);
        connect(item, SIGNAL(disappearAnimationFinished()), this, SLOT(deleteInterfaceItem()));
        item->disappear();
    }
}

void NMPopup::deleteInterfaceItem()
{
    // slot is called from animation's finished()
    InterfaceItem* item = dynamic_cast<InterfaceItem*>(sender());
    m_interfaceLayout->removeItem(item);
    delete item;
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
    if (!iface) {
        // the interface might be gone in the meantime...
        return;
    }
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * ifaceItem = 0;
        if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            // Create the wireless interface item
            WirelessInterfaceItem* wifiItem = 0;
            wifiItem = new WirelessInterfaceItem(static_cast<Solid::Control::WirelessNetworkInterface *>(iface), m_activatables, InterfaceItem::InterfaceName, this);
            ifaceItem = wifiItem;
            //connect(wirelessinterface, SIGNAL(stateChanged()), this, SLOT(updateNetworks()));
            wifiItem->setEnabled(Solid::Control::NetworkManager::isWirelessEnabled());
            kDebug() << "WiFi added";
            connect(wifiItem, SIGNAL(disconnectInterfaceRequested(const QString&)), m_connectionList, SLOT(deactivateConnection(const QString&)));
        } else {
            // Create the interfaceitem
            ifaceItem = new InterfaceItem(static_cast<Solid::Control::WiredNetworkInterface *>(iface), m_activatables, InterfaceItem::InterfaceName, this);
            connect(ifaceItem, SIGNAL(disconnectInterfaceRequested(const QString&)), m_connectionList, SLOT(deactivateConnection(const QString&)));
        }
        connect(ifaceItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
        connect(ifaceItem, SIGNAL(clicked(Solid::Control::NetworkInterface*)),
                m_connectionList,  SLOT(addInterface(Solid::Control::NetworkInterface*)));

        // Catch connection changes
        connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        m_interfaceLayout->addItem(ifaceItem);
        m_interfaces.insert(iface->uni(), ifaceItem);
    }
}

void NMPopup::addVpnInterface()
{
    m_vpnItem = new VpnInterfaceItem(0, m_activatables, InterfaceItem::InterfaceName, this);
    connect(m_vpnItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
    connect(m_vpnItem, SIGNAL(clicked(Solid::Control::NetworkInterface*)),
            m_connectionList,  SLOT(addInterface(Solid::Control::NetworkInterface*)));

    connect(m_vpnItem, SIGNAL(clicked()), m_connectionList, SLOT(toggleVpn()));

    m_leftLayout->insertItem(2, m_vpnItem);
}

void NMPopup::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Q_UNUSED( reason );
    // Switch to default tab if an interface has become available, or unavailable
    if (available(new_state) != available(old_state)) {
        m_connectionList->clearInterfaces();
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
    showMore(false);
    if (checked && Solid::Control::NetworkManager::isNetworkingEnabled()) {
        showMore(false);
        m_showMoreButton->show();
    } else {
        m_showMoreButton->hide();
    }
}

void NMPopup::networkingEnabledToggled(bool checked)
{
    // Switch networking on / off
    Solid::Control::NetworkManager::setNetworkingEnabled(checked);
    // Update wireless checkbox
    m_rfCheckBox->setEnabled(checked);
    m_rfCheckBox->setChecked(Solid::Control::NetworkManager::isWirelessHardwareEnabled() &&  Solid::Control::NetworkManager::isWirelessEnabled());
    m_showMoreButton->setChecked(false);
    if (checked && Solid::Control::NetworkManager::isWirelessHardwareEnabled() &&
                   Solid::Control::NetworkManager::isWirelessEnabled()) {
        showMore(false);
        m_showMoreButton->show();
    } else {
        m_showMoreButton->hide();
    }

}

void NMPopup::managerWirelessEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wireless enable state" << enabled;
    // it might have changed because we toggled the switch,
    // but it might have been changed externally, so set it anyway
    m_rfCheckBox->setChecked(enabled);
}

void NMPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    m_rfCheckBox->setChecked(enabled && Solid::Control::NetworkManager::isWirelessEnabled());
    m_rfCheckBox->setEnabled(!enabled);
}

void NMPopup::showMore()
{
    showMore(m_showMoreButton->isChecked());
}

void NMPopup::showMore(bool more)
{
    if (more) {
        m_showMoreButton->setText(i18nc("pressed show more button", "Show Less..."));
        m_showMoreButton->setIcon(KIcon("list-remove"));
        m_showMoreButton->setChecked(true);
        m_connectionList->setShowAllTypes(true);
    } else {
        m_showMoreButton->setText(i18nc("unpressed show more button", "Show More..."));
        m_showMoreButton->setChecked(false);
        m_connectionList->setShowAllTypes(false);
        m_showMoreButton->setIcon(KIcon("list-add"));
    }
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
        showMore(true);
        m_leftWidget->setCurrentIndex(1);
        // Enable / disable updating of the details widget
        m_interfaceDetailsWidget->setUpdateEnabled(true);

        if (item->interface()) {
            m_leftLabel->setText(QString("<h3>%1</h3>").arg(
                                UiUtils::interfaceNameLabel(item->interface()->uni())));
        }
    } else {
        m_leftLabel->setText(i18nc("title on the LHS of the plasmoid", "<h3>Interfaces</h3>"));
        showMore(false);
        m_interfaceDetailsWidget->setUpdateEnabled(false);
        m_leftWidget->setCurrentIndex(0);
    }
    //showMore();
}

QSizeF NMPopup::sizeHint (Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF sh = QGraphicsWidget::sizeHint(which, constraint);
    qreal temp1 = m_interfaceDetailsWidget->size().rwidth();

    if (temp1 < 300) {
        temp1 = 300;
    }

    m_leftWidget->setPreferredWidth(temp1);

    return sh;
}
// vim: sw=4 sts=4 et tw=100

