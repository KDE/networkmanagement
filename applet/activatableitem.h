/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2008, 2009 Sebastian K?gler <sebas@kde.org>

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

#ifndef ABSTRACTCONNECTABLEITEM_H
#define ABSTRACTCONNECTABLEITEM_H

#include <Plasma/IconWidget>

#include "remoteinterfaceconnection.h"

class RemoteActivatable;
extern int rowHeight;
extern int maxConnectionNameWidth;

class ActivatableItem : public Plasma::IconWidget
{
Q_OBJECT
public:
    ActivatableItem(RemoteActivatable *remote, QGraphicsItem * parent = 0);
    virtual ~ActivatableItem();
    virtual void setupItem() = 0;
    RemoteInterfaceConnection* interfaceConnection() const;
    void disappear();
    void hoverEnter();
    void hoverLeave();

Q_SIGNALS:
    /** * Indicate that the 'connect' button was clicked.  Used by the containing InterfaceGroup to
     * tell the manager to activate the connection on one of its devices
     */
    void clicked(ActivatableItem *);
    void hasDefaultRouteChanged(bool);
    void disappearAnimationFinished();
    void showInterfaceDetails(QString);

protected Q_SLOTS:
    void emitClicked();
    void handleHasDefaultRouteChanged(bool);
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);
    void activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState);
    void notifyNetworkingState();
    void connectionChanged();

protected:
    RemoteActivatable *m_activatable;
    bool m_hasDefaultRoute;
    Knm::InterfaceConnection::ActivationState m_state;
    bool m_deleting;
    int spacing;
    Plasma::IconWidget* m_connectButton;
};

#endif // ABSTRACTCONNECTABLEITEM_H
