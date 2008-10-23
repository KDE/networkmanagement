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

#ifndef WIRELESSENVIRONMENT_H
#define WIRELESSENVIRONMENT_H

#include <QObject>

#include "wirelessnetwork.h"

/**
 * Models a the wireless environment as seen by a particular network interface.
 * This may be merged into Solid::Control::WirelessNetworkInterface
 */
class WirelessEnvironment : public QObject
{
Q_OBJECT
public:
    WirelessEnvironment(Solid::Control::WirelessNetworkInterface * iface, QObject * parent = 0);
    virtual ~WirelessEnvironment();
    Solid::Control::WirelessNetworkInterface * interface() const;
    QStringList wirelessNetworks() const;
    WirelessNetwork * findWirelessNetwork(const QString&) const;
signals:
    void wirelessNetworksChanged();
public slots:
    void accessPointAppeared(const QString&);
    void networkDisappeared(const QString&);
private:
    void accessPointAppearedInternal(const QString&, bool quietly = false);
    class Private;
    Private *d;
};

#endif // WIRELESSENVIRONMENT_H
