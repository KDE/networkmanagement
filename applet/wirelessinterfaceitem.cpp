/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "wirelessinterfaceitem.h"

#include <KNotification>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "events.h"
#include "wirelessnetwork.h"

WirelessInterfaceItem::WirelessInterfaceItem(Solid::Control::WirelessNetworkInterface * iface, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, InterfaceItem::NameDisplayMode mode, QGraphicsItem* parent)
: InterfaceItem(iface, userSettings, systemSettings, mode, parent), m_wirelessIface(iface)
{
    foreach (QString apUni, iface->accessPoints()) {
        accessPointAppearedInternal(apUni);
    }
    // for managing our list of wireless networks
    connect(iface, SIGNAL(accessPointAppeared(const QString&)),
            SLOT(accessPointAppeared(const QString&)));
    // for updating our UI
    connect(iface, SIGNAL(activeAccessPointChanged(const QString&)),
            SLOT(activeAccessPointChanged(const QString&)));

    activeAccessPointChanged(m_wirelessIface->activeAccessPoint());
}

WirelessInterfaceItem::~WirelessInterfaceItem()
{

}

void WirelessInterfaceItem::accessPointAppeared(const QString &uni)
{
    accessPointAppearedInternal(uni);
    emit wirelessNetworksChanged();
}

void WirelessInterfaceItem::accessPointAppearedInternal(const QString &uni)
{
    Solid::Control::AccessPoint * ap = m_wirelessIface->findAccessPoint(uni);
    QString ssid = ap->ssid();
    if (!m_networks.contains(ssid)) {
        WirelessNetwork * net = new WirelessNetwork(ssid, m_wirelessIface, 0);
        m_networks.insert(ssid, net);
        //connect(net, SIGNAL(strengthChanged(const
        connect(net, SIGNAL(disappeared(const QString&)), SLOT(networkDisappeared(const QString&)));
        net->accessPointAppeared(uni);
        emit wirelessNetworksChanged();
        KNotification::event(Event::NetworkAppeared, i18nc("Notification text when a wireless network interface was found","Wireless network %1 found", ssid), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    }
}

void WirelessInterfaceItem::networkDisappeared(const QString &ssid)
{
    WirelessNetwork * net = m_networks.take(ssid);
    KNotification::event(Event::NetworkDisappeared, i18nc("Notification text when a wireless network interface disappeared","Wireless network %1 disappeared", ssid), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    delete net;
    emit wirelessNetworksChanged();
}


void WirelessInterfaceItem::activeAccessPointChanged(const QString &uni)
{
    m_activeAccessPoint = m_wirelessIface->findAccessPoint(uni);
    if (m_activeAccessPoint) {
        connect(m_activeAccessPoint, SIGNAL(signalStrengthChanged(int)), SLOT(activeSignalStrengthChanged(int)));
    }
    setConnectionInfo();
}

void WirelessInterfaceItem::activeSignalStrengthChanged(int)
{
    setConnectionInfo();
}

void WirelessInterfaceItem::setConnectionInfo()
{
    if (m_activeAccessPoint) {
        m_connectionInfoLabel->setText(QString::fromLatin1("%1 %").arg(m_activeAccessPoint->signalStrength()));
        // TODO update icon contents
        m_connectionInfoIcon->setIcon("system-lock-screen");
        m_connectionInfoIcon->show();
    } else {
        m_connectionInfoLabel->setText(QString());
        m_connectionInfoIcon->hide();
    }
}
// vim: sw=4 sts=4 et tw=100
