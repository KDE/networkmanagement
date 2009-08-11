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

#ifndef WIRELESSSTATUS_H
#define WIRELESSSTATUS_H

#include <QObject>

class WirelessInterfaceConnectionItem;
class WirelessNetworkItem;
class WirelessStatusPrivate;

/**
 * A class to add wireless status indicators (security, signal strength)
 * to a WirelessNetworkItem or an WirelessInterfaceConnectionItem
 * and keep them up to date.
 */
class WirelessStatus : public QObject
{
Q_OBJECT
public:
    /**
     * @param item takes ownership of the WirelessStatus
     */
    WirelessStatus(WirelessInterfaceConnectionItem * item);
    WirelessStatus(WirelessNetworkItem * item);
    ~WirelessStatus();
protected Q_SLOTS:
    void setStrength(int);
private:
    Q_DECLARE_PRIVATE(WirelessStatus)
    WirelessStatusPrivate * d_ptr;
};

#endif // WIRELESSSTATUS_H
