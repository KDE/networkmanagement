/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef WIRELESSNETWORKINTERFACEENVIRONMENT_H
#define WIRELESSNETWORKINTERFACEENVIRONMENT_H

#include <QObject>

#include <solid/control/wirelessnetworkinterface.h>
// to move to solid/control
#include "wirelessnetwork.h"

#include "solid_control_export.h"

/**
 * Models the wireless environment as seen by a particular network interface.
 * This may be merged into Solid::Control::WirelessNetworkInterface
 */
namespace Solid
{
namespace Control
{
class WirelessNetworkInterfaceEnvironmentPrivate;
class SOLIDCONTROL_EXPORT WirelessNetworkInterfaceEnvironment : public QObject
{
Q_OBJECT
Q_DECLARE_PRIVATE(WirelessNetworkInterfaceEnvironment)
public:
    /**
     * Construct this using a Solid::Control::WirelessNetworkInterface * iface which is the QObject
     * parent
     */
    WirelessNetworkInterfaceEnvironment(Solid::Control::WirelessNetworkInterface * iface);
    virtual ~WirelessNetworkInterfaceEnvironment();
    virtual QStringList networks() const;
    virtual Solid::Control::WirelessNetwork * findNetwork(const QString&) const;
    void dump();
    Solid::Control::WirelessNetworkInterface * interface() const;

private Q_SLOTS:
    void accessPointAppeared(const QString&);
    void removeNetwork(const QString&);
    void wirelessEnabledChanged(bool);

Q_SIGNALS:
    virtual void networkAppeared(const QString &ssid);
    virtual void networkDisappeared(const QString &ssid);
private:
    void accessPointAppearedInternal(const QString&);
    WirelessNetworkInterfaceEnvironmentPrivate * const d_ptr;
};
} // namespace Control
} // namespace Solid

#endif // WIRELESSNETWORKINTERFACEENVIRONMENT_H
