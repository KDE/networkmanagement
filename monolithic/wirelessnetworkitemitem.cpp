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

#include "wirelessnetworkitemitem.h"

#include "activatableitem_p.h"

#include <QLabel>

#include <KIconLoader>

#include <wirelessnetworkitem.h>

#include "wirelessstatus.h"

class WirelessNetworkItemItemPrivate : public ActivatableItemPrivate
{
    public:
};

WirelessNetworkItemItem::WirelessNetworkItemItem(Knm::WirelessNetworkItem * item, QWidget * parent)
: ActivatableItem(*new WirelessNetworkItemItemPrivate, item, parent)
{
    Q_D(WirelessNetworkItemItem);
    new WirelessStatus(this);
    setText(item->ssid());
    d->activeIcon->setPixmap(SmallIcon(iconName()));
    d->activeIcon->show();
}

WirelessNetworkItemItem::~WirelessNetworkItemItem()
{

}

QString WirelessNetworkItemItem::iconName() const
{
    return QLatin1String("network-wireless");
}

// vim: sw=4 sts=4 et tw=100
