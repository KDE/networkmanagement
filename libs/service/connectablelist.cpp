/*
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

#include "connectablelist.h"

#include <QList>

#include "connectable.h"

class ConnectableListPrivate
{
public:
    QList<Knm::Connectable*> connectables;
};

ConnectableList::ConnectableList(QObject * parent)
    : QObject(parent), d_ptr(new ConnectableListPrivate)
{

}

ConnectableList::~ConnectableList()
{
    delete d_ptr;
}

QList<Knm::Connectable*> ConnectableList::connectables() const
{
    Q_D(const ConnectableList);
    return d->connectables;
}

void ConnectableList::addConnectable(Knm::Connectable * connection)
{
    Q_D(ConnectableList);
    if (!d->connectables.contains(connection)) {
        d->connectables.append(connection);
        emit connectableAdded(connection);
    }
}

void ConnectableList::removeConnectable(Knm::Connectable * connection)
{
    Q_D(ConnectableList);
    if (d->connectables.contains(connection)) {
        d->connectables.removeOne(connection);
        emit connectableRemoved(connection);
    }
}

// vim: sw=4 sts=4 et tw=100
