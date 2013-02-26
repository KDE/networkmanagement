/*
    Copyright 2012-2013  Jan Grulich <jgrulich@redhat.com>

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

#include "connectiontypeitem.h"

#include <QtCore/QDateTime>
#include <KIcon>

ConnectionTypeItem::ConnectionTypeItem(QTreeWidget * parent, const QString & type):
    QTreeWidgetItem(parent, QStringList(type))
{
    QString text;

    if (type == QLatin1String("adsl")) {
        text = "Adsl";
        setIcon(0, KIcon("modem"));
    } else if (type == QLatin1String("bluetooth")) {
        text = "Bluetooth";
        setIcon(0, KIcon("bluetooth"));
    } else if (type == QLatin1String("bond")) {
        text = "Bond";
    } else if (type == QLatin1String("bridge")) {
        text = "Bridge";
    } else if (type == QLatin1String("cdma")) {
        text = "Mobile broadband";
        setIcon(0, KIcon("phone"));
    } else if (type == QLatin1String("gsm")) {
        text = "Mobile broadband";
        setIcon(0, KIcon("phone"));
    } else if (type == QLatin1String("infiniband")) {
        text = "InfiniBand";
    } else if (type == QLatin1String("802-11-olpc-mesh")) {
        text = "Olpc mesh";
    } else if (type == QLatin1String("pppoe")) {
        text = "Pppoe";
    } else if (type == QLatin1String("vlan")) {
        text = "VLAN";
    } else if (type == QLatin1String("vpn")) {
        text = "Vpn";
        setIcon(0, KIcon("secure-card"));
    } else if (type == QLatin1String("wimax")) {
        text = "WiMAX";
    } else if (type == QLatin1String("802-3-ethernet")) {
        text = "Ethernet";
        setIcon(0, KIcon("network-wired"));
    } else if (type == QLatin1String("802-11-wireless")) {
        text = "Wi-Fi";
        setIcon(0, KIcon("network-wireless"));
    }

    setText(0, text);

    QFont fnt = font(0);
    fnt.setBold(true);

    setFont(0, fnt);

    setExpanded(true);
}
