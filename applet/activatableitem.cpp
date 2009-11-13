/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2008, 2009 Sebastian Kügler <sebas@kde.org>

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

#include "activatableitem.h"
#include <kdebug.h>
#include "remoteactivatable.h"

ActivatableItem::ActivatableItem(RemoteActivatable * remote, QGraphicsItem * parent) : Plasma::IconWidget(parent), m_activatable(remote)
{
    setDrawBackground(true);
}

ActivatableItem::~ActivatableItem()
{
}

void ActivatableItem::emitClicked()
{
    //HACK this slot needs renaming
    kDebug() << "EMIT CLICKED";
    if (m_activatable) {
        m_activatable->activate();
    }
    emit clicked(this);
}

RemoteInterfaceConnection * ActivatableItem::interfaceConnection() const
{
    return qobject_cast<RemoteInterfaceConnection*>(m_activatable);
}

// vim: sw=4 sts=4 et tw=100
