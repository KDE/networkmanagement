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

GsmInterfaceConnectionItem::GsmInterfaceConnectionItem(RemoteGsmInterfaceConnection * remote, QGraphicsItem * parent)
: ActivatableItem(remote, parent),
    m_strengthMeter(0),
    m_connectButton(0),
    m_remote(remote)
{
    connect(remote, SIGNAL(signalQualityChanged(int)), this, SLOT(setQuality(int)));
    connect(remote, SIGNAL(accessTechnologyChanged(const QString)), this, SLOT(setAccessTechnology(const QString)));
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
    m_connectButton->setIcon("network-wired");
    if (interfaceConnection()) {
        m_connectButton->setIcon(interfaceConnection()->iconName());
        setAccessTechnology(m_remote->getAccessTechnology());
        handleHasDefaultRouteChanged(interfaceConnection()->hasDefaultRoute());
        QAction *a = new QAction(KIcon("emblem-favorite"), QString(), m_connectButton);
        m_connectButton->addIconAction(a);
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
    m_strengthMeter->setValue(m_remote->getSignalQuality());
    m_strengthMeter->setValue(0);
    m_strengthMeter->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_strengthMeter->setPreferredSize(QSizeF(60, rowHeight/2));
    m_strengthMeter->setMaximumHeight(rowHeight/2);
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

void GsmInterfaceConnectionItem::setAccessTechnology(const QString tech)
{
    if (!m_connectButton) {
        return;
    }

    if (tech != Solid::Control::ModemInterface::convertAccessTechnologyToString(Solid::Control::ModemInterface::UnknownTechnology)) {
        m_connectButton->setText(QString("%1 (%2)").
                                 arg(interfaceConnection()->connectionName()).
                                 arg(tech));
    }
    else {
        m_connectButton->setText(interfaceConnection()->connectionName());
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

    handleHasDefaultRouteChanged(interfaceConnection()->hasDefaultRoute());
    m_state = state;
    update();
    ActivatableItem::activationStateChanged(state);
}

void GsmInterfaceConnectionItem::update()
{
    setQuality(m_remote->getSignalQuality());
    setAccessTechnology(m_remote->getAccessTechnology());
}

// vim: sw=4 sts=4 et tw=100
