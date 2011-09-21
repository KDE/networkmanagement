/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>

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

#ifndef VPNINTERFACEITEM_H
#define VPNINTERFACEITEM_H

#include <QHash>
#include <QString>
#include "interfaceitem.h"

namespace Solid
{
    namespace Control
    {
        class NetworkInterface;
    }
}

class RemoteActivatable;

class VpnInterfaceItem : public InterfaceItem
{
Q_OBJECT
    public:
        VpnInterfaceItem(Solid::Control::NetworkInterfaceNm09 * iface, RemoteActivatableList* activatables, InterfaceItem::NameDisplayMode mode, QGraphicsWidget* parent = 0);
        virtual ~VpnInterfaceItem();
        virtual QString connectionName();
        virtual QString currentIpAddress();

    protected Q_SLOTS:
        virtual void setConnectionInfo();
        void disconnectCurrentConnection();
        void activatableAdded(RemoteActivatable*);
        void activatableRemoved(RemoteActivatable*);
        void listDisappeared();
        void listAppeared();

    protected:
        virtual RemoteInterfaceConnection* currentConnection();
        virtual void currentConnectionChanged();

        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    private:
        QList<RemoteActivatable*> m_vpnActivatables;
        bool accept(RemoteActivatable* activatable) const;

};
#endif // VPNINTERFACEITEM_H
