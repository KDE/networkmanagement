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

#include "sortedactivatablelist.h"

#include <KDebug>

#include <solid/control/networkmanager.h>

#include <activatable.h>
//debug
#include <interfaceconnection.h>
#include <unconfiguredinterface.h>
#include <vpninterfaceconnection.h>
#include <wirelessinterfaceconnection.h>
#include <wirelessnetwork.h>
// debug

#include <activatablelist_p.h>

class SortedActivatableListPrivate : public ActivatableListPrivate
{
public:
    Solid::Control::NetworkInterfaceNm09::Types types;
    QHash<Solid::Control::NetworkInterfaceNm09::Type, int> solidTypesToOrder;
};

// sorting activatables
// by interface type (compare on activatable::deviceUni()
// by activatableType() (interfaceconnections and wirelessinterfaceconnections before wirelessnetworks)
// then: for interfaceconnections - by activation state
//   then: alphabetically
//       for wirelessinterfaceconnections - by activation state
//         then: by strength
//           then: alphabetically
// InterfaceConnections)
// by type ([w]ic > wni)
// by signal strength (listen to strengthchanged signals)
//   or alphabetically
// QAction::addAction/removeAction
// resort everything or just find correct place for changed item?
//   (save pointer to last active connection?)
// make lists of
// k

bool activatableLessThan(const Knm::Activatable * first, const Knm::Activatable * second);

int compareDevices(const Knm::Activatable * first, const Knm::Activatable * second);

int compareActivatableType(const Knm::Activatable * first, const Knm::Activatable * second);

int compareActivationState(const Knm::InterfaceConnection * first, const Knm::InterfaceConnection * second);

int compareConnectionName(const Knm::InterfaceConnection * first, const Knm::InterfaceConnection * second);

int compareSignalStrength(const Knm::WirelessObject * first, const Knm::WirelessObject * second);

int compareSsid(const Knm::WirelessObject * first, const Knm::WirelessObject * second);

/* SortedActivatableList */

SortedActivatableList::WirelessSortPolicy SortedActivatableList::s_wirelessSortPolicy = SortedActivatableList::WirelessSortByStrength;
QHash<Solid::Control::NetworkInterfaceNm09::Types, int> SortedActivatableList::s_solidTypesToOrder = QHash<Solid::Control::NetworkInterfaceNm09::Types, int>();

SortedActivatableList::SortedActivatableList(Solid::Control::NetworkInterfaceNm09::Types types, QObject * parent)
    : ActivatableList(*new SortedActivatableListPrivate, parent)
{
    Q_D(SortedActivatableList);
    d->types = types;
    s_solidTypesToOrder.insert(Solid::Control::NetworkInterfaceNm09::Wifi, 0);
    s_solidTypesToOrder.insert(Solid::Control::NetworkInterfaceNm09::Wimax, 1);
    s_solidTypesToOrder.insert(Solid::Control::NetworkInterfaceNm09::Bluetooth, 2);
    s_solidTypesToOrder.insert(Solid::Control::NetworkInterfaceNm09::Ethernet, 3);
    s_solidTypesToOrder.insert(Solid::Control::NetworkInterfaceNm09::Modem, 4);
    s_solidTypesToOrder.insert(Solid::Control::NetworkInterfaceNm09::OlpcMesh, 5);
}

void SortedActivatableList::handleAdd(Knm::Activatable * activatable)
{
    Q_D(SortedActivatableList);
    if (!d->activatables.contains(activatable)) {
        Solid::Control::NetworkInterfaceNm09 * iface = Solid::Control::NetworkManagerNm09::findNetworkInterface(activatable->deviceUni());
        // add all vpn connections
        if ((iface && (d->types.testFlag(iface->type())))
                || (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection)) {
            addActivatableInternal(activatable);
            qSort(d->activatables.begin(), d->activatables.end(), activatableLessThan);
            notifyOnAddActivatable(activatable);
        }
    }
}

void SortedActivatableList::handleUpdate(Knm::Activatable *)
{
    Q_D(SortedActivatableList);
    qSort(d->activatables.begin(), d->activatables.end(), activatableLessThan);
}

