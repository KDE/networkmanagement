/*
Copyright 2009 Paul Marchouk <pmarchouk@gmail.com>
Copyright 2011,2013 Lamarque V. Souza <lamarque@kde.org>

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

#include "interfaceconnectionhelpers.h"
#include "interfaceconnectionhelpers_p.h"

#include "connection.h"
#include "interfaceconnection.h"

using namespace Knm;

Knm::InterfaceConnection* InterfaceConnectionHelpers::buildInterfaceConnection(Knm::Connection *connection, const QString & deviceUni, QObject * parent)
{
    InterfaceConnectionBuilder builder(connection, deviceUni, parent);

    return builder.build();
}

void InterfaceConnectionHelpers::syncInterfaceConnection(InterfaceConnection *ic, Knm::Connection *c)
{
    InterfaceConnectionSync sync;

    sync.sync(ic, c);
}

InterfaceConnectionBuilder::InterfaceConnectionBuilder(Knm::Connection *connection,
                                                       const QString & deviceUni,
                                                       QObject * parent)
: m_connection(connection), m_deviceUni(deviceUni), m_parent(parent)
{
}

InterfaceConnectionBuilder::~InterfaceConnectionBuilder()
{
}

Knm::InterfaceConnection* InterfaceConnectionBuilder::build()
{
    InterfaceConnection* ic =
            new InterfaceConnection(Activatable::InterfaceConnection, m_deviceUni, m_parent);

    if (ic) {
        init(ic);
    }

    return ic;
}

void InterfaceConnectionBuilder::init(InterfaceConnection *ic)
{
    ic->m_uuid = m_connection->uuid();
    ic->setConnectionType(m_connection->type());
    ic->setConnectionName(m_connection->name());
    ic->setIconName(m_connection->iconName());
    ic->setShared(m_connection->isShared());
}

InterfaceConnectionSync::InterfaceConnectionSync()
{
}

InterfaceConnectionSync::~InterfaceConnectionSync()
{
}

void InterfaceConnectionSync::sync(Knm::InterfaceConnection * interfaceConnection,
                                   Knm::Connection *connection)
{
    interfaceConnection->setConnectionName(connection->name());
    interfaceConnection->setIconName(connection->iconName());
    interfaceConnection->setShared(connection->isShared());
}
