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

#ifndef WIRELESSNETWORK_H
#define WIRELESSNETWORK_H

#include <QObject>

namespace Solid
{
namespace Control
{
    class WirelessNetworkInterface;
} // namespace Control
} // namespace Solid

/**
 * Models a wireless network, composed of one or more access points
 */
class WirelessNetwork : public QObject
{
Q_OBJECT
public:
    WirelessNetwork(const QString & ssid, Solid::Control::WirelessNetworkInterface * iface, QObject * parent = 0);
    ~WirelessNetwork();
    QString ssid() const;
    int strength() const;
public slots:
    void accessPointAppeared(const QString&);
    void accessPointDisappeared(const QString&);
    void updateStrength();
signals:
    void strengthChanged(const QString&, int);
    void disappeared(const QString&);
private:
    class Private;
    Private * d;
};

#endif // WIRELESSNETWORK_H
