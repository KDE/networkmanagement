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

#include "networkmanagerpopup.h"

#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QLabel>

#include <KDebug>
#include <KLocale>
#include <KPushButton>
#include <KToolInvocation>

#include <Plasma/Label>
#include <Plasma/PushButton>

#include <solid/control/networkmanager.h>

NetworkManagerPopup::NetworkManagerPopup(QGraphicsItem *parent)
    : QGraphicsWidget(parent)
{
    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    // a vertical list of appropriate connections
    m_connectionLayout = new QGraphicsLinearLayout(Qt::Vertical, m_layout);
    // just show the interfaces for now; graduate to showing appropriate connections later
    foreach (Solid::Control::NetworkInterface * interface,
            Solid::Control::NetworkManager::networkInterfaces()) {
        Plasma::Label * ifaceLabel = new Plasma::Label(this);
        ifaceLabel->setText(interface->interfaceName());
        m_connectionLayout->addItem(ifaceLabel);
    }
    m_layout->addItem(m_connectionLayout);
    // then a block of status labels and buttons
    // +----------------------------+
    // |              | [Manage...] |
    // | Wireless hw switch status  |
    // | [Networking] | [Wireless]  |
    // +----------------------------|
    QGraphicsGridLayout * gridLayout = new QGraphicsGridLayout(m_layout);
    m_btnManageConnections = new Plasma::PushButton(this);
    m_btnManageConnections->setText(i18nc("Button text for showing the Manage Connections KCModule", "Manage..."));
    gridLayout->addItem(m_btnManageConnections, 0, 1, 1, 1);
    m_lblRfkill = new Plasma::Label(this);
    m_lblRfkill->nativeWidget()->setWordWrap(false);
    managerWirelessHardwareEnabledChanged(Solid::Control::NetworkManager::isWirelessHardwareEnabled());
    gridLayout->addItem(m_lblRfkill, 1, 0, 1, 2);
    m_btnEnableNetworking = new Plasma::PushButton(this);
    m_btnEnableWireless = new Plasma::PushButton(this);
    m_btnEnableNetworking->nativeWidget()->setCheckable(true);
    m_btnEnableWireless->nativeWidget()->setCheckable(true);
    managerWirelessEnabledChanged(Solid::Control::NetworkManager::isWirelessEnabled());
    m_btnEnableNetworking->setText(i18nc("Label for pushbutton enabling networking", "Networking"));
    m_btnEnableWireless->setText(i18nc("Label for pushbutton enabling wireless", "Wireless"));
    gridLayout->addItem(m_btnEnableNetworking, 2, 0, 1, 2);
    gridLayout->addItem(m_btnEnableWireless, 3, 0, 1, 2);
    m_layout->addItem(gridLayout);
    setLayout(m_layout);
    // connect up the buttons and the manager's signals
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));
    QObject::connect(m_btnManageConnections, SIGNAL(clicked()),
            this, SLOT(manageConnections()));
    QObject::connect(m_btnEnableNetworking->nativeWidget(), SIGNAL(toggled(bool)),
            this, SLOT(userNetworkingEnabledChanged(bool)));
    QObject::connect(m_btnEnableWireless->nativeWidget(), SIGNAL(toggled(bool)),
            this, SLOT(userWirelessEnabledChanged(bool)));
}

NetworkManagerPopup::~NetworkManagerPopup()
{
    delete m_layout;

}

void NetworkManagerPopup::networkInterfaceAdded(const QString&)
{

}

void NetworkManagerPopup::networkInterfaceRemoved(const QString&)
{

}

void NetworkManagerPopup::overallStatusChanged(Solid::Networking::Status)
{

}

void NetworkManagerPopup::managerWirelessEnabledChanged(bool enabled)
{
    m_btnEnableWireless->nativeWidget()->setChecked(enabled);
}

void NetworkManagerPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    if (enabled) {
        m_lblRfkill->setText(i18nc("Label text when hardware wireless is enabled", "Wireless hardware is enabled"));
    } else {
        m_lblRfkill->setText(i18nc("Label text when hardware wireless is not enabled", "Wireless hardware is disabled"));
    }
}

void NetworkManagerPopup::userNetworkingEnabledChanged(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManager::setNetworkingEnabled(enabled);
}

void NetworkManagerPopup::userWirelessEnabledChanged(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManager::setWirelessEnabled(enabled);
}

void NetworkManagerPopup::manageConnections()
{
    kDebug() << "opening connection management dialog";
    QStringList args;
    args << "kcm_knetworkmanager";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

#include "networkmanagerpopup.moc"

// vim: sw=4 sts=4 et tw=100
