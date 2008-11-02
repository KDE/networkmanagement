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

class WirelessEnvironmentPrivate;

class AbstractWirelessEnvironment : public QObject
{
Q_OBJECT
public:
    AbstractWirelessEnvironment(QObject *parent);
    virtual ~AbstractWirelessEnvironment();
    virtual QStringList wirelessNetworks() const = 0;
    virtual AbstractWirelessNetwork * findWirelessNetwork(const QString&) const = 0;
Q_SIGNALS:
    virtual void wirelessNetworkAppeared(const QString&);
    virtual void wirelessNetworkDisappeared(const QString&);
};

/**
 * Models a the wireless environment as seen by a particular network interface.
 * This may be merged into Solid::Control::WirelessNetworkInterface
 */
class WirelessEnvironment : public AbstractWirelessEnvironment
{
Q_OBJECT
Q_DECLARE_PRIVATE(WirelessEnvironment)
public:
    WirelessEnvironment(Solid::Control::WirelessNetworkInterface * iface, QObject * parent = 0);
    virtual ~WirelessEnvironment();
    //virtual QList<Solid::Control::WirelessNetworkInterface*> interfaces() const;
    virtual QStringList wirelessNetworks() const;
    virtual AbstractWirelessNetwork * findWirelessNetwork(const QString&) const;
protected Q_SLOTS:
    void accessPointAppeared(const QString&);
    void networkDisappeared(const QString&);
    void wirelessEnabledChanged(bool);
private:
    void accessPointAppearedInternal(const QString&);
protected:
    WirelessEnvironmentPrivate * const d_ptr;
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
    QStringList wirelessNetworks() const;
    virtual AbstractWirelessNetwork * findWirelessNetwork(const QString&) const;
    void addWirelessEnvironment(WirelessEnvironment * wEnv);
protected Q_SLOTS:
    void onWirelessEnvironmentDestroyed(QObject*);
    void onWirelessNetworkAppeared(const QString&);
    void onWirelessNetworkDisappeared(const QString&);
private:
    void addWirelessNetworkInternal(WirelessNetwork* network, bool quietly = false);
protected:
    WirelessEnvironmentMergedPrivate * const d_ptr;
};


#endif // WIRELESSENVIRONMENT_H
