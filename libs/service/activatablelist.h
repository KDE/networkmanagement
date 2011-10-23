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

#ifndef ACTIVATABLELIST_H
#define ACTIVATABLELIST_H

#include <QObject>
#include <solid/control/networkmanager.h>
#include "knm_export.h"

namespace Knm
{
    class Activatable;
} // namespace Knm


class ActivatableObserver;
class ActivatableListPrivate;

class KNM_EXPORT ActivatableList : public QObject
{
Q_OBJECT
Q_DECLARE_PRIVATE(ActivatableList)
public:
    ActivatableList(QObject * parent);
    ActivatableList(ActivatableListPrivate &dd, QObject * parent);
    virtual ~ActivatableList();

    virtual QList<Knm::Activatable *> activatables() const;

    virtual void addActivatable(Knm::Activatable *);
    virtual void removeActivatable(Knm::Activatable *);
    virtual int activatableIndex(Knm::Activatable *);
    /**
     * Register an activatable observer.  ActivatableObservers encapsulate peripheral functionality.
     * ActivatableObservers are called in a defined order.
     * @param observer the observer to register.
     * @param insertAfter the point in the activatable observer chain to insert at.  Inserts at end if 0
     * is passed or if insertAfter is not found
     *
     * TODO: do we need separate observer lists for different operations?
     */
    virtual void registerObserver(ActivatableObserver * observer, ActivatableObserver * insertAfter = 0);
    virtual void unregisterObserver(ActivatableObserver *);
protected Q_SLOTS:
    void activatableChanged();
protected:
    void addActivatableInternal(Knm::Activatable * activatable);
    void notifyOnAddActivatable(Knm::Activatable * activatable);
    ActivatableListPrivate * d_ptr;
};

#endif // ACTIVATABLELIST_H
