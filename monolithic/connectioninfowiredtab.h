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
#ifndef CONNECTIONINFOWIREDTAB_H
#define CONNECTIONINFOWIREDTAB_H

#include <QtGui>

#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>

/**
 * Tab for ConnectionInfoDialog, that shows wired ethernet details
 */
class ConnectionInfoWiredTab : public QWidget
{
    Q_OBJECT

public:
    ConnectionInfoWiredTab(Solid::Control::WiredNetworkInterface *iface, QWidget *parent = 0);
    ~ConnectionInfoWiredTab() { };

public slots:
    void updateBitRate(int rate);

private:
    Solid::Control::WiredNetworkInterface *m_iface;
    QLabel *m_bitRateLabel,
           *m_hardwareAddressLabel;
};

#endif // CONNECTIONINFOWIREDTAB_H
