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
    class AccessPoint;
    class WirelessNetworkInterface;
} // namespace Control
} // namespace Solid

class AbstractWirelessNetwork : public QObject
{
Q_OBJECT
public:
    AbstractWirelessNetwork(QObject * parent);
    virtual ~AbstractWirelessNetwork();
    virtual QString ssid() const = 0;
    virtual int strength() const = 0;
    virtual Solid::Control::AccessPoint * referenceAccessPoint() const = 0;

protected:
    virtual void strengthChanged(const QString&, int) = 0;
    virtual void disappeared(const QString&) = 0;
};

/**
 * Models a wireless network, composed of one or more access points
 */
class WirelessNetwork : public AbstractWirelessNetwork
{
Q_OBJECT
public:
    WirelessNetwork(Solid::Control::AccessPoint * ap,
            Solid::Control::WirelessNetworkInterface * iface,
            QObject * parent = 0);
    ~WirelessNetwork();
    QString ssid() const;
    int strength() const;
    Solid::Control::AccessPoint * referenceAccessPoint() const;
signals:
    void strengthChanged(const QString&, int);
    void disappeared(const QString&);
protected Q_SLOTS:
    void accessPointAppeared(const QString&);
    void accessPointDisappeared(const QString&);
    void updateStrength();
private:
    void addAccessPointInternal(Solid::Control::AccessPoint * ap);
    class Private;
    Private * d;
};

class WirelessEnvironmentMerged;
class WirelessNetworkMergedPrivate;

/**
 * A 'virtual' wireless network object which merges several network interfaces' views of real wireless networks
 */
class WirelessNetworkMerged : public AbstractWirelessNetwork
{
Q_OBJECT
friend class WirelessEnvironmentMerged;
public:
    WirelessNetworkMerged(WirelessNetwork * network, QObject * parent = 0);
    ~WirelessNetworkMerged();
    QString ssid() const;
    int strength() const;
    // return the Access Point which is currently being used as a reference point
    Solid::Control::AccessPoint * referenceAccessPoint() const;
signals:
    void strengthChanged(const QString&, int);
    void disappeared(const QString&);
protected Q_SLOTS:
    void onStrengthChanged(const QString &, int strength);
    void onDisappeared(const QString&);
private:
    void addWirelessNetworkInternal(WirelessNetwork*);
    class Private;
    Private * d;
};

#endif // WIRELESSNETWORK_H
