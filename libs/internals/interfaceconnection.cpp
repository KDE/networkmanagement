/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2012 Lamarque V. Souza <lamarque@kde.org>

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
    if (name == m_name) {
        return;
    }
    m_name = name;
    QVariantMap map;
    map.insert("name", m_name);
    emit icPropertiesChanged(map);
    emit changed();
}

QString InterfaceConnection::connectionName() const
{
    return m_name;
}

void InterfaceConnection::setIconName(const QString & iconName)
{
    if (iconName == m_iconName) {
        return;
    }
    m_iconName = iconName;
    QVariantMap map;
    map.insert("iconName", m_iconName);
    emit icPropertiesChanged(map);
    emit changed();
}

QString InterfaceConnection::iconName() const
{
    return m_iconName;
}

void InterfaceConnection::setActivationState(InterfaceConnection::ActivationState state)
{
    if (m_state == state) {
        return;
    }
    m_oldState = m_state;
    m_state = state;
    QVariantMap map;
    map.insert("activationState", m_state);
    emit icPropertiesChanged(map);
    emit activationStateChanged(m_oldState, m_state);
    emit activationStateChanged((uint)m_oldState, (uint)m_state);
    emit changed();
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
    if (m_hasDefaultRoute == hasDefault) {
        return;
    }
    m_hasDefaultRoute = hasDefault;
    emit hasDefaultRouteChanged(m_hasDefaultRoute);
    QVariantMap map;
    map.insert("hasDefaultRoute", m_hasDefaultRoute);
    emit icPropertiesChanged(map);
}

bool InterfaceConnection::hasDefaultRoute() const
{
    return m_hasDefaultRoute;
}

void InterfaceConnection::deactivate()
{
    emit deactivated();
}

QVariantMap InterfaceConnection::toMap()
{
    QVariantMap map = Activatable::toMap();
    map.insert("connectionType", m_type);
    map.insert("uuid", m_uuid.toString());
    map.insert("name", m_name);
    map.insert("iconName", m_iconName);
    map.insert("activationState", m_state);
    map.insert("hasDefaultRoute", m_hasDefaultRoute);
    return map;
}
