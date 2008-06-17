/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

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

#include "connection.h"
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
        Q_SCRIPTABLE QList<QDBusObjectPath> ListConnections() const;
        /**
         * add/update a connection
         */
        void addConnection(const QVariantMapMap & settings);
        /**
         * remove a connection
         */
        void removeConnection(const QString & id);

    public Q_SLOTS:
        void onConnectionRemoved();

    Q_SIGNALS:
        /**
         * Signal from org.freedesktop.NetworkManagerSettings, exported via DBus
         */
        Q_SCRIPTABLE void NewConnection(QDBusObjectPath);

    private:
        void clearConnections();

        // Map of connection path to Connection
        QMap<QString, Connection*> m_connectionMap;
        uint mNextConnectionId;
};

#endif
