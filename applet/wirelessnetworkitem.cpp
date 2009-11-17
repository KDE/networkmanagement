/*
Copyright 2008,2009 Sebastian K?gler <sebas@kde.org>
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

#include <QAction>
#include <QLabel>
#include <QGraphicsGridLayout>

#include <KGlobalSettings>
#include <KIcon>
#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>
#include <Plasma/Theme>

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "remotewirelessnetwork.h"
#include "remotewirelessinterfaceconnection.h"
#include <wirelesssecurityidentifier.h>
#include "activatable.h"

WirelessNetworkItem::WirelessNetworkItem(RemoteWirelessNetwork * remote, QGraphicsItem * parent)
: ActivatableItem(remote, parent),
    m_connectButton(0),
    m_security(0),
    m_securityIcon(0),
    m_securityIconName(0),
    m_strength(0),
    m_remote(remote)
{
    m_strengthMeter = new Plasma::Meter(this);
    m_strength = 0;

    readSettings();
}

bool WirelessNetworkItem::readSettings()
{
    Solid::Control::AccessPoint::WpaFlags wpaFlags;
    Solid::Control::AccessPoint::WpaFlags rsnFlags;
    Solid::Control::AccessPoint::Capabilities capabilities;
    Solid::Control::WirelessNetworkInterface::Capabilities interfaceCapabilities;

    //Solid::Control::WirelessNetworkInterface::Capabilities interfaceCapabilities() const;

    m_state = Knm::InterfaceConnection::Unknown;
    int operationMode = 0;
    Knm::Activatable::ActivatableType aType = m_remote->activatableType();
    if (aType == Knm::Activatable::WirelessInterfaceConnection) {
        //kDebug() << "adding WirelessInterfaceConnection";
        RemoteWirelessInterfaceConnection* remoteconnection = static_cast<RemoteWirelessInterfaceConnection*>(m_activatable);
        m_ssid = remoteconnection->ssid();
        wpaFlags = remoteconnection->wpaFlags();
        rsnFlags = remoteconnection->rsnFlags();
        capabilities = remoteconnection->apCapabilities();
        interfaceCapabilities = remoteconnection->interfaceCapabilities();
        kDebug() <<  "========== RemoteActivationState" << remoteconnection->activationState();
        m_state = remoteconnection->activationState();
        activationStateChanged(m_state);
        connect(remoteconnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
                                    SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState)));
        RemoteWirelessObject * wobj  = static_cast<RemoteWirelessObject*>(remoteconnection);
        operationMode = wobj->operationMode();
    } else if (aType == Knm::Activatable::WirelessNetwork) {
        m_ssid = m_remote->ssid();
        wpaFlags = m_remote->wpaFlags();
        rsnFlags = m_remote->rsnFlags();
        capabilities = m_remote->apCapabilities();
        interfaceCapabilities = m_remote->interfaceCapabilities();
        RemoteWirelessObject * wobj  = static_cast<RemoteWirelessObject*>(m_remote);
        operationMode = wobj->apCapabilities();
    } else {
        return false;
    }

    setStrength(m_remote->strength());
    connect(m_remote, SIGNAL(changed()), SLOT(update()));
    connect(m_remote, SIGNAL(changed()), SLOT(stateChanged()));
    connect(m_remote, SIGNAL(strengthChanged(int)), SLOT(setStrength(int)));

    Knm::WirelessSecurity::Type best = Knm::WirelessSecurity::best(interfaceCapabilities, true, (operationMode == Solid::Control::WirelessNetworkInterface::Adhoc), capabilities, wpaFlags, rsnFlags);

    m_securityIconName = Knm::WirelessSecurity::iconName(best);
    m_securityIconToolTip = Knm::WirelessSecurity::shortToolTip(best);
    return true;
}

void WirelessNetworkItem::setupItem()
{
    //readSettings();
    //kDebug();// << "Connection Settings:" << m_connection->settings();
    //kDebug();// << "Security:" << m_connection->type() << m_security;
    // painting of a non-active wifi network
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

    m_connectButton->setIcon("network-wireless"); // Known connection, we probably have credentials
    if (interfaceConnection()) {
        m_connectButton->setText(interfaceConnection()->connectionName());
        QAction *a = new QAction(KIcon("emblem-favorite"), QString(), m_connectButton);
        m_connectButton->addIconAction(a);
    } else {
        m_connectButton->setText(m_ssid);
    }
    m_connectButton->setMinimumWidth(160);
    m_connectButton->setOrientation(Qt::Horizontal);
#if KDE_IS_VERSION(4,2,60)
    m_connectButton->setTextBackgroundColor(QColor());
#endif
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
    m_securityIcon->setIcon(m_securityIconName);
    m_securityIcon->setMinimumHeight(22);
    m_securityIcon->setMaximumHeight(22);
    m_securityIcon->setToolTip(m_securityIconToolTip);
    m_layout->addItem(m_securityIcon, 0, 2, 1, 1, Qt::AlignLeft);

    connect(m_connectButton, SIGNAL(clicked()), this, SIGNAL(clicked()));
    connect(this, SIGNAL(clicked()), this, SLOT(emitClicked()));

    // Forward clicks and presses between our widgets and this
    connect(this, SIGNAL(pressed(bool)), m_connectButton, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(pressed(bool)), this, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(emitClicked()));

    connect(this, SIGNAL(pressed(bool)), m_securityIcon, SLOT(setPressed(bool)));
    connect(m_securityIcon, SIGNAL(pressed(bool)), this, SLOT(setPressed(bool)));
    connect(m_securityIcon, SIGNAL(clicked()), this, SLOT(emitClicked()));
    //readSettings();
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
        kDebug() <<  "========== RemoteActivationState Changed: " << remoteconnection->activationState();
        activationStateChanged(remoteconnection->activationState());
    }
}

void WirelessNetworkItem::setStrength(int strength)
{
    //kDebug() << m_ssid << "signal strength changed from " << m_strength << "to " << strength;
    if (strength == m_strength) {
        return;
    }
    m_strength = strength;
    m_strengthMeter->setValue(m_strength);
    stateChanged();
}

void WirelessNetworkItem::activationStateChanged(Knm::InterfaceConnection::ActivationState state)
{
    kDebug() << m_state << "changes to" << state;
    if (!m_connectButton) {
        return;
    }
    // Indicate the active interface
    QString t;
    if (interfaceConnection()) {
        t = interfaceConnection()->connectionName();
    } else {
        t = m_ssid;
        //kDebug() << "ssid:"<< m_ssid;
        m_connectButton->setText(m_ssid);
        return;
    }
    //enum ActivationState { Unknown, Activating, Activated };
    if (interfaceConnection()) {
        //m_connectButton->setIcon("bookmarks"); // Known connection, we probably have credentials
        //m_connectButton->setText(interfaceConnection()->connectionName());
        switch (m_state) {
            //Knm::InterfaceConnectihon::ActivationState
            case Knm::InterfaceConnection::Activated:
                m_connectButton->setIcon("dialog-ok-apply"); // The active connection
                t = i18nc("label on the connectabel button", "%1 (connected)", t);
                //m_connectButton->setInfoText(i18nc("subtext on connection button", "Connected"));
                //kDebug() << "active" << t;
                break;
            case Knm::InterfaceConnection::Unknown:
                //m_connectButton->setInfoText(QString());
                break;
            case Knm::InterfaceConnection::Activating:
                t = i18nc("label on the connectabel button", "%1 (connecting...)", t);
                m_connectButton->setInfoText(i18nc("subtext on connection button", "Connecting..."));
        }
        m_connectButton->setIcon(interfaceConnection()->iconName());

    } else {
        m_connectButton->setText(m_ssid);
        m_connectButton->setIcon("network-wireless"); // "New" network
    }
    if (!t.isEmpty()) {
        m_connectButton->setText(t);
    }
    //kDebug() << "state updated" << t;
    m_state = state;
    update();
}

RemoteWirelessNetwork * WirelessNetworkItem::wirelessNetworkItem() const
{
    return static_cast<RemoteWirelessNetwork*>(m_activatable);
}

void WirelessNetworkItem::update()
{
    kDebug() << "updating" << m_ssid << wirelessNetworkItem()->strength();
    setStrength(wirelessNetworkItem()->strength());
    return;
}

// vim: sw=4 sts=4 et tw=100
