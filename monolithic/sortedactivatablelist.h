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

#ifndef SORTEDACTIVATABLELIST_H
#define SORTEDACTIVATABLELIST_H

#include <activatableobserver.h>

#include <QList>

#include <solid/control/networkinterface.h>

namespace Knm
{
    class Activatable;
} // namespace Knm

class ActivatableList;

/**
 * Keeps a sorted version of the activatable list - needs inserting before the UI so it sees changes
 * before the UI does
 */

class SortedActivatableListPrivate;

class SortedActivatableList : public QObject, public ActivatableObserver
{
Q_OBJECT
Q_DECLARE_PRIVATE(SortedActivatableList);
public:
    enum WirelessSortPolicy {WirelessSortAlphabetical = 1, WirelessSortByStrength };

    static WirelessSortPolicy s_wirelessSortPolicy;

    SortedActivatableList(Solid::Control::NetworkInterface::Types types, QObject * parent = 0);
    // respond to activatable changes
    void handleAdd(Knm::Activatable *);
    void handleUpdate(Knm::Activatable *);
    void handleRemove(Knm::Activatable *);
    /**
     * Look up the activatable after this
     * @return 0 if no activatables are after this one, ie it is the last
     */
    Knm::Activatable * activatableAfter(Knm::Activatable *);
    /**
     * Access the sorted list
     */
    QList<Knm::Activatable *> activatables() const;
    /**
     * debug
     */
    void dump() const;
signals:
    /**
     * Indicate that an activatable has changed relative position in the ordering and should be moved
     * Could be implemented by storing the old ordering and comparing the priors of each
     * or in-sort as we move each one - but this implies each item will move only once
     * KISS says just empty the menu and repopulate it from this list after any change
     */
    void moved(Knm::Activatable * moved, Knm::Activatable * successor);

private:
    SortedActivatableListPrivate * d_ptr;
};
#endif // SORTEDACTIVATABLELIST_H
