/*
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "vpninterfaceconnectionhelpers.h"
#include "vpninterfaceconnectionhelpers_p.h"

#include "connection.h"
#include "vpninterfaceconnection.h"

using namespace Knm;

Knm::VpnInterfaceConnection* VpnInterfaceConnectionHelpers::buildInterfaceConnection(
        Knm::Connection *connection, const QString & deviceUni, QObject * parent)
{
    VpnInterfaceConnectionBuilder builder(connection, deviceUni, parent);

    return builder.build();
}

void VpnInterfaceConnectionHelpers::syncInterfaceConnection(VpnInterfaceConnection *ic, Knm::Connection *c)
{
    VpnInterfaceConnectionSync sync;

    sync.sync(ic, c);
}

VpnInterfaceConnectionBuilder::VpnInterfaceConnectionBuilder(
                    Knm::Connection *connection,
                    const QString &deviceUni,
                    QObject *parent)
: InterfaceConnectionBuilder(connection, deviceUni, parent)
{
}

VpnInterfaceConnectionBuilder::~VpnInterfaceConnectionBuilder()
{
}

Knm::VpnInterfaceConnection* VpnInterfaceConnectionBuilder::build()
{
    VpnInterfaceConnection *ic =
            new VpnInterfaceConnection(Knm::Activatable::VpnInterfaceConnection,
                                       m_deviceUni, m_parent);

    if (ic) {
        init(ic);
    }

    return ic;
}

void VpnInterfaceConnectionBuilder::init(VpnInterfaceConnection *ic)
{
    InterfaceConnectionBuilder::init(ic);
}

VpnInterfaceConnectionSync::VpnInterfaceConnectionSync()
{
}

VpnInterfaceConnectionSync::~VpnInterfaceConnectionSync()
{
}

void VpnInterfaceConnectionSync::sync(Knm::VpnInterfaceConnection * interfaceConnection,
                                      Knm::Connection *connection)
{
    InterfaceConnectionSync::sync(interfaceConnection, connection);
}
