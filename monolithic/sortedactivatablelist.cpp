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

#include "sortedactivatablelist.h"

#include <KDebug>

#include <solid/control/networkmanager.h>

#include <activatable.h>
//debug
#include "interfaceconnection.h"
#include "wirelessinterfaceconnection.h"
#include "wirelessnetworkitem.h"
// debug

#include <activatablelist.h>

class SortedActivatableListPrivate
{
public:
    Solid::Control::NetworkInterface::Types types;
    QList<Knm::Activatable *> activatables;
};

bool activatableLessThan(const Knm::Activatable * first, const Knm::Activatable * second);

int compareDevices(const Knm::Activatable * first, const Knm::Activatable * second);

int compareActivatableType(const Knm::Activatable * first, const Knm::Activatable * second);

int compareActivationState(const Knm::InterfaceConnection * first, const Knm::InterfaceConnection * second);

int compareConnectionName(const Knm::InterfaceConnection * first, const Knm::InterfaceConnection * second);

int compareSignalStrength(const Knm::WirelessItem * first, const Knm::WirelessItem * second);

int compareSsid(const Knm::WirelessItem * first, const Knm::WirelessItem * second);

SortedActivatableList::WirelessSortPolicy SortedActivatableList::s_wirelessSortPolicy = SortedActivatableList::WirelessSortByStrength;

SortedActivatableList::SortedActivatableList(Solid::Control::NetworkInterface::Types types, QObject * parent)
    : QObject(parent), d_ptr(new SortedActivatableListPrivate)
{
    Q_D(SortedActivatableList);
    d->types = types;
}

void SortedActivatableList::handleAdd(Knm::Activatable * activatable)
{
    Q_D(SortedActivatableList);
    kDebug() << activatable;
    if (!d->activatables.contains(activatable)) {
        Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(activatable->deviceUni());
        if (d->types.testFlag(iface->type())) {
            d->activatables.append(activatable);
        }
    }
    qSort(d->activatables.begin(), d->activatables.end(), activatableLessThan);
}

void SortedActivatableList::handleUpdate(Knm::Activatable *)
{
    Q_D(SortedActivatableList);
    kDebug();
    qSort(d->activatables.begin(), d->activatables.end(), activatableLessThan);
}

void SortedActivatableList::handleRemove(Knm::Activatable * activatable)
{
    Q_D(SortedActivatableList);
    kDebug() << activatable;
    // this does not affect the total order
    d->activatables.removeAll(activatable);
}

QList<Knm::Activatable*> SortedActivatableList::activatables() const
{
    Q_D(const SortedActivatableList);
    return d->activatables;
}

void SortedActivatableList::dump() const
{
    Q_D(const SortedActivatableList);
    foreach (Knm::Activatable * activatable, d->activatables) {
        if (activatable->activatableType() == Knm::Activatable::InterfaceConnection) {
            Knm::InterfaceConnection * ic = static_cast<Knm::InterfaceConnection*>(activatable);
            kDebug() << "IC" << ic->connectionName();
        } else if ( activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
            Knm::WirelessInterfaceConnection * wic = static_cast<Knm::WirelessInterfaceConnection*>(activatable);
            kDebug() << "WIC" << wic->connectionName();
        } else if ( activatable->activatableType() == Knm::Activatable::WirelessNetworkItem) {
            Knm::WirelessNetworkItem * wni = static_cast<Knm::WirelessNetworkItem*>(activatable);
            kDebug() << "WNI" << wni->ssid();
        }
    }
}

