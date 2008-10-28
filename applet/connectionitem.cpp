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

#include "connectionitem.h"

#include <QGraphicsGridLayout>

#include <Plasma/Icon>
#include <Plasma/Label>

#include <solid/control/networkmanager.h>

#include "remoteconnection.h"
ConnectionItem::ConnectionItem(RemoteConnection * conn, QGraphicsItem * parent)
: QGraphicsWidget(parent), m_connection(conn)
{
}

void ConnectionItem::setupItem()
{
    // painting of a non-active connection (or wifi network)
    // icon on left
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

    m_icon = new Plasma::Icon(this);
    m_icon->setIcon("network-server");
    m_icon->setMinimumHeight(rowHeight);
    m_icon->setMaximumHeight(rowHeight);
    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setText("Connection:" + m_connection->id());
    m_connectButton = new Plasma::Icon(this);
    m_connectButton->setIcon("media-playback-start");
    m_connectButton->setMinimumHeight(rowHeight);
    m_connectButton->setMaximumHeight(rowHeight);

    m_layout->addItem(m_icon, 0, 0, 1, 1 );
    m_layout->addItem(m_connectionNameLabel, 0, 1, 1, 1);
    m_layout->addItem(m_connectButton, 0, 2, 1, 1);
    connect( m_connectButton, SIGNAL(clicked()), SLOT(emitClicked()));
}

ConnectionItem::~ConnectionItem()
{

}

RemoteConnection * ConnectionItem::connection() const
{
    return m_connection;
}

void ConnectionItem::emitClicked()
{
    kDebug() << "clicked";
    emit clicked(this);
}
// vim: sw=4 sts=4 et tw=100
