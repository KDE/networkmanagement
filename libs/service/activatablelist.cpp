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

#include "activatablelist.h"

#include <QList>

#include "activatable.h"
#include "activatableobserver.h"

class ActivatableListPrivate
{
public:
    QList<Knm::Activatable*> activatables;
};

ActivatableList::ActivatableList(QObject * parent)
    : QObject(parent), d_ptr(new ActivatableListPrivate)
{
}

ActivatableList::~ActivatableList()
{
    delete d_ptr;
}

void ActivatableList::connectObserver(ActivatableObserver * observer)
{
    QObject::connect(this, SIGNAL(activatableAdded(Knm::Activatable*)), observer, SLOT(handleAdd(Knm::Activatable*)));
    QObject::connect(this, SIGNAL(activatableUpdated(Knm::Activatable*)), observer, SLOT(handleUpdate(Knm::Activatable*)));
    QObject::connect(this, SIGNAL(activatableRemoved(Knm::Activatable*)), observer, SLOT(handleRemove(Knm::Activatable*)));
}

void ActivatableList::disconnectObserver(ActivatableObserver * observer)
{
    disconnect(observer);
}

QList<Knm::Activatable*> ActivatableList::activatables() const
{
    Q_D(const ActivatableList);
    return d->activatables;
}

void ActivatableList::addActivatable(Knm::Activatable * activatable)
{
    Q_D(ActivatableList);
    if (!d->activatables.contains(activatable)) {
        d->activatables.append(activatable);

        connect(activatable, SIGNAL(changed()), this, SLOT(activatableChanged()));
        emit activatableAdded(activatable);
    }
}

void ActivatableList::removeActivatable(Knm::Activatable * activatable)
{
    Q_D(ActivatableList);
    if (d->activatables.contains(activatable)) {
        d->activatables.removeOne(activatable);
        emit activatableRemoved(activatable);
    }
}

void ActivatableList::activatableChanged()
{
    Knm::Activatable * activatable = qobject_cast<Knm::Activatable*>(sender());

    if (activatable) {
        emit activatableUpdated(activatable);
    }
}

// vim: sw=4 sts=4 et tw=100