bool activatableLessThan(const Knm::Activatable * first, const Knm::Activatable * second)
{
    // just for debug
#if 0
    QString s1, s2;
    if (first->activatableType() == Knm::Activatable::InterfaceConnection) {
        const Knm::InterfaceConnection * ic = qobject_cast<const Knm::InterfaceConnection*>(first);
        s1 = ic->connectionName();
    } else if ( first->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
        const Knm::WirelessInterfaceConnection * wic = qobject_cast<const Knm::WirelessInterfaceConnection*>(first);
        s1 = wic->connectionName();
    } else if ( first->activatableType() == Knm::Activatable::WirelessNetworkItem) {
        const Knm::WirelessNetworkItem * wni = qobject_cast<const Knm::WirelessNetworkItem*>(first);
        s1 = wni->ssid();
    }

    if (second->activatableType() == Knm::Activatable::InterfaceConnection) {
        const Knm::InterfaceConnection * ic = qobject_cast<const Knm::InterfaceConnection*>(second);
        s2 = ic->connectionName();
    } else if ( second->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
        const Knm::WirelessInterfaceConnection * wic = qobject_cast<const Knm::WirelessInterfaceConnection*>(second);
        s2 = wic->connectionName();
    } else if ( second->activatableType() == Knm::Activatable::WirelessNetworkItem) {
        const Knm::WirelessNetworkItem * wni = qobject_cast<const Knm::WirelessNetworkItem*>(second);
        s2 = wni->ssid();
    }
    // debug ends
#endif

    // order by device
    int i = compareDevices(first, second);

    // then by activatable type
    if (i == 0) {
//X         kDebug() << s1 << "and" << s2 << "are on the same device, comparing activatable type";
        i = compareActivatableType(first, second);
    }

    // then by state
    if (i == 0) {
        const Knm::InterfaceConnection * firstIc
            = qobject_cast<const Knm::InterfaceConnection *>(first);
        const Knm::InterfaceConnection * secondIc
            = qobject_cast<const Knm::InterfaceConnection *>(second);

        // compare interfaceconnections
        if (firstIc && secondIc) {
//X             kDebug() << s1 << "and" << s2 << "have the same type, comparing activation state" << firstIc->activationState() << secondIc->activationState();
            i = compareActivationState(firstIc, secondIc);

            // then compare connections by name, if not wireless or set by policy
            if (i == 0) {
                if ((first->activatableType() == Knm::Activatable::InterfaceConnection
                            && second->activatableType() == Knm::Activatable::InterfaceConnection)
                        || SortedActivatableList::s_wirelessSortPolicy == SortedActivatableList::WirelessSortAlphabetical) {
//X                     kDebug() << s1 << "and" << s2 << "have the same state, comparing name";
                    i = compareConnectionName(firstIc, secondIc);
                } // wireless connection, not alphabetical - compare by strength
                else if ( first->activatableType() == Knm::Activatable::WirelessInterfaceConnection
                        && second->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {

//X                     kDebug() << s1 << "and" << s2 << "have the same state, comparing signal strength";

                    const Knm::WirelessItem * firstWi = static_cast<const Knm::WirelessInterfaceConnection *>(first);
                    const Knm::WirelessItem * secondWi = static_cast<const Knm::WirelessInterfaceConnection *>(second);
                    i = compareSignalStrength(firstWi, secondWi);
                }
            }
        } else {
            // compare wirelessnetworkitems
            const Knm::WirelessNetworkItem * firstW
                = qobject_cast<const Knm::WirelessNetworkItem *>(first);
            const Knm::WirelessNetworkItem * secondW
                = qobject_cast<const Knm::WirelessNetworkItem *>(second);
            if (firstW && secondW) {
                if (SortedActivatableList::s_wirelessSortPolicy == SortedActivatableList::WirelessSortAlphabetical) {
     //               kDebug() << s1 << "and" << s2 << "are both wireless networks, comparing SSID";
                    i = compareSsid(firstW, secondW);
                } else {
//X                     kDebug() << s1 << "and" << s2 << "are both wireless networks, comparing signal strength";
                    i = compareSignalStrength(firstW, secondW);
                }
            }
        }
    }

#if 0
    // debug
    if ( i < 0 )
        kDebug() << s1 << "is before"  << s2;
    else if (i > 0)
        kDebug() << s2 << "is before"  << s1;
    else {
        kDebug() << s1 << "and" << s2 << "are equal";
    }
    // end of debug
#endif
    return i < 1;
}

int compareDevices(const Knm::Activatable * first, const Knm::Activatable * second)
{
    // cheap optimisation!
    if (first == second) {
        return 0;
    }

    Solid::Control::NetworkInterface * firstIface = Solid::Control::NetworkManager::findNetworkInterface(first->deviceUni());
    Solid::Control::NetworkInterface * secondIface = Solid::Control::NetworkManager::findNetworkInterface(second->deviceUni());

    if (firstIface->type() == secondIface->type()) {
        return firstIface->interfaceName().compare(secondIface->interfaceName());
    } else {

        return (int)firstIface->type() - (int)secondIface->type();
    }
    return 0;
}

int compareActivatableType(const Knm::Activatable * first, const Knm::Activatable * second)
{
    return (int)first->activatableType() - (int)second->activatableType();
}

int compareActivationState(const Knm::InterfaceConnection * first, const Knm::InterfaceConnection * second)
{
    // first and second are reversed here because Activated is arithmetically greater than the
    // others, but should appear first
    return (int)second->activationState() - (int)first->activationState();
}

int compareConnectionName(const Knm::InterfaceConnection * first, const Knm::InterfaceConnection * second)
{
    return first->connectionName().compare(second->connectionName());
}

int compareSignalStrength(const Knm::WirelessItem * first, const Knm::WirelessItem * second)
{
    // first and second are reversed here because networks with arithmetically greater
    // signal strength but should appear first
    return second->strength() - first->strength();
}

int compareSsid(const Knm::WirelessItem * first, const Knm::WirelessItem * second)
{
    return first->ssid().compare(second->ssid());
}

//Knm::Activatable * SimpleUi::activatableBefore(Knm::Activatable * activatable) const


// vim: sw=4 sts=4 et tw=100
