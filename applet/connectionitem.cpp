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

#include <Plasma/IconWidget>
#include <Plasma/Label>

#include <solid/control/networkmanager.h>

#include "remoteconnection.h"
ConnectionItem::ConnectionItem(RemoteConnection * conn, QGraphicsItem * parent)
: AbstractConnectableItem(parent), m_connection(conn)
{
}

void ConnectionItem::setupItem()
{
    /*
    // painting of a non-active connection wired connection

    +----+------------------+------+
    |icon| connection name  |status|
    +----+------------------+------+
    */
    int rowHeight = 24;

    m_layout = new QGraphicsGridLayout(this);
    // last colunm has fixed width for the icon
    m_layout->setColumnFixedWidth(2, rowHeight);

    // icon on the left
    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setDrawBackground(true);
    m_connectButton->setIcon("network-wired");
    m_connectButton->setText(m_connection->id());
    m_connectButton->setMinimumWidth(160);
    m_connectButton->setMaximumHeight(rowHeight);
    m_connectButton->setOrientation(Qt::Horizontal);
    m_connectButton->setToolTip(i18nc("button to connect to wired network",
                                      "Connect to wired network %1", m_connection->id()));
    m_connectButton->setMinimumHeight(rowHeight);
    m_connectButton->setMaximumHeight(rowHeight);
    m_layout->addItem(m_connectButton, 0, 0, 1, 1 );

    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon("network-connect");
    m_icon->setMinimumHeight(22);
    m_icon->setMaximumHeight(22);
    m_layout->addItem(m_icon, 0, 2, 1, 1, Qt::AlignLeft);

    connect( m_connectButton, SIGNAL(clicked()), this, SLOT(emitClicked()));
}

ConnectionItem::~ConnectionItem()
{

}

RemoteConnection * ConnectionItem::connection() const
{
    return m_connection;
}

// vim: sw=4 sts=4 et tw=100
