/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
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

#include "wirelessinterfaceitem.h"
#include "uiutils.h"

#include <QGraphicsGridLayout>
#include <QLabel>

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkmanager.h>

WirelessInterfaceItem::WirelessInterfaceItem(Solid::Control::WirelessNetworkInterface * iface,  InterfaceItem::NameDisplayMode mode, QGraphicsWidget* parent)
: InterfaceItem(iface, mode, parent), m_wirelessIface(iface), m_activeAccessPoint(0)
{
    // for updating our UI
    connect(iface, SIGNAL(activeAccessPointChanged(const QString&)),
            SLOT(activeAccessPointChanged(const QString&)));

    activeAccessPointChanged(m_wirelessIface->activeAccessPoint());
}

WirelessInterfaceItem::~WirelessInterfaceItem()
{
}

void WirelessInterfaceItem::activeAccessPointChanged(const QString &uni)
{
    //kDebug() << "*** AP changed:" << uni << "***";
    // this is not called when the device is deactivated..
    if (m_activeAccessPoint) {
        m_activeAccessPoint->disconnect(this);
        m_activeAccessPoint = 0;
    }
    if (uni != "/") {
        m_activeAccessPoint = m_wirelessIface->findAccessPoint(uni);
        //kDebug() << "new:" << m_activeAccessPoint;
        if (m_activeAccessPoint) {
            connect(m_activeAccessPoint, SIGNAL(signalStrengthChanged(int)), SLOT(activeSignalStrengthChanged(int)));
            connect(m_activeAccessPoint, SIGNAL(destroyed(QObject*)),
                    SLOT(accessPointDestroyed(QObject*)));
        }
    }
    setConnectionInfo();
}

QString WirelessInterfaceItem::connectionName()
{
    if (m_activeAccessPoint) {
        return m_activeAccessPoint->ssid();
    }
    return QString();
}

void WirelessInterfaceItem::activeSignalStrengthChanged(int)
{
    setConnectionInfo();
}

void WirelessInterfaceItem::accessPointDestroyed(QObject* ap)
{
    //kDebug() << "*** AP gone ***";
    if (ap == m_activeAccessPoint) {
        m_activeAccessPoint = 0;
    }
}

void WirelessInterfaceItem::connectButtonClicked()
{
    // TODO
}

void WirelessInterfaceItem::setConnectionInfo()
{
    InterfaceItem::setConnectionInfo(); // Sets the labels
    switch ( m_iface->connectionState()) {
        case Solid::Control::NetworkInterface::Unavailable:
        case Solid::Control::NetworkInterface::Disconnected:
        case Solid::Control::NetworkInterface::Failed:
            //m_connectionInfoIcon->hide();
            break;
        default:
        {
            if (m_activeAccessPoint) {
                m_connectionInfoIcon->show();
            }
            break;
        }
    }
    //kDebug() << "Icon:" << UiUtils::iconName(m_iface);
    //m_icon->nativeWidget()->setPixmap(UiUtils::iconName(m_iface));
}

QList<Solid::Control::AccessPoint*> WirelessInterfaceItem::availableAccessPoints() const
{
    QList<Solid::Control::AccessPoint*> retVal;
    AccessPointList aps = m_wirelessIface->accessPoints(); //NOTE: AccessPointList is a QStringList
    foreach (const QString &ap, aps) {
        Solid::Control::AccessPoint *accesspoint = m_wirelessIface->findAccessPoint(ap);
        if(accesspoint) {
            retVal << accesspoint;
        }
    }
    return retVal;
}

void WirelessInterfaceItem::setEnabled(bool enable)
{
    InterfaceItem::setEnabled(enable);
}

// vim: sw=4 sts=4 et tw=100
