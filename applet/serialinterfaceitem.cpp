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

#include "serialinterfaceitem.h"

#include <nm-setting-serial.h>

#include <solid/control/networkserialinterface.h>

#include "../libs/types.h"
#include "events.h"
#include "remoteconnection.h"

SerialInterfaceItem::SerialInterfaceItem(Solid::Control::SerialNetworkInterface * iface, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, InterfaceItem::NameDisplayMode mode, QGraphicsItem* parent)
: InterfaceItem(iface, userSettings, systemSettings, mode, parent), m_serialIface(iface), m_bytesIn(0), m_bytesOut(0)

{
    // for updating our UI
    connect(iface, SIGNAL(pppStats(uint,uint)), SLOT(pppStats(uint,uint)));
}

SerialInterfaceItem::~SerialInterfaceItem()
{

}

void SerialInterfaceItem::pppStats(uint in, uint out)
{
    m_bytesIn = in;
    m_bytesOut = out;
    setConnectionInfo();
}

void SerialInterfaceItem::activeSignalStrengthChanged(int)
{
    setConnectionInfo();
}

void SerialInterfaceItem::setConnectionInfo()
{
    // Todo: GSM/UMTS/CDMA/HDSPA... status 
    // network provider name
    // all available once ModemManager is merged into NM
    
    if (m_activeConnections.isEmpty()) {
        m_connectionInfoLabel->setText(QString());
    } else {
        m_connectionInfoLabel->setText(i18nc("Label for PPP/cellular network connection traffic data, bytes in and out","In: %1 Out %2", m_bytesIn, m_bytesOut));
    }
}
// vim: sw=4 sts=4 et tw=100
