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

#include "networksettings.h"
#include "networksettingsadaptor.h"
#include "marshallarguments.h"

#include <QDbusObjectPath>

#include "NetworkManager.h"

NetworkSettings::NetworkSettings()
{
    //declare types
    qDBusRegisterMetaType< QList<QDBusObjectPath> >();

    new NetworkSettingsAdaptor(this);

    QDBusConnection dbus = QDBusConnection::systemBus();
    dbus.registerObject(objectPath(), this);
}

NetworkSettings::~NetworkSettings()
{
    QDBusConnection dbus = QDBusConnection::systemBus();
    dbus.unregisterObject(objectPath());
}

bool NetworkSettings::loadSettings(const KConfigGroup &settings)
{
}

QList<QDbusObjectPath> NetworkSettings::ListConnections() const
{
    QList<QDbusObjectPath> pathList;
    foreach(const QString &connName, connectionMap) {
        pathList << QDBusObjectPath(connectionMap[connName]->objectPath());
    }
    return pathList;
}

void NetworkSettings::clearConnections()
{
    foreach (const QString &conn, connectionMap.keys()) {
        connectionMap[conn]->Delete();
        connectionMap.remove(conn);
    }
}

QString NetworkSettings::objectPath()
{
    return QString(NM_DBUS_PATH_SETTINGS);
}

#include "networksettings.moc"
