/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Lamarque Souza <lamarque@gmail.com>

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

#ifdef COMPILE_MODEM_MANAGER_SUPPORT

#include "gsminterfaceconnectionitem.h"

#include <QAction>
#include <QLabel>
#include <QGraphicsGridLayout>

#include <KGlobalSettings>
#include <KIcon>
#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>

#include <solid/control/networkgsminterface.h>

#include "activatable.h"

using namespace Solid::Control;

GsmInterfaceConnectionItem::GsmInterfaceConnectionItem(RemoteGsmInterfaceConnection * remote, QGraphicsItem * parent)
: ActivatableItem(remote, parent),
    m_strengthMeter(0),
    m_connectButton(0)
{
    connect(remote, SIGNAL(signalQualityChanged(int)), this, SLOT(setQuality(int)));
    connect(remote, SIGNAL(accessTechnologyChanged(const int)), this, SLOT(setAccessTechnology(const int)));
    connect(remote, SIGNAL(changed()), SLOT(update()));
    connect(remote, SIGNAL(changed()), SLOT(stateChanged()));
    m_state = remote->activationState();
    stateChanged();
    update();
}

void GsmInterfaceConnectionItem::setupItem()
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
    m_layout->setColumnFixedWidth(1, 60);
    m_layout->setColumnFixedWidth(2, rowHeight);
    m_layout->setColumnSpacing(1, spacing);

    // icon on the left
    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setAcceptsHoverEvents(false);
    RemoteGsmInterfaceConnection * remote = qobject_cast<RemoteGsmInterfaceConnection*>(m_activatable);
    if (remote) {
        m_connectButton->setIcon(remote->iconName());
        setAccessTechnology(remote->getAccessTechnology());
        handleHasDefaultRouteChanged(remote->hasDefaultRoute());
        QAction *a = new QAction(KIcon("emblem-favorite"), QString(), m_connectButton);
        m_connectButton->addIconAction(a);
    } else {
        m_connectButton->setIcon("network-wired");
    }
    m_connectButton->setOrientation(Qt::Horizontal);
    m_connectButton->setTextBackgroundColor(QColor(Qt::transparent));
    //m_connectButton->setToolTip(i18nc("icon to connect to mobile broadband network", "Connect to mobile broadband network %1", ssid));
    m_layout->addItem(m_connectButton, 0, 0, 1, 1 );

    m_strengthMeter = new Plasma::Meter(this);
    m_strengthMeter->setMinimum(0);
    m_strengthMeter->setMaximum(100);
    if (remote) m_strengthMeter->setValue(remote->getSignalQuality());
    m_strengthMeter->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_strengthMeter->setPreferredSize(QSizeF(60, 12));
    m_strengthMeter->setMaximumHeight(12);
    m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout->addItem(m_strengthMeter, 0, 1, 1, 1, Qt::AlignCenter);

    connect(this, SIGNAL(clicked()), this, SLOT(emitClicked()));

    // Forward clicks and presses between our widgets and this
    connect(this, SIGNAL(pressed(bool)), m_connectButton, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(pressed(bool)), this, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(emitClicked()));

    activationStateChanged(m_state);

    update();
}

GsmInterfaceConnectionItem::~GsmInterfaceConnectionItem()
{
}

void GsmInterfaceConnectionItem::setQuality(int quality)
{
    if (m_strengthMeter) {
        m_strengthMeter->setValue(quality);
    }
}

void GsmInterfaceConnectionItem::setAccessTechnology(const int tech)
{
    RemoteGsmInterfaceConnection * remote = qobject_cast<RemoteGsmInterfaceConnection*>(m_activatable);
    if (!m_connectButton || !remote) {
        return;
    }

    if (tech != ModemInterface::UnknownTechnology) {
        m_connectButton->setText(QString("%1 (%2)").
                                 arg(remote->connectionName(),
                                     ModemInterface::convertAccessTechnologyToString(static_cast<ModemInterface::AccessTechnology>(tech))));
    } else {
        m_connectButton->setText(remote->connectionName());
    }
}

void GsmInterfaceConnectionItem::stateChanged()
{
    RemoteGsmInterfaceConnection* remoteconnection = static_cast<RemoteGsmInterfaceConnection*>(m_activatable);
    if (remoteconnection) {
        activationStateChanged(remoteconnection->activationState());
    }
}

void GsmInterfaceConnectionItem::activationStateChanged(Knm::InterfaceConnection::ActivationState state)
{
    if (!m_connectButton) {
        return;
    }

    RemoteGsmInterfaceConnection * remote = qobject_cast<RemoteGsmInterfaceConnection*>(m_activatable);

    if (remote) {
        handleHasDefaultRouteChanged(remote->hasDefaultRoute());
    }
    m_state = state;
    update();
    ActivatableItem::activationStateChanged(state);
}

void GsmInterfaceConnectionItem::update()
{
    RemoteGsmInterfaceConnection * remote = qobject_cast<RemoteGsmInterfaceConnection*>(m_activatable);
    if (remote) {
        setQuality(remote->getSignalQuality());
        setAccessTechnology(remote->getAccessTechnology());
    }
}

#endif
// vim: sw=4 sts=4 et tw=100
