/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010-2012 Lamarque V. Souza <lamarque@kde.org>

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
#include <QTimer>

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
    m_securityIcon(0),
    m_remote(remote),
    m_wirelessStatus(0),
    m_layoutIsDirty(true)
{
    m_wirelessStatus = new WirelessStatus(remote);
    connect(m_wirelessStatus, SIGNAL(strengthChanged(int)), this, SLOT(setStrength(int)));
    connect(m_remote, SIGNAL(changed()), SLOT(updateWifiInfo()));
    if (interfaceConnection()) {
        connect(remote, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
                this, SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));
    }
    m_state = Knm::InterfaceConnection::Unknown;
}

//HACK: hack to avoid misplacing of security icon. Check with Qt5 if still needed
void WirelessNetworkItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (m_layoutIsDirty) {
        m_layout->invalidate();
        m_layout->activate();
        m_layoutIsDirty = false;
    }
    ActivatableItem::paint(painter, option, widget);
}

void WirelessNetworkItem::setupItem()
{
    // painting of a wifi network, known connection or available access point
    /*
    +----+-------------+-----+---+
    |icon essid        |meter|sec|
    +----+-------------+-----+---+
    */
    m_layout = new QGraphicsGridLayout(this);
    // First, third and fourth colunm are fixed width for the icons
    m_layout->setColumnPreferredWidth(0, 150);
    m_layout->setColumnFixedWidth(2, 60);
    m_layout->setColumnFixedWidth(3, rowHeight);
    m_layout->setColumnSpacing(2, spacing);

    // icon on the left
    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setMaximumWidth(maxConnectionNameWidth);
    // to make default route overlay really be over the connection's icon.
    m_connectButton->setFlags(ItemStacksBehindParent);
    m_connectButton->setAcceptsHoverEvents(false);
    m_connectButton->setIcon("network-wireless"); // Known connection, we probably have credentials
    RemoteInterfaceConnection *remoteconnection = interfaceConnection();
    if (remoteconnection) {
        m_connectButton->setText(remoteconnection->connectionName(true));
        activationStateChanged(Knm::InterfaceConnection::Unknown, remoteconnection->activationState());
    } else {
        m_connectButton->setText(m_wirelessStatus->ssid());
    }
    m_connectButton->setOrientation(Qt::Horizontal);
    m_connectButton->setTextBackgroundColor(QColor(Qt::transparent));
    //m_connectButton->setToolTip(i18nc("icon to connect to wireless network", "Connect to wireless network %1", ssid));
    m_layout->addItem(m_connectButton, 0, 0, 2, 2, Qt::AlignVCenter | Qt::AlignLeft);

    // spacer to force the strength meter and the security icon to the right.
    QGraphicsWidget *widget = new QGraphicsWidget(this);
    widget->setMaximumHeight(12);
    m_layout->addItem(widget, 0, 1);

    if (m_remote->strength()>=0)
    {
        m_strengthMeter = new Plasma::Meter(this);
        m_strengthMeter->setMinimum(0);
        m_strengthMeter->setMaximum(100);
        m_strengthMeter->setValue(m_wirelessStatus->strength());
        m_strengthMeter->setToolTip(i18n("Strength: %1%", m_wirelessStatus->strength()));
        m_strengthMeter->setMeterType(Plasma::Meter::BarMeterHorizontal);
        m_strengthMeter->setPreferredSize(QSizeF(60, 12));
        m_strengthMeter->setMaximumHeight(12);
        m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_layout->addItem(m_strengthMeter, 0, 2, 1, 1, Qt::AlignVCenter | Qt::AlignRight);
    }
    else
    {
        widget = new QGraphicsWidget(this);
        widget->setPreferredSize(QSizeF(60, 12));
        widget->setMaximumHeight(12);
        widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_layout->addItem(widget, 0, 2, 1, 1, Qt::AlignVCenter | Qt::AlignRight);
    }

    m_securityIcon = new Plasma::Label(this);
    m_securityIcon->nativeWidget()->setPixmap(KIcon(m_wirelessStatus->securityIcon()).pixmap(22,22));
    m_securityIcon->setToolTip(m_wirelessStatus->securityTooltip());
    m_layout->addItem(m_securityIcon, 0, 3, 1, 1, Qt::AlignVCenter | Qt::AlignRight);

    connect(this, SIGNAL(clicked()), this, SLOT(emitClicked()));

    // Forward clicks and presses between our widgets and this
    connect(this, SIGNAL(pressed(bool)), m_connectButton, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(pressed(bool)), this, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(emitClicked()));

    m_layoutIsDirty = true;
    QTimer::singleShot(0, this, SLOT(updateWifiInfo()));
}

WirelessNetworkItem::~WirelessNetworkItem()
{
}

void WirelessNetworkItem::setStrength(int strength)
{
    if (m_strengthMeter) {
        m_strengthMeter->setValue(strength);
        m_strengthMeter->setToolTip(i18n("Strength: %1%", strength));
    }
}

void WirelessNetworkItem::activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    if (!m_connectButton) {
        return;
    }
    // Indicate the active interface
    QString t;
    if (interfaceConnection()) {
        t = interfaceConnection()->connectionName(true);
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
    ActivatableItem::activationStateChanged(oldState, newState);
    update();
}

void WirelessNetworkItem::updateWifiInfo()
{
    //kDebug() << "updating" << m_wirelessStatus->ssid() << wirelessNetworkItem()->strength();
    if (m_activatable) {
        setStrength((static_cast<RemoteWirelessNetwork*>(m_activatable))->strength());
        update();
    }
}

QString WirelessNetworkItem::ssid()
{
    if (m_wirelessStatus) {
        return QString();
    }
    return m_wirelessStatus->ssid();
}

// vim: sw=4 sts=4 et tw=100
