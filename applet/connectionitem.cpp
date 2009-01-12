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

#include <QLabel>
#include <QGridLayout>
#include <QToolButton>
#include <KIconLoader>
#include <KLocale>

#include <solid/control/networkmanager.h>

#include "remoteconnection.h"
ConnectionItem::ConnectionItem(RemoteConnection * conn, QWidget * parent)
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


    m_layout = new QGridLayout(this);
    // last colunm has fixed width for the icon
    // WILLTODO - see what the appropriate equivalent is to set column fixed width on qgridlayout
    //m_layout->setColumnFixedWidth(2, rowHeight);
    // tighten
    //m_layout->setColumnSpacing(0, 0);
    //m_layout->setColumnSpacing(1, 0);
    //m_layout->setColumnSpacing(2, 0);

    // icon on the left
    m_connectButton = new QToolButton(this);
    m_connectButton->setIcon(MainBarIcon("network-wired"));
    m_connectButton->setText(m_connection->id());
    m_connectButton->setMinimumWidth(160);
    m_connectButton->setMaximumHeight(rowHeight);

    m_layout->addWidget(m_connectButton, 0, 0, 1, 1);

    m_icon = new QLabel(this);
    m_icon->setPixmap(MainBarIcon("network-connect"));
    m_icon->setMinimumHeight(22);
    m_icon->setMaximumHeight(22);
    m_layout->addWidget(m_icon, 0, 2, 1, 1, Qt::AlignLeft);

    connect( m_connectButton, SIGNAL(clicked()), SLOT(emitClicked()));
}

ConnectionItem::~ConnectionItem()
{

}

RemoteConnection * ConnectionItem::connection() const
{
    return m_connection;
}

// vim: sw=4 sts=4 et tw=100
