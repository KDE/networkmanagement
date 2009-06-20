/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
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

#include "interfaceconnection.h"

using namespace Knm;

InterfaceConnection::InterfaceConnection(QObject * parent)
: Connectable(Connectable::Connection, parent)
{
}

InterfaceConnection::InterfaceConnection(ConnectableType type, QObject * parent)
: Connectable(type, parent)
{
}

InterfaceConnection::~InterfaceConnection()
{

}

void InterfaceConnection::activate()
{
    emit activated();
}

void InterfaceConnection::setConnectionType(Knm::Connection::Type type)
{
    m_type = type;
}

Knm::Connection::Type InterfaceConnection::connectionType() const
{
    return m_type;
}

void InterfaceConnection::setConnectionUni(const QString& uni)
{
    m_uni = uni;
}

QString InterfaceConnection::connectionUni() const
{
    return m_uni;
}

void InterfaceConnection::setConnectionName(const QString& name)
{
    m_name = name;
}

QString InterfaceConnection::connectionName() const
{
    return m_name;
}

void InterfaceConnection::setConnectionState(Solid::Control::NetworkInterface::ConnectionState state)
{
    m_state = state;
    emit connectionStateChanged(m_state);
}

Solid::Control::NetworkInterface::ConnectionState InterfaceConnection::connectionState() const
{
    return m_state;
}

