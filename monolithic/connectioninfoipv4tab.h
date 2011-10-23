/*
Copyright 2009 Andrey Batyiev <batyiev@gmail.com>

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
#ifndef CONNECTIONINFOIPV4TAB_H
#define CONNECTIONINFOIPV4TAB_H

#include <QtGui>

#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4confignm09.h>

/**
 * Tab for ConnectionInfoDialog, that shows IP details
 */
class ConnectionInfoIPv4Tab : public QWidget
{
    Q_OBJECT

public:
    ConnectionInfoIPv4Tab(Solid::Control::NetworkInterface *iface, QWidget *parent = 0);
    ~ConnectionInfoIPv4Tab() { };

public slots:
    void updateIpDetails();

private:
    Solid::Control::NetworkInterface *m_iface;
    QLabel *m_connectionStateLabel,
           *m_ipAddressesLabel,
           *m_netmasksLabel,
           *m_gatewaysLabel,
           *m_nameserversLabel,
           *m_domainsLabel;
};

#endif // CONNECTIONINFOIPV4TAB_H
