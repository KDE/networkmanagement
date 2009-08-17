/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "wirelessinterfaceconnectionitem.h"

#include <KDebug>

#include "interfaceconnectionitem_p.h"
#include "wirelessinterfaceconnection.h"

#include "wirelessstatus.h"

class WirelessInterfaceConnectionItemPrivate : public InterfaceConnectionItemPrivate
{
};

WirelessInterfaceConnectionItem::WirelessInterfaceConnectionItem(Knm::WirelessInterfaceConnection * wic, QWidget * parent)
: InterfaceConnectionItem(*new WirelessInterfaceConnectionItemPrivate, wic, parent)
{
    new WirelessStatus(this);
}

WirelessInterfaceConnectionItem::~WirelessInterfaceConnectionItem()
{
}

Knm::WirelessInterfaceConnection* WirelessInterfaceConnectionItem::wirelessInterfaceConnection() const
{
    Q_D(const WirelessInterfaceConnectionItem);
    return qobject_cast<Knm::WirelessInterfaceConnection*>(d->activatable);
}

// vim: sw=4 sts=4 et tw=100
