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

#ifndef SERIALINTERFACEITEM_H
#define SERIALINTERFACEITEM_H

#include <QHash>
#include "interfaceitem.h"

namespace Solid
{
namespace Control
{
    class SerialNetworkInterface;
}
}

/** Represents any network interface that uses PPP
 * Provides custom UI for PPP stats (bytes up and down)
 */
class SerialInterfaceItem : public InterfaceItem
{
Q_OBJECT
public:
    SerialInterfaceItem(Solid::Control::SerialNetworkInterface * iface, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, InterfaceItem::NameDisplayMode mode, QGraphicsItem* parent = 0);
    virtual ~SerialInterfaceItem();
    /* @reimp InterfaceItem */
    virtual void setEnabled(bool enable);
    /* @reimp InterfaceItem */
    virtual void setUnavailable();
    /* @reimp InterfaceItem */
    virtual void setInactive();
    /* @reimp InterfaceItem */
    virtual void setActiveConnection(int);
public slots:
    void pppStats(uint,uint);
    void activeSignalStrengthChanged(int);
    void connectButtonClicked();
private:
    void setConnectionInfo();
    Solid::Control::SerialNetworkInterface * m_serialIface;
    Plasma::IconWidget * m_connectButton;
    uint m_bytesIn;
    uint m_bytesOut;
};
#endif // SERIALINTERFACEITEM_H
