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

#ifndef MERGEDWIRELESS_H
#define MERGEDWIRELESS_H

#include "wirelessnetwork.h"
#include "wirelessenvironment.h"

#include <QTimer>
#include <QStringList>

class WirelessEnvironment;


/**
 * A 'virtual' wireless network object which merges several network interfaces' views of real wireless networks
 */
class WirelessNetworkMerged : public AbstractWirelessNetwork
{
Q_OBJECT
friend class WirelessEnvironmentMerged;
public:
    WirelessNetworkMerged(WirelessEnvironment * envt, WirelessNetwork * network, QObject * parent = 0);
    ~WirelessNetworkMerged();
    QString ssid() const;
    int strength() const;
    // return the Access Point which is currently being used as a reference point
    Solid::Control::AccessPoint * referenceAccessPoint() const;
protected Q_SLOTS:
    void onStrengthChanged(const QString &, int strength);
    void disappeared(const QString&);
private:
    void addWirelessNetworkInternal(WirelessEnvironment * envt, WirelessNetwork*);
    class Private;
    Private * d;
};

class WirelessEnvironmentMergedPrivate;
/**
 * Models the net wireless environment visible to the entire system as the sum of the all the
 * wireless environments of all the wireless interfaces
 */
class WirelessEnvironmentMerged : public AbstractWirelessEnvironment
{
Q_OBJECT
Q_DECLARE_PRIVATE(WirelessEnvironmentMerged)
public:
    WirelessEnvironmentMerged(QObject * parent = 0);
    virtual ~WirelessEnvironmentMerged();
    QStringList networks() const;
    virtual AbstractWirelessNetwork * findNetwork(const QString&) const;
    void addWirelessEnvironment(WirelessEnvironment * wEnv);
protected Q_SLOTS:
    void onWirelessEnvironmentDestroyed(QObject*);
    void onNetworkAppeared(const QString&);
    void disappeared(const QString&);
    void notifyNewNetwork();
    void notifyDisappearedNetwork();
Q_SIGNALS:
    void networkAppeared(const QString &ssid);
private:
    void addNetworkInternal(WirelessEnvironment *, WirelessNetwork* network, bool quietly = false);
    QTimer *m_newNetworkTimer;
    QStringList m_newNetworks;
    QTimer *m_disappearedNetworkTimer;
    QStringList m_disappearedNetworks;
protected:
    WirelessEnvironmentMergedPrivate * const d_ptr;
};


#endif // MERGEDWIRELESS_H
