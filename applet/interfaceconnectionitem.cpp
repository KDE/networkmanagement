/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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

InterfaceConnectionItem::InterfaceConnectionItem(RemoteInterfaceConnection* conn, QGraphicsItem* parent)
: ActivatableItem(conn, parent)
{
}

void InterfaceConnectionItem::setupItem()
{
    m_layout = new QGraphicsGridLayout(this);
    m_layout->setColumnPreferredWidth(0, 150);
    m_layout->setColumnFixedWidth(2, 60);
    m_layout->setColumnFixedWidth(3, rowHeight);
    m_layout->setColumnSpacing(2, spacing);

    // icon on the left
    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setMaximumWidth(maxConnectionNameWidth);
    // to make default route overlay really be over the connection's icon.
    m_connectButton->setFlags(ItemStacksBehindParent);
    m_connectButton->setOrientation(Qt::Horizontal);
    m_connectButton->setTextBackgroundColor(QColor(Qt::transparent));
    //m_connectButton->setZValue(100); // FIXME: doesn't work

    m_layout->addItem(m_connectButton, 0, 0, 1, 1, Qt::AlignVCenter | Qt::AlignLeft);

    // spacer to make highlighting's borders to expand to the maximum.
    QGraphicsWidget *widget = new QGraphicsWidget(this);
    widget->setMaximumHeight(12);
    m_layout->addItem(widget, 0, 1, 3, 3);

    RemoteInterfaceConnection *remoteconnection = interfaceConnection();
    if (remoteconnection) {
        m_connectButton->setIcon(remoteconnection->iconName());
        m_connectButton->setText(remoteconnection->connectionName(true));
        //kDebug() << remoteconnection->connectionName() << remoteconnection->iconName();
        handleHasDefaultRouteChanged(remoteconnection->hasDefaultRoute());
        activationStateChanged(Knm::InterfaceConnection::Unknown, remoteconnection->activationState());
    } else {
        m_connectButton->setIcon("network-wired");
        m_connectButton->setText(i18nc("name of the connection not known", "Unknown"));
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
