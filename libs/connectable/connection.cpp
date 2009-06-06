/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>

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

#include "connection.h"

#include "connectionadaptor.h"

namespace Knm {

Connection::Connection()
    :Connectable(Connectable::Connection)
{
    new ConnectionAdaptor(this);
}

Connection::Connection(ConnectableType type)
    :Connectable(type)
{
    new ConnectionAdaptor(this);
}

void Connection::setConnectionType(const QString& type)
{
    m_type = type;
}

QString Connection::connectionType()
{
    return m_type;
}

void Connection::setConnectionUni(const QString& uni)
{
    m_uni = uni;
}

QString Connection::connectionUni()
{
    return m_uni;
}

void Connection::setConnectionName(const QString& name)
{
    m_name = name;
}

QString Connection::connectionName()
{
    return m_name;
}

void Connection::setConnectionState(Solid::Control::NetworkInterface::ConnectionState state)
{
    m_state=state;
}

Solid::Control::NetworkInterface::ConnectionState Connection::connectionState()
{
    return m_state;
}

} // namespace
