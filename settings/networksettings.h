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
#include <QVariant>
#include <QMap>
#include <QString>

//DBus specific includes
#include <QtDBus/QtDBus>
#include <QDBusObjectPath>

#include <KConfigGroup>

#include <NetworkManager.h>

#include "connection.h"
#include "marshalarguments.h"

class NetworkSettings : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManagerSettings")

    public:
        NetworkSettings(const KConfigGroup &settings, QObject *parent=0);
        ~NetworkSettings();

        bool loadProfile(const QString &profile);
        bool isValid() const;

        QDBusConnection dbusConnection() const;

        Q_SCRIPTABLE QList<QDBusObjectPath> ListConnections() const;

    public Q_SLOTS:
        void onConnectionRemoved();

    Q_SIGNALS:
        void NewConnection(QDBusObjectPath);

    private:
        void clearConnections();
        QString objectPath() const;

        KConfigGroup m_settings;
        QDBusConnection m_conn;
        QMap<QString, Connection*> m_connectionMap;
        bool m_valid;
};

#endif
