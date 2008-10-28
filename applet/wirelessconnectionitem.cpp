/*
Copyright 2008 Sebastian Kügler <sebas@kde.org>

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

#include "wirelessconnectionitem.h"

#include <QGraphicsGridLayout>

#include <Plasma/Icon>
#include <Plasma/Label>
#include <Plasma/Meter>

#include <solid/control/networkmanager.h>

#include "remoteconnection.h"
WirelessConnectionItem::WirelessConnectionItem(RemoteConnection * conn, QGraphicsItem * parent)
: ConnectionItem(conn, parent)
{
}

void WirelessConnectionItem::setupItem()
{
    // painting of a non-active wifi network
    // icon on the left
    int rowHeight = 24;
    m_layout = new QGraphicsGridLayout(this);
    // First and third colunm are fixed width for the icons
    m_layout->setColumnFixedWidth(0, rowHeight);
    m_layout->setColumnPreferredWidth(1, 140);
    m_layout->setColumnFixedWidth(2, rowHeight);
    // tighten
    m_layout->setColumnSpacing(0, 0);
    m_layout->setColumnSpacing(1, 0);
    m_layout->setColumnSpacing(2, 0);

    // TODO: security symbol

    m_icon = new Plasma::Icon(this);
    m_icon->setIcon("network-wireless");
    m_icon->setMinimumHeight(rowHeight);
    m_icon->setMaximumHeight(rowHeight);
    m_layout->addItem(m_icon, 0, 0, 1, 1 );

    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setText("Network:" + m_connection->id());
    m_layout->addItem(m_connectionNameLabel, 0, 1, 1, 1);

//     m_strengthMeter = new Plasma::Meter(this);
//     m_strengthMeter->setMinimum(0);
//     m_strengthMeter->setMaximum(100);
//     m_strengthMeter->setValue(87);
//     m_strengthMeter->setLabel(0, "Network:" + conn->id());
//     m_layout->addItem(m_strengthMeter, 0, 1, 1, 1);

    m_connectButton = new Plasma::Icon(this);
    m_connectButton->setIcon("media-playback-start");
    m_connectButton->setMinimumHeight(rowHeight);
    m_connectButton->setMaximumHeight(rowHeight);
    m_layout->addItem(m_connectButton, 0, 2, 1, 1);

    connect( m_connectButton, SIGNAL(clicked()), SLOT(emitClicked()));
}

WirelessConnectionItem::~WirelessConnectionItem()
{
}

// vim: sw=4 sts=4 et tw=100
