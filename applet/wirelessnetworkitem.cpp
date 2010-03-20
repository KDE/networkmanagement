/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
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

#include "wirelessnetworkitem.h"
#include "wirelessstatus.h"

#include <QAction>
#include <QLabel>
#include <QGraphicsGridLayout>

#include <KGlobalSettings>
#include <KIcon>
#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "remotewirelessnetwork.h"
#include "remotewirelessinterfaceconnection.h"
#include <wirelesssecurityidentifier.h>
#include "activatable.h"

WirelessNetworkItem::WirelessNetworkItem(RemoteWirelessNetwork * remote, QGraphicsItem * parent)
: ActivatableItem(remote, parent),
    m_strengthMeter(0),
    m_connectButton(0),
    m_securityIcon(0),
    //m_securityIconName(),
    m_strength(0),
    m_remote(remote),
    m_wirelessStatus(0)
{
    //m_strength = 0;
    m_wirelessStatus = new WirelessStatus(remote);
    connect(m_wirelessStatus, SIGNAL(strengthChanged(int)), this, SLOT(setStrength(int)));
    connect(m_remote, SIGNAL(changed()), SLOT(update()));
    connect(m_remote, SIGNAL(changed()), SLOT(stateChanged()));
    m_state = Knm::InterfaceConnection::Unknown;
    RemoteWirelessInterfaceConnection* remoteconnection = static_cast<RemoteWirelessInterfaceConnection*>(m_activatable);
    if (remoteconnection) {
        m_state = remoteconnection->activationState();
    }
    stateChanged();
    update();

}

void WirelessNetworkItem::setupItem()
{
    // painting of a wifi network, known connection or available access point
    /*
    +----+-------------+-----+---+
    |icon essid        |meter|sec|
    +----+-------------+-----+---+
    */
    int rowHeight = 24;
    int spacing = 4;

    m_layout = new QGraphicsGridLayout(this);
    // First, third and fourth colunm are fixed width for the icons
    m_layout->setColumnPreferredWidth(0, 160);
    m_layout->setColumnFixedWidth(1, 60);
    m_layout->setColumnFixedWidth(2, rowHeight);
    m_layout->setColumnSpacing(1, spacing);

    // icon on the left
    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setAcceptsHoverEvents(false);
    m_connectButton->setIcon("network-wireless"); // Known connection, we probably have credentials
    if (interfaceConnection()) {
        m_connectButton->setText(interfaceConnection()->connectionName());
        QAction *a = new QAction(KIcon("emblem-favorite"), QString(), m_connectButton);
        m_connectButton->addIconAction(a);
    } else {
        m_connectButton->setText(m_wirelessStatus->ssid());
    }
    m_connectButton->setMinimumWidth(160);
    m_connectButton->setOrientation(Qt::Horizontal);
    m_connectButton->setTextBackgroundColor(QColor(Qt::transparent));
    //m_connectButton->setToolTip(i18nc("icon to connect to wireless network", "Connect to wireless network %1", ssid));
    m_connectButton->setMinimumHeight(rowHeight);
    m_connectButton->setMaximumHeight(rowHeight);
    m_layout->addItem(m_connectButton, 0, 0, 1, 1 );

    m_strengthMeter = new Plasma::Meter(this);
    m_strengthMeter->setMinimum(0);
    m_strengthMeter->setMaximum(100);
    m_strengthMeter->setValue(m_strength);
    m_strengthMeter->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_strengthMeter->setPreferredSize(QSizeF(60, rowHeight/2));
    m_strengthMeter->setMaximumHeight(rowHeight/2);
    m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout->addItem(m_strengthMeter, 0, 1, 1, 1, Qt::AlignCenter);

    m_securityIcon = new Plasma::IconWidget(this);
    m_securityIcon->setIcon(m_wirelessStatus->securityIcon());
    m_securityIcon->setMinimumHeight(22);
    m_securityIcon->setMaximumHeight(22);
    m_securityIcon->setToolTip(m_securityIconToolTip);
    m_layout->addItem(m_securityIcon, 0, 2, 1, 1, Qt::AlignLeft);

    connect(this, SIGNAL(clicked()), this, SLOT(emitClicked()));

    // Forward clicks and presses between our widgets and this
    connect(this, SIGNAL(pressed(bool)), m_connectButton, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(pressed(bool)), this, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(emitClicked()));

    connect(this, SIGNAL(pressed(bool)), m_securityIcon, SLOT(setPressed(bool)));
    connect(m_securityIcon, SIGNAL(pressed(bool)), this, SLOT(setPressed(bool)));
    connect(m_securityIcon, SIGNAL(clicked()), this, SLOT(emitClicked()));
    activationStateChanged(m_state);

    update();
}

WirelessNetworkItem::~WirelessNetworkItem()
{
}

void WirelessNetworkItem::stateChanged()
{
    RemoteWirelessInterfaceConnection* remoteconnection = static_cast<RemoteWirelessInterfaceConnection*>(m_activatable);
    if (remoteconnection) {
        activationStateChanged(remoteconnection->activationState());
    }
}

void WirelessNetworkItem::setStrength(int strength)
{
    //kDebug() << m_wirelessStatus->ssid() << "signal strength changed from " << m_strength << "to " << strength;
    if (strength == m_strength) {
        return;
    }
    m_strength = strength;
    if (m_strengthMeter) {
        m_strengthMeter->setValue(m_strength);
    }
}

void WirelessNetworkItem::activationStateChanged(Knm::InterfaceConnection::ActivationState state)
{
    if (!m_connectButton) {
        return;
    }
    // Indicate the active interface
    QString t;
    if (interfaceConnection()) {
        t = interfaceConnection()->connectionName();
        m_connectButton->setIcon(interfaceConnection()->iconName());
    } else {
        t = m_wirelessStatus->ssid();
        m_connectButton->setText(t);
        m_connectButton->setIcon("network-wireless"); // "New" network
        return;
    }

    if (!t.isEmpty()) {
        m_connectButton->setText(t);
    }
    handleHasDefaultRouteChanged(interfaceConnection()->hasDefaultRoute());
    m_state = state;
    update();
    ActivatableItem::activationStateChanged(state);
}

void WirelessNetworkItem::update()
{
    //kDebug() << "updating" << m_wirelessStatus->ssid() << wirelessNetworkItem()->strength();
    setStrength((static_cast<RemoteWirelessNetwork*>(m_activatable))->strength());
    return;
}

// vim: sw=4 sts=4 et tw=100
