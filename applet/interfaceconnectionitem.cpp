/*
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

#include "interfaceconnectionitem.h"

#include <QGraphicsGridLayout>

#include <Plasma/IconWidget>
#include <Plasma/Label>

#include <solid/control/networkmanager.h>

#include "remoteinterfaceconnection.h"

InterfaceConnectionItem::InterfaceConnectionItem(RemoteInterfaceConnection * conn, QGraphicsItem * parent)
: ActivatableItem(conn, parent)
{
}

void InterfaceConnectionItem::setupItem()
{
    int rowHeight = 24;

    m_layout = new QGraphicsGridLayout(this);
    // last colunm has fixed width for the icon
    //m_layout->setColumnFixedWidth(2, rowHeight);

    // icon on the left
    m_connectButton = new Plasma::IconWidget(this);
    kDebug() << "====> init face connection" << m_connectButton->text();
    m_connectButton->setMinimumWidth(160);
    m_connectButton->setMaximumHeight(rowHeight);
    m_connectButton->setOrientation(Qt::Horizontal);
#if KDE_IS_VERSION(4,2,60)
    m_connectButton->setTextBackgroundColor(QColor());
#endif

    m_connectButton->setMinimumHeight(rowHeight);
    m_connectButton->setMaximumHeight(rowHeight);
    m_layout->addItem(m_connectButton, 0, 0, 1, 1 );

    m_routeIcon = new Plasma::IconWidget(this);
    m_routeIcon->setIcon("emblem-favorite");
    m_routeIcon->setGeometry(QRectF(m_connectButton->geometry().topLeft(), QSizeF(16, 16)));
    m_routeIcon->hide(); // this will be shown in handleHasDefaultRouteChanged(bool);

    if (interfaceConnection()) {
        m_connectButton->setIcon(interfaceConnection()->iconName());
        m_connectButton->setText(interfaceConnection()->connectionName());
        handleHasDefaultRouteChanged(interfaceConnection()->hasDefaultRoute());
    } else {
        m_connectButton->setIcon("network-wired");
        //m_connectButton->setText("missing name");
    }
    connect(m_connectButton, SIGNAL(clicked()), this, SIGNAL(clicked()));
    connect(this, SIGNAL(clicked()), this, SLOT(emitClicked()));
    connect(this, SIGNAL(pressed(bool)), m_connectButton, SLOT(setPressed(bool)));
    connect(m_connectButton, SIGNAL(pressed(bool)), this, SLOT(setPressed(bool)));
}

InterfaceConnectionItem::~InterfaceConnectionItem()
{

}


// vim: sw=4 sts=4 et tw=100
