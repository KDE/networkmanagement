/*
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

#ifndef INTERFACECONNECTIONITEM_H
#define INTERFACECONNECTIONITEM_H

#include "activatableitem.h"
#include "interfaceconnection.h"

class InterfaceConnectionItemPrivate;

/**
 * Widget for displaying/controlling generic InterfaceConnections
 */
class InterfaceConnectionItem : public ActivatableItem
{
Q_OBJECT

Q_DECLARE_PRIVATE(InterfaceConnectionItem)
signals:
    void hasDefaultRouteChanged(bool);
public:
    InterfaceConnectionItem(Knm::InterfaceConnection *, QWidget * parent = 0);
    virtual ~InterfaceConnectionItem();
    Knm::InterfaceConnection* interfaceConnection() const;
public Q_SLOTS:
    void changed();
    void setActivationState(Knm::InterfaceConnection::ActivationState state);
    void updateToolTip();
protected Q_SLOTS:
    void setHasDefaultRoute(bool);
    void disconnectClicked();
protected:
    InterfaceConnectionItem(InterfaceConnectionItemPrivate &, Knm::InterfaceConnection *, QWidget * parent = 0);
    virtual QString iconName() const;
    virtual QString textForConnection(Knm::InterfaceConnection::ActivationState state) const;
};

#endif // INTERFACECONNECTIONITEM_H
