/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
    Copyright (C) 2008,2009 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef NETWORK_SETTINGS_H
#define NETWORK_SETTINGS_H

#include <QObject>
#include <QMap>
#include <QString>

//DBus specific includes
#include <QtDBus/QtDBus>
#include <QDBusObjectPath>

//#include <KConfigGroup>

class Connection;

#include <NetworkManager.h>

#include "busconnection.h"
#include "marshalarguments.h"

typedef QMap<QString,QVariantMap> QVariantMapMap;

class NetworkSettings : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManagerSettings")

    public:
        NetworkSettings(QObject *parent=0);
        virtual ~NetworkSettings();

        // Move to Storage
        //bool loadSettings(const KConfigGroup &settings);
        /**
         * Method from org.freedesktop.NetworkManagerSettings, exported via DBus
         */
        /**
         * add a connection
         * @return object path of the new connection
         */
        QString addConnection(Knm::Connection *);
        /**
         * update a connection
         */
        void updateConnection(const QString & objectPath, Knm::Connection *);
        /**
         * remove a connection
         */
        void removeConnection(const QString & id);

    public Q_SLOTS:
        Q_SCRIPTABLE QList<QDBusObjectPath> ListConnections() const;
        /**
         * Monitor the devices in the system to update connections' timestamps when they become
         * active
         */
        void networkInterfaceAdded(const QString&);
        void networkInterfaceConnectionStateChanged(int);

    Q_SIGNALS:
        /**
         * Signal from org.freedesktop.NetworkManagerSettings, exported via DBus
         */
        Q_SCRIPTABLE void NewConnection(QDBusObjectPath);

        /**
         * Indicates that a connection was activated
         */
        void connectionActivated(const QString & uuid);

    private:
        /**
         * Delete all listed connection objects
         */
        void clearConnections();
        /**
         * utility to generate the next unused object path for a connection
         * TODO: reuse deleted connections' object paths?
         */
        QString nextObjectPath();

        QMap<QString, BusConnection*> m_connectionMap;
        uint mNextConnectionId;
};

#endif
