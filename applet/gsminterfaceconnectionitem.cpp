/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010-2012 Lamarque Souza <lamarque@kde.org>

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
#include <QTimer>

#include <KGlobalSettings>
#include <KIcon>
#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>

#include <solid/control/networkmodeminterface.h>

#include "activatable.h"

using namespace Solid::Control;

GsmInterfaceConnectionItem::GsmInterfaceConnectionItem(RemoteGsmInterfaceConnection * remote, QGraphicsItem * parent)
: ActivatableItem(remote, parent),
    m_strengthMeter(0),
    m_layoutIsDirty(true)
{
    connect(remote, SIGNAL(signalQualityChanged(int)), this, SLOT(setQuality(int)));
    connect(remote, SIGNAL(accessTechnologyChanged(int)), this, SLOT(setAccessTechnology(int)));
    connect(remote, SIGNAL(changed()), SLOT(updateGsmInfo()));
    connect(remote, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
            this, SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));
    m_state = Knm::InterfaceConnection::Unknown;
}

//HACK: hack to avoid misplacing of security icon. Check with Qt5 if still needed
void GsmInterfaceConnectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (m_layoutIsDirty) {
        m_layout->invalidate();
        m_layout->activate();
        m_layoutIsDirty = false;
    }
    ActivatableItem::paint(painter, option, widget);
}

void GsmInterfaceConnectionItem::setupItem()
{
    // painting of a wifi network, known connection or available access point
    /*
    +----+-------------+-----+---+
    |icon essid        |meter|   |
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
    m_connectButton->setOrientation(Qt::Horizontal);
    m_connectButton->setTextBackgroundColor(QColor(Qt::transparent));
    //m_connectButton->setToolTip(i18nc("icon to connect to mobile broadband network", "Connect to mobile broadband network %1", ssid));
    m_layout->addItem(m_connectButton, 0, 0, 2, 2, Qt::AlignVCenter | Qt::AlignLeft);

    // spacer to force the strength meter to the right.
    QGraphicsWidget *widget = new QGraphicsWidget(this);
    widget->setMaximumHeight(12);
    m_layout->addItem(widget, 0, 1);

    m_strengthMeter = new Plasma::Meter(this);
    m_strengthMeter->setMinimum(0);
    m_strengthMeter->setMaximum(100);

    RemoteGsmInterfaceConnection * remoteconnection = qobject_cast<RemoteGsmInterfaceConnection*>(m_activatable);
    if (remoteconnection) {
        m_connectButton->setIcon(remoteconnection->iconName());
        m_connectButton->setText(remoteconnection->connectionName(true));
        m_strengthMeter->setValue(remoteconnection->getSignalQuality());
        activationStateChanged(Knm::InterfaceConnection::Unknown, remoteconnection->activationState());
    } else {
        m_connectButton->setIcon("network-wired");
    }

    m_strengthMeter->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_strengthMeter->setPreferredSize(QSizeF(60, 12));
    m_strengthMeter->setMaximumHeight(12);
    m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout->addItem(m_strengthMeter, 0, 2, 1, 1, Qt::AlignVCenter | Qt::AlignRight);

    widget = new Plasma::Label(this);
    widget->setMaximumHeight(22);
    widget->setMaximumWidth(22);
    m_layout->addItem(widget, 0, 3, 1, 1, Qt::AlignVCenter | Qt::AlignRight);

    connect(this, SIGNAL(clicked()), this, SLOT(emitClicked()));

    // Forward clicks and presses between our widgets and this
    connect(this, SIGNAL(pressed(bool)), m_connectButton, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(pressed(bool)), this, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(emitClicked()));

    m_layoutIsDirty = true;
    QTimer::singleShot(0, this, SLOT(updateGsmInfo()));
}

GsmInterfaceConnectionItem::~GsmInterfaceConnectionItem()
{
}

void GsmInterfaceConnectionItem::setQuality(int quality)
{
    if (m_strengthMeter) {
        m_strengthMeter->setValue(quality);
        m_strengthMeter->setToolTip(i18n("Signal quality: %1%", quality));
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
                                 arg(remote->connectionName(true),
                                     ModemInterface::convertAccessTechnologyToString(static_cast<ModemInterface::AccessTechnology>(tech))));
    } else {
        m_connectButton->setText(remote->connectionName(true));
    }
}

void GsmInterfaceConnectionItem::activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    if (!m_connectButton) {
        return;
    }

    RemoteGsmInterfaceConnection * remote = qobject_cast<RemoteGsmInterfaceConnection*>(m_activatable);

    if (remote) {
        handleHasDefaultRouteChanged(remote->hasDefaultRoute());
    }
    ActivatableItem::activationStateChanged(oldState, newState);
    update();
}

void GsmInterfaceConnectionItem::updateGsmInfo()
{
    RemoteGsmInterfaceConnection * remote = qobject_cast<RemoteGsmInterfaceConnection*>(m_activatable);
    if (remote) {
        setQuality(remote->getSignalQuality());
        setAccessTechnology(remote->getAccessTechnology());
        update();
    }
}

// vim: sw=4 sts=4 et tw=100
