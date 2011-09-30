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

#include <kdebug.h>
#include "activatablelist.h"

#include "activatablelist_p.h"

#include "activatable.h"
#include "activatableobserver.h"

ActivatableList::ActivatableList(QObject * parent)
    : QObject(parent), d_ptr(new ActivatableListPrivate)
{
}

ActivatableList::ActivatableList(ActivatableListPrivate & dd, QObject * parent)
    : QObject(parent), d_ptr(&dd)
{
}

ActivatableList::~ActivatableList()
{
    delete d_ptr;
}

void ActivatableList::registerObserver(ActivatableObserver * observer, ActivatableObserver * insertAfter)
{
    Q_D(ActivatableList);
    if (observer) {
        // each observer may only be registered once
        if (d->observers.contains(observer)) {
            return;
        }

        // inserts at end if insertAfter not found (therefore if it is 0)
        QMutableListIterator<ActivatableObserver*> i(d->observers);
        i.findNext(insertAfter);
        i.insert(observer);

        // initialise the observer
        foreach (Knm::Activatable * activatable, d->activatables) {
            observer->handleAdd(activatable);
        }
    }
}

void ActivatableList::unregisterObserver(ActivatableObserver * observer)
{
    Q_D(ActivatableList);
    if (observer && !d->observers.isEmpty()) {
        d->observers.removeOne(observer);
    }
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
        addActivatableInternal(activatable);
        notifyOnAddActivatable(activatable);
    }
}

void ActivatableList::addActivatableInternal(Knm::Activatable * activatable)
{
    Q_D(ActivatableList);
    d->activatables.append(activatable);
    connect(activatable, SIGNAL(changed()), this, SLOT(activatableChanged()));
}

void ActivatableList::notifyOnAddActivatable(Knm::Activatable * activatable)
{
    Q_D(ActivatableList);

    foreach (ActivatableObserver * observer, d->observers) {
        observer->handleAdd(activatable);
    }
}

void ActivatableList::removeActivatable(Knm::Activatable * activatable)
{
    Q_D(ActivatableList);
    if (d->activatables.contains(activatable)) {
        d->activatables.removeOne(activatable);
        QListIterator<ActivatableObserver*> it(d->observers);
        it.toBack();
        while (it.hasPrevious()) {
            ActivatableObserver * observer = it.previous();
            observer->handleRemove(activatable);
        }
    }
    kDebug() << "----- Removing activatable, left: " << d->activatables.count();
}

void ActivatableList::activatableChanged()
{
    Q_D(ActivatableList);
    Knm::Activatable * activatable = qobject_cast<Knm::Activatable*>(sender());

    if (activatable) {
        foreach (ActivatableObserver * observer, d->observers) {
            observer->handleUpdate(activatable);
        }
    }
}

int ActivatableList::activatableIndex(Knm::Activatable *activatable)
{
    Q_D(ActivatableList);
    return d->activatables.indexOf(activatable);
}

// vim: sw=4 sts=4 et tw=100
