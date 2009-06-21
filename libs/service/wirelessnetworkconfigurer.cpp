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

#include "wirelessnetworkconfigurer.h"

#include <QList>
#include <QPair>

#include <KDebug>

#include "wirelessnetworkitem.h"
#include "wirelessinterfaceconnection.h"

typedef QPair<QString,QString> PendingNetwork;
class WirelessNetworkConfigurerPrivate
{
public:
    // list of <ssid,deviceUni> tuples
    QList<PendingNetwork> pendingNetworks;
};

WirelessNetworkConfigurer::WirelessNetworkConfigurer(QObject * parent)
: ActivatableObserver(parent), d_ptr(new WirelessNetworkConfigurerPrivate)
{
}

WirelessNetworkConfigurer::~WirelessNetworkConfigurer()
{
}

void WirelessNetworkConfigurer::handleAdd(Knm::Activatable *added)
{
    Q_D(WirelessNetworkConfigurer);
    Knm::WirelessNetworkItem * wni = qobject_cast<Knm::WirelessNetworkItem*>(added);
    // listen to the WNI
    if (wni) {
        connect(wni, SIGNAL(activated()), this, SLOT(wirelessNetworkItemActivated()));
    } else {
        Knm::WirelessInterfaceConnection * wic = qobject_cast<Knm::WirelessInterfaceConnection*>(added);
        // check if it's WIC we're waiting for
        if (wic) {
            foreach (PendingNetwork pending, d->pendingNetworks) {
                if (pending.first == wic->ssid() && pending.second == wic->deviceUni()) {
                    kDebug() << "activating WIC for" << wic->ssid() << "on" << wic->deviceUni();
                    wic->activate();
                    d->pendingNetworks.removeOne(pending);
                }
            }
        }
    }
}

void WirelessNetworkConfigurer::wirelessNetworkItemActivated()
{
    Q_D(WirelessNetworkConfigurer);
    Knm::WirelessNetworkItem * wni = qobject_cast<Knm::WirelessNetworkItem*>(sender());
    if (wni) {
        d->pendingNetworks.append(QPair<QString,QString>(wni->ssid(), wni->deviceUni()));
        kDebug() << "watching for connection for" << wni->ssid() << "on" << wni->deviceUni();
    }
}

void WirelessNetworkConfigurer::handleChange(Knm::Activatable *)
{
    // dummy impl
}

void WirelessNetworkConfigurer::handleRemove(Knm::Activatable *)
{
    // dummy impl
}
// vim: sw=4 sts=4 et tw=100
