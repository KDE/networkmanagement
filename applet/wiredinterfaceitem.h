/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2008, 2009 Sebastian Kügler <sebas@kde.org>

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

#ifndef WIREDINTERFACEITEM_H
#define WIREDINTERFACEITEM_H

#include <QHash>
#include "interfaceitem.h"

namespace Solid
{
namespace Control
{
    class WiredNetworkInterface;
}
}

/**
 * Represents any network interface that uses IEEE 802.3
 */
class WiredInterfaceItem : public InterfaceItem
{
Q_OBJECT
public:
    WiredInterfaceItem(Solid::Control::WiredNetworkInterface * iface, InterfaceItem::NameDisplayMode mode, QGraphicsItem* parent = 0);
    virtual ~WiredInterfaceItem();
public slots:
    void connectButtonClicked();
private:
    Solid::Control::WiredNetworkInterface * m_wiredIface;
};
#endif // SERIALINTERFACEITEM_H
