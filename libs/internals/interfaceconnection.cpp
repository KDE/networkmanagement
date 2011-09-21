/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "interfaceconnection.h"

using namespace Knm;

InterfaceConnection::InterfaceConnection(ActivatableType type, const QString & deviceUni, QObject * parent)
: Activatable(type, deviceUni, parent), m_type(Knm::Connection::Wired),
  m_state(InterfaceConnection::Unknown), m_oldState(InterfaceConnection::Unknown), m_hasDefaultRoute(false)
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
    if (name != m_name) {
        m_name = name;
        emit changed();
    }
}

QString InterfaceConnection::connectionName() const
{
    return m_name;
}

void InterfaceConnection::setIconName(const QString & iconName)
{
    if (iconName != m_iconName) {
        m_iconName = iconName;
        emit changed();
    }
}

QString InterfaceConnection::iconName() const
{
    return m_iconName;
}

void InterfaceConnection::setActivationState(InterfaceConnection::ActivationState state)
{
    if (m_state != state) {
        m_oldState = m_state;
        m_state = state;
        emit changed();
        emit activationStateChanged(m_oldState, m_state);
        emit activationStateChanged((uint)m_oldState, (uint)m_state);
    }
}

InterfaceConnection::ActivationState InterfaceConnection::activationState() const
{
    return m_state;
}

InterfaceConnection::ActivationState InterfaceConnection::oldActivationState() const
{
    return m_oldState;
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

void InterfaceConnection::deactivate()
{
    emit deactivated();
}