void SortedActivatableList::handleRemove(Knm::Activatable * activatable)
{
    ActivatableList::removeActivatable(activatable);
    //dump();
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
        } else if ( activatable->activatableType() == Knm::Activatable::WirelessNetwork) {
            Knm::WirelessNetwork * wni = static_cast<Knm::WirelessNetwork*>(activatable);
            kDebug() << "WNI" << wni->ssid();
        } else if ( activatable->activatableType() == Knm::Activatable::UnconfiguredInterface) {
            Knm::UnconfiguredInterface * unco = static_cast<Knm::UnconfiguredInterface*>(activatable);
            kDebug() << "UCI" << unco->deviceUni();
        } else if (activatable->activatableType() == Knm::Activatable::VpnInterfaceConnection) {
            Knm::VpnInterfaceConnection * vpn = static_cast<Knm::VpnInterfaceConnection*>(activatable);
            kDebug() << "VPN" << vpn->connectionName();
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
    } else if ( first->activatableType() == Knm::Activatable::WirelessNetwork) {
        const Knm::WirelessNetwork * wni = qobject_cast<const Knm::WirelessNetwork*>(first);
        s1 = wni->ssid();
    }

    if (second->activatableType() == Knm::Activatable::InterfaceConnection) {
        const Knm::InterfaceConnection * ic = qobject_cast<const Knm::InterfaceConnection*>(second);
        s2 = ic->connectionName();
    } else if ( second->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
        const Knm::WirelessInterfaceConnection * wic = qobject_cast<const Knm::WirelessInterfaceConnection*>(second);
        s2 = wic->connectionName();
    } else if ( second->activatableType() == Knm::Activatable::WirelessNetwork) {
        const Knm::WirelessNetwork * wni = qobject_cast<const Knm::WirelessNetwork*>(second);
        s2 = wni->ssid();
    }
    // debug ends
#endif

    // order by activatable type
    int i = compareActivatableType(first, second);

    // then by device
    if (i == 0 && !first->activatableType() != Knm::Activatable::VpnInterfaceConnection && !second->activatableType() != Knm::Activatable::VpnInterfaceConnection) {
//X         kDebug() << s1 << "and" << s2 << "are on the same device, comparing activatable type";
        i = compareDevices(first, second);
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

                    const Knm::WirelessObject * firstWi = static_cast<const Knm::WirelessInterfaceConnection *>(first);
                    const Knm::WirelessObject * secondWi = static_cast<const Knm::WirelessInterfaceConnection *>(second);
                    i = compareSignalStrength(firstWi, secondWi);
                }
            }
        } else {
            // compare wirelessnetworks
            const Knm::WirelessNetwork * firstW
                = qobject_cast<const Knm::WirelessNetwork *>(first);
            const Knm::WirelessNetwork * secondW
                = qobject_cast<const Knm::WirelessNetwork *>(second);
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
    // cheap optimization!
    if (first == second) {
        return 0;
    }

    Solid::Control::NetworkInterfaceNm09 * firstIface = Solid::Control::NetworkManagerNm09::findNetworkInterface(first->deviceUni());
    Solid::Control::NetworkInterfaceNm09 * secondIface = Solid::Control::NetworkManagerNm09::findNetworkInterface(second->deviceUni());

    if (firstIface != 0 && secondIface != 0) {
        if (firstIface->type() == secondIface->type()) {
            return firstIface->interfaceName().compare(secondIface->interfaceName());
        } else {
            return (int)firstIface->type() - (int)secondIface->type();
        }
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

int compareSignalStrength(const Knm::WirelessObject * first, const Knm::WirelessObject * second)
{
    // first and second are reversed here because networks with arithmetically greater
    // signal strength but should appear first
    return second->strength() - first->strength();
}

int compareSsid(const Knm::WirelessObject * first, const Knm::WirelessObject * second)
{
    return first->ssid().compare(second->ssid());
}

void SortedActivatableList::addActivatable(Knm::Activatable * activatable)
{
    ActivatableList::addActivatable(activatable);
}

void SortedActivatableList::removeActivatable(Knm::Activatable * activatable)
{
    ActivatableList::removeActivatable(activatable);
}
void removeActivatable(Knm::Activatable *);
//Knm::Activatable * SimpleUi::activatableBefore(Knm::Activatable * activatable) const


// vim: sw=4 sts=4 et tw=100
