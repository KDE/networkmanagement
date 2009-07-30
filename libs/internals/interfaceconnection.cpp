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

InterfaceConnection::InterfaceConnection(const QUuid & connectionUuid, const QString & connectionName, const QString & deviceUni, QObject * parent)
: Activatable(Activatable::InterfaceConnection, deviceUni, parent), m_type(Knm::Connection::Wired), m_uuid(connectionUuid), m_name(connectionName), m_state(InterfaceConnection::Unknown), m_hasDefaultRoute(false)
{
}

InterfaceConnection::InterfaceConnection(const QUuid & connectionUuid, const QString & connectionName, ActivatableType type, const QString & deviceUni, QObject * parent)
: Activatable(type, deviceUni, parent), m_type(Knm::Connection::Wired), m_uuid(connectionUuid), m_name(connectionName), m_state(InterfaceConnection::Unknown), m_hasDefaultRoute(false)
{
}

InterfaceConnection::~InterfaceConnection()
{

}

void InterfaceConnection::setConnectionType(Knm::Connection::Type type)
{
    m_type = type;
}

Knm::Connection::Type InterfaceConnection::connectionType() const
{
    return m_type;
}

QUuid InterfaceConnection::connectionUuid() const
{
    return m_uuid;
}

void InterfaceConnection::setConnectionName(const QString& name)
{
    m_name = name;
    emit changed();
}

QString InterfaceConnection::connectionName() const
{
    return m_name;
}

void InterfaceConnection::setActivationState(InterfaceConnection::ActivationState state)
{
    if (m_state != state)
    {
        m_state = state;
        emit changed();
        emit activationStateChanged(m_state);
    }
}

InterfaceConnection::ActivationState InterfaceConnection::activationState() const
{
    return m_state;
}

void InterfaceConnection::setHasDefaultRoute(bool hasDefault)
{
    if (m_hasDefaultRoute != hasDefault) {
        m_hasDefaultRoute = hasDefault;
        emit hasDefaultRouteChanged(m_hasDefaultRoute);
    }
}

bool InterfaceConnection::hasDefaultRoute() const
{
    return m_hasDefaultRoute;
}

void InterfaceConnection::disconnect()
{
    emit deactivated();
}
